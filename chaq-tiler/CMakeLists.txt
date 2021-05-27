set(src_files
	Desktop.cpp
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
	Globals.h)

add_executable(chaq_tiler WIN32 ${src_files})
