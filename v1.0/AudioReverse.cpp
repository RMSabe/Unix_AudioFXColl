/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioReverse.hpp"

AudioReverse::AudioReverse(void) : AudioBaseClass()
{
}

AudioReverse::AudioReverse(std::string filein_dir) : AudioBaseClass(filein_dir)
{
}

AudioReverse::AudioReverse(std::string filein_dir, std::string fileout_dir) : AudioBaseClass(filein_dir, fileout_dir)
{
}

AudioReverse::~AudioReverse(void)
{
	if(this->buffer_input != nullptr) std::free(this->buffer_input);
	if(this->buffer_output != nullptr) std::free(this->buffer_output);

	this->filein_close();
	this->fileout_close();
	this->filetemp_close();
}

bool AudioReverse::runDSP(void)
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

bool AudioReverse::runDSP_i16(void)
{
	if(!this->filetemp_create())
	{
		this->error_msg = "Could not create temporary DSP file.";
		return false;
	}

	this->buffer_input = std::malloc(this->BUFFER_SIZE_BYTES);
	this->buffer_output = std::malloc(this->BUFFER_SIZE_BYTES);

	this->dsp_init_i16();
	this->dsp_loop_i16();

	std::free(this->buffer_input);
	std::free(this->buffer_output);

	this->buffer_input = nullptr;
	this->buffer_output = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

bool AudioReverse::runDSP_i24(void)
{
	if(!this->filetemp_create())
	{
		this->error_msg = "Could not create temporary DSP file.";
		return false;
	}

	unsigned char *bytebuf = (unsigned char*) std::malloc(this->BUFFER_SIZE_BYTES);
	this->buffer_input = std::malloc(this->BUFFER_SIZE_SAMPLES*sizeof(int));
	this->buffer_output = std::malloc(this->BUFFER_SIZE_SAMPLES*sizeof(int));

	this->dsp_init_i24(bytebuf);
	this->dsp_loop_i24(bytebuf);

	std::free(bytebuf);

	std::free(this->buffer_input);
	std::free(this->buffer_output);

	this->buffer_input = nullptr;
	this->buffer_output = nullptr;

	this->filetemp_close();

	return this->rawtowav_proc();
}

void AudioReverse::dsp_init_i16(void)
{
	const off64_t AUDIO_DATALENGTH_BYTES = this->audio_data_end - this->audio_data_begin;
	const off64_t AUDIO_DATALENGTH_SAMPLES = AUDIO_DATALENGTH_BYTES/2;
	const off64_t AUDIO_DATALENGTH_FRAMES = AUDIO_DATALENGTH_SAMPLES/((off64_t) this->n_channels);

	const size_t N_FRAMES_REMAINING = (size_t) (AUDIO_DATALENGTH_FRAMES%((off64_t) this->BUFFER_SIZE_FRAMES));
	const size_t N_SAMPLES_REMAINING = N_FRAMES_REMAINING*((size_t) this->n_channels);
	const size_t N_BYTES_REMAINING = N_SAMPLES_REMAINING*2u;

	short *bufferin_i16 = (short*) this->buffer_input;
	short *bufferout_i16 = (short*) this->buffer_output;

	size_t n_frame = 0u;
	size_t n_counterframe = 0u;
	size_t n_sample = 0u;
	size_t n_countersample = 0u;
	size_t n_channel = 0u;

	this->filein_pos = this->audio_data_end - ((off64_t) N_BYTES_REMAINING);

	lseek64(this->filein, this->filein_pos, SEEK_SET);
	read(this->filein, bufferin_i16, N_BYTES_REMAINING);
	this->filein_pos -= (off64_t) this->BUFFER_SIZE_BYTES;

	for(n_frame = 0u; n_frame < N_FRAMES_REMAINING; n_frame++)
	{
		n_counterframe = N_FRAMES_REMAINING - n_frame - 1u;

		for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
		{
			n_sample = n_frame*((size_t) this->n_channels) + n_channel;
			n_countersample = n_counterframe*((size_t) this->n_channels) + n_channel;

			bufferout_i16[n_sample] = bufferin_i16[n_countersample];
		}
	}

	lseek64(this->filetemp, 0, SEEK_SET);
	write(this->filetemp, bufferout_i16, N_BYTES_REMAINING);
	this->filetemp_pos = (off64_t) N_BYTES_REMAINING;

	return;
}

void AudioReverse::dsp_init_i24(unsigned char *bytebuf)
{
	const off64_t AUDIO_DATALENGTH_BYTES = this->audio_data_end - this->audio_data_begin;
	const off64_t AUDIO_DATALENGTH_SAMPLES = AUDIO_DATALENGTH_BYTES/3;
	const off64_t AUDIO_DATALENGTH_FRAMES = AUDIO_DATALENGTH_SAMPLES/((off64_t) this->n_channels);

	const size_t N_FRAMES_REMAINING = (size_t) (AUDIO_DATALENGTH_FRAMES%((off64_t) this->BUFFER_SIZE_FRAMES));
	const size_t N_SAMPLES_REMAINING = N_FRAMES_REMAINING*((size_t) this->n_channels);
	const size_t N_BYTES_REMAINING = N_SAMPLES_REMAINING*3u;

	int *bufferin_i32 = (int*) this->buffer_input;
	int *bufferout_i32 = (int*) this->buffer_output;

	size_t n_frame = 0u;
	size_t n_counterframe = 0u;
	size_t n_sample = 0u;
	size_t n_countersample = 0u;
	size_t n_channel = 0u;
	size_t n_byte = 0u;

	this->filein_pos = this->audio_data_end - ((off64_t) N_BYTES_REMAINING);

	lseek64(this->filein, this->filein_pos, SEEK_SET);
	read(this->filein, bytebuf, N_BYTES_REMAINING);
	this->filein_pos -= (off64_t) this->BUFFER_SIZE_BYTES;

	n_byte = 0u;
	for(n_sample = 0u; n_sample < N_SAMPLES_REMAINING; n_sample++)
	{
		bufferin_i32[n_sample] = ((bytebuf[n_byte + 2u] << 16) | (bytebuf[n_byte + 1u] << 8) | (bytebuf[n_byte]));

		if(bufferin_i32[n_sample] & 0x00800000) bufferin_i32[n_sample] |= 0xff800000;
		else bufferin_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

		n_byte += 3u;
	}

	for(n_frame = 0u; n_frame < N_FRAMES_REMAINING; n_frame++)
	{
		n_counterframe = N_FRAMES_REMAINING - n_frame - 1u;

		for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
		{
			n_sample = n_frame*((size_t) this->n_channels) + n_channel;
			n_countersample = n_counterframe*((size_t) this->n_channels) + n_channel;

			bufferout_i32[n_sample] = bufferin_i32[n_countersample];
		}
	}

	n_byte = 0u;
	for(n_sample = 0u; n_sample < N_SAMPLES_REMAINING; n_sample++)
	{
		bytebuf[n_byte] = (bufferout_i32[n_sample] & 0xff);
		bytebuf[n_byte + 1u] = ((bufferout_i32[n_sample] >> 8) & 0xff);
		bytebuf[n_byte + 2u] = ((bufferout_i32[n_sample] >> 16) & 0xff);

		n_byte += 3u;
	}

	lseek64(this->filetemp, 0, SEEK_SET);
	write(this->filetemp, bytebuf, N_BYTES_REMAINING);
	this->filetemp_pos = (off64_t) N_BYTES_REMAINING;

	return;
}

void AudioReverse::dsp_loop_i16(void)
{
	short *bufferin_i16 = (short*) this->buffer_input;
	short *bufferout_i16 = (short*) this->buffer_output;

	size_t n_frame = 0u;
	size_t n_counterframe = 0u;
	size_t n_sample = 0u;
	size_t n_countersample = 0u;
	size_t n_channel = 0u;

	while(true)
	{
		if(this->filein_pos < this->audio_data_begin) break;

		memset(bufferin_i16, 0, this->BUFFER_SIZE_BYTES);

		lseek64(this->filein, this->filein_pos, SEEK_SET);
		read(this->filein, bufferin_i16, this->BUFFER_SIZE_BYTES);

		if(this->filein_pos <= ((off64_t) this->BUFFER_SIZE_BYTES)) this->filein_pos = 0;
		else this->filein_pos -= (off64_t) this->BUFFER_SIZE_BYTES;

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			n_counterframe = this->BUFFER_SIZE_FRAMES - n_frame - 1u;

			for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((size_t) this->n_channels) + n_channel;
				n_countersample = n_counterframe*((size_t) this->n_channels) + n_channel;

				bufferout_i16[n_sample] = bufferin_i16[n_countersample];
			}
		}

		lseek64(this->filetemp, this->filetemp_pos, SEEK_SET);
		write(this->filetemp, bufferout_i16, this->BUFFER_SIZE_BYTES);
		this->filetemp_pos += (off64_t) this->BUFFER_SIZE_BYTES;
	}

	return;
}

void AudioReverse::dsp_loop_i24(unsigned char *bytebuf)
{
	int *bufferin_i32 = (int*) this->buffer_input;
	int *bufferout_i32 = (int*) this->buffer_output;

	size_t n_frame = 0u;
	size_t n_counterframe = 0u;
	size_t n_sample = 0u;
	size_t n_countersample = 0u;
	size_t n_channel = 0u;
	size_t n_byte = 0u;

	while(true)
	{
		if(this->filein_pos < this->audio_data_begin) break;

		memset(bytebuf, 0, this->BUFFER_SIZE_BYTES);

		lseek64(this->filein, this->filein_pos, SEEK_SET);
		read(this->filein, bytebuf, this->BUFFER_SIZE_BYTES);

		if(this->filein_pos <= ((off64_t) this->BUFFER_SIZE_BYTES)) this->filein_pos = 0;
		else this->filein_pos -= (off64_t) this->BUFFER_SIZE_BYTES;

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			bufferin_i32[n_sample] = ((bytebuf[n_byte + 2u] << 16) | (bytebuf[n_byte + 1u] << 8) | (bytebuf[n_byte]));

			if(bufferin_i32[n_sample] & 0x00800000) bufferin_i32[n_sample] |= 0xff800000;
			else bufferin_i32[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

			n_byte += 3u;
		}

		for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
		{
			n_counterframe = this->BUFFER_SIZE_FRAMES - n_frame - 1u;

			for(n_channel = 0u; n_channel < ((size_t) this->n_channels); n_channel++)
			{
				n_sample = n_frame*((size_t) this->n_channels) + n_channel;
				n_countersample = n_counterframe*((size_t) this->n_channels) + n_channel;

				bufferout_i32[n_sample] = bufferin_i32[n_countersample];
			}
		}

		n_byte = 0u;
		for(n_sample = 0u; n_sample < this->BUFFER_SIZE_SAMPLES; n_sample++)
		{
			bytebuf[n_byte] = (bufferout_i32[n_sample] & 0xff);
			bytebuf[n_byte + 1u] = ((bufferout_i32[n_sample] >> 8) & 0xff);
			bytebuf[n_byte + 2u] = ((bufferout_i32[n_sample] >> 16) & 0xff);

			n_byte += 3u;
		}

		lseek64(this->filetemp, this->filetemp_pos, SEEK_SET);
		write(this->filetemp, bytebuf, this->BUFFER_SIZE_BYTES);
		this->filetemp_pos += (off64_t) this->BUFFER_SIZE_BYTES;
	}

	return;
}

