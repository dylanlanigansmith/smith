


c = run_command('find.sh', check: true)


sources = c.stdout().strip().split('\n') 

if not sources.contains('idontthinkthisisneeded')
    run_command('find.sh', check: false)
 #reminder u have to touch this file to make it build
endif


if host_machine.system() == 'linux'

x_dep = dependency('x11')
gl_dep = dependency('gl')
sdl3_dep = dependency('sdl3')
thread_dep = dependency('threads')

executable('smith', [sources, '../thirdparty/stb/stb_vorbis.c'], include_directories : inc, 
    dependencies : [
        x_dep,
        thread_dep,
        sdl3_dep,
        gl_dep,
        libimgui_dep,
        nlohmann_json_dep,
        magic_enum_dep,
       # [enet_lib]
    ],
    cpp_args : [ '-Og',
    '-Wno-unused', 
    '-Wno-narrowing',
    '-Wno-ignored-attributes',
    '-Wno-attributes',
    '-Wno-format', 
    '-Wno-overloaded-virtual',
    '-Wno-sign-compare',
     #'-fopt-info-vec-all'
    ]

)
elif host_machine.system() == 'darwin'



#gl_dep = dependency('gl')
sdl3_dep = dependency('sdl3')
thread_dep = dependency('threads')

    executable('smith', [sources, '../thirdparty/stb/stb_vorbis.c'], include_directories : inc, 
    dependencies : [
        thread_dep,
        sdl3_dep,
        libimgui_dep,
        nlohmann_json_dep,
        magic_enum_dep,
       # [enet_lib]
    ],
    cpp_args : [ '-Og','-Wno-unused', '-Wno-narrowing', '-fpermissive' #bad
    ]
    )
endif
#