#pragma once

#include <common.hpp>
#include <util/hash_fnv1a.hpp>
#include <entity/entity_types.hpp>


using hEvent = uint32_t;
using eventID_t = std::pair<const char*, hEvent>;



using event_args = std::shared_ptr<std::any>;
#define EventArg(val) std::make_shared<std::any>(val)
using eventCallbackFn = std::function<void(uint32_t, event_args)>;

 



struct event_listener {
    hEntity m_ID;
    eventCallbackFn m_fn;
    event_listener() : m_ID(-1) {};
    event_listener(hEntity m_ID, eventCallbackFn m_fn) : m_ID(m_ID), m_fn(m_fn) {}
    inline void call(const event_args& arg, hEntity m_caller = ENT_INVALID) const {
        m_fn(m_caller, arg);
    }

    bool operator==(const hEntity& rhs) const {
        return m_ID == rhs;
    }
    bool operator==(const event_listener& rhs) const {
        return m_ID == rhs.m_ID;
    }
};


struct event_t
{
    hEntity m_caller;
    eventID_t m_id;
    event_args args;
    int m_priority;
    
    event_t(){}
    event_t(const hEntity m_caller, const eventID_t& m_id, event_args args = event_args(), int m_priority = 0) 
                    : m_caller(m_caller), m_id(m_id), args(args), m_priority(m_priority) {}

    bool operator<(const event_t& rhs) const {
        return m_priority < rhs.m_priority;
    }
    bool operator>(const event_t& rhs) const {
        return m_priority > rhs.m_priority;
    }
    bool operator==(const event_t& rhs) const {
        return m_priority == rhs.m_priority;
    }
};

namespace Event
{
    enum EventPriority : int
    {
        Minimum = -1,
        Default = 0,
        Medium = 1,
        High = 2, 
        Urgent, //player esque
        System = 10 //for like changelevel and engine level stuff
    };

    inline std::string_view PriorityToStr(int p){
        return magic_enum::enum_name( (EventPriority)p);
    }
}




template <>
struct std::hash<eventID_t>
{
  std::size_t operator()(const eventID_t &k) const 
  {
    using std::hash;
    using std::size_t;
    using std::string;
     if(k.second < 100)
        throw std::runtime_error("did you hash EventID_t hEvent!?"); //idiot check 

    return k.second; //already hashed!
  }
};


