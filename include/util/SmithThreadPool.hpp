#pragma once
#include <common.hpp>

class CThreadPool
{
    public:
    CThreadPool(size_t threads) : m_bHalt(false) {

    }
    ~CThreadPool(){}
    
private:

    bool m_bHalt;
};