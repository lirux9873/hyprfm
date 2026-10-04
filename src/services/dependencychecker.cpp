#include "services/dependencychecker.h"
#include <QFileInfo>
#include <QStandardPaths>

DependencyChecker::DependencyChecker(QObject *parent) : QObject(parent)
{
    detectDistro();
    populate();
}

void DependencyChecker::detectDistro()
{
    m_distroId = QStringLiteral("freebsd");
    m_distroName = QStringLiteral("FreeBSD");
}

void DependencyChecker::refresh()
{
    populate();
    emit dependenciesChanged();
}

void DependencyChecker::populate()
{
    m_deps.clear();
    auto tool = [this](const QString &id, const QString &purpose, const QString &package,
                       const QStringList &commands, bool required = false) {
        bool available = true;
        for (const QString &command : commands)
            available = available && hasExecutable(command);
        const QString hint = QStringLiteral("pkg install %1").arg(package);
        m_deps.append({id, id, purpose, Kind::Tool, required, available, commands,
                       {{QStringLiteral("freebsd"), hint}, {QStringLiteral("generic"), hint}}});
    };
    tool("gio", "File operations and application launching", "glib", {"gio"}, true);
    tool("fd", "File search", "fd", {"fd"});
    tool("git", "Repository status overlays", "git", {"git"});
    tool("bat", "Syntax-highlighted previews", "bat", {"bat"});
    tool("md2html", "Rendered Markdown previews", "md4c", {"md2html"});
    tool("ffmpeg", "Video thumbnails and media metadata", "ffmpeg", {"ffmpeg", "ffprobe"});
    tool("poppler-utils", "PDF previews", "poppler-utils", {"pdftoppm", "pdfinfo"});
    tool("exiftool", "Image metadata", "p5-Image-ExifTool", {"exiftool"});
    tool("zip", "ZIP archive creation", "zip", {"zip"});
    tool("unzip", "ZIP archive extraction", "unzip", {"unzip"});
    tool("7zip", "7-Zip archives", "7-zip", {"7zz"});
    tool("rclone", "Cloud filesystem mounts (requires FUSE configuration)", "rclone", {"rclone"});
    const QString gvfsHint = QStringLiteral("pkg install gvfs");
    m_deps.append({"gvfs", "GVFS", "Remote URI browsing (SFTP, SMB, WebDAV)", Kind::Tool,
        false, QFileInfo::exists(QStringLiteral("/usr/local/share/gvfs/mounts")), {"gio"},
        {{"freebsd", gvfsHint}, {"generic", gvfsHint}}});
    const QString mountHint = QStringLiteral("Not implemented on FreeBSD; see docs/FUTURE.md (FBSD-01/02).");
    m_deps.append({"deviceMount", "Device mounting", "Privileged mount/unmount and unmounted device discovery",
        Kind::Feature, false, false, {}, {{"freebsd", mountHint}, {"generic", mountHint}}});
}

QVariantList DependencyChecker::dependencies() const
{
    QVariantList out;
    out.reserve(m_deps.size());
    for (const auto &dep : m_deps)
        out.append(toVariant(dep));
    return out;
}

QVariantList DependencyChecker::missingDependencies() const
{
    QVariantList out;
    for (const auto &dep : m_deps) {
        if (!dep.available)
            out.append(toVariant(dep));
    }
    return out;
}

bool DependencyChecker::hasAnyMissing() const
{
    for (const auto &dep : m_deps) {
        if (!dep.available)
            return true;
    }
    return false;
}

bool DependencyChecker::hasMissingRequired() const
{
    for (const auto &dep : m_deps) {
        if (!dep.available && dep.required)
            return true;
    }
    return false;
}

QString DependencyChecker::installCommandFor(const QString &id) const
{
    for (const auto &dep : m_deps) {
        if (dep.id != id)
            continue;
        if (dep.installHints.contains(m_distroId))
            return dep.installHints.value(m_distroId).toString();
        return dep.installHints.value(QStringLiteral("generic")).toString();
    }
    return {};
}

QVariantMap DependencyChecker::toVariant(const Dependency &dep) const
{
    QVariantMap m;
    m[QStringLiteral("id")]          = dep.id;
    m[QStringLiteral("displayName")] = dep.displayName;
    m[QStringLiteral("purpose")]     = dep.purpose;
    m[QStringLiteral("required")]    = dep.required;
    m[QStringLiteral("available")]   = dep.available;
    m[QStringLiteral("commands")]    = dep.commands;
    m[QStringLiteral("installHints")]= dep.installHints;

    QString kindStr;
    switch (dep.kind) {
    case Kind::Tool:    kindStr = QStringLiteral("tool"); break;
    case Kind::Feature: kindStr = QStringLiteral("feature"); break;
    case Kind::Service: kindStr = QStringLiteral("service"); break;
    }
    m[QStringLiteral("kind")] = kindStr;

    const QString hint = dep.installHints.contains(m_distroId)
        ? dep.installHints.value(m_distroId).toString()
        : dep.installHints.value(QStringLiteral("generic")).toString();
    m[QStringLiteral("installCommand")] = hint;

    return m;
}

bool DependencyChecker::hasExecutable(const QString &name)
{
    return !QStandardPaths::findExecutable(name).isEmpty();
}
