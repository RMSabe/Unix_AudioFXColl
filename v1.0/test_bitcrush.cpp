/*
	Test code to test AudioBitCrush class

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globaldef.h"

#include <iostream>
#include <string>

#include "AudioBaseClass.hpp"
#include "AudioBitCrush.hpp"

unsigned char cutoff = 0u;
char *filein_dir = nullptr;
char *fileout_dir = nullptr;

AudioBitCrush *pAudio = nullptr;

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		std::cout << "Error: missing arguments\nThis test routine requires 3 arguments:\n<input file directory> <output file directory> <bit cutoff>\nThey must be in that order\n";
		return 1;
	}

	filein_dir = argv[1];
	fileout_dir = argv[2];

	try
	{
		cutoff = std::stoi(argv[3]);
	}
	catch(...)
	{
		std::cout << "Error: value entered for bit cutoff is invalid\n";
		return 1;
	}

	pAudio = new AudioBitCrush(filein_dir, fileout_dir);

	if(!pAudio->initialize())
	{
		std::cout << "Error occurred: " << pAudio->getLastErrorMessage() << std::endl;
		delete pAudio;
		return 1;
	}

	if(!pAudio->setCutoff(cutoff))
	{
		std::cout << "Error setting cutoff\n";
		std::cout << pAudio->getLastErrorMessage() << std::endl;
		delete pAudio;
		return 1;
	}

	std::cout << "DSP started...\n";

	if(!pAudio->runDSP())
	{
		std::cout << "DSP failed\n";
		std::cout << "Error: " << pAudio->getLastErrorMessage() << std::endl;
		delete pAudio;
		return 1;
	}

	std::cout << "DSP finished\n";
	delete pAudio;
	return 0;
}

