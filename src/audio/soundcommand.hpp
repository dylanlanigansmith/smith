#pragma once

#include <common.hpp>
#include <queue>

enum SoundCommand_Types : uint8_t
{
    SoundCmd_Invalid = 0xff,
    SoundCmd_Default = 0,
    SoundCmd_Music = 1,
    SoundCmd_Effect,
    SoundCommand_SIZE
};

struct SoundCommand{
    std::string m_szName;
    float m_flVolume; //0.0.-1.0
    float m_flPan;
    uint8_t m_nType; 
    bool m_bLoop;
    SoundCommand() : m_nType(SoundCmd_Invalid) {}
    SoundCommand(const std::string& m_szName, float m_flVolume = 1.f, float m_flPan = 0.f, bool m_bLoop = false) : m_szName(m_szName), m_flVolume(m_flVolume), m_flPan(m_flPan), m_nType(SoundCmd_Default), m_bLoop(m_bLoop) {}
};
class SoundQueue {
public:
    void pushCommand(const SoundCommand& command) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(command);
    }

    bool popCommand(SoundCommand& command) {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) {
            return false;
        }
        command = queue.front();
        queue.pop();
        return true;
    }

private:
    std::mutex mtx;
    std::queue<SoundCommand> queue;
};