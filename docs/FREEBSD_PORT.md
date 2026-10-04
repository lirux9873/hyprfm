# FreeBSD port notes

## Scope and status

The application now targets FreeBSD. All changes are uncommitted. No release,
package or deployment has been produced. Source/static checks were performed
on Windows; native compilation, Qt tests and desktop behavior remain unverified.
See [installation and validation](INSTALL_FREEBSD.md) and the
[future-work list](FUTURE.md) before using this as a release candidate.

## Changes by subsystem

| Area | Implementation |
| --- | --- |
| Build/install | FreeBSD target check; `/usr/local` default; GNUInstallDirs for data/binary/docs/license; early missing-submodule diagnostic; installed runtime data path |
| Startup/display | Removed the Wayland-only rejection. Qt selects X11 or Wayland; explicit platform environment settings are respected. CLI help/version remain usable without a display. |
| Storage sidebar | Replaced the platform-specific device service with `QStorageInfo::mountedVolumes()` and a root fallback. Filters devfs/procfs/fdescfs, deduplicates mount paths, bounds space/usage values, and polls every five seconds. |
| Device actions | Preserved the QML-facing model roles and action signals. Backend role is `freebsd`; `deviceMountAvailable=false` gates UI hints. Privileged actions have explicit placeholders. |
| Application launching | Uses Qt desktop services and ordinary `gio` execution; removed sandbox host-process forwarding and host path rewriting. |
| Trash | GIO moves files to trash. The existing direct XDG trash reader/restore logic remains, respecting the configured data location and per-volume trash. |
| Clipboard | Qt handles files, text, copied paths and image data. Raw image MIME payloads are decoded and saved as PNG; no external clipboard utility is required. |
| Archives | Uses FreeBSD's `7zz` command for 7-Zip/password support; retains base tar/compression and optional zip/unzip helpers. |
| Cloud | Retains rclone mount; uses `/sbin/umount`, propagates unmount failures, validates remote path components, refuses existing mounts, and unmounts before terminating its worker during normal operation. |
| Dependencies | Replaced distribution detection and foreign package-manager commands with FreeBSD package hints. Missing optional integrations do not count as required failures. |
| Desktop effects | Removed compositor CLI calls; wallpaper reports unsupported, rounding/border hooks are no-ops. Optional KWindowSystem integration is retained. |
| Allocator | Removed the foreign allocator trim call/header; retained an explicit page-reclamation placeholder after Qt garbage collection. |
| Tests | Updated device expectations, added placeholder/dependency tests, and changed cross-volume trash tests to an explicit scratch mount. Instance checks no longer depend on a compositor CLI. Added an isolated FreeBSD test script. |
| Distribution | Removed the previous operating-system packaging manifests, release jobs and related scripts. Retained generic desktop/AppStream metadata and the demo-file generator. Documentation is no longer ignored by Git. |

The small obsolete platform-detection macro and comment were removed from the
vendored TOML header; parser behavior and license remain unchanged.

## Architecture retained

`src/qml` contains the views, settings and dialogs. Models in `src/models`
expose file, tab, bookmark, device, search and recent-file data. Services handle
configuration, transfers, undo, search, metadata, previews and remote access.
Providers generate icons and thumbnails. `main.cpp` wires these objects into
QML contexts and handles single-instance communication with a per-user local
socket. POSIX calls such as `lstat`, directory enumeration, user IDs and process
signals remain appropriate for FreeBSD.

The Quill and icon libraries remain pinned upstream Git submodules. Only the
QML components listed in `src/CMakeLists.txt` are built; the upstream Quill
Showcase application and its compositor-specific dependencies are not part of
HyprFM. Upstream examples and documentation are not ported or installed.

No administrative policy is changed by the program. System services, removable
mounts and FUSE access are configured outside HyprFM. Remote URI access depends
on the FreeBSD GVFS package's enabled backends, not on the device sidebar.

## Validation record

The pinned QML submodules were initialized without changing their revisions.
Static review covers obsolete platform references, QML/C++ property names,
build source paths, XML metadata, shell syntax and whitespace. These checks do
not establish ABI compatibility or runtime correctness. No FreeBSD compiler or
Qt development tools were available on the editing host; no C++/QML test-pass
claim is made. Run the commands and manual checks in INSTALL_FREEBSD.md on a
FreeBSD desktop before release.

Completed static checks: 111 QML and 29 C++ application source paths exist;
local documentation links resolve; AppStream XML parses; maintained source has
no obsolete platform integration references; the three retained shell scripts
pass Bash syntax checking; `git diff --check` passes. The new Qt regression tests
were added but could not be executed on this host.
