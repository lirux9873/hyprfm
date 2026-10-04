# Publishing the FreeBSD fork

## Origin and ownership

Publish the FreeBSD version from [lirux9873/hyprfm](https://github.com/lirux9873/hyprfm)
and describe it as an independent fork of
[soyeb-jim285/hyprfm](https://github.com/soyeb-jim285/hyprfm), by Soyeb Pervez Jim.
Keep that attribution in the README and retain the upstream Git history.
Maintaining a fork does not make its maintainer the author or copyright owner
of the original code. Do not suggest upstream endorsement.

The repository's license file is named `LICENSE`. It contains the MIT license
and the original notice `Copyright (c) 2025-present Jim`; both are preserved.
MIT permits modification, publication, distribution and sale, subject to
retaining the copyright and permission notices in copies or substantial
portions of the software. Separate permission from upstream is not required
for those licensed activities. See the [MIT license text](https://opensource.org/license/mit)
and [the license distributed with this fork](../LICENSE).

Continue distributing this fork under MIT. A contributor may add a separate
copyright notice for copyrightable work they own, using the correct holder and
year; that is not necessary merely to publish a fork and must not replace the
original notice. No new ownership claim has been inserted on behalf of an
unconfirmed person or organization. A source-license grant should not be treated
as a separate trademark or endorsement permission.

## Source publication

1. Retain `LICENSE`, the README's upstream attribution and
   [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md).
2. Retain the notices embedded in `src/third_party/toml.hpp` and the license
   files in the pinned Quill and icon submodules. Do not apply the root MIT
   label to every dependency: the icon bundle also contains ISC-licensed work.
3. Preserve `.gitmodules` and its pinned revisions. If providing a complete
   source archive, include the populated submodule contents and their licenses;
   an ordinary repository archive does not embed those external repositories.
4. Describe releases as FreeBSD fork releases and distinguish their tags from
   upstream versions, for example `v0.6.1-freebsd.1`. Record the source commit,
   FreeBSD/Qt versions, desktop session, test results and known placeholders.
5. Direct fork-specific issues to this repository. GitHub's fork relationship
   is useful provenance, but a README link alone does not create that UI
   relationship. An independently created repository can still distribute a
   properly attributed MIT-derived work.

The reported FreeBSD 15.1 success may be included in release notes. Do not
describe it as a complete automated test pass or coverage of every optional
feature without corresponding results.

## Binary packages and installed notices

The CMake install rules include the application's `LICENSE` and the Quill,
quill-icons (including Lucide/Feather) and toml++ notices under
`${CMAKE_INSTALL_DATADIR}/licenses/hyprfm`. Documentation includes the component
inventory. Check those files in the staged package before distributing it.

External libraries and tools keep their own licenses. When linking or bundling
Qt, GLib/GIO, optional KDE libraries, or helper executables, review the exact
versions, modules, build options and distribution licenses. The application
being MIT does not discharge those obligations. In particular, distributing
LGPL components can require license/source availability and relinking rights;
static linking or bundling needs a separate review. A package using system
dependencies should declare them accurately rather than copy arbitrary binaries.
See [Qt's open-source licensing guidance](https://www.qt.io/licensing/open-source-lgpl-obligations).

Before an independently branded binary release, review the inherited
`io.github.soyeb_jim285.HyprFM` desktop/AppStream identity, upstream homepage and
issue URLs, icon naming and developer metadata. Change identities consistently
if side-by-side installation is intended; keep original author attribution
separate from fork-maintainer metadata. This documentation update does not
rename application IDs or alter configuration paths.

This guide records the notices reviewed in the current tree; it is not a
license audit of every possible third-party binary a distributor might bundle.
