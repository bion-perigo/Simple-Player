#include "core.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <xaudio2.h>

/********************APIs********************/
static struct api_ole{
    HRESULT (*CoInitializeEx)(LPVOID pvReserved, DWORD dwCoInit);
    void (*CoUninitialize)();
} api_ole;

static struct api_xaudio2{
    HRESULT (*XAudio2Create)(IXAudio2 **ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);
} api_xaudio2;
/********************APIs********************/

/********************Variables********************/
static HANDLE lib_com = NULL;
static HANDLE lib_xaudio2 = NULL;

static IXAudio2 *audio_engine = NULL;
static IXAudio2MasteringVoice *mastering_voice = NULL;

/********************Variables********************/
/********************Declaration********************/

struct audio_buffer_p{
	unsigned int Flags;                 // Either 0 or XAUDIO2_END_OF_STREAM.
	unsigned int AudioBytes;            // Size of the audio data buffer in bytes.
	const unsigned char* pAudioData;    // Pointer to the audio data buffer.
	unsigned int PlayBegin;             // First sample in this buffer to be played.
	unsigned int PlayLength;            // Length of the region to be played in samples,
	unsigned int LoopBegin;             // First sample of the region to be looped.
	unsigned int LoopLength;            // Length of the desired loop region in samples,
	unsigned int LoopCount;             // Number of times to repeat the loop region,
	void* pContext;                     // Context value to be passed back in callbacks.
};

bool api_xaudio2_init(struct audio_api_p *api_audio);
void api_xaudio2_close();
struct sound_f *api_xaudio2_create_sound(struct wave_f *wave, unsigned short **data, unsigned long data_size);
void api_xaudio2_destroy_sound(struct sound_f *sound);
void api_xaudio2_play_sound(struct sound_f *sound);
void api_xaudio2_pause_sound(struct sound_f *sound);
void api_xaudio2_stop_sound(struct sound_f *sound);
void api_xaudio2_set_loop_sound(struct sound_f *sound, bool looping);

/********************Declaration********************/
/********************Definition********************/
bool api_xaudio2_init(struct audio_api_p *api_audio){
    lib_com = LoadLibraryA("Ole32.dll");
    lib_xaudio2 = LoadLibraryA("XAudio2_8.dll");

    if(lib_com == NULL || lib_xaudio2 == NULL){
        printf("ERROR: Ole32.dll Or XAudio2_8.dll Not Loaded\n");
        return false;
    }

    api_ole.CoInitializeEx = (void*)GetProcAddress(lib_com, "CoInitializeEx");
    api_ole.CoUninitialize = (void*)GetProcAddress(lib_com, "CoUninitialize");
    api_xaudio2.XAudio2Create = (void*)GetProcAddress(lib_xaudio2, "XAudio2Create");

    if(!SUCCEEDED(api_ole.CoInitializeEx(NULL, COINIT_MULTITHREADED))){
        printf("ERROR: COM Not Initialized\n");
        return false;
    }

    if(!SUCCEEDED(api_xaudio2.XAudio2Create(&audio_engine, 0, XAUDIO2_DEFAULT_PROCESSOR ))){
        printf("ERROR: XAudio2 Not Create Engine\n");
        return false;
    }
    
    if(!SUCCEEDED(audio_engine->lpVtbl->CreateMasteringVoice(audio_engine, &mastering_voice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL, 0))){
        printf("ERROR XAudio2 Not Create Mastering Voice\n");
        return false;
    }

    api_audio->on_create_sound = &api_xaudio2_create_sound;
    api_audio->on_destroy_sound = &api_xaudio2_destroy_sound;
    api_audio->on_play_sound = &api_xaudio2_play_sound;
    api_audio->on_pause_sound = &api_xaudio2_pause_sound;
    api_audio->on_stop_sound = &api_xaudio2_stop_sound;
    api_audio->on_set_loop_sound = &api_xaudio2_set_loop_sound;

    printf("INFO:Audio System Initialized => Windows XAudio2\n");
    return true;
}

void api_xaudio2_close(){
    if(mastering_voice){
        mastering_voice->lpVtbl->DestroyVoice(mastering_voice);
    }
    if(audio_engine){
        audio_engine->lpVtbl->Release(audio_engine);
    }
    api_ole.CoUninitialize();

    printf("INFO:Audio System Closed => Windows XAudio2\n");
}

// [sound | wave | audio_buffer | data] layout in memory
struct sound_f *api_xaudio2_create_sound(struct wave_f *wave, unsigned short **data, unsigned long data_size){
    size_t total_size = sizeof(struct sound_f) + sizeof(struct wave_f) + sizeof(struct audio_buffer_p) + data_size;
    
    IXAudio2SourceVoice *source_voice = NULL;
    audio_engine->lpVtbl->CreateSourceVoice(audio_engine, &source_voice, (WAVEFORMATEX*)wave, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);
    
    struct sound_f *ptr_sound = (struct sound_f*)calloc(1, total_size);
    struct wave_f *ptr_wave = (struct wave_f*)(ptr_sound + sizeof(struct sound_f));
    struct audio_buffer_p *ptr_buffer = (struct audio_buffer_p *)(ptr_wave + sizeof(struct wave_f));
    unsigned short *ptr_data = (unsigned short*)(ptr_buffer + sizeof(char*));

    struct audio_buffer_p audio_buffer = {0};
    audio_buffer.pAudioData = (unsigned char*)ptr_data;
    audio_buffer.AudioBytes = data_size;
    audio_buffer.Flags = XAUDIO2_END_OF_STREAM;
    *data = ptr_data;

    memcpy((void*)ptr_wave, (void*)wave, sizeof(struct wave_f));
    memcpy((void*)ptr_buffer, (void*)&audio_buffer, sizeof(struct audio_buffer_p));
    
    ptr_sound->stream.audio_processor = (struct audio_processor_p*)source_voice;
    ptr_sound->stream.wave_format = ptr_wave;
    ptr_sound->stream.audio_buffer = ptr_buffer;
    ptr_sound->frames = wave->samples_per_sec * wave->bits_per_sample * wave->channels;

    return ptr_sound;
}

void api_xaudio2_destroy_sound(struct sound_f *sound){
    IXAudio2SourceVoice *source_voice = (IXAudio2SourceVoice*)sound->stream.audio_processor;
    source_voice->lpVtbl->DestroyVoice(source_voice);
    free(sound);
}

void api_xaudio2_play_sound(struct sound_f *sound){
    IXAudio2SourceVoice *source_voice = (IXAudio2SourceVoice*)sound->stream.audio_processor;
    source_voice->lpVtbl->SubmitSourceBuffer(source_voice, (XAUDIO2_BUFFER*)sound->stream.audio_buffer, NULL);
    source_voice->lpVtbl->Start(source_voice, 0, XAUDIO2_COMMIT_NOW);
}

void api_xaudio2_pause_sound(struct sound_f *sound){
    IXAudio2SourceVoice *source_voice = (IXAudio2SourceVoice*)sound->stream.audio_processor;
    source_voice->lpVtbl->Stop(source_voice, 0, XAUDIO2_COMMIT_NOW);
}

void api_xaudio2_stop_sound(struct sound_f *sound){
    IXAudio2SourceVoice *source_voice = (IXAudio2SourceVoice*)sound->stream.audio_processor;
    source_voice->lpVtbl->Stop(source_voice, 0, XAUDIO2_COMMIT_NOW);
    source_voice->lpVtbl->FlushSourceBuffers(source_voice);
    source_voice->lpVtbl->SubmitSourceBuffer(source_voice, (XAUDIO2_BUFFER*)sound->stream.audio_buffer, NULL);
}

void api_xaudio2_set_loop_sound(struct sound_f *sound, bool looping){
    IXAudio2SourceVoice *source_voice = (IXAudio2SourceVoice*)sound->stream.audio_processor;
    sound->stream.audio_buffer->LoopCount = (looping) ? XAUDIO2_LOOP_INFINITE : 0;
    source_voice->lpVtbl->FlushSourceBuffers(source_voice);
    source_voice->lpVtbl->SubmitSourceBuffer(source_voice, (XAUDIO2_BUFFER*)sound->stream.audio_buffer, NULL);
}

/********************Definition********************/

#endif // PLATFORM_WINDOWS