#pragma once
#include <common.hpp>
#include <util/rtti.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class CBaseSerializable
{
public:
    CBaseSerializable(std::string m_szClassType) : m_szClassType(m_szClassType) { }
    virtual ~CBaseSerializable() {}

    virtual json ToJSON() = 0;
    virtual bool FromJSON(const json& j ) = 0;
    virtual bool Validate() = 0;
    virtual std::string GetError() const { return m_szError; }
    virtual std::string GetType() const { return m_szClassType; }
private:
    std::string m_szError;
    std::string m_szClassType;
};