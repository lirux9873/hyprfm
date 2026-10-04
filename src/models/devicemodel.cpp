#include "models/devicemodel.h"
#include <QStorageInfo>
#include <QSet>
#include <QLoggingCategory>
#include <algorithm>

Q_LOGGING_CATEGORY(lcDeviceRefresh, "hyprfm.devices", QtWarningMsg)

DeviceModel::DeviceModel(QObject *parent, bool deferInitialRefresh)
    : QAbstractListModel(parent)
{
    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(250);
    connect(&m_refreshTimer, &QTimer::timeout, this, &DeviceModel::refresh);
    // TODO(FBSD-02): devd/GEOM discovery and removable-media identification.
    auto *poll = new QTimer(this);
    poll->setInterval(5000);
    connect(poll, &QTimer::timeout, this, &DeviceModel::scheduleRefresh);
    poll->start();
    if (deferInitialRefresh)
        scheduleRefresh();
    else
        refresh();
}

DeviceModel::~DeviceModel() = default;

void DeviceModel::refresh()
{
    QList<DeviceEntry> devices;
    QSet<QString> seen;
    auto volumes = QStorageInfo::mountedVolumes();
    volumes.prepend(QStorageInfo::root());
    for (const QStorageInfo &volume : volumes) {
        const QString path = volume.rootPath();
        const QByteArray fs = volume.fileSystemType();
        if (!volume.isValid() || !volume.isReady() || path.isEmpty() || seen.contains(path))
            continue;
        if (fs == "devfs" || fs == "procfs" || fs == "fdescfs")
            continue;
        seen.insert(path);
        DeviceEntry entry{};
        entry.deviceName = volume.displayName().isEmpty() ? path : volume.displayName();
        entry.devicePath = QString::fromLocal8Bit(volume.device());
        entry.mountPoint = path;
        entry.fsType = QString::fromLatin1(fs);
        entry.totalSize = qMax(qint64(0), volume.bytesTotal());
        entry.freeSpace = qBound(qint64(0), volume.bytesAvailable(), entry.totalSize);
        entry.usagePercent = entry.totalSize > 0
            ? qBound(0, int(100.0 * (1.0 - double(entry.freeSpace) / entry.totalSize)), 100) : 0;
        entry.mounted = true;
        entry.removable = false; // Unknown until FBSD-02; do not infer from device names.
        devices.append(entry);
    }
    std::sort(devices.begin(), devices.end(), [](const DeviceEntry &a, const DeviceEntry &b) {
        return a.mountPoint < b.mountPoint;
    });
    // Capacity changes during polling must not destroy/recreate delegates.
    // Reset only when the mounted filesystem identities actually change.
    const bool sameMounts = devices.size() == m_devices.size()
        && std::equal(devices.cbegin(), devices.cend(), m_devices.cbegin(),
                      [](const DeviceEntry &a, const DeviceEntry &b) {
            return a.mountPoint == b.mountPoint && a.devicePath == b.devicePath
                && a.fsType == b.fsType;
        });
    if (sameMounts) {
        int changed = 0;
        for (int row = 0; row < devices.size(); ++row) {
            const auto &old = m_devices.at(row);
            const auto &next = devices.at(row);
            QList<int> roles;
            if (old.deviceName != next.deviceName) roles << DeviceNameRole;
            if (old.totalSize != next.totalSize) roles << TotalSizeRole;
            if (old.freeSpace != next.freeSpace) roles << FreeSpaceRole;
            if (old.usagePercent != next.usagePercent) roles << UsagePercentRole;
            if (old.removable != next.removable) roles << RemovableRole;
            if (old.mounted != next.mounted) roles << MountedRole;
            m_devices[row] = next;
            if (!roles.isEmpty()) {
                ++changed;
                emit dataChanged(index(row), index(row), roles);
            }
        }
        qCDebug(lcDeviceRefresh) << "poll:" << changed << "rows updated; no reset";
        return;
    }
    qCDebug(lcDeviceRefresh) << "mount topology changed; reset" << devices.size() << "rows";
    beginResetModel();
    m_devices = devices;
    endResetModel();
}

void DeviceModel::refreshAsync() { scheduleRefresh(); }
void DeviceModel::scheduleRefresh() { m_refreshTimer.start(); }

void DeviceModel::mount(int index)
{
    if (index < 0 || index >= m_devices.size())
        return;
    if (m_devices[index].mounted) {
        emit deviceMounted(m_devices[index].mountPoint);
        return;
    }
    // TODO(FBSD-01): authorized device mount backend.
    emit mountError(tr("Device mounting is not implemented on FreeBSD."));
}

void DeviceModel::unmount(int index)
{
    if (index < 0 || index >= m_devices.size())
        return;
    // TODO(FBSD-01): authorized device unmount backend.
    emit mountError(tr("Device unmounting is not implemented on FreeBSD. Unmount it outside HyprFM."));
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this || index.row() < 0 || index.row() >= m_devices.size())
        return {};

    const auto &d = m_devices.at(index.row());
    switch (role) {
    case DeviceNameRole:   return d.deviceName;
    case DevicePathRole:   return d.devicePath;
    case MountPointRole:   return d.mountPoint;
    case TotalSizeRole:    return d.totalSize;
    case FreeSpaceRole:    return d.freeSpace;
    case UsagePercentRole: return d.usagePercent;
    case RemovableRole:    return d.removable;
    case MountedRole:      return d.mounted;
    case BackendRole:      return QStringLiteral("freebsd");
    }
    return {};
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {
        {DeviceNameRole,   "deviceName"},
        {DevicePathRole,   "devicePath"},
        {MountPointRole,   "mountPoint"},
        {TotalSizeRole,    "totalSize"},
        {FreeSpaceRole,    "freeSpace"},
        {UsagePercentRole, "usagePercent"},
        {RemovableRole,    "removable"},
        {MountedRole,      "mounted"},
        {BackendRole,      "backend"},
    };
}
