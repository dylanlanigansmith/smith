
sdl3_dep = dependency('sdl3')


libimgui = static_library(
  'imgui',
  files('imgui.cpp', 'imgui_draw.cpp', 'imgui_widgets.cpp', 'imgui_tables.cpp', 'imgui_demo.cpp', 'imgui_impl_sdlrenderer3.cpp', 'imgui_impl_sdl3.cpp', 'imgui_stdlib.cpp'), #,'imgui_impl_sdl3.cpp'
  cpp_args : ['-w','-O2'],
  install : false,
  dependencies : [sdl3_dep]
)

libimgui_dep = declare_dependency(
  link_with : libimgui,
  include_directories : include_directories('.')
)
