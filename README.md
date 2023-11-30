# SMITH: 
## A Very Low-Tech "3D" Engine 

### Description
- wolfenstein style raycasting for now
    - for linux x86/64 and eventually Apple silicon Macs next time I go on vacation and only have a laptop
- multiplayer support and modern entity system as final end goal
    - skeletal 2d sprite animations? 
- flesh out entity system and editor then upgrade to BSP/doom style graphics
    
    - I find graphics programming the most rewarding but least fun, and designing the engine's structure and systems the most fun but least rewarding. Funny how that works. 
    
    - this will result in most of the editor being scrapped but ImGui Canvas looks like a blast to use for this style
    - I also realize that switching to straight OpenGL w/ glm would make this easier as well (simple transforms)
    - might be more rewarding to make a really great wolf3d graphics engine and add multiplayer, then fork it for "real" 3d
        - this is the the current plan

### Gallery



### Dependencies
- Meson Buildsystem
    - the only C++ buildsystem that never gets in my way 
        - sidenote: partially because it is really easy to hack your way out of messy build situations unlike CMake
        - never tried the Xcode integration but I hope it works (fav C++ ide), VSCode plugin locks up once a day but works well 
- SDL3
    - great mac/linux support 
- SDL3_Image
    -  the only annoying dependency 
- Dear ImGUI
    - obviously <3
- nlohmann/json 
    - why make things difficult, I have yet to master serialization and hope to structure my scene/entity saving in a way I can be proud of this time. This library isn't my choice for lightweight/simple apps but is it ever nice to use.. 
- magic_enum 
    - sidenote: where has this been all my life holy! it is actually magic!

### License/Usage
- use the code all you want but the files in /resource are either: randomly found online, or bought from itch.io in the past and not mine to share. acquire them yourself. 
- not allowed to make a better game than me :D 
