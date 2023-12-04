#include "core.h"
const unsigned long fourccRIFF = 1179011410; // RIFF
const unsigned long fourccDATA = 1635017060; // data
const unsigned long fourccFMT  = 544501094;  // fmt
const unsigned long fourccWAVE = 1163280727; // WAVE

struct wave_f *load_wave();
bool find_chunk(FILE* file, unsigned long fourcc, unsigned long* chunk_Size, unsigned long* chunk_data_position);
bool read_chunk_data(FILE* file, void* buffer, unsigned long buffer_size, unsigned long buffer_offset);

// [sound | wave | audio_buffer]
struct sound_f *load_sound_f(const char *file_name){
    FILE* file = NULL;
    unsigned long filetype = 0;
    struct wave_f wave;
    
    file = fopen(file_name, "rb");
	if (file == NULL) {
		return NULL;
	}

	unsigned long chunk_size;
	unsigned long chunk_position;
	find_chunk(file, fourccRIFF, &chunk_size, &chunk_position);

	read_chunk_data(file, &filetype, sizeof(unsigned int), chunk_position);
	if (filetype != fourccWAVE) {
		return NULL;
	}

	find_chunk(file, fourccFMT, &chunk_size, &chunk_position);
	read_chunk_data(file, &wave, chunk_size, chunk_position);

	find_chunk(file, fourccDATA, &chunk_size, &chunk_position);

    unsigned short *data_buffer = NULL;
    struct sound_f *sound = create_sound_p(&wave, &data_buffer, chunk_size);
	read_chunk_data(file, data_buffer, chunk_size, chunk_position);
	
    return sound;
}

void unload_sound_f(struct sound_f *sound){
	destroy_sound_p(sound);
}

bool find_chunk(FILE* file, unsigned long fourcc, unsigned long* chunk_size, unsigned long* chunk_data_position){
	bool ret = true;
	unsigned long chunk_type;
	unsigned long chunk_data_size;
	unsigned long file_type;
	unsigned long offset = 0;

	if (fseek(file, 0, SEEK_SET)){
		return false;
	}

	while (ret == true){
		if (fread(&chunk_type, sizeof(unsigned long), 1, file) == 0){
			ret = false;
		}
			
		if (fread(&chunk_data_size, sizeof(unsigned long), 1, file) == 0){
			ret = false;
		}

		if(chunk_type == fourccRIFF){
			chunk_data_size = 4;
			if (fread(&file_type, sizeof(unsigned long), 1, file) == 0){
				ret = false;
			}
		} else{
			fseek(file, chunk_data_size, SEEK_CUR);
		}

		offset += sizeof(unsigned long) * 2;

		if (chunk_type == fourcc){
			*chunk_size = chunk_data_size;
			*chunk_data_position = offset;
			return true;
		}
		offset += chunk_data_size;
	}

	return true;
}

bool read_chunk_data(FILE* file, void* buffer, unsigned long buffer_size, unsigned long buffer_offset){
	fseek(file, buffer_offset, SEEK_SET);
	if (fread(buffer, buffer_size, 1, file) == 0){
		return false;
	}
	
	return true;
}