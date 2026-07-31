# Looper

An 8-track microphone looper. Each track records into its own fixed PSRAM slot,
with one extra slot kept for undo.

## Controls

- Each track uses one playback row and one control row, except the last track
  which uses the final playback row.
- Bottom row, from the right: play, stop, clear, record, tap tempo.
- Tap tempo also acts as shift while held.
- Bottom row pads 6-10 show the live input VU meter.
- Hold record and tap a track to arm it. Recording starts from input level while
  stopped. While playing, the primary track starts on a barline and secondary
  tracks start when the primary latched loop wraps.
- If tempo is known, the touched pad is the recording start. If this is the
  first internal-clock recording, recording starts at pad 0 and sets tempo when
  it finishes.
- Tap record by itself to arm the first blank track.
- Press record while recording to finish. Press stop to finish and stop
  transport, play to finish and start from stopped transport, or clear to cancel.
- Hold clear and tap a track to stop its playback heads. Hold longer to delete
  the track. Tap clear alone to toggle undo for the last recording.
- Touch track pads normally to launch up to three transient playheads. Each one
  follows the pressure on its assigned pad and stops when that pressure reaches
  zero.
- Hold tap tempo as shift to edit the latched loop playhead. Tap inside its loop
  to stop it, tap outside to move the loop start and launch it, or touch two
  pads to set a wrapped loop range.
- Control rows provide volume range, DJ filter, delay send, reverb send,
  reverse, and beat repeat. Light beat-repeat pressure loops 1/8ths; stronger
  pressure loops 1/16ths. Reverse and beat repeat are momentary effects; when
  released, playback resumes from where it would have been without the effect.
- If reverse or beat repeat is pressed while a track has no active playheads,
  head 1 starts from the loop start and follows the maximum effect-pad pressure.
  Its beat repeat stays at the 1/8th rate and jumps to random 1/8th-note
  positions inside the current loop.

The first recording on internal clock can set the tempo from its length. If it
is very short, the loop length is reduced before clamping tempo between 80 and
160 BPM. When that first take finishes, the transport is cued to zero and starts
with the new loop latched.
