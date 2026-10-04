# Third-party notices

HyprFM for FreeBSD is derived from
[HyprFM by Soyeb Pervez Jim](https://github.com/soyeb-jim285/hyprfm).
The root [LICENSE](LICENSE) retains the original MIT copyright and permission
notice. This inventory supplements the license texts; it does not replace them.

| Included component | License / holders | License text |
| --- | --- | --- |
| HyprFM application | MIT; `2025-present Jim` | [LICENSE](LICENSE) |
| [Quill](https://github.com/soyeb-jim285/quill) | MIT; `2025-present Jim` | `src/qml/Quill/LICENSE` (submodule) |
| [quill-icons](https://github.com/soyeb-jim285/quill-icons) | ISC; Lucide Icons Contributors and Jim; also MIT for Feather-derived paths, Cole Bemis | `src/qml/icons/LICENSE` (submodule; includes both notices) |
| [toml++ 3.4.0](https://github.com/marzer/tomlplusplus) | MIT; Mark Gillard | [Installed notice copy](licenses/tomlplusplus-LICENSE); also embedded in `src/third_party/toml.hpp` |

Quill and icon revisions are pinned by the repository's submodule entries.
Preserve their complete notices when redistributing the components, including
compiled QML resources. The CMake install rules copy those notices directly
from the submodules and install the toml++ notice alongside the application
license. When updating a dependency, recheck its license and any copied notice.

Qt, GLib/GIO, optional KWindowSystem and external runtime helpers are separate
dependencies, not covered by the application's MIT license. Package/binary
distributors must meet the licenses applicable to the exact dependencies they
ship. See [the publishing guide](docs/PUBLISHING.md).
