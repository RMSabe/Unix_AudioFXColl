#!/bin/bash

g++ test_bitcrush.cpp AudioBaseClass.cpp AudioBitCrush.cpp -o test_bitcrush.elf
g++ test_reverse.cpp AudioBaseClass.cpp AudioReverse.cpp -o test_reverse.elf
g++ test_channelswap.cpp AudioBaseClass.cpp AudioChannelSwap.cpp -o test_channelswap.elf
g++ test_channelsubtract.cpp AudioBaseClass.cpp AudioChannelSubtract.cpp -o test_channelsubtract.elf

