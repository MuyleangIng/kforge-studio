#include "BuildJobWidget.h"
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QDateTime>
#include <QSplitter>
#include <QFrame>
#include <QTimer>

// ════════════════════════════════════════════════════════════
// BuildJobWidget
// ════════════════════════════════════════════════════════════

BuildJobWidget::BuildJobWidget(MainWindow *mw, QWidget *parent)
    : QWidget(parent), m_mw(mw)
{
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);

    // ── Left: build form ──────────────────────────
    QWidget *formPanel = new QWidget(splitter);
    formPanel->setMinimumWidth(360);
    formPanel->setMaximumWidth(480);
    QVBoxLayout *formLayout = new QVBoxLayout(formPanel);
    formLayout->setContentsMargins(24, 24, 12, 24);
    formLayout->setSpacing(16);
    buildForm(formLayout);

    // ── Right: jobs list ──────────────────────────
    QWidget *jobsPanel = new QWidget(splitter);
    QVBoxLayout *jobsLayout = new QVBoxLayout(jobsPanel);
    jobsLayout->setContentsMargins(12, 24, 24, 24);
    jobsLayout->setSpacing(12);
    buildJobList(jobsLayout);

    splitter->addWidget(formPanel);
    splitter->addWidget(jobsPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(splitter);

    // Connect main window signals
    connect(mw, &MainWindow::jobStatusChanged,
            this, &BuildJobWidget::onJobStatusChanged);
    connect(mw, &MainWindow::jobLogAppended,
            this, &BuildJobWidget::onJobLogAppended);
}

void BuildJobWidget::buildForm(QVBoxLayout *layout)
{
    // Header
    QLabel *header = new QLabel("New Build", this);
    header->setStyleSheet("font-size: 20px; font-weight: 800; color: #e6edf3;");
    layout->addWidget(header);

    QLabel *sub = new QLabel("Configure and launch a kforge build", this);
    sub->setStyleSheet("color: #8b949e; font-size: 12px;");
    layout->addWidget(sub);

    // Image tag
    QLabel *tagLbl = new QLabel("Image Tag *", this);
    tagLbl->setStyleSheet("color: #8b949e; font-size: 11px; font-weight: 700; letter-spacing: 0.5px;");
    layout->addWidget(tagLbl);
    m_tagEdit = new QLineEdit(this);
    m_tagEdit->setPlaceholderText("e.g. myuser/myapp:latest");
    layout->addWidget(m_tagEdit);

    // Context dir
    QLabel *ctxLbl = new QLabel("Build Context *", this);
    ctxLbl->setStyleSheet("color: #8b949e; font-size: 11px; font-weight: 700; letter-spacing: 0.5px;");
    layout->addWidget(ctxLbl);
    QHBoxLayout *ctxRow = new QHBoxLayout();
    m_contextEdit = new QLineEdit(this);
    m_contextEdit->setPlaceholderText("Path to Dockerfile directory");
    m_contextEdit->setText(".");
    QPushButton *browseBtn = new QPushButton("Browse", this);
    connect(browseBtn, &QPushButton::clicked, this, &BuildJobWidget::onBrowseContext);
    ctxRow->addWidget(m_contextEdit, 1);
    ctxRow->addWidget(browseBtn);
    layout->addLayout(ctxRow);

    // Platforms
    QGroupBox *platformGroup = new QGroupBox("Target Platforms", this);
    QVBoxLayout *platLayout = new QVBoxLayout(platformGroup);
    m_amd64Check = new QCheckBox("linux/amd64  (x86_64)", platformGroup);
    m_arm64Check = new QCheckBox("linux/arm64  (Apple Silicon / AWS Graviton)", platformGroup);
    m_arm7Check  = new QCheckBox("linux/arm/v7 (Raspberry Pi)", platformGroup);
    m_amd64Check->setChecked(true);
    platLayout->addWidget(m_amd64Check);
    platLayout->addWidget(m_arm64Check);
    platLayout->addWidget(m_arm7Check);
    layout->addWidget(platformGroup);

    // Progress style
    QLabel *progLbl = new QLabel("Progress Style", this);
    progLbl->setStyleSheet("color: #8b949e; font-size: 11px; font-weight: 700; letter-spacing: 0.5px;");
    layout->addWidget(progLbl);
    m_progressBox = new QComboBox(this);
    m_progressBox->addItems({"auto", "plain", "tty"});
    layout->addWidget(m_progressBox);

    // Optional section
    QGroupBox *optGroup = new QGroupBox("Optional", this);
    QGridLayout *optGrid = new QGridLayout(optGroup);
    optGrid->setColumnStretch(1, 1);

    auto addOpt = [&](int row, const QString &lbl, QLineEdit *&field,
                      const QString &placeholder) {
        optGrid->addWidget(new QLabel(lbl, optGroup), row, 0);
        field = new QLineEdit(optGroup);
        field->setPlaceholderText(placeholder);
        optGrid->addWidget(field, row, 1);
    };

    addOpt(0, "Cache From", m_cacheFromEdit, "type=registry,ref=user/app:cache");
    addOpt(1, "Cache To",   m_cacheToEdit,   "type=registry,ref=user/app:cache,mode=max");
    addOpt(2, "Build Arg",  m_buildArgEdit,  "KEY=VALUE");
    addOpt(3, "Secret",     m_secretEdit,    "id=mysecret,src=./token.txt");
    addOpt(4, "Target Stage", m_targetEdit,  "release");

    m_pushCheck = new QCheckBox("Push to registry after build", optGroup);
    optGrid->addWidget(m_pushCheck, 5, 0, 1, 2);

    layout->addWidget(optGroup);

    layout->addStretch();

    // Build button
    m_buildBtn = new QPushButton("▶  Launch Build", this);
    m_buildBtn->setObjectName("primaryBtn");
    m_buildBtn->setFixedHeight(44);
    m_buildBtn->setStyleSheet(m_buildBtn->styleSheet() +
                              "font-size: 14px; font-weight: 700;");
    connect(m_buildBtn, &QPushButton::clicked, this, &BuildJobWidget::onBuildClicked);
    layout->addWidget(m_buildBtn);
}

void BuildJobWidget::buildJobList(QVBoxLayout *layout)
{
    QHBoxLayout *hdr = new QHBoxLayout();
    QLabel *title = new QLabel("Build Jobs", this);
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #e6edf3;");
    hdr->addWidget(title, 1);

    QPushButton *clearBtn = new QPushButton("Clear Finished", this);
    clearBtn->setStyleSheet("font-size: 11px; padding: 4px 10px;");
    connect(clearBtn, &QPushButton::clicked, [this]() {
        for (auto it = m_cards.begin(); it != m_cards.end(); ) {
            const QString &id = it.key();
            const QString &status = m_mw->jobs().value(id).status;
            if (status == "success" || status == "failed" || status == "cancelled") {
                delete it.value();
                it = m_cards.erase(it);
            } else ++it;
        }
    });
    hdr->addWidget(clearBtn);
    layout->addLayout(hdr);

    QLabel *sub = new QLabel("Live status for all builds in this session", this);
    sub->setStyleSheet("color: #8b949e; font-size: 12px; margin-bottom: 8px;");
    layout->addWidget(sub);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    m_cardsLayout = new QVBoxLayout(scrollContent);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(12);
    m_cardsLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    layout->addWidget(m_scrollArea, 1);
}

void BuildJobWidget::refreshJobs()
{
    // Rebuild cards for any jobs not yet shown
    const auto &jobs = m_mw->jobs();
    for (auto it = jobs.cbegin(); it != jobs.cend(); ++it) {
        if (!m_cards.contains(it.key())) {
            JobCardWidget *card = new JobCardWidget(it.value(), this);
            connect(card, &JobCardWidget::cancelRequested,
                    m_mw, &MainWindow::cancelBuild);
            // Insert before the stretch item
            m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, card);
            m_cards[it.key()] = card;
        }
    }
}

void BuildJobWidget::onBuildClicked()
{
    if (m_tagEdit->text().trimmed().isEmpty()) {
        m_tagEdit->setStyleSheet("border-color: #f85149;");
        return;
    }
    m_tagEdit->setStyleSheet("");

    BuildJob job;
    job.tag      = m_tagEdit->text().trimmed();
    job.context  = m_contextEdit->text().trimmed();
    job.progress = m_progressBox->currentText();

    if (m_amd64Check->isChecked()) job.platforms << "linux/amd64";
    if (m_arm64Check->isChecked()) job.platforms << "linux/arm64";
    if (m_arm7Check->isChecked())  job.platforms << "linux/arm/v7";

    m_mw->launchBuild(job);
    refreshJobs();
}

void BuildJobWidget::onBrowseContext()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Build Context",
                                                    m_contextEdit->text());
    if (!dir.isEmpty()) m_contextEdit->setText(dir);
}

void BuildJobWidget::onJobStatusChanged(const QString &id, const QString &status)
{
    if (m_cards.contains(id)) m_cards[id]->setStatus(status);
}

void BuildJobWidget::onJobLogAppended(const QString &id, const QString &line)
{
    if (m_cards.contains(id)) m_cards[id]->appendLog(line);
}

// ════════════════════════════════════════════════════════════
// JobCardWidget
// ════════════════════════════════════════════════════════════

JobCardWidget::JobCardWidget(const BuildJob &job, QWidget *parent)
    : QWidget(parent), m_jobId(job.id)
{
    setStyleSheet(R"(
        JobCardWidget {
            background: #161b22;
            border: 1px solid #30363d;
            border-radius: 10px;
        }
        JobCardWidget:hover { border-color: #8b949e; }
    )");

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(16, 14, 16, 14);
    vl->setSpacing(10);

    // Header row
    QHBoxLayout *hdr = new QHBoxLayout();
    QLabel *idLabel = new QLabel("  #" + job.id.left(8), this);
    idLabel->setStyleSheet("color: #30363d; font-size: 11px;");

    m_titleLabel = new QLabel(job.tag, this);
    m_titleLabel->setStyleSheet("font-weight: 700; font-size: 14px; color: #e6edf3;");

    m_statusLabel = new QLabel("● RUNNING", this);
    m_statusLabel->setStyleSheet("color: #f0883e; font-size: 11px; font-weight: 700;");

    m_cancelBtn = new QPushButton("✕ Cancel", this);
    m_cancelBtn->setObjectName("dangerBtn");
    m_cancelBtn->setFixedHeight(28);
    m_cancelBtn->setStyleSheet("font-size: 11px; padding: 2px 10px;");
    connect(m_cancelBtn, &QPushButton::clicked, this, &JobCardWidget::onCancel);

    m_toggleBtn = new QPushButton("▾ Logs", this);
    m_toggleBtn->setFixedHeight(28);
    m_toggleBtn->setStyleSheet("font-size: 11px; padding: 2px 10px;");
    connect(m_toggleBtn, &QPushButton::clicked, this, &JobCardWidget::onToggleLog);

    hdr->addWidget(m_titleLabel, 1);
    hdr->addWidget(idLabel);
    hdr->addWidget(m_statusLabel);
    hdr->addWidget(m_toggleBtn);
    hdr->addWidget(m_cancelBtn);
    vl->addLayout(hdr);

    // Platforms
    if (!job.platforms.isEmpty()) {
        QLabel *plat = new QLabel("🖥  " + job.platforms.join("   "), this);
        plat->setStyleSheet("color: #8b949e; font-size: 11px;");
        vl->addWidget(plat);
    }

    // Progress bar
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 0); // indeterminate (busy)
    m_progress->setFixedHeight(6);
    m_progress->setTextVisible(false);
    vl->addWidget(m_progress);

    // Log view (collapsed by default)
    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setFixedHeight(200);
    m_logView->setStyleSheet(R"(
        QTextEdit {
            background: #010409;
            border: 1px solid #21262d;
            border-radius: 6px;
            font-family: 'JetBrains Mono', 'Consolas', monospace;
            font-size: 11px;
            color: #3fb950;
            padding: 8px;
        }
    )");
    m_logView->hide();
    vl->addWidget(m_logView);
}

void JobCardWidget::appendLog(const QString &line)
{
    m_logView->append(line.trimmed());
    QTextCursor c = m_logView->textCursor();
    c.movePosition(QTextCursor::End);
    m_logView->setTextCursor(c);
    pulseProgress();
}

void JobCardWidget::setStatus(const QString &status)
{
    if (status == "success") {
        m_statusLabel->setText("✓ SUCCESS");
        m_statusLabel->setStyleSheet("color: #3fb950; font-size: 11px; font-weight: 700;");
        m_progress->setRange(0, 100);
        m_progress->setValue(100);
        m_cancelBtn->hide();
        setStyleSheet(R"(
            JobCardWidget {
                background: #161b22;
                border: 1px solid #238636;
                border-radius: 10px;
            }
        )");
    } else if (status == "failed") {
        m_statusLabel->setText("✗ FAILED");
        m_statusLabel->setStyleSheet("color: #f85149; font-size: 11px; font-weight: 700;");
        m_progress->setRange(0, 100);
        m_progress->setValue(100);
        m_progress->setStyleSheet("QProgressBar::chunk { background: #da3633; }");
        m_cancelBtn->hide();
        setStyleSheet(R"(
            JobCardWidget {
                background: #161b22;
                border: 1px solid #da3633;
                border-radius: 10px;
            }
        )");
    } else if (status == "cancelled") {
        m_statusLabel->setText("⊘ CANCELLED");
        m_statusLabel->setStyleSheet("color: #8b949e; font-size: 11px; font-weight: 700;");
        m_progress->setRange(0, 100);
        m_progress->setValue(0);
        m_cancelBtn->hide();
    }
}

void JobCardWidget::pulseProgress()
{
    // just keep it spinning (range 0,0 = indeterminate)
}

void JobCardWidget::onToggleLog()
{
    m_logVisible = !m_logVisible;
    m_logView->setVisible(m_logVisible);
    m_toggleBtn->setText(m_logVisible ? "▴ Logs" : "▾ Logs");
}

void JobCardWidget::onCancel()
{
    emit cancelRequested(m_jobId);
}
