cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(GeodeTrafficController VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME} EXTERNALS geode.node-ids:1.23.3)

# --- MinHook: fetch source only, compile directly into our own target ---
CPMAddPackage(
    NAME minhook
    GITHUB_REPOSITORY TsudaKageyu/minhook
    GIT_TAG v1.3.3
    DOWNLOAD_ONLY TRUE
)

target_sources(${PROJECT_NAME} PRIVATE
    ${minhook_SOURCE_DIR}/src/buffer.c
    ${minhook_SOURCE_DIR}/src/hook.c
    ${minhook_SOURCE_DIR}/src/trampoline.c
)

# Точечно, только для этого файла: не даём ему подключить MSVC-интринсик
# __movsb, который наш cross-clang не резолвит при линковке.
set_source_files_properties(
    ${minhook_SOURCE_DIR}/src/trampoline.c
    PROPERTIES COMPILE_OPTIONS "-U_MSC_VER"
)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_sources(${PROJECT_NAME} PRIVATE ${minhook_SOURCE_DIR}/src/hde/hde64.c)
else()
    target_sources(${PROJECT_NAME} PRIVATE ${minhook_SOURCE_DIR}/src/hde/hde32.c)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${minhook_SOURCE_DIR}/include
    ${minhook_SOURCE_DIR}/src
    ${minhook_SOURCE_DIR}/src/hde
)

target_link_libraries(${PROJECT_NAME} ws2_32)

# --- BoringTun (WireGuard engine, prebuilt via cargo-xwin) ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/boringtun/include
)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/boringtun/lib/win64/boringtun.lib
    bcrypt
    ntdll
    userenv
    advapi32
    secur32
)
