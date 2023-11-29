#include "Texture.hpp"
#include <engine/engine.hpp>


json CTexture::ToJSON()
{  

    json ret = { m_szName, (uint32_t)m_handle, (uint32_t)m_clrKey, m_flShade, m_nMode};

    return ret;
}

bool CTexture::FromJSON(const json& j)
{
    assert(j.size() == 5);
    std::string name = j.at(0);
    if(name.empty() || name.find("unknown") != std::string::npos) return false;

    m_szName = name;
    m_handle = j.at(1);
    m_clrKey = j.at(2);
    m_flShade = j.at(3);
    m_nMode = j.at(4);
    engine->dbg("%s %x %i %f", m_szName.c_str(), m_handle, m_clrKey, m_flShade);
    return true;
}

bool CTexture::Validate()
{
    if(m_handle <= 0 || m_szName.empty() || m_szName.find("unknown") != std::string::npos) return false;

    return true;
}

texture_t CTexture::Data() const
{
    texture_t add;
        add.m_handle = this->m_handle;
        add.m_size = this->m_size;
        add.m_texture = this->m_texture;
        add.m_clrKey = this->m_clrKey;
        add.m_flShade = this->m_flShade;
        add.m_nMode = this->m_nMode;
    return add;
}