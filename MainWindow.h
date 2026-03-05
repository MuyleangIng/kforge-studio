#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMap>
#include <QUuid>

class QStackedWidget;
class QListWidget;
class QLabel;
class QPushButton;
class QProgressBar;
class QTextEdit;
class QSplitter;
class BuildJobWidget;
class BuilderManagerWidget;
class BakeWidget;
class SettingsWidget;
class SidebarWidget;
class DashboardWidget;

// ──────────────────────────────────────────────
// BuildJob — represents one running kforge build
// ──────────────────────────────────────────────
struct BuildJob {
    QString id;
    QString tag;
    QString context;
    QStringList platforms;
    QString progress;
    QProcess *process = nullptr;
    QString status;   // "running" | "success" | "failed" | "cancelled"
    qint64  startedAt = 0;
    QString logOutput;
};

// ──────────────────────────────────────────────
// MainWindow
// ──────────────────────────────────────────────
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

    // ── layout ──
    SidebarWidget   *m_sidebar   = nullptr;
    QStackedWidget  *m_stack     = nullptr;
    DashboardWidget *m_dashboard = nullptr;
    BuildJobWidget  *m_buildPage = nullptr;
    BuilderManagerWidget *m_builderPage = nullptr;
    BakeWidget      *m_bakePage  = nullptr;
    SettingsWidget  *m_settingsPage = nullptr;

    // ── state ──
    QMap<QString, BuildJob> m_jobs;
    QSystemTrayIcon *m_tray  = nullptr;
    QTimer          *m_timer = nullptr;
};
