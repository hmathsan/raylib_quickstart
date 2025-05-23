cmake_minimum_required(VERSION 3.31...4.0.2)
project(vs_modmanager)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Create include and lib directories in the project root
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

file(MAKE_DIRECTORY ${INCLUDE_DIR})
file(MAKE_DIRECTORY ${LIB_DIR})

# Include FetchContent module for dependency management
include(FetchContent)

# =============================================================================
# DEPENDENCY CONFIGURATION FLAGS
# Set these to ON/OFF to enable/disable dependencies
# =============================================================================
option(USE_RAYLIB "Enable Raylib graphics library" ON)
option(USE_IMGUI "Enable ImGui library" ON)
option(USE_RLIMGUI "Enable RlImGui (ImGui + Raylib integration)" ON)

# =============================================================================
# RAYLIB DEPENDENCY
# =============================================================================
if(USE_RAYLIB)
    message(STATUS "=== Configuring Raylib ===")

    set(RAYLIB_VERSION master)
    FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG ${RAYLIB_VERSION}
            GIT_SHALLOW TRUE
    )

    # Set raylib build options before fetching
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # Force static library

    # Make raylib available
    FetchContent_MakeAvailable(raylib)

    # Get raylib source directory for header files
    FetchContent_GetProperties(raylib SOURCE_DIR RAYLIB_SOURCE_DIR)

    # Create custom target to copy raylib files
            add_custom_target(copy_raylib_files ALL
            DEPENDS raylib
            COMMENT "Copying raylib library and headers to project directories"
    )

    # Copy raylib static library
    add_custom_command(TARGET copy_raylib_files POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:raylib>
            ${LIB_DIR}/
            COMMENT "Copying raylib static library to ${LIB_DIR}/"
    )

    # Copy only necessary raylib headers
    set(RAYLIB_HEADERS
            raylib.h
            raymath.h
            rlgl.h
    )

    foreach(HEADER ${RAYLIB_HEADERS})
        add_custom_command(TARGET copy_raylib_files POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${RAYLIB_SOURCE_DIR}/src/${HEADER}
                ${INCLUDE_DIR}/raylib/${HEADER}
                COMMENT "Copying ${HEADER} to ${INCLUDE_DIR}/raylib/"
        )
    endforeach()

    # Store raylib info for linking later
    set(RAYLIB_LIBRARIES raylib)
    set(RAYLIB_COPY_TARGET copy_raylib_files)

    message(STATUS "Raylib ${RAYLIB_VERSION} configured successfully")
endif()

# =============================================================================
# IMGUI DEPENDENCY
# =============================================================================
if(USE_IMGUI)
    message(STATUS "=== Configuring ImGui===")

    set(IMGUI_VERSION "v1.91.9b")
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG ${IMGUI_VERSION}
            GIT_SHALLOW TRUE
    )

    # Make imgui available
    FetchContent_MakeAvailable(imgui)

    # Get imgui source directory for header files
    FetchContent_GetProperties(imgui SOURCE_DIR IMGUI_SOURCE_DIR)

    # Define ImGui source files
    set(IMGUI_SOURCES
            ${IMGUI_SOURCE_DIR}/imgui.cpp
            ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
            ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
            ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
            ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    )

    # Define ImGui header files to copy
    set(IMGUI_HEADERS
            imgui.h
            imgui_internal.h
            imstb_rectpack.h
            imstb_textedit.h
            imstb_truetype.h
            imconfig.h
    )

    # Create ImGui static library
    add_library(imgui_lib STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui_lib PUBLIC ${IMGUI_SOURCE_DIR})

    # Create custom target to copy ImGui files
    add_custom_target(copy_imgui_files ALL
            DEPENDS imgui_lib
            COMMENT "Copying ImGui library and headers to project directories"
    )

    # Copy ImGui static library
    add_custom_command(TARGET copy_imgui_files POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:imgui_lib>
            ${LIB_DIR}/
            COMMENT "Copying ImGui static library to ${LIB_DIR}/"
    )

    # Copy ImGui headers
    foreach(HEADER ${IMGUI_HEADERS})
        add_custom_command(TARGET copy_imgui_files POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${IMGUI_SOURCE_DIR}/${HEADER}
                ${INCLUDE_DIR}/imgui/${HEADER}
                COMMENT "Copying ${HEADER} to ${INCLUDE_DIR}/imgui/"
        )
    endforeach()

    # Copy backends folder (optional, but useful for integration)
    add_custom_command(TARGET copy_imgui_files POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${IMGUI_SOURCE_DIR}/backends
            ${INCLUDE_DIR}/imgui/backends
            COMMENT "Copying ImGui backends to ${INCLUDE_DIR}/imgui/backends/"
    )

    # Store ImGui info for linking later
    set(IMGUI_LIBRARIES imgui_lib)
    set(IMGUI_COPY_TARGET copy_imgui_files)

    message(STATUS "Dear ImGui ${IMGUI_VERSION} configured successfully")
endif ()

# =============================================================================
# RLIMGUI DEPENDENCY (ImGui + Raylib Integration)
# =============================================================================
if(USE_RLIMGUI)
    message(STATUS "=== Configuring RlImGui ===")

    # RlImGui requires both Raylib and ImGui
    if(NOT USE_RAYLIB)
        message(FATAL_ERROR "RlImGui requires Raylib to be enabled. Set USE_RAYLIB=ON")
    endif()
    if(NOT USE_IMGUI)
        message(FATAL_ERROR "RlImGui requires ImGui to be enabled. Set USE_IMGUI=ON")
    endif()

    set(RLIMGUI_VERSION main)  # Use master branch as it's actively maintained
    FetchContent_Declare(
            rlimgui
            GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
            GIT_TAG ${RLIMGUI_VERSION}
            GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(rlimgui)

    # Get RlImGui source directory
    FetchContent_GetProperties(rlimgui SOURCE_DIR RLIMGUI_SOURCE_DIR)

    # Define RlImGui source files
    set(RLIMGUI_SOURCES
            ${RLIMGUI_SOURCE_DIR}/rlImGui.cpp
    )

    # Define RlImGui header files to copy
    set(RLIMGUI_HEADERS
            rlImGui.h
    )

    # Create RlImGui static library
    add_library(rlimgui_lib STATIC ${RLIMGUI_SOURCES})

    # Ensure raylib and imgui are built before rlimgui
    add_dependencies(rlimgui_lib raylib imgui_lib)

    # RlImGui needs to include both Raylib and ImGui headers during compilation
    target_include_directories(rlimgui_lib PRIVATE
            ${RAYLIB_SOURCE_DIR}/src
            ${IMGUI_SOURCE_DIR}
    )

    # Also make headers available to consumers of this library
    target_include_directories(rlimgui_lib PUBLIC
            ${RLIMGUI_SOURCE_DIR}
    )

    # Link RlImGui with its dependencies
    target_link_libraries(rlimgui_lib PUBLIC raylib imgui_lib)

    # Create custom target to copy RlImGui files
    add_custom_target(copy_rlimgui_files ALL
            DEPENDS rlimgui_lib
            COMMENT "Copying RlImGui library and headers to project directories"
    )

    # Copy RlImGui static library
    add_custom_command(TARGET copy_rlimgui_files POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:rlimgui_lib>
            ${LIB_DIR}/
            COMMENT "Copying RlImGui static library to ${LIB_DIR}/"
    )

    # Copy RlImGui headers
    foreach(HEADER ${RLIMGUI_HEADERS})
        add_custom_command(TARGET copy_rlimgui_files POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${RLIMGUI_SOURCE_DIR}/${HEADER}
                ${INCLUDE_DIR}/rlimgui/${HEADER}
                COMMENT "Copying ${HEADER} to ${INCLUDE_DIR}/rlimgui/"
        )
    endforeach()

    # Copy examples for reference (optional)
    add_custom_command(TARGET copy_rlimgui_files POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${RLIMGUI_SOURCE_DIR}/examples
            ${INCLUDE_DIR}/rlimgui/examples
            COMMENT "Copying RlImGui examples to ${INCLUDE_DIR}/rlimgui/examples/"
    )

    # Store RlImGui info for linking later
    set(RLIMGUI_LIBRARIES rlimgui_lib)
    set(RLIMGUI_COPY_TARGET copy_rlimgui_files)

    message(STATUS "RlImGui configured successfully")
endif()

# =============================================================================
# MAIN APPLICATION
# =============================================================================

# Your main application source files
set(SOURCES
        src/main.cpp
        # Add more source files here as needed
)

# Create the main executable
add_executable(${PROJECT_NAME} ${SOURCES})

# =============================================================================
# DEPENDENCY LINKING
# =============================================================================

# Link libraries based on enabled dependencies
if(USE_RAYLIB)
    target_link_libraries(${PROJECT_NAME} ${RAYLIB_LIBRARIES})
    add_dependencies(${PROJECT_NAME} ${RAYLIB_COPY_TARGET})

    # Platform-specific libraries for raylib
    if(WIN32)
        target_link_libraries(${PROJECT_NAME} winmm)
    endif()

    if(APPLE)
        target_link_libraries(${PROJECT_NAME} "-framework IOKit")
        target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
        target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
    endif()

    if(UNIX AND NOT APPLE)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

        target_link_libraries(${PROJECT_NAME}
                ${CMAKE_DL_LIBS}
                pthread
                GL
                m
                X11
                Xrandr
                Xinerama
                Xi
                Xcursor
        )
    endif()
endif()

if(USE_IMGUI)
    target_link_libraries(${PROJECT_NAME} ${IMGUI_LIBRARIES})
    add_dependencies(${PROJECT_NAME} ${IMGUI_COPY_TARGET})
endif()

if(USE_RLIMGUI)
    target_link_libraries(${PROJECT_NAME} ${RLIMGUI_LIBRARIES})
    add_dependencies(${PROJECT_NAME} ${RLIMGUI_COPY_TARGET})
endif()

# =============================================================================
# CLEANUP TARGET
# =============================================================================
add_custom_target(clean_copied_files
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${INCLUDE_DIR}/raylib
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${INCLUDE_DIR}/imgui
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${INCLUDE_DIR}/rlimgui
        COMMAND ${CMAKE_COMMAND} -E remove -f ${LIB_DIR}/libraylib.a ${LIB_DIR}/raylib.lib
        COMMAND ${CMAKE_COMMAND} -E remove -f ${LIB_DIR}/libimgui_lib.a ${LIB_DIR}/imgui_lib.lib
        COMMAND ${CMAKE_COMMAND} -E remove -f ${LIB_DIR}/librlimgui_lib.a ${LIB_DIR}/rlimgui_lib.lib
        COMMENT "Removing all copied dependency files"
)

# =============================================================================
# BUILD INFORMATION
# =============================================================================
message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Dependencies:")
if(USE_RAYLIB)
    message(STATUS "  ✓ Raylib ${RAYLIB_VERSION}")
endif()
if(USE_IMGUI)
    message(STATUS "  ✓ Dear ImGui ${IMGUI_VERSION}")
endif()
if(USE_RLIMGUI)
    message(STATUS "  ✓ RlImGui (Raylib + ImGui integration)")
endif()
message(STATUS "Static libraries: ${LIB_DIR}/")
message(STATUS "Headers: ${INCLUDE_DIR}/")
message(STATUS "=============================")