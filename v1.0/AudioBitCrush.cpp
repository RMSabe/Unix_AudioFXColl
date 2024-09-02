/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioBitCrush.hpp"

AudioBitCrush::AudioBitCrush(void) : AudioBaseClass()
{
}

AudioBitCrush::AudioBitCrush(std::string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioBitCrush::AudioBitCrush(std::string filein_dir, std::string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioBitCrush::~AudioBitCrush(void)
{
	if(this->buffer != nullptr) std::free(this->buffer);

	this->filein_close();
	this->fileout_close();
	this->filetemp_close();
}

bool AudioBitCrush::setCutoff(unsigned char bitcrush)
{
	switch(this->format)
	{
		case this->FORMAT_I16:
			return this->setCutoff_i16(bitcrush);

		case this->FORMAT_I24:
			return this->setCutoff_i24(bitcrush);
	}

	return false;
}

bool AudioBitCrush::runDSP(void)
{
	if(this->status < 1) return false;

	if(this->filein < 0)
	{
		this->status = this->STATUS_ERROR_NOFILE;
		return false;
	}

	switch(this->format)
	{
		case this->FORMAT_I16:
			return this->runDSP_i16();

		case this->FORMAT_I24:
			return this->runDSP_i24();
	}

	return false;
}

bool AudioBitCrush::setCutoff_i16(unsigned char bitcrush)
{
	if(bitcrush >= 15u)
	{
		this->error_msg = "Cutoff exceeds sample limit.";
		return false;
	}

	this->cutoff = 0;

	unsigned char b = 0u;
	while(b < bitcrush)
	{
		this->cutoff |= (1 << b);
		b++;
	}

	return true;
}

bool AudioBitCrush::setCutoff_i24(unsigned char bitcrush)
{
	if(bitcrush >= 23u)
	{
		this->error_msg = "Cutoff exceeds sample limit.";
		return false;
	}

	this->cutoff = 0;

	unsigned char b = 0u;
	while(b < bitcrush)
	{
		this->cutoff |= (1 << b);
		b++;
	}

	return true;
}

bool AudioBitCrush::runDSP_i16(void)
{
	if(!this->filetemp_create())
	{
		this->error_msg = "Could not create temporary DSP file.";
		return false;
	}

	this->buffer = std::malloc(this->BUFFER_SIZE_BYTES);

	this->filein_pos = this->audio_data_begin;
	this->filetemp_pos = 0;

	this->dsp_loop_i16();

	std::free(this->buffer);
	this->buffer = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

bool AudioBitCrush::runDSP_i24(void)
{
	if(!this->filetemp_create())
	{
		this->error_msg = "Could not create temporary DSP file.";
		return false;
	}

	unsigned char *bytebuf = (unsigned char*) std::malloc(this->BUFFER_SIZE_BYTES);
	this->buffer = std::malloc(this->BUFFER_SIZE_SAMPLES*sizeof(int));

	this->filein_pos = this->audio_data_begin;
	this->filetemp_pos = 0;

	this->dsp_loop_i24(bytebuf);

	std::free(bytebuf);

	std::free(this->buffer);
	this->buffer = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

void AudioBitCrush::dsp_loop_i16(void)
{
	short *buffer_i16 = (short*) this->buffer;
	size_t n_sample = 0u;

	while(true)
	{
		if(this->filein_pos >= this->audio_data_end) break;

		memset(buffer_i16, 0, this->BUFFER_SIZE_BYTES);

		lseek64(this->filein, this->filein_pos, SEEK_SET);
		read(this->filein, buffer_i16, this->BUFFER_SIZE_BYTES);
		this->filein_pos += (off64_t) this->BUFFER_SIZE_BYTES;

		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			buffer_i16[n_sample] &= ~((short) this->cutoff);
		}

		lseek64(this->filetemp, this->filetemp_pos, SEEK_SET);
		write(this->filetemp, buffer_i16, this->BUFFER_SIZE_BYTES);
		this->filetemp_pos += (off64_t) this->BUFFER_SIZE_BYTES;
	}

	return;
}

void AudioBitCrush::dsp_loop_i24(unsigned char *bytebuf)
{
	int *buffer_i32 = (int*) this->buffer;
	size_t n_sample = 0u;
	size_t n_byte = 0u;

	while(true)
	{
		if(this->filein_pos >= this->audio_data_end) break;

		memset(bytebuf, 0, this->BUFFER_SIZE_BYTES);

		lseek64(this->filein, this->filein_pos, SEEK_SET);
		read(this->filein, bytebuf, this->BUFFER_SIZE_BYTES);
		this->filein_pos += (off64_t) this->BUFFER_SIZE_BYTES;

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			buffer_i32[n_sample] = ((bytebuf[n_byte + 2u] << 16) | (bytebuf[n_byte + 1u] << 8) | (bytebuf[n_byte]));

			if(buffer_i32[n_sample] & 0x00800000) buffer_i32[n_sample] |= 0xff800000;
			else buffer_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

			buffer_i32[n_sample] &= ~this->cutoff;

			bytebuf[n_byte] = (buffer_i32[n_sample] & 0xff);
			bytebuf[n_byte + 1u] = ((buffer_i32[n_sample] >> 8) & 0xff);
			bytebuf[n_byte + 2u] = ((buffer_i32[n_sample] >> 16) & 0xff);

			n_byte += 3u;
		}

		lseek64(this->filetemp, this->filetemp_pos, SEEK_SET);
		write(this->filetemp, bytebuf, this->BUFFER_SIZE_BYTES);
		this->filetemp_pos += (off64_t) this->BUFFER_SIZE_BYTES;
	}

	return;
}

