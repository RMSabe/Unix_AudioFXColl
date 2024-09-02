/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioChannelSubtract.hpp"

AudioChannelSubtract::AudioChannelSubtract(void) : AudioBaseClass()
{
}

AudioChannelSubtract::AudioChannelSubtract(std::string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioChannelSubtract::AudioChannelSubtract(std::string filein_dir, std::string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioChannelSubtract::~AudioChannelSubtract(void)
{
	if(this->buffer != nullptr) std::free(this->buffer);

	this->filein_close();
	this->fileout_close();
	this->filetemp_close();
}

bool AudioChannelSubtract::runDSP(void)
{
	if(this->status < 1) return false;

	if(this->filein < 0)
	{
		this->status = this->STATUS_ERROR_NOFILE;
		return false;
	}

	if(this->n_channels < 2u)
	{
		this->error_msg = "This effect cannot be run on single channel audio.";
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

bool AudioChannelSubtract::runDSP_i16(void)
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

bool AudioChannelSubtract::runDSP_i24(void)
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

void AudioChannelSubtract::dsp_loop_i16(void)
{
	short *buffer_i16 = (short*) this->buffer;

	size_t n_frame = 0u;
	size_t n_sample = 0u;
	size_t n_channel = 0u;

	const int SAMPLE_MAX_VALUE = 0x7fff;
	const int SAMPLE_MIN_VALUE = -0x8000;

	int mono_sample = 0;
	int channel_sample = 0;

	while(true)
	{
		if(this->filein_pos >= this->audio_data_end) break;

		memset(buffer_i16, 0, this->BUFFER_SIZE_BYTES);

		lseek64(this->filein, this->filein_pos, SEEK_SET);
		read(this->filein, buffer_i16, this->BUFFER_SIZE_BYTES);
		this->filein_pos += (off64_t) this->BUFFER_SIZE_BYTES;

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			mono_sample = 0;

			for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((size_t) this->n_channels) + n_channel;

				mono_sample += (int) buffer_i16[n_sample];
			}

			for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((size_t) this->n_channels) + n_channel;

				channel_sample = (int) buffer_i16[n_sample];
				channel_sample *= (int) this->n_channels;
				channel_sample -= mono_sample;
				channel_sample /= (int) this->n_channels;

				if(channel_sample >= SAMPLE_MAX_VALUE) buffer_i16[n_sample] = (short) SAMPLE_MAX_VALUE;
				else if(channel_sample <= SAMPLE_MIN_VALUE) buffer_i16[n_sample] = (short) SAMPLE_MIN_VALUE;
				else buffer_i16[n_sample] = (short) channel_sample;
			}
		}

		lseek64(this->filetemp, this->filetemp_pos, SEEK_SET);
		write(this->filetemp, buffer_i16, this->BUFFER_SIZE_BYTES);
		this->filetemp_pos += (off64_t) this->BUFFER_SIZE_BYTES;
	}

	return;
}

void AudioChannelSubtract::dsp_loop_i24(unsigned char *bytebuf)
{
	int *buffer_i32 = (int*) this->buffer;

	size_t n_frame = 0u;
	size_t n_sample = 0u;
	size_t n_channel = 0u;
	size_t n_byte = 0u;

	const int SAMPLE_MAX_VALUE = 0x7fffff;
	const int SAMPLE_MIN_VALUE = -0x800000;

	int mono_sample = 0;

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

			n_byte += 3u;
		}

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			mono_sample = 0;

			for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((size_t) this->n_channels) + n_channel;

				mono_sample += buffer_i32[n_sample];
			}

			for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((size_t) this->n_channels) + n_channel;

				buffer_i32[n_sample] *= (int) this->n_channels;
				buffer_i32[n_sample] -= mono_sample;
				buffer_i32[n_sample] /= (int) this->n_channels;

				if(buffer_i32[n_sample] > SAMPLE_MAX_VALUE) buffer_i32[n_sample] = SAMPLE_MAX_VALUE;
				else if(buffer_i32[n_sample] < SAMPLE_MIN_VALUE) buffer_i32[n_sample] = SAMPLE_MIN_VALUE;
			}
		}

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
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

