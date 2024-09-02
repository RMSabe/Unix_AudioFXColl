/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOBITCRUSH_HPP
#define AUDIOBITCRUSH_HPP

#include "AudioBaseClass.hpp"

class AudioBitCrush : public AudioBaseClass {
	public:
		AudioBitCrush(void);
		AudioBitCrush(std::string filein_dir);
		AudioBitCrush(std::string filein_dir, std::string fileout_dir);
		~AudioBitCrush(void);

		bool setCutoff(unsigned char bitcrush);
		bool runDSP(void) override;

	private:
		void *buffer = nullptr;
		int cutoff = 0;

		bool setCutoff_i16(unsigned char bitcrush);
		bool setCutoff_i24(unsigned char bitcrush);

		bool runDSP_i16(void);
		bool runDSP_i24(void);

		void dsp_loop_i16(void);
		void dsp_loop_i24(unsigned char *bytebuf);
};

#endif //AUDIOBITCRUSH_HPP

