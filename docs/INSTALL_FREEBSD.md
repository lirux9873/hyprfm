# Installing HyprFM on FreeBSD

This Qt 6 source port is maintained at
[lirux9873/hyprfm](https://github.com/lirux9873/hyprfm). The fork maintainer has
tested it on FreeBSD 15.1 and reports no issues so far. The precise Qt version,
display session and automated-test results have not been recorded; the
validation procedure below remains relevant for release testing.

## Dependencies

Use a supported FreeBSD release and its matching package repository. In a root
shell, install the build tools and required libraries:

```sh
pkg install git cmake ninja pkgconf qt6-base qt6-declarative qt6-svg glib dbus
```

The FreeBSD Qt packages provide Qt Core, GUI, Network, Concurrent, DBus, Test,
QML, Quick, Quick Controls and SVG. Qt's QML imports must be available at runtime.
For a Wayland desktop also install `qt6-wayland`. X11 uses Qt's `xcb` plugin.
An existing working graphical desktop, suitable graphics drivers and fonts are
prerequisites; HyprFM does not install or configure a desktop session.

Optional packages, installed as root:

```sh
pkg install fd bat md4c ffmpeg poppler-utils p5-Image-ExifTool zip unzip 7-zip
pkg install gvfs rclone bash
```

| Package | Use |
| --- | --- |
| `fd` | Faster search; Qt provides a fallback |
| `bat`, `md4c` | Highlighted text and rendered Markdown (`md2html`) |
| `ffmpeg` | Video thumbnails and audio/video metadata (`ffprobe`) |
| `poppler-utils` | PDF thumbnails, previews and metadata |
| `p5-Image-ExifTool` | Image metadata |
| `zip`, `unzip`, `7-zip` | Archives; this port invokes `7zz` |
| `gvfs` | Remote URI access via GIO; enabled protocols depend on port options |
| `rclone` | Cloud remotes; mounting also needs working FUSE and user permissions |
| `bash` | Optional graphical multi-instance integration test |
| `kf6-kwindowsystem` | Optional build-time KWin blur/contrast integration |

FreeBSD's base `tar`, `gzip`, `bzip2` and `xz` are used for supported archive
formats. No installation command is executed by the dependency dialog: it shows
commands to run as root. Package availability and enabled GVFS backends can vary
by release and repository. Check `pkg search` and `pkg info` if a package or helper
is absent. The application itself must run as your desktop user.

## Obtain and build the sources

For a fresh checkout:

```sh
git clone --recurse-submodules https://github.com/lirux9873/hyprfm.git
cd hyprfm
```

Use this fork for the FreeBSD changes. The original project is
[soyeb-jim285/hyprfm](https://github.com/soyeb-jim285/hyprfm).
For an existing checkout, populate the pinned submodules with:

```sh
git submodule update --init --recursive
```

Configure and build as an ordinary user, using an sh-compatible shell:

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/local \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DBUILD_TESTS=ON
cmake --build build --parallel "$(sysctl -n hw.ncpu)"
```

CMake deliberately rejects other target operating systems. It also reports
missing QML submodules before processing the application. For easier compiler
diagnostics, add `-DHYPRFM_ENABLE_UNITY_BUILD=OFF -DHYPRFM_ENABLE_PCH=OFF`.
Release builds enable QML cache generation; disable it only for debugging with
`-DHYPRFM_ENABLE_QML_CACHEGEN=OFF`.

Run `./build/src/hyprfm --help` and `./build/src/hyprfm --version`, then run the
test procedure below. After a successful build and tests, in a root shell:

```sh
cmake --install build
```

The default layout is:

- `/usr/local/bin/hyprfm`
- `/usr/local/share/hyprfm/themes` and `/usr/local/share/hyprfm/HyprFM`
- `/usr/local/share/applications/io.github.soyeb_jim285.HyprFM.desktop`
- `/usr/local/share/icons/hicolor/scalable/apps/io.github.soyeb_jim285.HyprFM.svg`
- `/usr/local/share/metainfo/io.github.soyeb_jim285.HyprFM.metainfo.xml`
- `/usr/local/share/doc/hyprfm` and `/usr/local/share/licenses/hyprfm`

For a user installation, configure with
`-DCMAKE_INSTALL_PREFIX="$HOME/.local"`; install without root and add
`$HOME/.local/bin` to `PATH`. For a staging install use
`DESTDIR="$PWD/stage" cmake --install build`; `DESTDIR` is a packaging root,
not a runtime prefix. There is no packaged FreeBSD release or automatic
uninstall target yet. `build/install_manifest.txt` records installed files.

## Run

Inside your normal X11 or Wayland desktop session:

```sh
hyprfm
hyprfm "$HOME/Documents"
hyprfm --new-window "$HOME"
```

Use your desktop's existing D-Bus session. If it has none, start the application
with `dbus-run-session -- hyprfm`. Qt chooses the platform plugin; explicitly
select one with `QT_QPA_PLATFORM=xcb hyprfm` or
`QT_QPA_PLATFORM=wayland hyprfm` when diagnosing a mixed session.
Set `TERMINAL` to your installed terminal emulator (the default is `kitty`).
Set `VISUAL` or `EDITOR` for text editing actions.

Configuration follows `$XDG_CONFIG_HOME/hyprfm`, normally
`~/.config/hyprfm`. Settings, bookmarks and session state are user-owned.
The application writes commented configuration and theme samples. The README
documents the settings and shortcuts. Trash uses the XDG Trash format, including
per-volume trash directories when supported and writable.

## Devices, remote access and cloud mounts

The sidebar lists already mounted filesystems, including UFS and ZFS mounts,
and polls every five seconds. Mount removable media with your normal FreeBSD
administration tools first, then browse its mount path. Device mounting,
unmounting, ejecting and USB/mobile discovery are not implemented in this port;
installing additional packages does not activate those placeholders.

Remote URLs use the installed GVFS backends and a user D-Bus session. Test the
backend independently with `gio list sftp://user@host/path` before diagnosing
HyprFM. Authentication and protocol availability depend on that backend.

For cloud storage, configure and test a remote using `rclone config` and
`rclone lsd remote:`. HyprFM invokes `rclone mount` and `/sbin/umount` for mounts
it owns under `~/.local/share/hyprfm/mounts`. The administrator must configure
FreeBSD FUSE (`fusefs`), access to `/dev/fuse`, and any user-mount policy required
by the installed rclone build. HyprFM does not change kernel modules, devfs rules
or system policy. Verify a foreground `rclone mount` manually first; a build
without the mount command cannot be used for this feature. Busy or unauthorized
unmounts report failure. After a crash, inspect and unmount a stale mount
externally before reconnecting; HyprFM does not take over an existing mount.

## Validation

Run from the source root on FreeBSD:

```sh
sh scripts/check-freebsd.sh
```

This creates a Debug build without unity/PCH, runs CTest with a temporary home,
offscreen Qt and a private D-Bus session, and removes the temporary test home.
It does not run the graphical multi-instance test. Archive and remote tests may
skip when their tools or backends are missing; review skipped tests as well as
failures. For GUI checks, run in a disposable desktop-user account:

```sh
ctest --test-dir build --output-on-failure -R tst_instance_launch
```

That test requires `bash` and a live display. It verifies process/IPC handoff and
session ownership, but window counts require manual inspection. Mounted-volume
trash tests require an explicitly selected writable scratch mount:

```sh
HYPRFM_TEST_MOUNT=/mnt/hyprfm-test ctest --test-dir build --output-on-failure \
  -R 'tst_(fileoperations|undomanager)'
```

Use a disposable filesystem for this opt-in test. Before release, also check a
Release/cachegen build, staged installation, X11 and Wayland launch, copy/move,
trash/restore, clipboard images, ZIP/7z/tar archives, mounted UFS/ZFS volumes,
remote authentication, and successful and busy rclone unmounts.

If QML imports fail, check submodules and `qt6-declarative`, then rebuild from a
fresh build directory. For graphics failures, try `QSG_RHI_BACKEND=opengl`; the
offscreen software renderer is for tests. Missing previews usually indicate a
missing helper in `PATH`; Settings lists the dependencies.

## References

- [FreeBSD Porter's Handbook: Qt and GNOME dependencies](https://docs.freebsd.org/en/books/porters-handbook/special/)
- [FreeBSD GVFS port and backend options](https://raw.githubusercontent.com/freebsd/freebsd-ports/main/filesystems/gvfs/Makefile)
- [FreeBSD 7-zip port](https://cgit.freebsd.org/ports/tree/archivers/7-zip/Makefile)
- [Rclone mount documentation](https://rclone.org/commands/rclone_mount/)
- [Qt QStorageInfo](https://doc.qt.io/qt-6/qstorageinfo.html)
