find_package(SDL3 REQUIRED CONFIG)

add_executable(glparity-render glparity_render.cpp)

target_include_directories(glparity-render PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/Inventor/annex
  ${PROJECT_BINARY_DIR}/include
  ${COIN_TARGET_INCLUDE_DIRECTORIES}
)

if(COIN_GL_MODERN)
  target_compile_definitions(glparity-render PRIVATE COIN_GL_MODERN=1)
  if(COIN_GL3_CORE)
    target_compile_definitions(glparity-render PRIVATE COIN_GL3_CORE=1)
  endif()
  if(COIN_GLES3)
    target_compile_definitions(glparity-render PRIVATE COIN_GLES3=1)
  endif()
  if(COIN_GLES2)
    target_compile_definitions(glparity-render PRIVATE COIN_GLES2=1)
  endif()
endif()

if(TARGET SDL3::SDL3)
  target_link_libraries(glparity-render PRIVATE SDL3::SDL3)
elseif(TARGET SDL3::SDL3-shared)
  target_link_libraries(glparity-render PRIVATE SDL3::SDL3-shared)
else()
  find_path(SDL3_INCLUDE_DIR SDL3/SDL.h)
  find_library(SDL3_LIBRARY NAMES SDL3)
  target_include_directories(glparity-render PRIVATE ${SDL3_INCLUDE_DIR})
  target_link_libraries(glparity-render PRIVATE ${SDL3_LIBRARY})
endif()

target_link_libraries(glparity-render PRIVATE Coin ${COIN_TARGET_LINK_LIBRARIES_GL})
