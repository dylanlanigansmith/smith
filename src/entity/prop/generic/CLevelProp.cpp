#include "CLevelProp.hpp"
#include <util/saving.hpp>

REGISTER_DEF_ENT(CLevelProp);

void CLevelProp::OnCreate()
{
    m_loaded = false;
    LoadDefaults();
}

void CLevelProp::OnDestroy()
{
}

void CLevelProp::CreateRenderable()
{

}

void CLevelProp::Render(CRenderer *renderer)
{
    if(!m_loaded) return;
    DrawProp(renderer, draw_params.wScale, draw_params.vScale, draw_params.vOffset);
}
void CLevelProp::LoadDefaults()
{
    ENT_SETUP();
    m_szSubclass = "LevelPropEmpty";
    m_uid = m_szSubclass + std::string("_") + std::to_string(IEngineTime->GetCurTime().ns()); 

    m_flags = 0;
    //inheritance
    m_inherits = false;
    m_parentname = m_szName;

    m_textureName = "barrel.png";
    m_altTextureName = m_textureName;
    m_color = Color::White();
    draw_params.wScale = 1.0;
    draw_params.vScale = 1.0,
    draw_params.vOffset = 0;
    m_vecPosition = {
        15.7,
        9.5,
        0.0,
    };
    m_bounds = 0.35f;
    m_bbox = { 
        Vector2(), Vector2()
    };

    //triggers go here
    m_specialname =  "m_specialname";
    m_triggername =  "m_triggername";
    m_callbackname = "m_callbackname";
    m_actionname =   "m_actionname";
    m_slavename =    "m_slavename";

    m_maxhealth = 10;
    m_health = m_maxhealth;

    SetPropBlocking(true);
    SetPropStatic(true);
    SetPropPosFloor(true);
}

bool CLevelProp::FromJSON(const json &j)
{
    //https://json.nlohmann.me/api/basic_json/value/#examples
    m_uid = j["UID"];
    m_szName = j["m_name"];
    m_szSubclass = j["m_subclass"];
    m_flags = j["m_flags"];
    //inheritance
        m_inherits = j["inheritance"].at(0).get<bool>();
        m_parentname = j["inheritance"].at(1).get<std::string>();

    m_textureName = j["m_textureName"];
    m_altTextureName = j["m_altTextureName"];
    m_color = j["m_color"].get<uint32_t>();
   
    draw_params.wScale = j["draw_params"].at(0).get<double>();
    draw_params.vScale = j["draw_params"].at(1).get<double>();
    draw_params.vOffset = j["draw_params"].at(2).get<int>();

    m_vecPosition = {
        j["m_vecPosition"].at(0).get<double>(),
        j["m_vecPosition"].at(1).get<double>(),
        j["m_vecPosition"].at(2).get<double>(),
    };
   
    m_bounds = j["m_bounds"].get<float>();
    m_bbox = { 
        {j["m_bbox"].at(0).at(0).get<double>(), j["m_bbox"].at(0).at(1).get<double>()}, 
        {j["m_bbox"].at(1).at(0).get<double>(), j["m_bbox"].at(1).at(1).get<double>()}
    };

    //triggers go here
    m_specialname =  j["m_triggers"].at(0).get<std::string>();
    m_triggername =  j["m_triggers"].at(1).get<std::string>();
    m_callbackname = j["m_triggers"].at(2).get<std::string>();
    m_actionname =   j["m_triggers"].at(3).get<std::string>();
    m_slavename =    j["m_triggers"].at(4).get<std::string>();


     m_maxhealth = j["m_maxhealth"];
    UpdateData();
   
    return m_loaded;
}

void CLevelProp::UpdateData()
{
     SET_ENT_TYPE();

    SetPosition(m_vecPosition); //for callback
    SetupTexture();
    m_health = m_maxhealth;
    m_loaded = (m_Texture != nullptr);
}
/*
Sooo we actually want to save a universal prop + one with params
maybe we can add inheritance?

*/
json CLevelProp::ToJSON() 
{
    json j;
    std::string uid = m_szSubclass + std::string("_") + std::to_string(IEngineTime->GetCurTime().ns()); 
    j.emplace("UID", uid);
    j.emplace("m_name", m_szName);
    j.emplace("m_subclass", m_szSubclass);
    j.emplace("m_flags", m_flags);
    j["inheritance"] = {m_inherits, m_parentname};
    j["m_textureName"] = m_textureName;
    j["m_altTextureName"] = m_altTextureName;
    j["m_color"] = (uint32_t)m_color; //for future
    j["draw_params"] = Util::double2int1json(draw_params.wScale, draw_params.vScale, draw_params.vOffset);
    j["m_vecPosition"] = Util::vec3json(m_vecPosition);
    j["m_bounds"] = m_bounds;
    j["m_bbox"] = Util::bboxjson(m_bbox);
    j["m_triggers"] = Util::string_array_json({m_specialname, m_triggername, m_callbackname, m_actionname, m_slavename}); 
    //should make this std pair with var names and macro wizardry ^^^
    j["m_maxhealth"] = m_maxhealth;
    j["m_custom"] = {0,1,2,3,4}; //for future

    return j;
}

void CLevelProp::SetupTexture()
{
    if(m_textureName.empty()){
        return gError("%s: texture name empty!", m_szSubclass.c_str());
    }

    m_Texture = ITextureSystem->FindOrCreatetexture(m_textureName);
    if(!m_Texture){
        return gError("%s: texture %s not found?", m_szSubclass.c_str(), m_textureName.c_str());
    }
    m_hTexture = m_Texture->m_handle;

    //if(m_altTextureName && this->Flag)
    ///

}


