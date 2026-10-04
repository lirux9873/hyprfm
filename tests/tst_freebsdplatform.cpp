#include <QTest>
#include "services/dependencychecker.h"
#include "services/runtimefeaturesservice.h"

class TestFreeBSDPlatform : public QObject
{
    Q_OBJECT
private slots:
    void deviceMountIsExplicitlyUnavailable()
    {
        RuntimeFeaturesService features;
        QVERIFY(!features.deviceMountAvailable());
        QVERIFY(features.installHint("deviceMount").contains("not implemented"));
    }

    void dependencyHintsMatchFreeBSD()
    {
        DependencyChecker dependencies;
        QCOMPARE(dependencies.distroId(), QStringLiteral("freebsd"));
        QVERIFY(dependencies.installCommandFor("gio").contains("pkg install glib"));
        QVERIFY(dependencies.installCommandFor("7zip").contains("pkg install 7-zip"));
        QVERIFY(dependencies.installCommandFor("deviceMount").contains("Not implemented"));
        for (const QVariant &entry : dependencies.dependencies()) {
            const QVariantMap dep = entry.toMap();
            QVERIFY(!dep.value("installCommand").toString().isEmpty());
            if (dep.value("id").toString() == QStringLiteral("deviceMount")) {
                QVERIFY(!dep.value("available").toBool());
                QVERIFY(!dep.value("required").toBool());
            }
        }
    }
};

QTEST_GUILESS_MAIN(TestFreeBSDPlatform)
#include "tst_freebsdplatform.moc"
