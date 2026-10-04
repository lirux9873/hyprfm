# HyprFM for FreeBSD

A keyboard-friendly Qt 6/QML file manager for FreeBSD X11 and Wayland desktops.
This working tree contains an uncommitted source port. It has not yet been
compiled or run on FreeBSD; follow the validation guide before deployment.

![HyprFM grid view](docs/screenshots/grid-view.png)

## Documentation

- [FreeBSD installation, dependencies, build and tests](docs/INSTALL_FREEBSD.md)
- [Port architecture, changes and validation status](docs/FREEBSD_PORT.md)
- [Unsupported functions and future work](docs/FUTURE.md)

## Features

- Grid, detailed and Miller column views, tabs and split panes
- Keyboard navigation, configurable shortcuts, bookmarks and search
- GIO copy/move with progress, trash/restore, bulk rename and undo
- Archive creation/extraction and application associations
- Image, text, Markdown, video and PDF previews with optional helper packages
- Qt clipboard support, drag/drop, Git status overlays and TOML themes
- Mounted-filesystem browsing through Qt, including FreeBSD UFS and ZFS
- Optional remote URI access via GVFS and cloud mounts via rclone/FUSE

Device mount/unmount, unmounted/mobile-device discovery, wallpaper changes and
compositor rounding/border controls are placeholders. See the future-work list
for the exact behavior and implementation locations. Historical screenshots
may show integrations unavailable in this port.

## Quick build

On FreeBSD, install the prerequisites as root:

```sh
pkg install git cmake ninja pkgconf qt6-base qt6-declarative qt6-svg glib dbus
```

From this modified source tree, as your desktop user:

```sh
git submodule update --init --recursive
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/local -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_TESTS=ON
cmake --build build --parallel "$(sysctl -n hw.ncpu)"
```

See the installation guide for optional tools, test isolation and installation.
After validation, run `cmake --install build` as root. Launch `hyprfm` as an
ordinary desktop user. A fresh upstream clone does not contain these unpublished
FreeBSD changes.

## ⌨ Keyboard shortcuts

### Navigation

| Shortcut | Action |
|----------|--------|
| `Return` / `Double-click` | Open file or directory |
| `Backspace` / `Alt+Up` | Parent directory |
| `Alt+Left` / `Alt+Right` | Back / Forward in history |
| `Alt+Home` | Home directory |
| `Ctrl+L` | Focus path bar |
| `Ctrl+F` | Search |
| `F5` | Refresh |
| `Ctrl+Return` | Open in a new tab |
| `Ctrl+Shift+Return` | Open in the split pane |
| `Type any letter` | Type-ahead jump to file |

### Views

| Shortcut | Action |
|----------|--------|
| `Ctrl+1` | Grid view |
| `Ctrl+2` | Miller column view |
| `Ctrl+3` | Detailed view |
| `Ctrl+Scroll` | Zoom (icon size or row height); also Settings → Layout → Icon Size |
| `Space` | Quick preview |
| `F3` | Toggle split pane |
| `F9` | Toggle sidebar |
| `Ctrl+H` | Toggle hidden files |
| `Ctrl+Shift+B` | Toggle transparency |
| `F6` / `Shift+F6` | Focus next / previous pane |
| `Ctrl+Alt+Left` / `Ctrl+Alt+Right` | Focus left / right pane |
| `Ctrl+,` | Settings |
| `Ctrl+Shift+,` | Open `config.toml` in your editor |
| `Ctrl+?` | Keyboard shortcut reference |

### Tabs & windows

| Shortcut | Action |
|----------|--------|
| `Ctrl+T` | New tab |
| `Ctrl+W` | Close tab |
| `Ctrl+Shift+T` | Reopen closed tab |
| `Ctrl+Tab` / `Ctrl+Shift+Tab` | Next / previous tab |
| `Ctrl+PgDown` / `Ctrl+PgUp` | Next / previous tab |
| `Alt+1` … `Alt+8` | Jump to tab 1-8 |
| `Alt+9` | Jump to the last tab |
| `Ctrl+Alt+N` | New window |

Launching `hyprfm` while it is already running opens another independent
window. The one exception is `hyprfm <path>`, which forwards the path to the
running window as a new tab, so desktop launchers and `xdg-open` keep behaving
as expected. Pass `--new-window` (or `-n`) to get a separate window for a path
too.

Only the first window keeps the saved session (tabs + window geometry);
additional windows start fresh and leave it untouched.

Run `hyprfm --help` for the full list of flags and environment variables.

### File operations

| Shortcut | Action |
|----------|--------|
| `Ctrl+C` / `Ctrl+X` / `Ctrl+V` | Copy / Cut / Paste |
| `Ctrl+A` | Select all |
| `Ctrl+Z` / `Ctrl+Shift+Z` | Undo / Redo |
| `F2` | Rename |
| `Delete` | Move to trash |
| `Shift+Delete` | Permanent delete |
| `Ctrl+Shift+N` | New folder |
| `Ctrl+N` | New file |
| `Alt+Return` | Properties |
| `Ctrl+Alt+T` | Open terminal here |
| `Shift+F10` | Context menu |

Shortcuts can be remapped in `~/.config/hyprfm/config.toml` under the `[shortcuts]` section (see the generated `config.toml.sample` for the full key list). Fixed: `Backspace`, `Alt+1`…`Alt+9`, `Ctrl+PgUp`/`Ctrl+PgDown`, `Ctrl+Scroll`, `Escape`, `Menu`.

---

## ⚙ Configuration

Config lives at `~/.config/hyprfm/config.toml`. On first run HyprFM writes it fully commented; changing settings inside the app rewrites the file without comments, so `~/.config/hyprfm/config.toml.sample` (regenerated on every start) is the always-documented reference.

```toml
[general]
# theme = "catppuccin-mocha"   # active theme; filename in themes/ without .toml
light_theme = "catppuccin-latte"  # the Dark Mode switch in Settings flips
dark_theme = "catppuccin-mocha"   # between these two
icon_theme = "Adwaita"         # system icon theme fallback
font_family = ""               # UI font; empty = desktop font
default_view = "grid"          # grid | detailed | miller
show_hidden = false
dependency_startup_check = true # warn on startup when a required tool is missing
sort_by = "name"               # name | size | modified | type
sort_ascending = true
remember_sort_per_folder = true

[sidebar]
position = "left"
width = 200
visible = true
# Quick-access entries to hide. Valid names:
# "Home", "Recents", "Trash", "Network", "Pictures", "Downloads"
hidden_quick_access = []

[appearance]
radius_small = 4
radius_medium = 8
radius_large = 12
transparency_enabled = true    # needs compositor blur rules to look good
transparency_level = 1.0       # 0.0 transparent .. 1.0 opaque
animations_enabled = true
anim_duration_fast = 100       # ms
anim_duration = 200
anim_duration_slow = 350
anim_curve_enter = "OutCubic"  # Qt easing name, or "Bezier"
anim_curve_exit = "InCubic"
anim_curve_transition = "Bezier"

[window]
# show_controls = false        # unset = only when the compositor draws no decorations
button_layout = ":minimize,maximize,close"   # ":" splits left from right side

[list_view]
# Columns in the detailed view, in display order ("name" is always first).
# Right-click the header to toggle columns, drag headers to reorder, drag a
# header's right edge to resize. Available: size, modified, type, permissions,
# owner, group, created, accessed, extension, mime, git, symlink
columns = ["name", "size", "modified", "type"]
column_widths = { size = 110, modified = 140, type = 80 }

[miller_view]
# Column widths as fractions of the view; the preview column takes the rest.
# Drag the lines between columns to change them (each keeps at least 12%).
parent_fraction = 0.2
current_fraction = 0.5

[bookmarks]
# paths = ["~/Documents", "~/Downloads", "~/Pictures", "~/Projects"]   # unset = XDG user folders
names = { "~/Projects" = "Work" }   # optional display names (right-click → Rename)

[[context_menu.actions]]          # extra right-click entries; %f = path, runs per item
name = "Optimize PNG"
command = "oxipng -o 4 %f"
types = ["png"]                     # "*", "dir", extension, or MIME ("image/*")
shortcut = "Ctrl+E"                 # optional; runs the action on the selection

[shortcuts]
# Override any shortcut. Examples:
# rename       = "F2"
# new_tab      = "Ctrl+T"
# miller_view  = "Ctrl+2"
```

---

## 🎨 Theming

Themes are plain TOML files. Nothing is hardcoded in the binary. Bundled themes
ship in `/usr/local/share/hyprfm/themes/*.toml` — `catppuccin-mocha`,
`catppuccin-latte`, `rose-pine`, `rose-pine-moon` and `rose-pine-dawn`. Copy one
as a starting point:

```sh
cp /usr/local/share/hyprfm/themes/catppuccin-mocha.toml ~/.config/hyprfm/themes/mytheme.toml
```

`~/.config/hyprfm/themes/` is created on first run and searched first, so a file
there shadows a bundled theme of the same name. Every `*.toml` in either
directory appears in the theme picker. Select it there, or set it in config:

```toml
[general]
theme = "mytheme"
```

A theme is just a colour table, and any key you omit falls back to the default:

```toml
[colors]
base    = "#1e1e2e"
mantle  = "#181825"
crust   = "#11111b"
surface = "#313244"
overlay = "#45475a"
text    = "#cdd6f4"
subtext = "#bac2de"
muted   = "#6c7086"
accent  = "#89b4fa"
success = "#a6e3a1"
warning = "#f9e2af"
error   = "#f38ba8"
```

`~/.config/hyprfm/themes/example.toml.sample` is rewritten on every start with
the same table plus a comment per colour, so the directory documents itself.

Themes reload live on save.

### Light and dark

Name two themes as a pair and the Dark Mode switch in Settings flips between
them:

```toml
[general]
light_theme = "rose-pine-dawn"
dark_theme = "rose-pine"
```

Both are dropdowns under Settings, so you can set them there instead. `theme`
is whichever one is currently in effect.

HyprFM does not watch your desktop for light/dark changes. If you want it to
follow a system-wide toggle, have that toggle rewrite `theme` in
`config.toml`: the file is watched and the new theme applies immediately, with
no restart and no need for HyprFM to be running at the time.

```sh
sed -i '' 's/^theme = .*/theme = "rose-pine-dawn"/' ~/.config/hyprfm/config.toml
```

The only time the desktop is consulted is the very first launch, when there is
no `theme` yet: HyprFM asks the XDG desktop portal whether you prefer light or
dark so the initial theme matches rather than always starting dark.

---

## Architecture and development

QML provides the frontend; C++ models, services and providers supply the
backend. Qt and GIO provide the desktop and filesystem integration. The Quill
and icon libraries are pinned submodules. See the port notes for details.

Run `sh scripts/check-freebsd.sh` on FreeBSD for an isolated offscreen test run.
Use a disposable desktop account for graphical tests and explicitly set
`HYPRFM_TEST_MOUNT` for scratch-volume tests. Match the existing four-space
C++/QML style. Publishing workflows are absent from this port.

## License

[MIT](LICENSE). Original project by Soyeb Pervez Jim; upstream library licenses
remain in their source trees.
