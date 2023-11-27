#include <common.hpp>

#include <SDL3/SDL.h>
#include <engine/engine.hpp>

CEngine* engine;

int main(int argc, char** argv)
{
   engine = new CEngine();

   engine->Start("smith-engine v0.0.0");

   int err = engine->Run();

   delete engine;

   return err; 
}