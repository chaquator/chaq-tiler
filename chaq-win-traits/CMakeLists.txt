add_executable(chaq_win_traits main.cpp)

set_target_properties(
  chaq_win_traits
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

if(MINGW)
  target_compile_options(chaq_win_traits PRIVATE -flto -municode)
endif()
