#include "audio.hpp"
#include <engine/engine.hpp>
//https://github.com/libsdl-org/SDL_mixer/blob/main/include/SDL3_mixer/SDL_mixer.h

bool CSoundSystem::Init(int plat)
{
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            Error("failed to init SDLAudio %s", SDL_GetError());
            return false;
        }
        log("SDL_Audio Init");
    }

    m_bVerbose = true;
    log("start thread.");
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    m_szAudioResourcePath = IResourceSystem->GetResourceSubDir("audio");
    if(m_szAudioResourcePath.empty()){
        warn("No Resource Path!");
        return false;
    }
    m_mainThread = SDL_CreateThread((SDL_ThreadFunction)(&CSoundSystem::Loop), "CSoundSystem::Loop", this);
    if(m_mainThread == NULL){
        Error("failed to create audio thread: %s", SDL_GetError()); 
    }
   

    return (m_mainThread != NULL);
}

void CSoundSystem::Shutdown()
{
    SDL_DetachThread(m_mainThread);
    m_bShouldQuit = true;
}
//https://github.com/libsdl-org/SDL_mixer/blob/main/src/mixer.c#L343
int CSoundSystem::Loop(void *sndsys)
{
    m_bShouldQuit = false;
    if(SDL_GetCurrentAudioDriver() == NULL)
    {
        int driver = GetAudioDriver();
        if(driver < 0){
            Error("No Audio Driver Found. Fatal (%d)", driver); return 1;
        }
    }
    log("audio driver: %s", SDL_GetCurrentAudioDriver());
    SetupAudioDevice();
    LogAudioDevice();
    LoadAudioFile("dev_testf32.wav");
    LoadAudioFile("dev_test_scores.wav");
    LogAudioData("dev_testf32");
    LogAudioData("dev_test_scores");
    auto test = GetAudioByName("dev_testf32");
    auto scores = GetAudioByName("dev_test_scores");
    while(!m_bShouldQuit)
    {
        //SDL_memset(m_device.m_stream, SDL_GetSilenceValueForFormat(m_device.m_spec.format), (size_t)m_device.m_stream
        SDL_PutAudioStreamData(m_device.m_stream, test->m_buf, test->m_len);//queue system next
        //how the fuck do channels work
         SDL_PutAudioStreamData(m_device.m_stream, scores->m_buf, scores->m_len);
        SDL_Delay(4000);
    }
    log("shutting down..");
    
    SDL_DestroyAudioStream(m_device.m_stream);
    return 0;
}

int CSoundSystem::GetAudioDriver(bool list)
{
    int numDrivers = SDL_GetNumAudioDrivers();
    if(numDrivers == 0){
        Error("system reported %i audio drivers", numDrivers); return -1;
    }
    const int default_driver = 0;
    if(!list){
        log("using default audio driver: %s, [%d/%d]", SDL_GetAudioDriver(default_driver), default_driver, numDrivers);
        return default_driver;
    }
    dbg("== Audio Drivers [%i] ==", numDrivers);
    for(int i = 0; i < numDrivers; ++i){
        dbg("[%d] - %s ", i, SDL_GetAudioDriver(i));
    }
    return default_driver;
}

void CSoundSystem::SetupAudioDevice()
{
    m_device = audiodevice_t();
    m_device.m_spec ={
        .format = SDL_AUDIO_F32,
        .channels = 2,
        .freq = 48000
    };
    m_device.m_deviceID = SDL_AUDIO_DEVICE_DEFAULT_OUTPUT;

    //https://wiki.libsdl.org/SDL3/SDL_OpenAudioDeviceStream
    
    m_device.m_stream = SDL_OpenAudioDeviceStream(m_device.m_deviceID, &m_device.m_spec, NULL, NULL);
    m_device.m_deviceID = SDL_GetAudioStreamDevice(m_device.m_stream);
    if(m_device.m_stream == NULL || m_device.m_deviceID == 0){
        Error("failed to open audio device %d, %s", m_device.m_deviceID, SDL_GetError());
    }
    
    SDL_ResumeAudioDevice(m_device.m_deviceID);
    //https://wiki.libsdl.org/SDL3/Tutorials-AudioStream
}

void CSoundSystem::AudioCallback(void *userdata, uint8_t *stream, int len) //https://wiki.libsdl.org/SDL3/SDL_AudioSpec#callback
{} //not using 



void CSoundSystem::LoadAudioFile(const std::string &name, uint8_t format)
{
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    std::string fullpath = IResourceSystem->FindResource(m_szAudioResourcePath, name);
    std::string name_noext = name.substr(0, name.find_last_of("."));
    if(fullpath.empty()){
        Error("couldn't find %s in %s", name.c_str(), m_szAudioResourcePath.c_str()); return;
    }
    auto ad_ptr = new audiodata_t();
    auto added = soundboard.emplace(name_noext, ad_ptr);
    if(added.second == false){
        Error("couldn't add %s to soundboard", name_noext.c_str()); return;
    }
    auto& audiodata = added.first->second; //audiodata_t

    if(SDL_LoadWAV(fullpath.c_str(), &audiodata->m_spec, &audiodata->m_buf, &audiodata->m_len) != 0 ){
         Error(" WAV %s failed to load from %s -> %s", added.first->first.c_str(),fullpath.c_str(),  SDL_GetError()); 
         soundboard.erase(added.first);
         delete ad_ptr;
         log("removed %s from soundboard", name_noext.c_str());
         return;
    }

     //check if format matches, todo: convert/also check sample rate
    if(m_device.m_spec.format != audiodata->m_spec.format){
        Error("format for %s (%x) incompatible with device format (%x)",name_noext.c_str(), audiodata->m_spec.format, m_device.m_spec.format);
         delete ad_ptr;
        soundboard.erase(added.first); //should automatically SDL_free
        log("removed %s from soundboard", name_noext.c_str()); return;
    }

    dbg("added %s to soundboard, new size (%li)", added.first->first.c_str(), soundboard.size());
  

}

audiodata_t* CSoundSystem::GetAudioByName(const std::string &name)
{
    return soundboard.at(name); //todo handle excepts
}

void CSoundSystem::LogAudioData(const std::string& name)
{
    auto& ad = soundboard.at(name);
    note("audiodata for %s: ", name.c_str());
    float size_kb = (float)ad->m_len / 1000.f ;
    if(size_kb < 1000.f) log(">buffer size: %0.4f KB ", size_kb);
    else log(">buffer size: %0.4f MB ", size_kb / 1000.f);
        
    log(">sample rate: %d Hz ", ad->m_spec.freq );
    log(">channels: %d ", ad->m_spec.channels);
    log(">format %x", ad->m_spec.format);
}

void CSoundSystem::LogAudioDevice()
{
    
    note("output device [%d]: %s",m_device.m_deviceID, SDL_GetAudioDeviceName(m_device.m_deviceID));
    log(">sample rate: %d Hz ",  m_device.m_spec.freq );
    log(">channels: %d ",  m_device.m_spec.channels);
    log(">format %x ",  m_device.m_spec.format);
}
