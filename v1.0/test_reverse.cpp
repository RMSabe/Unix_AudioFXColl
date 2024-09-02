/*
	Test code to test AudioReverse class

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globaldef.h"

#include <iostream>
#include <string>

#include "AudioBaseClass.hpp"
#include "AudioReverse.hpp"

char *filein_dir = nullptr;
char *fileout_dir = nullptr;

AudioReverse *pAudio = nullptr;

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		std::cout << "Error: missing arguments\nThis test routine requires 2 arguments:\n<input file directory> <output file directory>\nThey must be in that order\n";
		return 1;
	}

	filein_dir = argv[1];
	fileout_dir = argv[2];

	pAudio = new AudioReverse(filein_dir, fileout_dir);

	if(!pAudio->initialize())
	{
		std::cout << "Error occurred: " << pAudio->getLastErrorMessage() << std::endl;
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

