#include "audio.hpp"
#include "streamgroup.hpp"
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
bool CSoundSystem::PlaySound(const std::string &name, float m_flVolume, bool m_bLoop)
{
    SoundCommand cmd(name, m_flVolume, m_bLoop);
    m_cmdQueue.pushCommand(cmd);
    return false;
}

//aren't you glad you chose SDL3 
//(yes its more fun without documentation)
//https://wiki.libsdl.org/SDL3/CategoryAudio
// https://github.com/libsdl-org/SDL_mixer/blob/main/src/mixer.c#L343
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

  
    CStreamGroup<SMITH_AUDIOSTREAMS> streams(&m_device);
    streams.Create();

    SDL_ResumeAudioDevice(m_device.m_deviceID);

    //LogAudioDevice();

    //should just load the whole folder
    LoadAudioFile("dev_tests16.wav");
    LoadAudioFile("dev_test_scores.wav");
    LoadAudioFile("van_Wiese_bass_beat.wav");
    LoadAudioFile("dev_gunshot0.wav");
   // LogAudioData("van_Wiese_bass_beat");
   // LogAudioData("dev_tests16");
   // LogAudioData("dev_test_scores");
  
    auto test = GetAudioByName("van_Wiese_bass_beat");
    auto shot = GetAudioByName("dev_gunshot0");

    size_t buf_len = shot->m_len; //idk
   
    uint8_t* buf =  (uint8_t*) SDL_malloc(buf_len);
    
    while(!m_bShouldQuit)
    {
        SoundCommand command;
        if(m_cmdQueue.popCommand(command) && command.m_nType != SoundCmd_Invalid)
        {
            auto cmd_data = GetAudioByName(command.m_szName);
            if(cmd_data == nullptr) continue;
            if(buf_len < cmd_data->m_len){
                SDL_free(buf);
                buf = nullptr;
                dbg("resizing buffer from %li bytes to %i bytes ", buf_len, cmd_data->m_len);
                buf_len = cmd_data->m_len;
                buf =  (uint8_t*) SDL_malloc(buf_len); //sdlmix3 uses simd alligned buffer
            }
            
            SDL_memset(buf, SDL_GetSilenceValueForFormat(m_device.m_spec.format), cmd_data->m_len); //silence
            SDL_MixAudioFormat(buf, cmd_data->m_buf, SMITH_AUDIOFMT, cmd_data->m_len, (SDL_MIX_MAXVOLUME / 2) * command.m_flVolume);

            if(!streams.PutStreamData(cmd_data, buf, cmd_data->m_len)){
                Error("soundcmd %s failed to send to stream", command.m_szName.c_str());
            }
          //  SDL_PutAudioStreamData(m_streams[stream_to_use], buf, cmd_data->m_len);
          
        }
        SDL_DelayNS(4000);
    }
    log("shutting down..");
    SDL_free(buf);
 
    SDL_CloseAudioDevice(m_device.m_deviceID); 
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
        .format = SMITH_AUDIOFMT,
        .channels = 2,
        .freq = 48000
    };
    m_device.m_deviceID = SDL_AUDIO_DEVICE_DEFAULT_OUTPUT;

    //https://wiki.libsdl.org/SDL3/SDL_OpenAudioDeviceStream
    
    m_device.m_deviceID = SDL_OpenAudioDevice(m_device.m_deviceID, &m_device.m_spec);

    if( m_device.m_deviceID == 0){
        Error("failed to open audio device %d, %s", m_device.m_deviceID, SDL_GetError());
    }
    //https://wiki.libsdl.org/SDL3/Tutorials-AudioStream
}




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
    audiodata->m_duration_ms = GetSoundDuration(audiodata).ms();
    dbg("added %s to soundboard, new size (%li)", added.first->first.c_str(), soundboard.size());
    

}

audiodata_t* CSoundSystem::GetAudioByName(const std::string &name)
{
    audiodata_t* ret = nullptr;
    try
    {
        ret = soundboard.at(name);
        
    }
    catch(const std::exception& e)
    {
       Error("no audio found for %s, [%s]", name.c_str(), e.what());
      
    }
    
    return ret;
}

void CSoundSystem::LogAudioData(const std::string& name)
{
    auto& ad = soundboard.at(name);
    note("audiodata for %s: ", name.c_str());
    auto dur = GetSoundDuration(ad);
    log(">duration [%.4f s] (%d ms)", dur.sec(), dur.ms());

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

Time_t CSoundSystem::GetSoundDuration(audiodata_t *audio)
{

    uint32_t samplesize = SDL_AUDIO_BITSIZE(audio->m_spec.format) / 8;
    uint32_t sampleCount = audio->m_len / samplesize;
    if(audio->m_len % samplesize != 0){
        warn("audioLen / samplesize has a remainder");
    }
    uint32_t sampleLen = sampleCount;
    if(audio->m_spec.channels){
        sampleLen = sampleCount / audio->m_spec.channels;
    }
    double seconds = (double)sampleLen / (double)audio->m_spec.freq;

    return Time_t(seconds);
}
