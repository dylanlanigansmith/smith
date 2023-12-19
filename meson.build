project('smith','c','cpp',
    default_options: ['c_std=c11', 
                     'cpp_std=c++20',
                     ],
    version : '0.0.0'
)
#enet_lib =  meson.get_compiler('c').find_library('enet', dirs : ['/home/dylan/code/smith/thirdparty/enet/.libs/'])
 

inc = include_directories(
    'include', 
    'thirdparty/imgui', 
    'thirdparty/json/include', 
    'thirdparty/magic_enum/include', 
    #'thirdparty/enet/include', 
    'thirdparty/glm/',
    'thirdparty/stb/'
    )

subdir('thirdparty/imgui')
subdir('thirdparty/json')
subdir('thirdparty/magic_enum')
subdir('src')
