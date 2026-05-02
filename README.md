# Sirius Sprite Editor

A Unity-style sprite-sheet slicer built in C++ with [raylib](https://www.raylib.com/) and [Dear ImGui](https://github.com/ocornut/imgui) (docking branch).

Load an image, define rectangles around individual sprites — manually, on a grid, or auto-detected from alpha — then export as JSON, per-slice PNG, or CSV.

## Features

- **Three slicing modes**: manual rectangle drawing, fixed grid (by size or count), and auto-detect by alpha-channel connectivity
- **9-slice border editing** and **per-slice pivots** with 3×3 presets + custom values
- **Undo / redo** for every edit (snapshot-coalesced)
- **Remappable keybindings** with persistence
- **Custom canvas** with checker grid, optional grid snap, and pixel-grid overlay
- **Project files** (`.srsprite`) to save your work
- **Recent files** menu, in-app **Documentation**, **right-click context menus** on slices and the canvas
- Modern dark UI: **DM Sans** for labels, **JetBrains Mono** for values

---

## Prerequisites

You need a C++17 compiler, CMake ≥ 3.16, and Git.

### macOS
- Xcode Command Line Tools: `xcode-select --install`
- CMake: `brew install cmake`

### Windows
- Visual Studio 2022 (Community Edition is fine) with the **Desktop development with C++** workload
- CMake (bundled with VS, or install from [cmake.org](https://cmake.org/download/))
- Git for Windows

### Linux (bonus — should work, untested)
- `sudo apt install build-essential cmake git libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`

All other dependencies (raylib, Dear ImGui, rlImGui, portable-file-dialogs, nlohmann/json) are pulled automatically by CMake's `FetchContent` — no manual setup.

---

## Build

### macOS

```bash
git clone <repo-url> sirius_sprite_editor
cd sirius_sprite_editor
cmake -S . -B build
cmake --build build -j
./build/sprite_editor
```

### Windows

```cmd
git clone <repo-url> sirius_sprite_editor
cd sirius_sprite_editor
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
build\Release\sprite_editor.exe
```

Or open `build\sprite_editor.sln` in Visual Studio after the configure step and build / debug from there.

### Linux

```bash
git clone <repo-url> sirius_sprite_editor
cd sirius_sprite_editor
cmake -S . -B build
cmake --build build -j
./build/sprite_editor
```

The first configure takes a few minutes — it downloads raylib, ImGui, etc. Subsequent builds are fast.

---

## Where files live

The build copies `resources/` (icons, fonts) next to the executable, so you can launch the binary from any working directory.

User settings persist to a per-user config directory:

| Platform | Path |
|---|---|
| Windows | `%APPDATA%\SiriusSpriteEditor\` |
| macOS | `~/Library/Application Support/SiriusSpriteEditor/` |
| Linux | `$XDG_CONFIG_HOME/SiriusSpriteEditor/` (or `~/.config/SiriusSpriteEditor/`) |

Three JSON files live there:
- `keybindings.json` — your custom keyboard shortcuts
- `recent_files.json` — the **File → Open Recent** list
- `canvas_options.json` — checker grid / grid snap / pixel grid preferences

Delete any of them to restore defaults.

---

## Quick keys

| Action | Default |
|---|---|
| Open Project / Save / Save As | Ctrl+O / Ctrl+S / Ctrl+Shift+S |
| Open Image | Ctrl+Shift+I |
| Export | Ctrl+E |
| Grid Slice / Auto Slice | Ctrl+G / Ctrl+Shift+A |
| Undo / Redo | Ctrl+Z / Ctrl+Shift+Z |
| Duplicate / Delete | Ctrl+D / Del |
| Select / Move / Rectangle tool | V / H / R |
| Zoom Fit / 100% / In / Out | F / 1 / = / - |

On macOS, `Ctrl` keybindings work as `Cmd`. Remap any of these in **Edit → Keybindings…**.

For full usage instructions, open **Help → Documentation** in the app.

---

## Project layout

```
src/
  app/         Editor, Project, RecentFiles, Keybindings, CanvasOptions
  commands/    Undo/redo command stack
  model/       Slice + SliceStore
  ops/         GridSlicer, AutoSlicer, Exporter, Importer, Trim
  ui/          Panels, modals, custom widgets, theme/palette
  util/        Coords, HitTest, Geometry, JsonFile, ConfigDir
  main.cpp
resources/
  fonts/       DM Sans + JetBrains Mono
  icons/       Tool + modifier-key icons
docs/
  project_plan/  Original design docs (UI tokens, deep dives)
CMakeLists.txt
```

Built with raylib 6.0, Dear ImGui (docking branch), rlImGui, nlohmann/json 3.11.3, portable-file-dialogs.
