#pragma once
//854 480

/*
640 x 360 (nHD)
854 x 480 (FWVGA)
960 x 540 (qHD)
1024 x 576 (WSVGA)
1280 x 720 (HD)
1366 x 768 (FWXGA)
1600 x 900 (HD+)
1920 x 1080 (Full HD)
*/

#define MSAA_BUFFER 1
#define MSAA_SAMPLES 2

//0 nearest pixel sampling 1 linear filter 2 anisotropic 
#define TEXTURE_SCALE_QUALITY "2"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

#define SCREEN_WIDTH_FULL 1920 //2560
#define SCREEN_HEIGHT_FULL 1080 //1440
//#define SCREEN_FULLSCREEN
#define TICKS_PER_S 32
//todo: make a non interface system / semi platform specific class that is a level below IResourceManager and handles raw FS / homedir stuff
//these are only used by logger for global file
#define LOG_FILEGLOBAL false
#define LOG_HOME_PATH std::string("/home/dylan")
#define LOG_RESOURCE_PATH std::string("/code/smith/resource")
#define DEV_BASEPATH std::string("/code/smith/")
#define LOG_SUBDIR std::string("/logs")


#define SMITH_PIXELFMT SDL_PIXELFORMAT_RGBA8888