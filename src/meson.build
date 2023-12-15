
x_dep = dependency('x11')
gl_dep = dependency('gl')
sdl3_dep = dependency('sdl3')
sdl3_img_dep = dependency('SDL3_image')
thread_dep = dependency('threads')



c = run_command('find.sh', check: true)


sources = c.stdout().strip().split('\n') 

if not sources.contains('idontthinkthisisneeded')
    run_command('find.sh', check: false)
 #reminder u have to touch this file to make it build
endif


executable('smith', sources, include_directories : inc, 
    dependencies : [
        x_dep,
        thread_dep,
        sdl3_dep,
        gl_dep,
        libimgui_dep,
        nlohmann_json_dep,
        magic_enum_dep,
        [enet_lib]
    ],
    cpp_args : [ '-Og','-Wno-unused', '-Wnarrowing', #'-fopt-info-vec-all'
    ]

)

#