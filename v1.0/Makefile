test_bitcrush.elf: test_bitcrush.cpp AudioBaseClass.cpp AudioBitCrush.cpp
	g++ test_bitcrush.cpp AudioBaseClass.cpp AudioBitCrush.cpp -o test_bitcrush.elf

test_reverse.elf: test_reverse.cpp AudioBaseClass.cpp AudioReverse.cpp
	g++ test_reverse.cpp AudioBaseClass.cpp AudioReverse.cpp -o test_reverse.elf

test_channelswap.elf: test_channelswap.cpp AudioBaseClass.cpp AudioChannelSwap.cpp
	g++ test_channelswap.cpp AudioBaseClass.cpp AudioChannelSwap.cpp -o test_channelswap.elf

test_channelsubtract.elf: test_channelsubtract.cpp AudioBaseClass.cpp AudioChannelSubtract.cpp
	g++ test_channelsubtract.cpp AudioBaseClass.cpp AudioChannelSubtract.cpp -o test_channelsubtract.elf

all: test_bitcrush.elf test_reverse.elf test_channelswap.elf test_channelsubtract.elf

clear:
	rm test_bitcrush.elf
	rm test_reverse.elf
	rm test_channelswap.elf
	rm test_channelsubtract.elf

