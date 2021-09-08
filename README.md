# PTPlayer
A player library for [Polytone files](spec.md), with the ability to also load legacy MONOTONE files. It is an supersession of [MTPlayer](https://github.com/prochazkaml/MTPlayer), used primarily in the [Polytone](https://github.com/prochazkaml/Polytone) tracker. However, it can also be used as a standalone player program (supplied in this repo) or it can even be incorporated into your own productions, should you choose.

## What is MONOTONE?
MONOTONE (created by Trixter / Hornet) is a tracker (song editor), similar to programs like [OpenMPT](https://openmpt.org/).
However, unlike OpenMPT, which is sample/FM-based, MONOTONE is aimed for much simpler audio devices,
it only supports one kind of instrument (a square wave) at a constant volume.
Its music file format (.MON files) is usually only a couple of kB in size.

This library is mostly just a translation of the original MONOTONE tracker source code from Pascal to C. It includes a simple example program, which loads and plays a .MON file using PortAudio.

For more info about MONOTONE & example .MON files, please visit the [MONOTONE GitHub repo](https://github.com/MobyGamer/MONOTONE).

## What is Polytone?
Polytone is my own modern implementation of MONOTONE. Please visit its [GitHub repo](https://github.com/prochazkaml/Polytone) for more information.
