/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOCHANNELSUBTRACT_HPP
#define AUDIOCHANNELSUBTRACT_HPP

#include "AudioBaseClass.hpp"

class AudioChannelSubtract : public AudioBaseClass {
	public:
		AudioChannelSubtract(void);
		AudioChannelSubtract(std::string filein_dir);
		AudioChannelSubtract(std::string filein_dir, std::string fileout_dir);
		~AudioChannelSubtract(void);

		bool runDSP(void) override;

	private:
		void *buffer = nullptr;

		bool runDSP_i16(void);
		bool runDSP_i24(void);

		void dsp_loop_i16(void);
		void dsp_loop_i24(unsigned char *bytebuf);
};

#endif //AUDIOCHANNELSUBTRACT_HPP

