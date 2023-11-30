project('smith','c','cpp',
    default_options: ['c_std=c11', 
                     'cpp_std=c++20',
                     ],
    version : '0.0.0'
)


inc = include_directories('include', 'thirdparty/imgui', 'thirdparty/json/include', 'thirdparty/magic_enum/include')

subdir('thirdparty/imgui')
subdir('thirdparty/json')
subdir('thirdparty/magic_enum')
subdir('src')
