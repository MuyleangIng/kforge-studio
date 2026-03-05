#pragma once
#include <QMainWindow>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMap>
#include <QStackedWidget>
#include "BuildJob.h"

class SidebarWidget;
class BuildJobWidget;
class BuilderManagerWidget;
class BakeWidget;
class SettingsWidget;
class DashboardWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void launchBuild(const BuildJob &job);
    void cancelBuild(const QString &jobId);
    const QMap<QString, BuildJob>& jobs() const { return m_jobs; }

signals:
    void jobStatusChanged(const QString &jobId, const QString &status);
    void jobLogAppended(const QString &jobId, const QString &line);

private slots:
    void onSidebarNavChanged(int index);
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onRefreshTimer();

private:
    void setupUI();
    void setupTray();
    void applyDarkTheme();
    QString jobIdFromProcess(QProcess *p) const;

    SidebarWidget        *m_sidebar      = nullptr;
    QStackedWidget       *m_stack        = nullptr;
    DashboardWidget      *m_dashboard    = nullptr;
    BuildJobWidget       *m_buildPage    = nullptr;
    BuilderManagerWidget *m_builderPage  = nullptr;
    BakeWidget           *m_bakePage     = nullptr;
    SettingsWidget       *m_settingsPage = nullptr;

    QMap<QString, BuildJob> m_jobs;
    QSystemTrayIcon *m_tray  = nullptr;
    QTimer          *m_timer = nullptr;
};