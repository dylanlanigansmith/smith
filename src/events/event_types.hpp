#pragma once

#include <common.hpp>
#include <util/hash_fnv1a.hpp>
#include <entity/entity_types.hpp>


using hEvent = uint32_t;
using eventID_t = std::pair<const char*, hEvent>;



using event_args = std::shared_ptr<std::any>;
#define EventArg(val) std::make_shared<std::any>(val)
using eventCallbackFn = std::function<void(uint32_t, event_args)>;

 //copy pasta this:
 /*

 eventCallbackFn eventfn = [this](uint32_t caller, event_args args){ 
    try {
            Vector2 pos = std::any_cast<Vector2>(*args);  
    } catch (const std::bad_any_cast& e) {
        // Handle or log the exception
        this->log("Error in event argument casting: %s expected %s", e.what(), (*args).type().name() );
    }

 };
 
 */
//


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
/*
std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> * const this) (/usr/include/c++/13.2.1/bits/shared_ptr_base.h:337)
std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_count(std::__shared_count<(__gnu_cxx::_Lock_policy)2> * const this) (/usr/include/c++/13.2.1/bits/shared_ptr_base.h:1071)
std::__shared_ptr<std::any, (__gnu_cxx::_Lock_policy)2>::~__shared_ptr(std::__shared_ptr<std::any, (__gnu_cxx::_Lock_policy)2> * const this) (/usr/include/c++/13.2.1/bits/shared_ptr_base.h:1524)
std::shared_ptr<std::any>::~shared_ptr(std::shared_ptr<std::any> * const this) (/usr/include/c++/13.2.1/bits/shared_ptr.h:175)
event_t::~event_t(event_t * const this) (/home/dylan/code/smith/src/events/event_types.hpp:52)
std::destroy_at<event_t>(event_t * __location) (/usr/include/c++/13.2.1/bits/stl_construct.h:88)
std::allocator_traits<std::allocator<event_t> >::destroy<event_t>(event_t * __p, std::allocator_traits<std::allocator<event_t> >::allocator_type & __a) (/usr/include/c++/13.2.1/bits/alloc_traits.h:553)
std::vector<event_t, std::allocator<event_t> >::pop_back(std::vector<event_t, std::allocator<event_t> > * const this) (/usr/include/c++/13.2.1/bits/stl_vector.h:1323)
std::priority_queue<event_t, std::vector<event_t, std::allocator<event_t> >, std::less<event_t> >::pop(std::priority_queue<event_t, std::vector<event_t, std::allocator<event_t> >, std::less<event_t> > * const this) (/usr/include/c++/13.2.1/bits/stl_queue.h:777)
CEventManager::ProcessQueue(CEventManager * const this, int limit) (/home/dylan/code/smith/src/events/CEventManager.hpp:35)
CEntitySystem::OnLoopStart(CEntitySystem * const this) (/home/dylan/code/smith/src/interfaces/IEntitySystem/IEntitySystem.cpp:23)
CEngine::Run(CEngine * const this) (/home/dylan/code/smith/src/engine/engine.cpp:116)
main(int argc, char ** argv) (/home/dylan/code/smith/src/main.cpp:20)


*/

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
        System = 10, //for like changelevel and engine level stuff
        OnlyProcess = 100,
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


