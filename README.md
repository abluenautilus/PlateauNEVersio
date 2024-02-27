## Plateau for Nosie Engineering's Versio

Firmware port of Valley's Plateau for the Noise Engineering Versio product line. Includes output threshold clipper 
and output scaling to preserve the signal while it passes through output opamps no matter how much distortion is present.

## Build Log

## Version 0.9a 02/27/2024

Fixes: 
Fixed loud initialization using hold boolean on the linear interpolating delay until the Dattorro has reached equilibrium
Implemented knobs
Implemented CVs
Fixed some heavy static as result of unoptimized code, unstable knobs, and sample rate choices.
Longer predelay buffer

To-do:
Implement low cut and high cut filters on the tank and on the inputs
Implement freeze function


## Version 0.8a, 02/25/2024:

Working Plateau reverb code on Daisy Seed
Bug where the output is extremely loud and distorted for 10 seconds
Knobs not set up
CVs not set up


## Bibliography

Dattorro, J. (1997). Effect design part 1: Reverberator and other filters, J. Audio Eng. Soc, 45(9), 660-684.

## License

All code is licensed under GNU Public License v3.0, Copyright (c) 2023 Dale Johnson
