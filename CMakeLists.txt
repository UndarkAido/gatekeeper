cmake_minimum_required(VERSION 3.7)
project(gatekeeper)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/discordpp)          # discordpp
add_subdirectory(lib/rest-beast)         # discordpp-rest-beast
add_subdirectory(lib/websocket-simpleweb)    # discordpp-websocket-simpleweb
add_subdirectory(lib/plugin-overload)    # discordpp-plugin-overload
add_subdirectory(lib/plugin-responder)   # discordpp-plugin-responder
add_subdirectory(lib/plugin-ratelimit)   # discordpp-plugin-ratelimit

set(SOURCE_FILES main.cc)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost 1.71.0 REQUIRED system date_time)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601) # Windows 7
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /wd4250 /wd4244")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fno-omit-frame-pointer")

INCLUDE_DIRECTORIES(
        ${Boost_INCLUDE_DIRS}
        ${discordpp_SOURCE_DIR}
        ${discordpp-rest-beast_SOURCE_DIR}
        ${discordpp-websocket-simpleweb_SOURCE_DIR}
        ${discordpp-plugin-overload_SOURCE_DIR}
        ${discordpp-plugin-responder_SOURCE_DIR}
        ${discordpp-plugin-ratelimit_SOURCE_DIR}
        ${json_SOURCE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        ${Boost_LIBRARIES}
        discordpp
        discordpp-rest-beast
        discordpp-websocket-simpleweb
        Threads::Threads
        ${OPENSSL_LIBRARIES}
)

if (EXISTS ${CMAKE_SOURCE_DIR}/token.dat)
    configure_file(token.dat ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
elseif (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/token.dat)
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/token.dat)
endif ()
