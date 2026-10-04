# Future FreeBSD work

These are explicit limitations, not features that a missing package will enable.
Source placeholders use the same identifiers so implementation and documentation
remain linked.

| ID | Function / integration | Current behavior | Future implementation |
| --- | --- | --- | --- |
| FBSD-01 | `DeviceModel::mount`, `DeviceModel::unmount`, `RuntimeFeaturesService::deviceMountAvailable` | Enumerates mounted filesystems; navigating an existing mount works. Device unmount emits `mountError`; mount of an unmounted entry would emit the same explicit unsupported error. Availability is always false. | Add a FreeBSD mount authorization service with filesystem-specific support, cancellation and useful permission errors. Include safe eject/power-off only where supported. Do not run the GUI as root. |
| FBSD-02 | Device discovery / `DeviceModel` constructor and refresh | Polls Qt's mount list every five seconds. Unmounted disks, USB phones/cameras, and removable-media classification are absent; all entries have `removable=false` (unknown). | Use devd/GEOM or an appropriate FreeBSD desktop service for hotplug, labels and removable identification. Add optional GIO mobile discovery after validating the FreeBSD backends. |
| FBSD-03 | `FileOperations::setWallpaper` | Emits `operationFinished(false, message)` without launching a process. | Implement supported desktop-specific wallpaper APIs and advertise availability. |
| FBSD-04 | `FileOperations::setWindowRounding`, `setWindowBorder` | Deliberate no-ops; Qt/QML styling and the window manager determine appearance. | Integrate with a supported compositor only where its API exists. These settings are cosmetic and must not spawn failing helper processes. |
| FBSD-05 | `releaseUnusedAllocatorPages` in `main.cpp` | No-op after window-close garbage collection; ordinary allocator reclamation remains active. | Evaluate FreeBSD allocator-specific purge APIs only after measuring retained memory. |

The maintainer has tested the port on FreeBSD 15.1 with no issues reported so
far. The placeholders above remain intentional limitations. Further validation
and follow-ups:

- Record full-suite results on FreeBSD, in both ordinary and unity/PCH
  builds; validate Qt's compiled QML on an installed Release build.
- Run X11 and Wayland desktop tests, including clipboard ownership, decorations,
  drag/drop, portals and optional KWin effects.
- Test UFS/ZFS trash, read-only and remote mounts, device polling latency, and
  inaccessible storage. Move storage probes off the GUI thread if slow mounts
  cause stalls; `refreshAsync()` currently schedules a synchronous Qt query.
- Validate rclone/FUSE permissions and shutdown on FreeBSD. Cleanup at process
  exit is best effort; busy mounts or pending VFS uploads require explicit user
  handling. Add integration coverage for these cases and crash recovery.
- Test protocol-specific GVFS authentication and backend options, especially
  mobile devices, before claiming them as supported integrations.
- Add a FreeBSD-native CI runner and a ports/package recipe once the build and
  runtime checks pass. No automated publishing is configured.

GIO transfers, D-Bus, XDG trash/application entries, POSIX filesystem calls and
Qt are retained because FreeBSD supports them. Rclone's mount command also
supports FreeBSD; it is an optional real implementation, not a placeholder.
