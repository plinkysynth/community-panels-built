# Worm - granular ambient live sampler

Worm lets you record from the mic or audio input into the top 16x15 pads of plinky, then play back the recordings
as a granular wash by pressing one or many pads with your fingers. Pressure
controls grain density, volume, and length.

## How to record a sound.

Hold the bottom-right red record pad while touching any of the upper pads to record stereo
audio input into PSRAM. The record head follows the touch centroid, preferring nearby
empty pads.

The left portion of the bottom row serves as a VU meter for the incoming audio. Worm defaults to using the line input, but this can be changed to microphone. (See below)

```illus
leds: ../../../docs/leds/worm.png
panel: pick
highlight: 0,15,13,16 | VU meter
highlight: 13,15,14,16 | Clear
highlight: 14,15,15,16 | Octave
highlight: 15,15,16,16 | Record
crop: 0,12,16,16
press: 15,15,4,0
caption: The bottom row shows input level on the left, then clear, octave, and record controls on the right.
```

* Hold the red record button then press a pad in the upper part of the panel to record
* The white clear button lets you clear recordings.
* The teal button in the bottom row acts as a shift, adding an octave-up and down effect when playing back grains.

## Selecting audio input source

Press the right clicky 'up' button to access the settings pages as usual. The first page in Worm is 'src' - the audio source. Use the left clicky buttons to select between mic and line. The mics are small holes on the left and right side near the top of the panel. The 2 LEDs either side of the mic are disabled when the mics are active, to reduce noise.

```illus
leds: ../../../docs/leds/worm-input.png
panel: pick
crop: false
caption: The source setting switches between line input and the built-in microphones.
```

## Changing the mix

Tapping the VU meter on the bottom row takes you to a mix page, with sliders in much the same order as Blocks and Toadstep's 'synth up' page.
The leftmost 3 sliders are Dry Volume, Delay amount, and Reverb amount.
Turn up the delay and reverb to add additional ambient wash to your sounds.

```illus
leds: ../../../docs/leds/worm-mix.png
panel: pick
highlight: 1,0,4,7 | Volume/Delay/Reverb
highlight: 5,0,16,7 | Mix sliders
crop: false
caption: Tap the VU meter to reveal the mix sliders; tap the bottom row again, or wait a few seconds, to return to playing.
```

The mix sliders are, in order from left to right:

* Audio in dry (turn this up to hear the audio input straight thru to the output)
* Audio in delay (...or with delay)
* Audio in reverb (...or with reverb)
* Delay time
* Delay feedback
* Reverb shimmer
* Reverb feedback
* Bass EQ
* Mid EQ
* Treble EQ
* Master Output level

## Saving your recordings

Press down on the right clicky button to access the usual save and load screen, that works much the same way as the stock panels: folders on the left, file slots on the right. Long recordings may take a few seconds to save.
