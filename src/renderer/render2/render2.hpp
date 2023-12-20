#pragma once

#ifdef __linux__
#include <renderer/renderer.hpp>
#include "../renderer.hpp"
#include <types/Vector.hpp>

#include <engine/engine.hpp>

#include <GL/gl.h>
#include <interfaces/interfaces.hpp>
#include <interfaces/IEngineTime/IEngineTime.hpp>
#include <util/misc.hpp>
#include "../raycast.hpp"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct segment
{
    std::array<glm::vec3, 4> pt;
    uint32_t texture;
    SDL_Color color;
    float floor_height = 0.f;
    segment() {}
    segment(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, SDL_Color color = {255,255,255,255}) : color(color){
        pt[0] = p0; pt[1] = p1; pt[2] = p2; pt[3] = p3; texture = 0;
    }
};

struct space
{
    std::vector<segment> walls;
};


namespace R2
{
    struct render_camera
    {
        glm::vec3 cameraPos = glm::vec3(18.0f, 13.0f, 16.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f); // or a direction vector
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        render_camera(){
            cameraPos = glm::vec3(18.0f, 0.5f, 16.0f);  // Position the camera at (0, 3, -10)
            cameraTarget = glm::vec3(0.0f, 0.5f, -1.0f); // Target a point directly ahead on the wall
            cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    };
    struct render_view
    {
        render_camera camera;
        glm::mat4 view;
        glm::mat4 projection;
        render_view(){
            
            projection = glm::perspective(glm::radians(90.0f), (float)SCREEN_WIDTH_FULL / (float)SCREEN_HEIGHT_FULL, 0.05f, 200.0f); //near far planes
        }
    };
    struct render_data
    {
       
        render_view view;

    };

    inline space get_world(){
        space sp;
        auto& segs = sp.walls;
      segs.push_back(segment({21,0,5}, {21,0,21}, {21,0,21}, {21,0,5}, {255, 0, 0, 255})); // Floor, CW order


        segs.push_back(segment({9,0,18}, {9,4,18}, {10,4,18}, {10,0,18}, {255,0,255,255}));
        segs.push_back(segment({2,0,0}, {2,4,0}, {0,4,0}, {0,0,0}));
        segs.push_back(segment( {4,0,2}, {4,4,2}, {2,4,0}, {2,0,0}, {0,255,0,255}));
        return sp;

    }
    static void init_view()
    {

    }
    inline glm::vec2 world_to_screen(const glm::vec3& pointInWorld, render_view& view_data)
    {
        view_data.view = glm::lookAt(view_data.camera.cameraPos, view_data.camera.cameraTarget, view_data.camera.cameraUp);
        glm::vec4 pointInView = view_data.view * glm::vec4(pointInWorld, 1.0f);
        glm::vec4 pointInClip = view_data.projection * pointInView;
        glm::vec3 pointInNDC = glm::vec3(pointInClip) / pointInClip.w;

        float screenWidth = (float)SCREEN_WIDTH_FULL;
        float screenHeight = (float)SCREEN_HEIGHT_FULL;
        glm::vec2 pointOnScreen;
        pointOnScreen.x = (pointInNDC.x + 1) * screenWidth / 2;
        pointOnScreen.y = (1 - pointInNDC.y) * screenHeight / 2; 

        return pointOnScreen;

    }

    /*
    
    little experiment

    editor first and foremost for this
    and porting camera for noclip asap
    
    */
    static void render_frame(CRenderer* renderer)
    {
        static render_data rd;



        static SDL_Texture* test = SDL_CreateTextureFromSurface(renderer->get(), ITextureSystem->FindOrCreatetexture("Bricks.png")->m_texture );
        auto player = IEntitySystem->GetLocalPlayer();
        const auto& cam = player->Camera();
        rd.view.camera.cameraPos = {-cam.m_vecPosition.x,1,-cam.m_vecPosition.y};
        rd.view.camera.cameraTarget = {cam.m_vecDir.x,0.5,cam.m_vecDir.y};

        static auto scene = get_world();
        int total_vertices = 4 * scene.walls.size();
        SDL_Vertex vertices[total_vertices];

        std::vector<int> indices(6 * scene.walls.size());
        for (int i = 0; i < scene.walls.size(); ++i) {
            indices[i*6 + 0] = i*4 + 0;
            indices[i*6 + 1] = i*4 + 1;
            indices[i*6 + 2] = i*4 + 2;
            indices[i*6 + 3] = i*4 + 2;
            indices[i*6 + 4] = i*4 + 3;
            indices[i*6 + 5] = i*4 + 0;
        }
        int vert_index = 0;
        for(auto& seg : scene.walls){
         
            for(int i = 0; i < seg.pt.size(); ++i){
                auto screen = world_to_screen(seg.pt[i], rd.view);
              //  renderer->log(" w2s for pt: %f %f %d", screen.x, screen.y, i);
                vertices[vert_index] = {{screen.x, screen.y}, SDL_Color{244,244,244,244}, {1.f - (1.f - float(i/10)), 1.f - float(i/10)}} ;
                
                vert_index++;
            }
            
        }
        for(int j = 0; j < total_vertices; ++j){
         //   renderer->log("%f %f %d ", vertices[j].position.x, vertices[j].position.y, j);
        }
       // SDL_RenderClear(renderer->get());
        SDL_SetRenderTarget(renderer->get(),(NULL));
        SDL_Vertex hello[3] = {
        {.position = {0.f, 0.f}, .color = {255, 255, 0, 255}},
        {.position = {12.f, 12.f}, .color = {255, 255, 0, 255}},
        {.position = {1280.f, 100.f}, .color = {255, 0, 0, 255}},
    };
   // int indicest[3] = {1, 2, 3};


        SDL_RenderGeometry(renderer->get(), test, vertices , total_vertices, indices.data(), indices.size());
     
       // SDL_RenderGeometry(renderer->get(), NULL, hello, 3, NULL, 3);
    }
}

#endif
