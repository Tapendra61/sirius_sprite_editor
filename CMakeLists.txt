cmake_minimum_required(VERSION 3.16)
project(sirius_sprite_editor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
    GIT_SHALLOW TRUE
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# Imgui (docking branch — needed for ImGuiConfigFlags_DockingEnable)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# rlimgui
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(rlimgui)

add_library(rlimgui STATIC ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PUBLIC raylib imgui)

# portable-file-dialogs (single header, native open/save dialogs)
# SOURCE_SUBDIR set to a nonexistent path so we skip pfd's stale CMakeLists.
# We just want the header file, not their build config.
FetchContent_Declare(
    pfd
    GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR _none_
)
FetchContent_MakeAvailable(pfd)

# nlohmann/json — for .slicerproj save/load
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# My Binary
add_executable(sprite_editor
    src/main.cpp
    src/app/CanvasOptions.cpp
    src/app/Editor.cpp
    src/app/Keybindings.cpp
    src/app/Project.cpp
    src/app/RecentFiles.cpp
    src/model/SliceStore.cpp
    src/ui/MainMenu.cpp
    src/ui/Toolbar.cpp
    src/ui/Canvas.cpp
    src/ui/Inspector.cpp
    src/ui/SliceList.cpp
    src/ui/StatusBar.cpp
    src/ui/GridSliceModal.cpp
    src/ui/AutoSliceModal.cpp
    src/ui/SliceModalUtil.cpp
    src/ui/ExportModal.cpp
    src/ui/KeybindingsModal.cpp
    src/ui/DocsWindow.cpp
    src/ui/CanvasOptionsPanel.cpp
    src/ui/Palette.cpp
    src/ui/Theme.cpp
    src/ui/CanvasDraw.cpp
    src/ui/CanvasInput.cpp
    src/ops/GridSlicer.cpp
    src/ops/AutoSlicer.cpp
    src/ops/Exporter.cpp
    src/ops/Importer.cpp
    src/ops/Trim.cpp
    src/commands/CommandStack.cpp
    src/commands/SliceCommands.cpp
    src/util/ConfigDir.cpp
    src/util/HitTest.cpp
    src/util/JsonFile.cpp
)
target_include_directories(sprite_editor PRIVATE src ${pfd_SOURCE_DIR})
target_link_libraries(sprite_editor PRIVATE rlimgui nlohmann_json::nlohmann_json)

# Copy `resources/` next to the executable so relative paths work regardless
# of the cwd the binary is launched from (Windows VS / Xcode / running from
# build dir all just work).
add_custom_command(TARGET sprite_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:sprite_editor>/resources"
    COMMENT "Copying resources to executable directory"
    VERBATIM)