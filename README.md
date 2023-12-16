# SMITH: 
## 2023 meets 1993 in this "3D" game engine

### Description
- wolfenstein style raycasting for now
    - for linux x86/64 and eventually Apple silicon Macs next time I go on vacation and only have a laptop

- full multithreaded game engine with editor, semi-dynamic lighting, enemy AI with A*, and many more subsystems

-  everything besides the deps. below is done from scratch

### Gallery
TODO


### Dependencies
- Meson Buildsystem
    - the only C++ buildsystem that never gets in my way 
        - sidenote: partially because it is really easy to hack your way out of messy build situations unlike CMake
        - never tried the Xcode integration but I hope it works (fav C++ ide), VSCode plugin locks up once a day but works well 
- SDL3
- stb_image header 
- stb_vorbis header
- Dear ImGUI
- nlohmann/json 
    - why make things difficult, I have yet to master serialization and hope to structure my scene/entity saving in a way I can be proud of this time. This library isn't my choice for lightweight/simple apps, but is it ever nice to use.. 
- magic_enum 
    - sidenote: where has this been all my life holy! it is actually magic!


- for some SDL stuff: sudo pacman -Sy xorg xorg-fonts  & a restart of X11 


### License/Usage
- use the code all you want but the files in /resource are either: randomly found online, or bought from itch.io in the past and not mine to share. acquire them yourself. 

