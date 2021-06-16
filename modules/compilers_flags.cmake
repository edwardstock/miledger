set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY lib)


if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2 /MP")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DEBUG /Od") # /bigobj
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	string(REGEX REPLACE "\\/W3" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	add_compile_options(/wd4068)
	add_compile_options(/wd4834)
	add_compile_options(/wd4099)

	add_definitions(-DNOMINMAX=1)

	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

elseif (MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-unknown-pragmas -Wno-shift-count-overflow -Wno-unused-function")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
elseif (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-unused-parameter -Wno-shift-count-overflow -Wno-unused-function -mmacosx-version-min=10.14")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wextra -pedantic -Wno-attributes")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-unused-parameter -Wno-shift-count-overflow -Wno-unused-function")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -pedantic -Wno-attributes")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif ()

if (APPLE)
	#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-install_name,@executable_path/../Frameworks/")
endif ()

