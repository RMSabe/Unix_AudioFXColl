/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOCHANNELSWAP_HPP
#define AUDIOCHANNELSWAP_HPP

#include "AudioBaseClass.hpp"

class AudioChannelSwap : public AudioBaseClass {
	public:
		AudioChannelSwap(void);
		AudioChannelSwap(std::string filein_dir);
		AudioChannelSwap(std::string filein_dir, std::string fileout_dir);
		~AudioChannelSwap(void);

		bool runDSP(void) override;

	private:
		void *buffer_input = nullptr;
		void *buffer_output = nullptr;

		bool runDSP_i16(void);
		bool runDSP_i24(void);

		void dsp_loop_i16(void);
		void dsp_loop_i24(unsigned char *bytebuf);
};

#endif //AUDIOCHANNELSWAP_HPP

