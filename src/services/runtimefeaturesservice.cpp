#include "services/runtimefeaturesservice.h"

#include <QStandardPaths>

namespace {

bool isIntegratedWindowControlsDesktop()
{
    const QString desktopId = (qEnvironmentVariable("XDG_CURRENT_DESKTOP")
        + QLatin1Char(';')
        + qEnvironmentVariable("DESKTOP_SESSION")).toLower();

    return desktopId.contains(QStringLiteral("gnome"))
        || desktopId.contains(QStringLiteral("plasma"))
        || desktopId.contains(QStringLiteral("kde"));
}

} // namespace

RuntimeFeaturesService::RuntimeFeaturesService(QObject *parent)
    : QObject(parent)
{
}

bool RuntimeFeaturesService::ffmpegAvailable() const
{
    return hasExecutable(QStringLiteral("ffmpeg"));
}

bool RuntimeFeaturesService::batAvailable() const
{
    return hasExecutable(QStringLiteral("bat"));
}

bool RuntimeFeaturesService::deviceMountAvailable() const
{
    // TODO(FBSD-01): authorized device mount/unmount backend.
    return false;
}

bool RuntimeFeaturesService::gitAvailable() const
{
    return hasExecutable(QStringLiteral("git"));
}

bool RuntimeFeaturesService::useIntegratedWindowControls() const
{
    return isIntegratedWindowControlsDesktop();
}

QString RuntimeFeaturesService::installHint(const QString &feature) const
{
    if (feature == QStringLiteral("videoPreview"))
        return QStringLiteral("Install ffmpeg to enable video poster previews.");
    if (feature == QStringLiteral("pdfPreview"))
        return QStringLiteral("Run pkg install poppler-utils as root to enable PDF previews.");
    if (feature == QStringLiteral("remoteAccess"))
        return QStringLiteral("Install gvfs to browse remote filesystems through Connect to Server.");
    if (feature == QStringLiteral("smbRemoteAccess"))
        return QStringLiteral("Install gvfs with its SMB backend to browse SMB/CIFS shares.");
    if (feature == QStringLiteral("deviceMount"))
        return QStringLiteral("Device mount/unmount is not implemented on FreeBSD; mount the filesystem outside HyprFM.");
    if (feature == QStringLiteral("clipboardImage"))
        return QStringLiteral("Clipboard images require an active desktop clipboard offer.");
    if (feature == QStringLiteral("textHighlight"))
        return QStringLiteral("Install bat for syntax-highlighted text previews.");
    return {};
}

bool RuntimeFeaturesService::hasExecutable(const QString &name)
{
    return !QStandardPaths::findExecutable(name).isEmpty();
}
