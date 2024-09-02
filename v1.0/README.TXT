Audio FX Collection version 1.0

This is a collection of audio effects for Unix based systems.
I made it and tested it on GNU-Linux, however it only uses standard libraries, so I believe it should be compatible with any POSIX compliant system.

All effects are non real-time effects, meaning they generate an output audio file with the processed signal from the input file.

It only supports .wav files, 16-bit and 24-bit encoding.

Effect description:

Bit Crush: reduce the sample resolution by removing a certain amount of data bits. Only data is changed, output file format remains the same as input file.

Reverse (backward audio): copies all the samples from the input file to the output file in opposite order (timewise), but without swapping channels.

Channel Swap: invert channels from the input file.

Channel Subtract: Subtract the summed signal of all other audio channels from each audio channel.
Example:
Stereo Audio: L = L - R, R = R - L
5.1 Audio: FL = FL - (FR + C + SL + SR + Sub), FR = FR - (FL + C + SL + SR + Sub), C = C - (FL + FR + SL + SR + Sub), ...

Extras:
I left some test C++ source files to test the effects. Also, I left a bash script and a Makefile as options to compile all of them.
I used the GNU C++ Compiler (g++).

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com

