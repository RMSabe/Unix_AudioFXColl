/*
 * Audio FX Collection version 1.0 for Unix based systems.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOBASECLASS_HPP
#define AUDIOBASECLASS_HPP

#include "globaldef.h"
#include <string>

class AudioBaseClass {
	public:
		AudioBaseClass(void);
		AudioBaseClass(std::string filein_dir);
		AudioBaseClass(std::string filein_dir, std::string fileout_dir);

		void setInputFileDirectory(std::string file_dir);
		std::string getInputFileDirectory(void);

		void setOutputFileDirectory(std::string file_dir);
		std::string getOutputFileDirectory(void);

		bool initialize(void);

		virtual bool runDSP(void) = 0;

		std::string getLastErrorMessage(void);

		unsigned int getSampleRate(void);
		unsigned short getBitDepth(void);
		unsigned short getNumberChannels(void);

	protected:
		const char *FILETEMP_DIR = "temp.raw";
		const char *FILEOUT_DIR_DEFAULT = "output.wav";

		//These values are set during initialization
		//Consider these values constant
		size_t BUFFER_SIZE_FRAMES = 512u;
		size_t BUFFER_SIZE_SAMPLES = 0u;
		size_t BUFFER_SIZE_BYTES = 0u;

		enum FORMATS {
			FORMAT_UNSUPPORTED = -1,
			FORMAT_NULL = 0,
			FORMAT_I16 = 1,
			FORMAT_I24 = 2
		};

		enum STATUS {
			STATUS_ERROR_BROKENHEADER = -5,
			STATUS_ERROR_FORMATNOTSUPPORTED = -4,
			STATUS_ERROR_FILENOTSUPPORTED = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_INITIALIZED = 1
		};

		int format = this->FORMAT_NULL;
		int status = this->STATUS_UNINITIALIZED;

		std::string error_msg = "";

		unsigned int sample_rate = 0u;
		unsigned short bit_depth = 0u;
		unsigned short n_channels = 0u;

		off64_t audio_data_begin = 0;
		off64_t audio_data_end = 0;

		std::string filein_dir = "";
		std::string fileout_dir = "";

		int filein = -1;
		int fileout = -1;
		int filetemp = -1;

		off64_t filein_size = 0;
		off64_t filetemp_size = 0;

		off64_t filein_pos = 0;
		off64_t fileout_pos = 0;
		off64_t filetemp_pos = 0;

		bool filein_open(void);
		void filein_close(void);

		bool fileout_create(void);
		void fileout_close(void);

		bool filetemp_create(void);
		bool filetemp_open(void);
		void filetemp_close(void);

		bool filein_get_params(void);
		void fileout_write_header(void);

		bool file_ext_check(const char *file_dir);
		bool compare_signature(const char *auth, const char *bytebuf, size_t offset);

		bool rawtowav_proc(void);
		void rawtowav_proc_loop(void *buffer);
};

#endif //AUDIOBASECLASS_HPP

