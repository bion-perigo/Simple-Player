#ifdef PLATFORM_WINDOWS
#include "core.h"
#include <windows.h>
#include <libloaderapi2.h>
/********************Extern********************/
bool api_xaudio2_init(struct audio_api_p *api_audio);
void api_xaudio2_close();
/********************Extern********************/
/********************APIs********************/

/********************APIs********************/
/********************Variables********************/

static struct audio_api_p api_audio;

/********************Variables********************/
/********************Declaration********************/


/********************Declaration********************/

bool init_audio_system_p(){
    if(api_xaudio2_init(&api_audio)){
        return true;
    }

    return false;
}

void close_audio_system_p(){
   api_xaudio2_close();
}

struct sound_f *create_sound_p(struct wave_f *wave, unsigned short **data, unsigned long data_size){
    return api_audio.on_create_sound(wave, data, data_size);
}

void destroy_sound_p(struct sound_f *sound){
    api_audio.on_destroy_sound(sound);
}

void play_sound_p(struct sound_f *sound){
    api_audio.on_play_sound(sound);
}

void pause_sound_p(struct sound_f *sound){
    api_audio.on_pause_sound(sound);
}

void stop_sound_p(struct sound_f *sound){
    api_audio.on_stop_sound(sound);
}

void set_loop_sound_p(struct sound_f *sound, bool looping){
    api_audio.on_set_loop_sound(sound, looping);
}

#endif // PLATFORM_WINDOWS