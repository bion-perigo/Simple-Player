#include "core.h"

int main(int argc, char **argv){
    printf("\n========== Simple-Player ==========\n");
    
    init_audio_system_p();

    char resp = 0;

    struct sound_f *music = load_sound_f(FIND_ASSET("audio/hollow_bells.wav"));
    printf("Control => Play = r, Pause = p, Stop = s, Loop = l, Exit = e\n");

    while(resp != 'e'){
        resp = 0;
        printf("Control => Play = r, Pause = p, Stop = s, Loop = l, Exit = e\n");
        resp = getchar();
        
        if(resp ==  'p'){
           pause_sound_p(music);
           printf("Sound Pause\n");
        }else if(resp == 'r'){
            play_sound_p(music);
            printf("Sound Play\n");
        }else if(resp == 's'){
            stop_sound_p(music);
            printf("Sound Stop\n");
        }else if(resp == 'l'){
            set_loop_sound_p(music, true);
            printf("Sound Loop\n");
        }
        
    }

    unload_sound_f(music);
    close_audio_system_p();

    return 0;
}