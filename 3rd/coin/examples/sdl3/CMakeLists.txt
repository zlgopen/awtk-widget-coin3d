if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DCOIN_DEBUG=1)
else()
  add_definitions(-DCOIN_DEBUG=0)
endif()

find_package(SDL3 REQUIRED CONFIG)

add_executable(sdl3-example sdl3.cpp)

target_include_directories(sdl3-example PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/include/Inventor/annex
	${PROJECT_BINARY_DIR}/include
	${COIN_TARGET_INCLUDE_DIRECTORIES}
)

target_compile_definitions(sdl3-example PRIVATE
	COIN_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# Match Coin's compiled GL profile so the example requests a compatible context.
if(COIN_GL_MODERN)
  target_compile_definitions(sdl3-example PRIVATE COIN_GL_MODERN=1)
  if(COIN_GL3_CORE)
    target_compile_definitions(sdl3-example PRIVATE COIN_GL3_CORE=1)
  endif()
  if(COIN_GLES3)
    target_compile_definitions(sdl3-example PRIVATE COIN_GLES3=1)
  endif()
  if(COIN_GLES2)
    target_compile_definitions(sdl3-example PRIVATE COIN_GLES2=1)
  endif()
endif()

# Prefer the imported CMake target so include dirs propagate.
if(TARGET SDL3::SDL3)
  target_link_libraries(sdl3-example PRIVATE SDL3::SDL3)
elseif(TARGET SDL3::SDL3-shared)
  target_link_libraries(sdl3-example PRIVATE SDL3::SDL3-shared)
else()
  find_path(SDL3_INCLUDE_DIR SDL3/SDL.h)
  find_library(SDL3_LIBRARY NAMES SDL3)
  target_include_directories(sdl3-example PRIVATE ${SDL3_INCLUDE_DIR})
  target_link_libraries(sdl3-example PRIVATE ${SDL3_LIBRARY})
endif()

target_link_libraries(sdl3-example PRIVATE Coin ${COIN_TARGET_LINK_LIBRARIES_GL})
