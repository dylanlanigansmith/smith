#include "audiofiles.hpp"
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#include <engine/engine.hpp>
int AudioImport::LoadWAV(const std::string &path, uint8_t **buf, uint32_t* len, SDL_AudioSpec* spec)
{
    if(SDL_LoadWAV(path.c_str(), spec, buf, len) != 0 ){
            engine->SoundSystem()->Error(" WAV failed to load from %s -> %s", path.c_str(),  SDL_GetError()); 
            return -1;
    }

    return 0;
}

void AudioImport::LoadOGG(const std::string &name, audiodata_t* ad)
{
    auto full_path = GetPathToAudio(name);
    if(full_path.empty())
        engine->warn("failed to find audiopath %s", name.c_str());
    
    int error = 0;
    stb_vorbis_alloc alloc;
   
    int channels, sample_rate;
    int16_t* ogg_buf; 
  
   int samples = stb_vorbis_decode_filename(full_path.c_str(), &channels, &sample_rate, &ogg_buf);
    engine->log("loaded %s %d samples %d channels %d rate", full_path.c_str(), samples, channels, sample_rate);
   //ez pz
   
   ad->m_buf = reinterpret_cast<uint8_t*>(ogg_buf);
   ad->m_len = channels * samples * sizeof(int16_t);
   ad->m_spec.channels = channels;
   ad->m_spec.freq = sample_rate;

}

audiodata_t *AudioImport::Load(const std::string &name_ext, SDL_AudioSpec *spec)
{
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    static auto IFileSystem = engine->CreateInterface<CFileSystem>("IFileSystem");

    std::string fullpath = GetPathToAudio(name_ext);
    if(fullpath.empty()){
     engine->SoundSystem()->Error("couldn't find %s ", name_ext.c_str()); return nullptr;
    }
    std::string ext = IFileSystem->GetExtension(name_ext);
   
    std::string name_noext = IFileSystem->RemoveExtension(name_ext);
    if(name_noext.length() > MAX_AUDIOFILENAME){
        engine->SoundSystem()->Error("name %s too long! max len %d", name_noext.c_str(), MAX_AUDIOFILENAME);  return nullptr;
    }
    if(name_noext.empty()){
        engine->SoundSystem()->Error("name noext empty for %s ", name_ext.c_str()); return nullptr;
    }
    audiodata_t* ad = nullptr;
    if(ext.find("wav") != std::string::npos)
    {
        ad = new audiodata_t();
        if(LoadWAV( fullpath.c_str(),  &ad->m_buf, &ad->m_len, &ad->m_spec) != 0 ){
            engine->SoundSystem()->Error(" WAVLoad  for %s failed from %s. ", name_noext.c_str(), fullpath.c_str()); 
            delete ad;
            return nullptr;
        }
         if(spec->format != ad->m_spec.format || spec->freq != ad->m_spec.freq || spec->channels != ad->m_spec.channels) {  //make this a function
            engine->SoundSystem()->Error("format for %s (%x)(%d hz %d ch) incompatible with device format (%x)(%d hz %d ch)",name_noext.c_str(),
                 ad->m_spec.format, ad->m_spec.freq, ad->m_spec.channels, spec->format, spec->freq, spec->channels);
            delete ad;
            return nullptr;
        }
    }
    else if(ext.find("ogg") != std::string::npos)
    {
        ad = new audiodata_t();
        ad->m_spec = *spec;
        LoadOGG(name_ext, ad);
        if(ad->m_len < 1){
            engine->SoundSystem()->Error("failed to load ogg file %s", name_ext.c_str());
            delete ad;
            return nullptr;
        }
        if(spec->format != ad->m_spec.format || spec->freq != ad->m_spec.freq || spec->channels != ad->m_spec.channels) { //i mean we could convert but.. sox.. sox is so good
            engine->SoundSystem()->Error("format for %s (%x)(%d hz %d ch) incompatible with device format (%x)(%d hz %d ch)",name_noext.c_str(),
                 ad->m_spec.format, ad->m_spec.freq, ad->m_spec.channels, spec->format, spec->freq, spec->channels);
            delete ad;
            return nullptr;
        }

    }
    else{
        engine->SoundSystem()->Error("unknown extension %s for file %s", ext.c_str(), name_ext.c_str());
   
        return nullptr;
    }
    if(ad == nullptr) return ad;
    
    strncpy(ad->m_name, name_noext.c_str(), name_noext.size());
    ad->m_name[name_noext.size()] = '\0';
     

    
   return ad;
}

std::string AudioImport::GetPathToAudio(const std::string &name)
{
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    static auto IFileSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");

    return IResourceSystem->FindResourceFromSubdir("audio", name);
    
}
