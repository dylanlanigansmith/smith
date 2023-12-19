# SMITH: 
## 2023 meets 1993 in this "3D" game engine

### Description
- full multi-threaded, multi-platform game engine with editor, dynamic lighting, enemy AI with A*, and many more subsystems

-  everything besides the deps. below is done from scratch

- Platforms:
    - Currently building on: 
        - Linux x86/64 with GCC 13.2.1 / OpenGL 1.4
        - MacOS Arm64 with Clang 14.0.3 / Metal 3 
    - Planned Platforms to support:
        - iOS 
            - so turns out Meson's XCode integration was a lie.. but that would have been too good to be true if it actually worked 
            - using ninja for Macos and need to write some scripts for making a .app
            - must research if there is any way to do iOS cpp without remaking this project in Xcode 
            - if i change VSCode's font to Menlo it's basically XCode right? surely.
        - Windows
            - 99.9% it will just build and run for Windows, not interested in this at the moment
        - Emscripten / wASM
            - when i feel like tinkering with something new

- why SDL3?
    - because having minimal documentation and no examples is way more fun. 
        - also it seemed like the smart thing to do rather than learn a library that is about to get a new API
    - it is really good! they are doing a great job! 

### Gallery
![Picture of the first time it built on macOS](https://raw.githubusercontent.com/dylanlanigansmith/smith/main/docs/macos.png "The first time it ran on macOS")
![Bad Example of the lighting](https://raw.githubusercontent.com/dylanlanigansmith/smith/main/docs/newnewlighting.png "A boring example of the lighting, which will likely look completely different in a week")

### Dependencies
- Meson Buildsystem
    - the only C++ buildsystem that never gets in my way, and grows with a project easily 
        - sidenote: Xcode integration is a lie and a pipedream, unfortunately

- SDL3 - crossplatform API for OS-level windowing, audio driver setup, etc. 
- stb_image header - for PNG import to raw pixel data
- stb_vorbis header - for Ogg import to raw audio data
- Dear ImGUI - obviously!! 
- nlohmann/json  - why make things difficult, not my choice for lightweight/simple apps, but is it ever nice to use!
- magic_enum  - where has this been all my life holy! it is actually magic for writing editor and UI code
   

### Install
- Random Notes:
    - On Arch for some SDL stuff: sudo pacman -Sy xorg xorg-fonts  & a restart of X11 


### License/Usage
- MIT license, see LICENSE for details
- the files in /resource are either: randomly found online, or bought from itch.io in the past and not mine to share. acquire them yourself. I only have these resources in this repo for ease of my personal development. 

