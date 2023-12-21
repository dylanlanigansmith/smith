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

### Goals
- Multi-Platform support [X]
- Fully Editable, no hardcoding 
    - I don't know if I will get to the scripting language part
        - for now goal is: defining some entity behaviour lambdas is the only coding needed to make a game 
    - Lighting, Map, Entities are all currently editable in the GUI
- Even better lighting
    - enough to make up for the 1993 renderer
    - easy to implement once I find a lighting model that can be adapted for this atypical renderer
- Fast [x]
    - I had an issue with the frame rate going over 1000fps and exceeding my mouse polling rate
    - continuing to design around multi-threading will be cruical as the engine gains complexity
- Smart AI with editable behaviour 
    - so much fun to do, so hard to do without getting messy!
- To actually make a game with it! 
    - ignore the occasional urge to rewrite this beast with polygons and more "1997" style
    - make a short story driven FPS 
    - find an art style I can manage to pull off
- After all these: 
    - simple P2P network shooter game
        - then I can finally start Smith2 with polygons lol.


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
   

### Install/Building
- Random Notes:
    - On Arch for some SDL stuff: sudo pacman -Sy xorg xorg-fonts  & a restart of X11 

```
git clone <url>
cd smith

git submodule init
git submodule update

meson setup builddir
ninja -C builddir

/* Running Manually for Development */
ln -S resource builddir/src/resource  //or configure dev resource path in global header
cd builddir/src
./smith {example launch options: -w1920 -h1080 -full }

/*VSCODE*/
- install Meson extension, C++ tools,
- vscode workspace in project is configured for Linux
- Ctrl+Shift+B, build all, Ctrl+F5 to launch with gdb
```

##### Standard Controls
- WASD + Mouse as you'd expect
- Number keys and R for weapon switch/reload
- F10 quit, F11 fullscreen
- Backslash for Dev. UI and Editor
- Esc unlocks/locks mouse
- Ctrl for noclip/godmode




### License/Usage
- MIT license, see LICENSE for details
- the files in /resource are either: randomly found online, or bought from itch.io in the past and not mine to share. acquire them yourself. I only have these resources in this repo for ease of my personal development. 

