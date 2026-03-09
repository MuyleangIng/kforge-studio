#pragma once
#include <QWidget>
#include <QMap>
#include "BuildJob.h"

class MainWindow;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QTextEdit;
class QLabel;
class JobCardWidget;

// ─────────────────────────────────────────────
// BuildJobWidget – "Build" page
// Shows a form to configure a new build + a
// live list of running / finished job cards.
// ─────────────────────────────────────────────
class BuildJobWidget : public QWidget {
    Q_OBJECT
public:
    explicit BuildJobWidget(MainWindow *mw, QWidget *parent = nullptr);

    void refreshJobs();

private slots:
    void onBuildClicked();
    void onBrowseContext();
    void onJobStatusChanged(const QString &id, const QString &status);
    void onJobLogAppended(const QString &id, const QString &line);

private:
    void buildForm(QVBoxLayout *layout);
    void buildJobList(QVBoxLayout *layout);

    MainWindow *m_mw = nullptr;

    // form fields
    QLineEdit *m_tagEdit      = nullptr;
    QLineEdit *m_contextEdit  = nullptr;
    QComboBox *m_progressBox  = nullptr;
    QCheckBox *m_amd64Check   = nullptr;
    QCheckBox *m_arm64Check   = nullptr;
    QCheckBox *m_arm7Check    = nullptr;
    QCheckBox *m_pushCheck    = nullptr;
    QLineEdit *m_cacheFromEdit= nullptr;
    QLineEdit *m_cacheToEdit  = nullptr;
    QLineEdit *m_buildArgEdit = nullptr;
    QLineEdit *m_secretEdit   = nullptr;
    QLineEdit *m_targetEdit   = nullptr;
    QPushButton *m_buildBtn   = nullptr;

    // job cards area
    QVBoxLayout *m_cardsLayout = nullptr;
    QScrollArea *m_scrollArea  = nullptr;
    QMap<QString, JobCardWidget*> m_cards;
};

// ─────────────────────────────────────────────
// JobCardWidget – one card per build job
// ─────────────────────────────────────────────
class QProgressBar;
class JobCardWidget : public QWidget {
    Q_OBJECT
public:
    explicit JobCardWidget(const BuildJob &job, QWidget *parent = nullptr);

    void appendLog(const QString &line);
    void setStatus(const QString &status);
    void pulseProgress();

private slots:
    void onToggleLog();
    void onCancel();

signals:
    void cancelRequested(const QString &jobId);

private:
    QString      m_jobId;
    QLabel      *m_titleLabel  = nullptr;
    QLabel      *m_statusLabel = nullptr;
    QProgressBar *m_progress   = nullptr;
    QTextEdit   *m_logView     = nullptr;
    QPushButton *m_toggleBtn   = nullptr;
    QPushButton *m_cancelBtn   = nullptr;
    bool         m_logVisible  = false;
    int          m_progressVal = 0;
};
