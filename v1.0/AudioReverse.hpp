/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOREVERSE_HPP
#define AUDIOREVERSE_HPP

#include "AudioBaseClass.hpp"

class AudioReverse : public AudioBaseClass {
	public:
		AudioReverse(void);
		AudioReverse(std::string filein_dir);
		AudioReverse(std::string filein_dir, std::string fileout_dir);
		~AudioReverse(void);

		bool runDSP(void) override;

	private:
		void *buffer_input = nullptr;
		void *buffer_output = nullptr;

		bool runDSP_i16(void);
		bool runDSP_i24(void);

		void dsp_init_i16(void);
		void dsp_init_i24(unsigned char *bytebuf);

		void dsp_loop_i16(void);
		void dsp_loop_i24(unsigned char *bytebuf);
};

#endif //AUDIOREVERSE_HPP

