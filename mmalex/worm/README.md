# Worm

A simple stereo granular sampler backed by PSRAM.

## Controls

- Hold the bottom-right record pad while touching main pads to start recording at the touch centroid.
- Tap the record pad by itself to cycle the colour used for the next recording.
- Recording advances to adjacent wrapped pads, biasing toward the current centroid and away from pads with audio.
- After recording starts, the record pad can be released; lift main-pad pressure below the threshold to fade recording out.
- The upper 15 rows are sample pads; the bottom row shows a live input VU plus clear, octave, and record controls.
- Hold the white clear pad and touch a main pad to clear its audio and colour.
- Hold the teal octave pad to make pressure set the chance that newly spawned grains play an octave up or down.
- Touch main pads without record to seed the 16-grain playback cloud. Pressure controls grain density, volume, and length.
- Tap the VU meter to temporarily replace the play page with the mix sliders; it returns after 3 seconds of inactivity or another bottom-row tap.
- Page 2 is the song load/save page.

Recorded pad colour is estimated from bass, mid, and treble band energy while audio is written.
