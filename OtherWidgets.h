#pragma once
#include "BuildJob.h"
#include <QWidget>
#include <QProcess>
#include "OtherWidgets.h"

class QTextEdit;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;
class QTreeWidget;

// ─────────────────────────────────────────────
// BakeWidget – declarative bake builds
// ─────────────────────────────────────────────
class BakeWidget : public QWidget {
    Q_OBJECT
public:
    explicit BakeWidget(QWidget *parent = nullptr);

private slots:
    void onBrowseFile();
    void onBake();
    void onProcessOutput();
    void onProcessDone(int exit, QProcess::ExitStatus);

private:
    QLineEdit   *m_fileEdit   = nullptr;
    QLineEdit   *m_targetEdit = nullptr;
    QLineEdit   *m_setEdit    = nullptr;
    QLineEdit   *m_envEdit    = nullptr;
    QTextEdit   *m_logView    = nullptr;
    QPushButton *m_bakeBtn    = nullptr;
    QLabel      *m_statusLabel= nullptr;
    QProcess    *m_process    = nullptr;
};

// ─────────────────────────────────────────────
// SettingsWidget – kforge global prefs
// ─────────────────────────────────────────────
class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = nullptr);

private slots:
    void onSave();
    void onRunSetupWizard();
    void onProcessOutput();
    void onProcessDone(int, QProcess::ExitStatus);

private:
    QLineEdit   *m_kforgePath  = nullptr;
    QComboBox   *m_defaultProgress = nullptr;
    QComboBox   *m_defaultPlatform = nullptr;
    QLineEdit   *m_registryEdit= nullptr;
    QTextEdit   *m_wizardLog   = nullptr;
    QPushButton *m_setupBtn    = nullptr;
    QProcess    *m_process     = nullptr;
};

// ─────────────────────────────────────────────
// DashboardWidget – overview / quick stats
// ─────────────────────────────────────────────
class MainWindow;
class DashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(MainWindow *mw, QWidget *parent = nullptr);
    void refresh();

private:
    MainWindow *m_mw   = nullptr;
    QLabel *m_running  = nullptr;
    QLabel *m_success  = nullptr;
    QLabel *m_failed   = nullptr;
    QLabel *m_total    = nullptr;
    QTreeWidget *m_recentTree = nullptr;
};
