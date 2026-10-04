#include "services/rcloneservice.h"

#include "services/cloudmounts.h"

#include <QDir>
#include <QPointer>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>
#include <QDebug>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

RcloneService::RcloneService(QObject *parent)
    : QObject(parent)
{
    m_rcloneAvailable = checkRcloneAvailable();
    m_mountsBaseDir = cloudMountsBaseDir();
    ensureMountsBaseDirExists();
}

RcloneService::~RcloneService()
{
    // Best-effort cleanup of mounts owned by this process at shutdown.
    const QStringList remotes = m_processes.keys();
    for (const QString &remote : remotes) {
        const QString mountPath = getMountPath(remote);
        QProcess cleanup;
        cleanup.start(QStringLiteral("/sbin/umount"), {mountPath});
        cleanup.waitForFinished(3000);

        QProcess *proc = m_processes.value(remote);
        if (proc && proc->state() != QProcess::NotRunning)
            proc->terminate();
    }
    m_processes.clear();
}

bool RcloneService::rcloneAvailable() const
{
    return m_rcloneAvailable;
}

QStringList RcloneService::activeMounts() const
{
    return m_processes.keys();
}

bool RcloneService::checkRcloneAvailable() const
{
    return !QStandardPaths::findExecutable(QStringLiteral("rclone")).isEmpty();
}

void RcloneService::ensureMountsBaseDirExists() const
{
    QDir().mkpath(m_mountsBaseDir);
}

bool RcloneService::isRclonePath(const QString &path) const
{
    return isCloudMountPath(path);
}

bool RcloneService::isMounted(const QString &remoteName) const
{
    return m_processes.contains(remoteName) && m_mountSuccessEmitted.value(remoteName);
}

bool RcloneService::isMounting(const QString &remoteName) const
{
    return m_processes.contains(remoteName) && !m_mountSuccessEmitted.value(remoteName);
}

bool RcloneService::isMountedForPath(const QString &path) const
{
    const QString remote = getRemoteNameFromPath(path);
    if (remote.isEmpty())
        return false;
    return isMounted(remote);
}

QString RcloneService::getRemoteNameFromPath(const QString &path) const
{
    if (!isRclonePath(path))
        return {};

    QString sub = path.mid(m_mountsBaseDir.length());
    if (sub.startsWith(QLatin1Char('/'))) {
        sub = sub.mid(1);
    }
    int slashIdx = sub.indexOf(QLatin1Char('/'));
    if (slashIdx != -1) {
        return sub.left(slashIdx);
    }
    return sub;
}

QString RcloneService::getMountPath(const QString &remoteName) const
{
    return m_mountsBaseDir + QStringLiteral("/") + remoteName;
}

void RcloneService::mountRemote(const QString &remoteName)
{
    // A remote name must never escape the private mounts directory.
    static const QRegularExpression validName(QStringLiteral("^[A-Za-z0-9_][A-Za-z0-9_. -]*$"));
    if (!validName.match(remoteName).hasMatch()) {
        emit mountFinished(remoteName, false, tr("Invalid cloud remote name."));
        return;
    }
    if (!m_rcloneAvailable) {
        emit mountFinished(remoteName, false, QStringLiteral("rclone executable not found"));
        return;
    }

    if (isMounted(remoteName)) {
        emit mountFinished(remoteName, true, QString());
        return;
    }

    if (isMounting(remoteName)) {
        return;
    }

    const QString mountPath = getMountPath(remoteName);
    if (QFileInfo(m_mountsBaseDir).isSymLink() || QFileInfo(mountPath).isSymLink()
        || !QDir().mkpath(mountPath)) {
        emit mountFinished(remoteName, false, tr("Cannot create a safe cloud mount directory."));
        return;
    }
    for (const auto &volume : QStorageInfo::mountedVolumes()) {
        if (QDir::cleanPath(volume.rootPath()) == QDir::cleanPath(mountPath)) {
            emit mountFinished(remoteName, false, tr("The mount point is already in use. Unmount it outside HyprFM first."));
            return;
        }
    }
    startRcloneMountProcess(remoteName, mountPath);
}

void RcloneService::releaseMountPoint(const QString &mountPath, const std::function<void(bool)> &then)
{
    for (const auto &volume : QStorageInfo::mountedVolumes()) {
        if (QDir::cleanPath(volume.rootPath()) == QDir::cleanPath(mountPath)) {
            runUnmountTool(QStringLiteral("/sbin/umount"), mountPath, then);
            return;
        }
    }
    then(true); // An unsuccessful mount attempt may have no filesystem to release.
}

void RcloneService::runUnmountTool(const QString &tool, const QString &mountPath,
                                   const std::function<void(bool)> &done)
{
    QProcess *proc = new QProcess(this);
    const auto settle = [proc, done](bool ok) {
        proc->disconnect();
        proc->deleteLater();
        done(ok);
    };

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [settle](int exitCode, QProcess::ExitStatus status) {
        settle(status == QProcess::NormalExit && exitCode == 0);
    });
    connect(proc, &QProcess::errorOccurred, this, [settle](QProcess::ProcessError) {
        settle(false);
    });

    proc->start(tool, {mountPath});
    QTimer::singleShot(5000, proc, [proc, settle]() {
        if (proc->state() != QProcess::NotRunning) {
            proc->disconnect();
            proc->kill();
            settle(false);
        }
    });
}

void RcloneService::startRcloneMountProcess(const QString &remoteName, const QString &mountPath)
{
    QProcess *proc = new QProcess(this);
    const quint64 generation = ++m_mountGeneration[remoteName];
    m_processes.insert(remoteName, proc);
    m_mountSuccessEmitted[remoteName] = false;
    emit activeMountsChanged();

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, remoteName, proc](int exitCode, QProcess::ExitStatus) {
        const QString err = QString::fromUtf8(proc->readAllStandardError()).trimmed();
        m_processes.remove(remoteName);
        emit activeMountsChanged();

        if (!m_mountSuccessEmitted.value(remoteName)) {
            emit mountFinished(remoteName, false, err.isEmpty() ? QStringLiteral("rclone mount exited unexpectedly.") : err);
        }
        m_mountSuccessEmitted.remove(remoteName);
        proc->deleteLater();
    });

    connect(proc, &QProcess::errorOccurred, this, [this, remoteName, proc](QProcess::ProcessError) {
        const QString err = proc->errorString();
        m_processes.remove(remoteName);
        emit activeMountsChanged();

        if (!m_mountSuccessEmitted.value(remoteName)) {
            emit mountFinished(remoteName, false, err);
        }
        m_mountSuccessEmitted.remove(remoteName);
        proc->deleteLater();
    });

    proc->start(QStringLiteral("rclone"), {
        QStringLiteral("mount"),
        remoteName + QStringLiteral(":"),
        mountPath,
        QStringLiteral("--vfs-cache-mode"),
        QStringLiteral("writes"),
        QStringLiteral("--vfs-cache-max-age"),
        QStringLiteral("72h"),
        QStringLiteral("--no-checksum"),
        QStringLiteral("--vfs-read-chunk-size"),
        QStringLiteral("1M"),
        QStringLiteral("--vfs-read-chunk-size-limit"),
        QStringLiteral("off"),
        QStringLiteral("--buffer-size"),
        QStringLiteral("32M"),
        QStringLiteral("--poll-interval"),
        QStringLiteral("15s")
    });

    // Poll mountpoint checks every 100ms to verify FUSE mount is active
    QTimer *mountTimer = new QTimer(this);
    connect(mountTimer, &QTimer::timeout, this, [this, remoteName, mountPath, mountTimer]() {
        if (!m_processes.contains(remoteName) || m_processes.value(remoteName)->state() != QProcess::Running) {
            mountTimer->stop();
            mountTimer->deleteLater();
            return;
        }

        struct stat st_dir, st_parent;
        QString parentPath = QDir(mountPath).filePath(QStringLiteral(".."));
        if (stat(mountPath.toLocal8Bit().constData(), &st_dir) == 0 &&
            stat(parentPath.toLocal8Bit().constData(), &st_parent) == 0) {

            if (st_dir.st_dev != st_parent.st_dev) {
                mountTimer->stop();
                mountTimer->deleteLater();

                m_mountSuccessEmitted[remoteName] = true;
                emit mountFinished(remoteName, true, QString());
            }
        }
    });

    // Timeout safety: if it doesn't mount in 10 seconds, abort. The poll timer
    // deletes itself as soon as the process is gone, so hold it weakly, and
    // bail out when a newer attempt has taken over this remote -- otherwise a
    // remount inside the timeout window would tear down the fresh mount.
    QPointer<QTimer> pollTimer(mountTimer);
    QTimer::singleShot(10000, this, [this, remoteName, generation, pollTimer]() {
        if (m_mountGeneration.value(remoteName) != generation)
            return;
        if (!m_processes.contains(remoteName) || m_mountSuccessEmitted.value(remoteName))
            return;

        if (pollTimer) {
            pollTimer->stop();
            pollTimer->deleteLater();
        }

        unmountRemote(remoteName);
        emit mountFinished(remoteName, false, QStringLiteral("Mount operation timed out. Verify your rclone remote or network connection."));
    });

    mountTimer->start(100);
}

void RcloneService::unmountRemote(const QString &remoteName)
{
    if (!m_processes.contains(remoteName)) {
        emit unmountFinished(remoteName, true);
        return;
    }

    // Detach the filesystem first. A busy or unauthorized mount must remain
    // usable, and failure must not be presented to QML as successful cleanup.
    const QPointer<QProcess> process = m_processes.value(remoteName);
    releaseMountPoint(getMountPath(remoteName), [this, remoteName, process](bool ok) {
        if (!ok) {
            emit unmountFinished(remoteName, false);
            return;
        }
        if (process && m_processes.value(remoteName) == process.data()) {
            disconnect(process, nullptr, this, nullptr);
            if (process->state() == QProcess::NotRunning) {
                process->deleteLater();
            } else {
                connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        process, &QObject::deleteLater);
                process->terminate();
                QTimer::singleShot(2000, process, [process]() {
                    if (process && process->state() != QProcess::NotRunning)
                        process->kill();
                });
            }
            m_processes.remove(remoteName);
            m_mountSuccessEmitted.remove(remoteName);
            emit activeMountsChanged();
        }
        emit unmountFinished(remoteName, true);
    });
}
