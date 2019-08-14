project(sisskey)

set(SOURCES Engine.h Engine.cpp Timer.h Timer.cpp Window.h Window.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if (UNIX)
target_link_libraries(${PROJECT_NAME} xcb xcb-image)
endif()

# find_package(Threads)
# target_link_libraries(${PROJECT_NAME} Threads::Threads)

# find_package(Vulkan)
# target_include_directories(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
# target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
