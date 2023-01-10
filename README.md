# PTPlayer
A player library for [Polytone files](spec.md) (.POL), with the ability to also load legacy MONOTONE (.MON) files. It is a supersession of [MTPlayer](https://github.com/prochazkaml/MTPlayer), used primarily in the [Polytone](https://github.com/prochazkaml/Polytone) tracker. However, it can also be used as a standalone player program (supplied in this repo, uses PortAudio for audio output) or it can even be incorporated into your own productions, should you choose.

[Here is a primitive web-based player built using this library.](https://ptplayer.prochazkaml.eu/)

## What is MONOTONE?
MONOTONE (created by Trixter / Hornet) is a tracker (song editor), similar to programs like [OpenMPT](https://openmpt.org/).
However, unlike OpenMPT, which is sample/FM-based, MONOTONE is aimed for much simpler audio devices,
it only supports one kind of instrument (a square wave) at a constant volume.
Its music file format (.MON files) is usually only a couple of kB in size.

My first attempt at .MON playback was [MTPlayer](https://github.com/prochazkaml/MTPlayer), which was mostly just a translation of the original MONOTONE tracker source code from Pascal to C.
PTPlayer is just an expansion of that library, with support for the new Polytone files.

For more info about MONOTONE & example .MON files, please visit the [MONOTONE GitHub repo](https://github.com/MobyGamer/MONOTONE).

## What is Polytone?
Polytone is my own modern implementation of MONOTONE. Please visit its [GitHub repo](https://github.com/prochazkaml/Polytone) for more info.

## Why was the new file format needed?
The main reason I reworked MTPlayer into PTPlayer and created a new file format is because of the limited 6-bit effect values of the old .MON format. That means that the largest value an effect can have is 63, or 7/7 with arpeggio/vibrato. You couldn't even make more advanced chords with it. But I understand why that decision by the MONOTONE devs was made – each data point (note+effect+value) neatly took up exactly 2 bytes. So order to have full 8-bit effect values (up to 255, or 15/15 with arpeggio/vibrato) without the file size bloating up, rudimentary pattern packing was implemented. It's by no means perfect, but it gets the job done, and often results in even _lower_ file sizes than .MON files themselves!

TL;DR: You can now have up to 15 semitone chords, which is over an octave! Seriously, try the 0CC effect on a note. It's awesome.
