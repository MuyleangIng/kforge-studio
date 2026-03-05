#include "MainWindow.h"
#include "SidebarWidget.h"
#include "BuildJobWidget.h"
#include "OtherWidgets.h"
#include "SidebarWidget.h"
#include "BuildJobWidget.h"
#include "OtherWidgets.h"
#include "OtherWidgets.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QProcess>
#include <QUuid>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("KForge GUI");
    setMinimumSize(1280, 800);
    resize(1440, 900);
    applyDarkTheme();
    setupUI();
    setupTray();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onRefreshTimer);
    m_timer->start(2000);
}

MainWindow::~MainWindow() {}

// ─── Setup ───────────────────────────────────────────────────────────────────

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_sidebar = new SidebarWidget(this);
    root->addWidget(m_sidebar);

    m_stack       = new QStackedWidget(this);
    m_dashboard   = new DashboardWidget(this, this);
    m_buildPage   = new BuildJobWidget(this, this);
    m_builderPage = new BuilderManagerWidget(this);
    m_bakePage    = new BakeWidget(this);
    m_settingsPage= new SettingsWidget(this);

    m_stack->addWidget(m_dashboard);   // index 0
    m_stack->addWidget(m_buildPage);   // index 1
    m_stack->addWidget(m_builderPage); // index 2
    m_stack->addWidget(m_bakePage);    // index 3
    m_stack->addWidget(m_settingsPage);// index 4

    root->addWidget(m_stack, 1);

    connect(m_sidebar, &SidebarWidget::navigationChanged,
            this, &MainWindow::onSidebarNavChanged);
}

void MainWindow::setupTray()
{
    m_tray = new QSystemTrayIcon(QIcon::fromTheme("docker"), this);
    QMenu *menu = new QMenu(this);
    menu->addAction("Show", this, &MainWindow::show);
    menu->addSeparator();
    menu->addAction("Quit", qApp, &QApplication::quit);
    m_tray->setContextMenu(menu);
    m_tray->setToolTip("KForge GUI");
    m_tray->show();
}

void MainWindow::applyDarkTheme()
{
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #0d1117;
            color: #e6edf3;
            font-family: "JetBrains Mono", "Consolas", monospace;
            font-size: 13px;
        }
        QLabel { color: #e6edf3; }
        QLineEdit, QTextEdit, QComboBox {
            background: #161b22;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 6px 10px;
            color: #e6edf3;
            selection-background-color: #1f6feb;
        }
        QLineEdit:focus, QTextEdit:focus, QComboBox:focus {
            border-color: #388bfd;
        }
        QPushButton {
            background: #21262d;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 7px 16px;
            color: #e6edf3;
            font-weight: 600;
        }
        QPushButton:hover { background: #30363d; border-color: #8b949e; }
        QPushButton:pressed { background: #161b22; }
        QPushButton#primaryBtn {
            background: #1f6feb;
            border-color: #388bfd;
            color: #ffffff;
        }
        QPushButton#primaryBtn:hover { background: #388bfd; }
        QPushButton#dangerBtn {
            background: #da3633;
            border-color: #f85149;
            color: #ffffff;
        }
        QPushButton#dangerBtn:hover { background: #f85149; }
        QScrollArea { border: none; background: transparent; }
        QScrollBar:vertical {
            background: #161b22; width: 8px; border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #30363d; border-radius: 4px; min-height: 20px;
        }
        QScrollBar::handle:vertical:hover { background: #8b949e; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        QTableWidget {
            background: #161b22;
            border: 1px solid #30363d;
            border-radius: 8px;
            gridline-color: #21262d;
        }
        QTableWidget::item { padding: 8px; }
        QTableWidget::item:selected {
            background: #1f6feb;
            color: #ffffff;
        }
        QHeaderView::section {
            background: #21262d;
            border: none;
            border-bottom: 1px solid #30363d;
            padding: 8px;
            font-weight: 700;
            color: #8b949e;
            text-transform: uppercase;
            font-size: 11px;
            letter-spacing: 0.5px;
        }
        QProgressBar {
            background: #21262d;
            border: none;
            border-radius: 4px;
            height: 6px;
            text-align: center;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #1f6feb, stop:1 #3fb950);
            border-radius: 4px;
        }
        QCheckBox { color: #e6edf3; spacing: 8px; }
        QCheckBox::indicator {
            width: 16px; height: 16px;
            border: 1px solid #30363d;
            border-radius: 4px;
            background: #21262d;
        }
        QCheckBox::indicator:checked {
            background: #1f6feb;
            border-color: #388bfd;
            image: url(:/icons/check.svg);
        }
        QGroupBox {
            border: 1px solid #30363d;
            border-radius: 8px;
            margin-top: 12px;
            padding: 12px;
            font-weight: 700;
            color: #8b949e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }
        QSplitter::handle { background: #30363d; width: 1px; }
        QTreeWidget {
            background: #161b22;
            border: 1px solid #30363d;
            border-radius: 8px;
        }
        QTreeWidget::item { padding: 6px; }
        QTreeWidget::item:selected { background: #1f6feb; }
    )");
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void MainWindow::onSidebarNavChanged(int index)
{
    m_stack->setCurrentIndex(index);
    if (index == 0) m_dashboard->refresh();
    if (index == 1) m_buildPage->refreshJobs();
}

void MainWindow::onRefreshTimer()
{
    if (m_stack->currentIndex() == 0) m_dashboard->refresh();

    // pulse progress on running jobs
    int running = 0;
    for (auto &job : m_jobs) {
        if (job.status == "running") running++;
    }
    m_sidebar->setBadge(1, running);
}

void MainWindow::onProcessReadyRead()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;
    QString id = jobIdFromProcess(proc);
    if (id.isEmpty()) return;

    QString out = proc->readAllStandardOutput();
    QString err = proc->readAllStandardError();
    QString combined = out + err;

    m_jobs[id].logOutput += combined;
    emit jobLogAppended(id, combined);
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus)
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;
    QString id = jobIdFromProcess(proc);
    if (id.isEmpty()) return;

    QString status = (exitCode == 0) ? "success" : "failed";
    m_jobs[id].status = status;
    emit jobStatusChanged(id, status);

    // tray notification
    QString tag = m_jobs[id].tag;
    if (status == "success")
        m_tray->showMessage("Build Complete ✓", tag + " built successfully",
                            QSystemTrayIcon::Information, 4000);
    else
        m_tray->showMessage("Build Failed ✗", tag + " failed",
                            QSystemTrayIcon::Critical, 4000);
}

// ─── Public API ──────────────────────────────────────────────────────────────

void MainWindow::launchBuild(const BuildJob &job)
{
    BuildJob j = job;
    j.id        = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    j.status    = "running";
    j.startedAt = QDateTime::currentMSecsSinceEpoch();
    j.process   = new QProcess(this);

    connect(j.process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::onProcessReadyRead);
    connect(j.process, &QProcess::readyReadStandardError,
            this, &MainWindow::onProcessReadyRead);
    connect(j.process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &MainWindow::onProcessFinished);

    // Build argument list
    QStringList args;
    args << "build";
    args << "--progress" << j.progress;
    if (!j.platforms.isEmpty())
        args << "--platform" << j.platforms.join(",");
    args << "-t" << j.tag;
    args << j.context;

    j.process->start("kforge", args);
    m_jobs[j.id] = j;
    emit jobStatusChanged(j.id, "running");
}

void MainWindow::cancelBuild(const QString &jobId)
{
    if (!m_jobs.contains(jobId)) return;
    BuildJob &j = m_jobs[jobId];
    if (j.process && j.process->state() != QProcess::NotRunning) {
        j.process->terminate();
        j.process->waitForFinished(2000);
        j.process->kill();
    }
    j.status = "cancelled";
    emit jobStatusChanged(jobId, "cancelled");
}

QString MainWindow::jobIdFromProcess(QProcess *p) const
{
    for (auto it = m_jobs.cbegin(); it != m_jobs.cend(); ++it)
        if (it.value().process == p) return it.key();
    return {};
}
