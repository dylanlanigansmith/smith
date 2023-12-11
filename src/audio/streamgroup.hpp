#include "audio.hpp"

template < size_t ARRAY_LEN >
class CStreamGroup : public CLogger
{
public:
   CStreamGroup(audiodevice_t* m_device) : CLogger(this), //should be CSoundsystem::CStreamGrp
                                             m_iNumStreams(ARRAY_LEN), m_device(m_device) 
    {
       
        Debug(false);
    }
    ~CStreamGroup(){
        
    }
    void Destroy(){

         //SDL3 bug?
        for(int i = 0; i < m_iNumStreams; ++i)
        {
            if(m_group[i].stream != nullptr)
                SDL_DestroyAudioStream(m_group[i].stream  ); 
        }
        log("destroyed %li streams", m_iNumStreams);
    }
    bool Create(){
        

        for(int i = 0; i < m_iNumStreams; ++i)
        {
             m_group[i].stream =  SDL_CreateAudioStream(&m_device->m_spec, &m_device->m_spec);
            if(m_group[i].stream == NULL){

                log("failed to create stream %d, %s",i, SDL_GetError()); return false; 
            }
          
            m_streams[i] = m_group[i].stream;
        }
        if(SDL_BindAudioStreams(m_device->m_deviceID, m_streams, m_iNumStreams) != 0){
            log("failed to bind %li streams to device, %s", m_iNumStreams, SDL_GetError()); return false;
        }
        
        note("created %li streams and bound to device: %d", m_iNumStreams, m_device->m_deviceID);
        return true;
    }
    bool PutStreamData(audiodata_t* audio_data, uint8_t* buf, uint32_t len){
         uint64_t cur_time = SDL_GetTicks();
        int stream_to_use = FindAvailableStream(cur_time);
        if(stream_to_use < 0 || stream_to_use >= m_iNumStreams){
            note("no available streams!"); return false;
        }
        if(m_group[stream_to_use].time_end > 0){
            note("stream found not reset, might be in use!"); return false; //how woudl this even happen
        }

        dbg("playing on stream %d for %li at %li", stream_to_use, audio_data->m_duration_ms, cur_time);
        m_group[stream_to_use].use_for(cur_time, audio_data->m_duration_ms);
        if(SDL_PutAudioStreamData(m_group[stream_to_use].stream, buf, audio_data->m_len) != 0){
            note("SDL_PutAudioStreamData failed, %s", SDL_GetError()); return false;
        }


        return true;
    }
private:
    int FindAvailableStream(uint64_t cur_time){
        if(RefreshStreams(cur_time) > 0){
            for(size_t i = 0; i < m_iNumStreams; ++i){
                if(!m_group[i].in_use)
                    return i;

            }
        }
        return -1;
    }
    int RefreshStreams(uint64_t cur_time){
        int free_streams = 0;
        for(auto& stream : m_group){
            if(stream.in_use){
                if(stream.time_end <= cur_time){
                    free_streams++;
                    stream.reset();
                }
            
            }
            else free_streams++;
        }

        return free_streams;
    }
private:
    size_t m_iNumStreams;
    std::array<audiostream_t, ARRAY_LEN> m_group;//>:(
    SDL_AudioStream* m_streams[ARRAY_LEN]; //just for passing to SDL_BindAudioStream, init/destroy
    audiodevice_t* m_device;
    
};