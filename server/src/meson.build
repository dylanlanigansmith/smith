c = run_command('find.sh', check: true)


sources = c.stdout().strip().split('\n') 

if not sources.contains('idontthinkthisisneeded')
    run_command('find.sh', check: false)
 #reminder u have to touch this file to make it build
endif


executable('smith-server', sources, include_directories : [ inc, '../include' ], 
    dependencies : [
        x_dep,
        thread_dep,
        sdl3_dep,
        gl_dep,
        sdl3_img_dep,
        libimgui_dep,
        nlohmann_json_dep,
        magic_enum_dep,
        [enet_lib]
    ],
    cpp_args : [ #'-Og'
    ]

)