#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// [sound | wave | audio_buffer | data] layout in memory

#define NO_EXPAND(a) #a
#define STR(a) NO_EXPAND(a)
#define FIND_ASSET(asset_path) STR(CONTENT_PATH) asset_path

struct wave_f{
	unsigned short format_tag;         /* format type */
	unsigned short channels;          /* number of channels (i.e. mono, stereo...) */
	unsigned long  samples_per_sec;     /* sample rate */
	unsigned long  avg_bytes_per_sec;    /* for buffer estimation */
	unsigned short block_align;        /* block size of data */
	unsigned short bits_per_sample;     /* number of bits per sample of mono data */
	unsigned short cb_size;             /* the count in bytes of the size of */
};

struct audio_buffer_p;    // Internal type for audio control on the platform.
struct audio_processor_p; // Internal type for audio control on the platform.

struct audio_stream_p{
    struct wave_f *wave_format;
    struct audio_buffer_p *audio_buffer;
    struct audio_processor_p *audio_processor;
};

struct sound_f{
    struct audio_stream_p stream;
    unsigned int frames;
};

// used only on the platform
struct audio_api_p{
   struct sound_f *(*on_create_sound)(struct wave_f *wave, unsigned short **data, unsigned long data_size);
   void (*on_destroy_sound)(struct sound_f *sound);
   void (*on_play_sound)(struct sound_f *sound);
   void (*on_pause_sound)(struct sound_f *sound);
   void (*on_stop_sound)(struct sound_f *sound);
   void (*on_set_loop_sound)(struct sound_f *sound, bool looping);
};

/*Framework*/
// Asset Manager
struct sound_f *load_sound_f(const char *file_name);
void unload_sound_f(struct sound_f *sound);

/*Platform*/
bool init_audio_system_p();
void close_audio_system_p();
void play_sound_p(struct sound_f *sound);
void pause_sound_p(struct sound_f *sound);
void stop_sound_p(struct sound_f *sound);
void set_loop_sound_p(struct sound_f *sound, bool looping);
struct sound_f *create_sound_p(struct wave_f *wave, unsigned short **data, unsigned long data_size);
void destroy_sound_p(struct sound_f *sound);
