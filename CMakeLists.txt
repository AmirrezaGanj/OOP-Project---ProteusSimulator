cmake_minimum_required(VERSION 3.16)
project(ProteusSimulator)

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Homebrew prefix (handles both Apple Silicon and Intel Mac) ----
# Apple Silicon: /opt/homebrew   Intel: /usr/local
if(APPLE)
    if(EXISTS "/opt/homebrew")
        set(HOMEBREW_PREFIX "/opt/homebrew")
    else()
        set(HOMEBREW_PREFIX "/usr/local")
    endif()

    list(APPEND CMAKE_PREFIX_PATH
        "${HOMEBREW_PREFIX}/opt/sdl2"
        "${HOMEBREW_PREFIX}/opt/sdl2_ttf"
        "${HOMEBREW_PREFIX}/opt/sdl2_image"
    )
endif()

# ---- Find SDL2 packages ----
find_package(SDL2       REQUIRED)
find_package(SDL2_ttf   REQUIRED)
find_package(SDL2_image REQUIRED)

# ---- Gather all source files automatically ----
# Adding a new .cpp file to src/ is picked up on the next cmake run
# with no edits needed here.
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# ---- Executable ----
add_executable(ProteusSimulator ${SOURCES})

# ---- Include directories ----
target_include_directories(ProteusSimulator PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    ${SDL2_INCLUDE_DIRS}
    "${HOMEBREW_PREFIX}/include"
)

# ---- Link libraries ----
target_link_libraries(ProteusSimulator PRIVATE
    ${SDL2_LIBRARIES}
    SDL2_ttf::SDL2_ttf
    SDL2_image::SDL2_image
)

# ---- Copy assets folder next to the built executable ----
# This ensures fonts and saved_projects are found at runtime
# regardless of where cmake puts the binary.
add_custom_command(TARGET ProteusSimulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ProteusSimulator>/assets"
    COMMENT "Copying assets to build directory"
)