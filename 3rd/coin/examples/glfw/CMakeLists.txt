if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DCOIN_DEBUG=1)
else()
  add_definitions(-DCOIN_DEBUG=0)
endif()

find_package(glfw3 3.3 REQUIRED)

add_executable(Glfw glfw.cpp)

target_include_directories(Glfw PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/include/Inventor/annex
	${PROJECT_BINARY_DIR}/include
	${COIN_TARGET_INCLUDE_DIRECTORIES}
)

if(COIN_GL_MODERN)
  target_compile_definitions(Glfw PRIVATE COIN_GL_MODERN=1)
  if(COIN_GL3_CORE)
    target_compile_definitions(Glfw PRIVATE COIN_GL3_CORE=1)
  endif()
  if(COIN_GLES3)
    target_compile_definitions(Glfw PRIVATE COIN_GLES3=1)
  endif()
  if(COIN_GLES2)
    target_compile_definitions(Glfw PRIVATE COIN_GLES2=1)
  endif()
endif()

target_link_libraries(Glfw glfw Coin)
target_link_libraries(Glfw ${COIN_TARGET_LINK_LIBRARIES_GL})
