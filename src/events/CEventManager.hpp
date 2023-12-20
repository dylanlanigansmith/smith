#pragma once

#include <common.hpp>
#include <util/hash_fnv1a.hpp>
#include <logger/logger.hpp>


#include "event_types.hpp"

class CEventManager : private CLogger
{
public: 
    CEventManager() : CLogger(this) {}
    virtual ~CEventManager(){}

    int ProcessQueue(int limit = 0){
        bool limited = (limit > 0);
        int events_processed = 0;
        int listeners_called = 0; //for curiousity

        auto pending_events = event_queue.size();
        while(!event_queue.empty() || (events_processed > limit && limited) )
        {
            auto& event = event_queue.top();
            auto& actions = event_actions[event.m_id.second];
            if(actions.empty()){
                dbg("event %s has no listeners!", event.m_id.first);
                event_queue.pop(); continue;
            }
            for(const auto& listener : actions){
                listener.call(event.args);
                listeners_called++;
            }

            event_queue.pop();
            events_processed++;
        } 

        return events_processed;
    }


    bool AddEvent(const eventID_t& eventID){
        
        auto ret = event_actions.emplace(eventID.second, std::vector<event_listener>());

        if(!ret.second){
            if(ret.first != nullptr){
                Error("failed to add event '%s': event exists already with %li listeners", eventID.first, ret.first->second.size());
            }
            else{
                Error("failed to add event '%s': event does not seem to exist already", eventID.first);
            }
             return false;
        }
        return true;
    }

    bool AddListener(CBaseEntity* ent, const eventID_t& eventID,  eventCallbackFn& callback){
        return AddListener(ent->GetID(), eventID, callback);
    }
    bool AddListener(hEntity m_id, const eventID_t& eventID,  eventCallbackFn& callback){
        listener_registry[m_id].push_back(eventID.second);
        event_actions[eventID.second].push_back({m_id, callback}); //create or add to

        dbg("added event listener for %s", eventID.first);
        return true;
    }

    template <typename T>
    inline void FireEvent(T* ent, const eventID_t& eventID, event_args args = event_args(), int priority = Event::Default){
        FireEvent(((CBaseEntity*)ent)->GetID(), eventID, args, priority);
        
    }
    inline void FireEvent(CBaseEntity* ent, const eventID_t& eventID, event_args args = event_args(), int priority = Event::Default){
        FireEvent(ent->GetID(), eventID, args, priority);
        
    }
    inline void FireEvent(const eventID_t& eventID, event_args args = event_args(), int priority = Event::Default){
        FireEvent(ENT_INVALID, eventID, args, priority);
        
    }
    inline void FireEvent(hEntity m_id, const eventID_t& eventID, event_args args = event_args(), int priority = Event::Default){
        event_queue.emplace(event_t(m_id, eventID, args, priority));
        dbg("event '%s' fired! priority: %s", eventID.first, Event::PriorityToStr((priority)).data());
    }


    

    size_t RemoveListener(hEntity id){
        if(id == ENT_INVALID){
            warn("trying to remove an entity's listeners when the entity has ID: -1 ?");
            return 0;
        }
        auto search = listener_registry.find(id);
        size_t numRemoved = 0;
        if(search != listener_registry.end())
        {
            auto& registered_events = search->second;

            for(auto& event : registered_events)
            {
                auto& listeners = event_actions[event];
                
                if(listeners.empty()) continue;
                size_t before = listeners.size();
                listeners.erase(std::remove_if(listeners.begin(), listeners.end(), [id](event_listener& el){  return el == id;  }), listeners.end());

                numRemoved += (before - listeners.size());
            }
        }
        dbg("removed %li event listeners for entity %d", numRemoved, id);
        return numRemoved;
    }

    

    static constexpr inline eventID_t EventID(const char* name){

        return {name, Util::fnv1a::hash_32_fnv1a_const(name)};
    }
private:
    

private:
    std::unordered_map<hEntity, std::vector<hEvent>> listener_registry; //for internal use... how do we get rid of old listeners.. 
    std::unordered_map<hEvent, std::vector<event_listener> > event_actions;

    std::priority_queue<event_t> event_queue;
};