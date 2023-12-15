#include <common.hpp>

#include <SDL3/SDL.h>
#include <engine/engine.hpp>
#include <plat/platform.hpp>
CEngine* engine;

int main(int argc, char** argv)
{
   if(!PLATFORM.Init(argc, argv)){
      return 1;
    }
   engine = new CEngine();

   if(!engine->Start("smith-engine v0.0.0")){
      PLATFORM.Dialog().MessageBox("engine init failed!"); 
      delete engine; return 1;
   }

   int err = engine->Run();

   delete engine;

   return err; 
}