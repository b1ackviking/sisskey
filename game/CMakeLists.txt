project(game)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

if(UNIX)
	set(SOURCES main.cpp)
elseif(WIN32)
	set(SOURCES WinMain.cpp)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

# target_include_directories(${PROJECT_NAME} PRIVATE ../sisskey)

# find_package(Threads)
# target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} sisskey)
