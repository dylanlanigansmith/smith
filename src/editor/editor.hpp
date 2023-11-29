#pragma once
#include <common.hpp>


class CEditor
{
public:
    static CEditor& instance(){
        static CEditor ce;
        return ce;
    }
    void render();
    bool isOpen() const { return m_bIsOpen; }
private:
    bool m_bIsOpen = false;
};