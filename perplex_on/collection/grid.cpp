/*
@Name: Grid
@Author: mmalex
@Date: 2026-05-14
@Level: Advanced
@Tags: grid, serial, device
@Artwork: pixel/P12ExampleGrid.png
@Description: A serial-protocol grid device implemented on Plinky.

Grid turns plinky into a serial-protocol grid device.
*/
#define PANEL_PAD_COLOR RAINBOW2

/*
As a serial device, this is a re-implementation of the serial protocol used by monome for their grid devices.
*/
#define extract_bit(bit) ((data[3 + bit / 8] & (1 << (bit & 7))) ? 15 : 0)
#define extract_nibble(nibble) ((data[3 + nibble / 2] >> (((nibble & 1) ^ 1) * 4)) & 15)

static inline uint8_t gamma(int x) { return ((x * 12) >> 4) & 15; }
static const uint32_t HUE_PREVIEW_US = 500000;
static const uint32_t USB_SERIAL_WRITE_TIMEOUT_US = 3000;
static const uint32_t USB_SERIAL_PARTIAL_PACKET_TIMEOUT_US = 25000;
static const int GRID_PALETTE_MAX_HUES = 32;
static const int GRID_PPM_FLAT_ROW_VARIATION = 4;
static const float GRID_PPM_EXPOSURE = 1.5f;
static const float GRID_PPM_GAMMA = 1.9f;
static const float GRID_PPM_PRIMARY_R_GAIN = 6.2683f;
static const float GRID_PPM_PRIMARY_G_GAIN = 3.5183f;
static const float GRID_PPM_PRIMARY_B_MIX[3] = {0.6778f, 0.9994f, 2.0345f};

static float grid_ppm_srgb_to_display_linear(uint8_t srgb) {
  if (!srgb)
    return 0.0f;
  return powf((float)srgb / 255.0f, GRID_PPM_GAMMA) * 255.0f;
}

static inline int grid_ppm_clamp_channel(float value) {
  return clampi((int)lroundf(value), 0, 255);
}

static uint32_t grid_ppm_srgb_to_device_rgb(const uint8_t *rgb) {
  float display_r = grid_ppm_srgb_to_display_linear(rgb[0]) * GRID_PPM_EXPOSURE;
  float display_g = grid_ppm_srgb_to_display_linear(rgb[1]) * GRID_PPM_EXPOSURE;
  float display_b = grid_ppm_srgb_to_display_linear(rgb[2]) * GRID_PPM_EXPOSURE;

  float device_b = display_b / GRID_PPM_PRIMARY_B_MIX[2];
  float device_g = (display_g - GRID_PPM_PRIMARY_B_MIX[1] * device_b) / GRID_PPM_PRIMARY_G_GAIN;
  float device_r = (display_r - GRID_PPM_PRIMARY_B_MIX[0] * device_b) / GRID_PPM_PRIMARY_R_GAIN;
  return LED_RGB(grid_ppm_clamp_channel(device_r), grid_ppm_clamp_channel(device_g), grid_ppm_clamp_channel(device_b));
}

static inline uint8_t grid_ppm_srgb_gradient_channel(uint8_t end_value, int intensity) {
  return (uint8_t)(((int)end_value * intensity + 7) / 15);
}

static void grid_ppm_expand_flat_intensity_ramps(uint8_t *ppm_rgb, int num_hues) {
  // if a given 'hue' row is mostly all the same colour,
  // lets replace it with a gradient from black to that colour.
  for (int hue = 0; hue < num_hues; ++hue) {
    uint8_t min_rgb[3] = {255, 255, 255};
    uint8_t max_rgb[3] = {0, 0, 0};
    for (int intensity = 0; intensity < 16; ++intensity) {
      uint8_t *rgb = ppm_rgb + (hue * 16 + intensity) * 3;
      for (int c = 0; c < 3; ++c) {
        if (rgb[c] < min_rgb[c]) min_rgb[c] = rgb[c];
        if (rgb[c] > max_rgb[c]) max_rgb[c] = rgb[c];
      }
    }
    if ((int)max_rgb[0] - min_rgb[0] >= GRID_PPM_FLAT_ROW_VARIATION ||
        (int)max_rgb[1] - min_rgb[1] >= GRID_PPM_FLAT_ROW_VARIATION ||
        (int)max_rgb[2] - min_rgb[2] >= GRID_PPM_FLAT_ROW_VARIATION) {
      continue;
    }

    uint8_t end_rgb[3] = {};
    memcpy(end_rgb, ppm_rgb + (hue * 16 + 15) * 3, sizeof(end_rgb));
    for (int intensity = 0; intensity < 16; ++intensity) {
      uint8_t *rgb = ppm_rgb + (hue * 16 + intensity) * 3;
      for (int c = 0; c < 3; ++c)
        rgb[c] = grid_ppm_srgb_gradient_channel(end_rgb[c], intensity);
    }
  }
}

struct grid : panel_t {
  uint8_t hue = 0;
  int8_t y_offset = (get_frontpanel_code() == FRONT_PANEL_CODE_CHORDS || get_frontpanel_code() == FRONT_PANEL_CODE_DRUMS) ? 2 : 0;
  uint8_t num_hues = 16;
  bool settings_changed = false;
  uint32_t hue_preview_until_us = 0;
  uint32_t usb_serial_partial_packet_since_us = 0;
  uint32_t mono_palette[GRID_PALETTE_MAX_HUES][16];
  uint8_t brightness[16][16]; // shadow copy so we can redraw every frame

  int get_num_panel_settings_pages() override { return 2; }

  int max_hue() const { return maxi(0, (int)num_hues - 1); }
  uint8_t current_hue() const { return (uint8_t)clampi(hue, 0, max_hue()); }
  void clamp_hue_to_palette() { hue = current_hue(); }

  void rebuild_mono_palette() {
    num_hues = 16;
    for (int h = 0; h < GRID_PALETTE_MAX_HUES; ++h) {
      for (int i = 0; i < 16; ++i) {
        mono_palette[h][i] = h < 16 ? BRIGHTER(palette[gamma(i)][h]) : BLACK;
      }
    }
    clamp_hue_to_palette();
  }

  bool load_mono_palette_ppm() {
    // the user can put a PPM file 16x32 (up to 32) in /PLINKY/u_grid/palette.ppm to override the default palette.
    auto ppm_rgb = make_temp_array<uint8_t>(16 * GRID_PALETTE_MAX_HUES * 3, 0x4750504d);
    int width = 16;
    int height = GRID_PALETTE_MAX_HUES;
    if (!read_rgb_ppm("/PLINKY/u_grid/palette.ppm", &width, &height, ppm_rgb.get()) || width != 16)
      return false;
    grid_ppm_expand_flat_intensity_ramps(ppm_rgb.get(), height);
    for (int hue = 0; hue < height; ++hue) {
      for (int intensity = 0; intensity < 16; ++intensity) {
        int offset = (hue * 16 + intensity) * 3;
        mono_palette[hue][intensity] = grid_ppm_srgb_to_device_rgb(ppm_rgb.get() + offset);
      }
    }
    num_hues = (uint8_t)height;
    clamp_hue_to_palette();
    return true;
  }

  void on_setup_default_panel_state() override {
    panel_t::on_setup_default_panel_state();
    printf("grid: on_setup_default_panel_state\n");
    rebuild_mono_palette();
    load_mono_palette_ppm();
    settings_changed = false;
    hue_preview_until_us = 0;
    usb_serial_partial_packet_since_us = 0;
  }

  bool on_serialise_settings(serialiser_t &s, int version) override {
    (void)version;
    OBJECT_BEGIN(s);
    FIELD("hue", hue, 0u, 31u);
    FIELD("y_offset", y_offset, -8, 8);
    OBJECT_END(s);
    return true;
  }

  void set_hue(int new_hue) {
    uint8_t clamped_hue = (uint8_t)clampi(new_hue, 0, max_hue());
    if (clamped_hue != hue)
      settings_changed = true;
    hue = clamped_hue;
    hue_preview_until_us = time_us_32() + HUE_PREVIEW_US;
  }

  void set_y_offset(int new_y_offset) {
    int8_t clamped_y_offset = (int8_t)clampi(new_y_offset, -8, 8);
    if (clamped_y_offset != y_offset) {
      y_offset = clamped_y_offset;
      set_all_leds_mono(0);
      settings_changed = true;
    }
  }

  void draw_hue_preview(uint8_t h, bool force_visible) {
    if (!force_visible) {
      if (!hue_preview_until_us)
        return;
      if ((int32_t)(hue_preview_until_us - time_us_32()) <= 0) {
        hue_preview_until_us = 0;
        return;
      }
    }
    int y = 15 + scroll_y;
    h = (uint8_t)clampi(h, 0, max_hue());
    for (int x = 0; x < 16; ++x)
      set_led(x, y, mono_palette[h][x]);
  }

  /*
  `on_click(...)` receives the four physical clicky-button presses after debouncing.
  This panel keeps the right pair on their normal system job (page navigation), but
  hijacks the left pair so users can change the grid LED hue instead of BPM.
  */
  void on_click(uint8_t button_mask) override {
    if (get_scroll_page() < 0) {
      panel_t::on_click(button_mask);
      return;
    }
    panel_t::on_click(button_mask & (BTN_BIT_RUP | BTN_BIT_RDOWN));
    if (button_mask & BTN_BIT_LUP)
      set_hue(hue + 1);
    if (button_mask & BTN_BIT_LDOWN)
      set_hue(hue - 1);
  }

  void on_sequence(int delta_time_us) override {}

  void on_touch(int x, int y, int down) override {
    char buf[3] = {(char)(down ? 0x21 : 0x20), (char)(x & 15), (char)((y - y_offset) & 15)};
    usb_serial_write_with_timeout(3, buf, USB_SERIAL_WRITE_TIMEOUT_US);
  }

  void on_ui(int delta_time_us) override {
    int page = get_scroll_page();
    uint8_t h = current_hue();
    bool hue_page = page == -1;
    if (hue_page) {
      char hue_buf[5];
      int hue_max = max_hue();
      snprintf(hue_buf, sizeof(hue_buf), "%d", h);
      int delta = draw_system_style_settings_page("hue", hue_buf, h, 0, 0, mono_palette[h][11]);
      set_hue((int)h + delta);
    } else if (page == -2) {
      char y_buf[5];
      snprintf(y_buf, sizeof(y_buf), "%+d", y_offset);
      int delta = draw_system_style_settings_page("yofs", y_buf, y_offset, 0, 0, mono_palette[h][11]);
      set_y_offset((int)y_offset + delta);
    } else {
      for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
          set_led(x, y, mono_palette[h][brightness[y][x] & 15]); // copy shadow to real LEDs every frame
        }
      }
    }
    h = current_hue();
    draw_hue_preview(h, hue_page);
    if (settings_changed) {
      (void)save_settings_to_sd(false);
      settings_changed = false;
    }
  }

  void set_led_mono(int x, int y, int i) {
    brightness[(y + y_offset) & 15][x & 15] = i & 15;
  }

  void set_all_leds_mono(int i) {
    uint8_t b = i & 15;
    for (int y = 0; y < 16; ++y)
      for (int x = 0; x < 16; ++x)
        brightness[y][x] = b;
  }
  // clang-format off
  void usb_serial_process_packet(const uint8_t *data) {
    uint8_t x = data[1], y = data[2];
    switch (data[0]) {
    case 0x10: set_led_mono(x, y, 0); break;
    case 0x11: set_led_mono(x, y, 15); break;
    case 0x12: set_all_leds_mono(0); break;
    case 0x13: set_all_leds_mono(15); break;
    case 0x14: for (int i = 0; i < 64; ++i) set_led_mono((x & 8) + (i & 7), (y & 8) + (i / 8), extract_bit(i)); break;
    case 0x15: for (int i = 0; i < 8; ++i) set_led_mono((x & 8) + i, y, extract_bit(i)); break;
    case 0x16: for (int i = 0; i < 8; ++i) set_led_mono(x, (y & 8) + i, extract_bit(i)); break;
    case 0x18: set_led_mono(x, y, data[3]); break;
    case 0x19: set_all_leds_mono(data[3]); break;
    case 0x1A: for (int i = 0; i < 64; ++i) set_led_mono((x & 8) + (i & 7), (y & 8) + (i / 8), extract_nibble(i)); break;
    case 0x1B: for (int i = 0; i < 8; ++i) set_led_mono((x & 8) + i, y, extract_nibble(i)); break;
    case 0x1C: for (int i = 0; i < 8; ++i) set_led_mono(x, (y & 8) + i, extract_nibble(i)); break;
    case 0:    usb_serial_write_with_timeout(6, "\x00\x01\x04\x00\x02\x04", USB_SERIAL_WRITE_TIMEOUT_US); break;
    case 1:    usb_serial_write_with_timeout(33, "\x01monome\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", USB_SERIAL_WRITE_TIMEOUT_US); break;
    case 5:    usb_serial_write_with_timeout(3, "\x03\x10\x10", USB_SERIAL_WRITE_TIMEOUT_US); break;
    case 0xf:  usb_serial_write_with_timeout(2, "\x05\x00", USB_SERIAL_WRITE_TIMEOUT_US); break;
    }
  }
  uint8_t packet_length(uint8_t type) {
    static const uint8_t packet_lengths[32] = {1, 1, 33, 1, 4, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 11, 4, 4, 2, 4, 2, 35, 7, 7, 1, 1, 1};
    return (type > 0x1c) ? 1 : packet_lengths[type];
  }
  void pump_usb_serial() override {
    for (uint8_t type = 0xff; usb_serial_peek(&type);) {
      uint8_t buf[64], len = packet_length(type);
      int avail = usb_serial_available();
      if (avail < len) {
        if (avail > 0) {
          uint32_t now = time_us_32();
          if (!usb_serial_partial_packet_since_us)
            usb_serial_partial_packet_since_us = now;
          else if ((uint32_t)(now - usb_serial_partial_packet_since_us) > USB_SERIAL_PARTIAL_PACKET_TIMEOUT_US) {
            // we've been sitting on a partial packet for 25ms.
            // something has gone wrong. lets drain a byte and try again.
            usb_serial_read(buf, 1);
            usb_serial_partial_packet_since_us = 0;
          }
        }
        break;
      }
      usb_serial_partial_packet_since_us = 0;
      usb_serial_read(buf, len);
      usb_serial_process_packet(buf);
    }
    usb_serial_flush();
  }
  // clang-format on

  // to be detected as a serial-protocol grid by libmonome, we have to set the USB descriptor.
  // this macro doesnt change any other behaviour of plinky, just how it names itself to the USB host.
  // on some platforms, (mac, linux, norns) libmonome checks the vendor and product strings.
  // on other platforms, (windows) it tries to parse a windows pnp id - that fails for composite devices
  // therefore if we want to work on windows hosts as a grid, we cannot also support usb audio or midi :(
  // on mac and linux, that causes no issues. but for the sake of compatibility, I have switched to 
  // usb serial only.
  SET_USB_DESCRIPTOR("monome", "grid", 0xcaff, 0x2011, USB_SERIAL);
};
