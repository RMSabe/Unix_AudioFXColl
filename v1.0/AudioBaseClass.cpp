/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioBaseClass.hpp"

AudioBaseClass::AudioBaseClass(void)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->fileout_dir = this->FILEOUT_DIR_DEFAULT;
}

AudioBaseClass::AudioBaseClass(std::string filein_dir) : AudioBaseClass()
{
	this->filein_dir = filein_dir;
}

AudioBaseClass::AudioBaseClass(std::string filein_dir, std::string fileout_dir) : AudioBaseClass(filein_dir)
{
	this->fileout_dir = fileout_dir;
}

void AudioBaseClass::setInputFileDirectory(std::string file_dir)
{
	this->filein_dir = file_dir;
	return;
}

std::string AudioBaseClass::getInputFileDirectory(void)
{
	return this->filein_dir;
}

void AudioBaseClass::setOutputFileDirectory(std::string file_dir)
{
	this->fileout_dir = file_dir;
	return;
}

std::string AudioBaseClass::getOutputFileDirectory(void)
{
	return this->fileout_dir;
}

bool AudioBaseClass::initialize(void)
{
	if(!this->file_ext_check(this->filein_dir.c_str()))
	{
		this->status = this->STATUS_ERROR_FILENOTSUPPORTED;
		return false;
	}

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		return false;
	}

	if(!this->filein_get_params())
	{
		this->filein_close();
		return false;
	}

	this->BUFFER_SIZE_SAMPLES = this->BUFFER_SIZE_FRAMES*((size_t) this->n_channels);
	this->BUFFER_SIZE_BYTES = this->BUFFER_SIZE_SAMPLES*((size_t) (this->bit_depth/8u));

	return true;
}

std::string AudioBaseClass::getLastErrorMessage(void)
{
	switch(this->status)
	{
		case this->STATUS_ERROR_BROKENHEADER:
			return "File header is missing information (probably corrupted).";

		case this->STATUS_ERROR_FORMATNOTSUPPORTED:
			return "Audio format is not supported.";

		case this->STATUS_ERROR_FILENOTSUPPORTED:
			return "File format is not supported.";

		case this->STATUS_ERROR_NOFILE:
			return "File does not exist, or it's not accessible.";

		case this->STATUS_ERROR_GENERIC:
			return "Something went wrong.";

		case this->STATUS_UNINITIALIZED:
			return "Audio object not initialized.";
	}

	return this->error_msg;
}

unsigned int AudioBaseClass::getSampleRate(void)
{
	return this->sample_rate;
}

unsigned short AudioBaseClass::getBitDepth(void)
{
	return this->bit_depth;
}

unsigned short AudioBaseClass::getNumberChannels(void)
{
	return this->n_channels;
}

bool AudioBaseClass::filein_open(void)
{
	this->filein = open(this->filein_dir.c_str(), O_RDONLY);
	if(this->filein < 0) return false;

	this->filein_size = lseek64(this->filein, 0, SEEK_END);
	return true;
}

void AudioBaseClass::filein_close(void)
{
	if(this->filein < 0) return;

	close(this->filein);
	this->filein = -1;
	this->filein_size = 0;
	return;
}

bool AudioBaseClass::fileout_create(void)
{
	this->fileout = open(this->fileout_dir.c_str(), (O_WRONLY | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR));
	return (this->fileout >= 0);
}

void AudioBaseClass::fileout_close(void)
{
	if(this->fileout < 0) return;

	close(this->fileout);
	this->fileout = -1;
	return;
}

bool AudioBaseClass::filetemp_create(void)
{
	this->filetemp = open(this->FILETEMP_DIR, (O_WRONLY | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR));
	return (this->filetemp >= 0);
}

bool AudioBaseClass::filetemp_open(void)
{
	this->filetemp = open(this->FILETEMP_DIR, O_RDONLY);
	if(this->filetemp < 0) return false;

	this->filetemp_size = lseek64(this->filetemp, 0, SEEK_END);
	return true;
}

void AudioBaseClass::filetemp_close(void)
{
	if(this->filetemp < 0) return;

	close(this->filetemp);
	this->filetemp = -1;
	this->filetemp_size = 0;
	return;
}

bool AudioBaseClass::filein_get_params(void)
{
	const size_t BUFFER_SIZE = 4096u;
	char *header_info = (char*) std::malloc(BUFFER_SIZE);
	unsigned short *pu16 = nullptr;
	unsigned int *pu32 = nullptr;

	size_t bytepos = 0u;

	lseek64(this->filein, 0, SEEK_SET);
	read(this->filein, header_info, BUFFER_SIZE);

	if(!this->compare_signature("RIFF", header_info, 0u))
	{
		std::free(header_info);
		this->status = this->STATUS_ERROR_BROKENHEADER;
		return false;
	}

	if(!this->compare_signature("WAVE", header_info, 8u))
	{
		std::free(header_info);
		this->status = this->STATUS_ERROR_BROKENHEADER;
		return false;
	}

	bytepos = 12u;

	while(!this->compare_signature("fmt ", header_info, bytepos))
	{
		if(bytepos >= (BUFFER_SIZE - 256u))
		{
			std::free(header_info);
			this->status = this->STATUS_ERROR_BROKENHEADER;
			return false;
		}

		pu32 = (unsigned int*) &header_info[bytepos + 4u];
		bytepos += (size_t) (*pu32 + 8u);
	}

	pu16 = (unsigned short*) &header_info[bytepos + 8u];

	if(pu16[0] != 1u)
	{
		std::free(header_info);
		this->status = this->STATUS_ERROR_FORMATNOTSUPPORTED;
		return false;
	}

	this->n_channels = pu16[1];

	pu32 = (unsigned int*) &header_info[bytepos + 12u];
	this->sample_rate = *pu32;

	pu16 = (unsigned short*) &header_info[bytepos + 22u];
	this->bit_depth = *pu16;

	pu32 = (unsigned int*) &header_info[bytepos + 4u];
	bytepos += (size_t) (*pu32 + 8u);

	while(!this->compare_signature("data", header_info, bytepos))
	{
		if(bytepos >= (BUFFER_SIZE - 256u))
		{
			std::free(header_info);
			this->status = this->STATUS_ERROR_BROKENHEADER;
			return false;
		}

		pu32 = (unsigned int*) &header_info[bytepos + 4u];
		bytepos += (size_t) (*pu32 + 8u);
	}

	pu32 = (unsigned int*) &header_info[bytepos + 4u];

	this->audio_data_begin = (off64_t) (bytepos + 8u);
	this->audio_data_end = this->audio_data_begin + ((off64_t) *pu32);

	std::free(header_info);

	switch(this->bit_depth)
	{
		case 16u:
			this->format = this->FORMAT_I16;
			this->status = this->STATUS_INITIALIZED;
			return true;

		case 24u:
			this->format = this->FORMAT_I24;
			this->status = this->STATUS_INITIALIZED;
			return true;
	}

	this->format = this->FORMAT_UNSUPPORTED;
	this->status = this->STATUS_ERROR_FORMATNOTSUPPORTED;
	return false;
}

void AudioBaseClass::fileout_write_header(void)
{
	char *header_info = (char*) std::malloc(44);
	unsigned short *pu16 = nullptr;
	unsigned int *pu32 = nullptr;

	header_info[0] = 'R';
	header_info[1] = 'I';
	header_info[2] = 'F';
	header_info[3] = 'F';

	pu32 = (unsigned int*) &header_info[4];
	*pu32 = (unsigned int) (this->filetemp_size + 36);

	header_info[8] = 'W';
	header_info[9] = 'A';
	header_info[10] = 'V';
	header_info[11] = 'E';

	header_info[12] = 'f';
	header_info[13] = 'm';
	header_info[14] = 't';
	header_info[15] = ' ';

	pu32 = (unsigned int*) &header_info[16];
	*pu32 = 16u;

	pu16 = (unsigned short*) &header_info[20];
	pu16[0] = 1u;
	pu16[1] = this->n_channels;

	pu32 = (unsigned int*) &header_info[24];
	pu32[0] = this->sample_rate;
	pu32[1] = this->sample_rate*((unsigned int) (this->n_channels*this->bit_depth/8u));

	pu16 = (unsigned short*) &header_info[32];
	pu16[0] = this->n_channels*this->bit_depth/8u;
	pu16[1] = this->bit_depth;

	header_info[36] = 'd';
	header_info[37] = 'a';
	header_info[38] = 't';
	header_info[39] = 'a';

	pu32 = (unsigned int*) &header_info[40];
	*pu32 = (unsigned int) this->filetemp_size;

	lseek64(this->fileout, 0, SEEK_SET);
	write(this->fileout, header_info, 44);
	this->fileout_pos = 44;

	std::free(header_info);
	return;
}

bool AudioBaseClass::file_ext_check(const char *file_dir)
{
	if(file_dir == nullptr) return false;

	size_t len = 0u;
	while(file_dir[len] != '\0') len++;

	if(len < 5u) return false;

	if(this->compare_signature(".wav", file_dir, (len - 4u))) return true;
	if(this->compare_signature(".WAV", file_dir, (len - 4u))) return true;

	return false;
}

bool AudioBaseClass::compare_signature(const char *auth, const char *bytebuf, size_t offset)
{
	if(auth == nullptr) return false;
	if(bytebuf == nullptr) return false;

	if(auth[0] != bytebuf[offset]) return false;
	if(auth[1] != bytebuf[offset + 1u]) return false;
	if(auth[2] != bytebuf[offset + 2u]) return false;
	if(auth[3] != bytebuf[offset + 3u]) return false;

	return true;
}

bool AudioBaseClass::rawtowav_proc(void)
{
	if(!this->filetemp_open())
	{
		this->error_msg = "Could not open temporary DSP file.";
		return false;
	}

	if(!this->fileout_create())
	{
		this->filetemp_close();
		this->error_msg = "Could not create output file.";
		return false;
	}

	void *buffer = std::malloc(this->BUFFER_SIZE_BYTES);

	this->fileout_write_header();
	this->filetemp_pos = 0;

	this->rawtowav_proc_loop(buffer);

	std::free(buffer);

	this->filetemp_close();
	this->fileout_close();

	return true;
}

void AudioBaseClass::rawtowav_proc_loop(void *buffer)
{
	while(true)
	{
		if(this->filetemp_pos >= this->filetemp_size) break;

		memset(buffer, 0, this->BUFFER_SIZE_BYTES);

		lseek64(this->filetemp, this->filetemp_pos, SEEK_SET);
		read(this->filetemp, buffer, this->BUFFER_SIZE_BYTES);
		this->filetemp_pos += (off64_t) this->BUFFER_SIZE_BYTES;

		lseek64(this->fileout, this->fileout_pos, SEEK_SET);
		write(this->fileout, buffer, this->BUFFER_SIZE_BYTES);
		this->fileout_pos += (off64_t) this->BUFFER_SIZE_BYTES;
	}

	return;
}

