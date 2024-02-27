## Plateau for Nosie Engineering's Versio

Firmware port of Valley's Plateau for the Noise Engineering Versio product line. Includes output threshold clipper 
and output scaling to preserve the signal while it passes through output opamps no matter how much distortion is present.

## To-do:

Implement low cut and high cut filters on the tank and on the inputs

Implement freeze function

## Build Log

## Version 0.91a, 02/27/2024

Fixes:

Removed an erroneous optimization in the interpolating delay that caused a loud burst and the Dattoro going out of equilibrium when the size knob was set all the way to zero with modulation up. This was due to an out of bounds access with one of the buffers.

## Version 0.9a, 02/27/2024

Fixes: 

Fixed loud initialization using hold boolean on the linear interpolating delay until the Dattorro has reached equilibrium
  (Edit: This was initialilly thought to be because the interpolation needed time to settle down but now I believe that the loudness directly after boot was because the SDRAM has a lot of garbage data. Reading through the daisy driver code, it seems that SDRAM does not currently get initialized to zero upon hardware Init(). I believe this is possible to do in the main function though and perhaps I am completely wrong about that, so I will look more into this later. For now this solution is a quick fix and has no risks because the buffers are completely traversed by the time the 4 second initial hold passes. Although such a problem may cause problems with changing delay times, such as with predelay changes. This explaisn why changing the pre delay time introduced static and why the unstable knobs cause static specifically with the pre delay.)

Implemented knobs

Implemented CVs

Fixed some heavy static as result of unoptimized code, unstable knobs, and sample rate choices.

Longer predelay buffer


## Version 0.8a, 02/25/2024:

Working Plateau reverb code on Daisy Seed

Bug where the output is extremely loud and distorted for 10 seconds\

Knobs not set up

CVs not set up


## Bibliography

Dattorro, J. (1997). Effect design part 1: Reverberator and other filters, J. Audio Eng. Soc, 45(9), 660-684.

## License

All code is licensed under GNU Public License v3.0, Copyright (c) 2023 Dale Johnson
