#pragma once

#include <QObject>

class RuntimeFeaturesService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ffmpegAvailable READ ffmpegAvailable CONSTANT)
    Q_PROPERTY(bool batAvailable READ batAvailable CONSTANT)
    Q_PROPERTY(bool deviceMountAvailable READ deviceMountAvailable CONSTANT)
    Q_PROPERTY(bool gitAvailable READ gitAvailable CONSTANT)
    Q_PROPERTY(bool useIntegratedWindowControls READ useIntegratedWindowControls CONSTANT)

public:
    explicit RuntimeFeaturesService(QObject *parent = nullptr);

    bool ffmpegAvailable() const;
    bool batAvailable() const;
    bool deviceMountAvailable() const;
    bool gitAvailable() const;
    bool useIntegratedWindowControls() const;

    Q_INVOKABLE QString installHint(const QString &feature) const;

private:
    static bool hasExecutable(const QString &name);
};
