# Life

A Game of Life sequencer. The 16×16 grid holds a Conway world that keeps evolving on its own
clock: columns are steps, rows are scale degrees. Four voices walk through it at their own
speeds and play whatever cells they find alive.

If a voice reaches a column with nothing alive in it, it rests. That is what turns the shape of
the world into rhythm.

> **Runs on any faceplate.** The synth page, the pickers and the play surface all move onto the
> printed pad circles on Chords and Drums, and take the full grid on Blocks and Toadstep.

## Hear it

- [Life on Plinky 12](https://www.youtube.com/watch?v=DeXRKCYJnzk)

## Start here

1. Press `(15,15)`, the green play pad. A freshly loaded panel seeds itself, so it makes sound
   straight away.
2. Tap pads to bring cells to life and disturb it.
3. Tap `(13,15)`, the magenta `×`, for mutes, solos, the world's rule, and the voice editors.

Nothing moves while the transport is stopped, so you can draw a pattern in peace.

## What makes it musical

**Rate is the groove.** All four voices cross all 16 columns, so they only sound polyrhythmic if
their speeds differ. They default to 8th, quarter triplet, quarter and half for that reason.

**Pick decides the note.** A column often has several live cells, and each voice chooses one its
own way: highest, lowest, nearest to the last note it played, or all of them at once for chords.
Eleven rules in all.

**The world stays alive.** Conway settles into still lifes within a couple of hundred
generations, and a frozen palette is a static loop. Life watches for that and sprinkles new
cells in, so it keeps moving without being touched.

**Everything is in key.** Rows are scale degrees, not semitones, so any cell anywhere is in the
scale. Key and scale come from the instrument, so the rest of your Plinky follows along.

## Beyond the grid

Each voice has its own MIDI channel and its own preset, so the four of them can play four
different sounds or drive four things in a DAW. There are conditional triggers read off the
cells themselves, so a glider passing through a voice's path audibly changes its rhythm. The
world can be driven from a controller over CC, drawn into from a keyboard, and it puts its own
density and rate of change out on CV.

**[Full manual](https://github.com/charlesvestal/plinky-life/blob/main/docs/manual.md)**, with
pad maps of every page.
