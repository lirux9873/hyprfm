#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QTimer>

struct DeviceEntry {
    QString deviceName;
    QString devicePath;   // block device path, e.g. /dev/ada0p2
    QString mountPoint;   // empty if unmounted
    QString fsType;       // e.g. "ufs", "zfs" — used for error messages
    qint64  totalSize;
    qint64  freeSpace;
    int     usagePercent;
    bool    removable;
    bool    mounted;
    QString alternateMountPoint;
};

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        DeviceNameRole = Qt::UserRole + 1,
        DevicePathRole,
        MountPointRole,
        TotalSizeRole,
        FreeSpaceRole,
        UsagePercentRole,
        RemovableRole,
        MountedRole,
        BackendRole,
    };

    explicit DeviceModel(QObject *parent = nullptr, bool deferInitialRefresh = false);
    ~DeviceModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void unmount(int index);
    Q_INVOKABLE void mount(int index);

public slots:
    void refresh();
    void refreshAsync();
    void scheduleRefresh();

signals:
    void deviceMounted(const QString &mountPoint);
    void mountError(const QString &message);

private:
    QList<DeviceEntry> m_devices;
    QTimer m_refreshTimer;
};
