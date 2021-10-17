add_executable(
  chaq_tiler WIN32 Desktop.cpp
  Window.cpp
  chaq-tiler.cpp
  Globals.cpp
  config.h
  debug.h
  Desktop.h
  Enums.h
  Rule.h
  Vec.h
  Views.h
  Window.h
  Globals.h
)

set_target_properties(
  chaq_tiler
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

if(MINGW)
  target_compile_options(chaq_tiler PRIVATE -flto -municode)
endif()
