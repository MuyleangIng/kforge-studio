#include "OtherWidgets.h"
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QScrollArea>
#include <QTreeWidget>
#include <QFrame>
#include <QProcess>
#include <QDateTime>

// ════════════════════════════════════════════════════════════
// BuilderManagerWidget
// ════════════════════════════════════════════════════════════

BuilderManagerWidget::BuilderManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(16);

    // Header
    QLabel *title = new QLabel("Builder Management", this);
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #e6edf3;");
    root->addWidget(title);
    QLabel *sub = new QLabel("Create, list, switch, and remove BuildKit builders", this);
    sub->setStyleSheet("color: #8b949e; font-size: 12px;");
    root->addWidget(sub);

    // Create new builder group
    QGroupBox *createBox = new QGroupBox("Create Builder", this);
    QGridLayout *grid = new QGridLayout(createBox);
    grid->setColumnStretch(1, 1);

    grid->addWidget(new QLabel("Name:", createBox), 0, 0);
    m_nameEdit = new QLineEdit(createBox);
    m_nameEdit->setPlaceholderText("mybuilder");
    grid->addWidget(m_nameEdit, 0, 1);

    grid->addWidget(new QLabel("Driver:", createBox), 1, 0);
    m_driverBox = new QComboBox(createBox);
    m_driverBox->addItems({"docker-container", "kubernetes", "remote"});
    grid->addWidget(m_driverBox, 1, 1);

    grid->addWidget(new QLabel("Endpoint:", createBox), 2, 0);
    m_endpointEdit = new QLineEdit(createBox);
    m_endpointEdit->setPlaceholderText("tcp://buildkitd:1234  (only for remote driver)");
    grid->addWidget(m_endpointEdit, 2, 1);

    m_createBtn = new QPushButton("＋  Create Builder", createBox);
    m_createBtn->setObjectName("primaryBtn");
    connect(m_createBtn, &QPushButton::clicked, this, &BuilderManagerWidget::onCreateBuilder);
    grid->addWidget(m_createBtn, 3, 0, 1, 2);

    root->addWidget(createBox);

    // Action buttons row
    QHBoxLayout *acts = new QHBoxLayout();
    m_refreshBtn = new QPushButton("↻  Refresh", this);
    m_useBtn     = new QPushButton("✓  Use Selected", this);
    m_rmBtn      = new QPushButton("🗑  Remove", this);
    m_rmBtn->setObjectName("dangerBtn");
    connect(m_refreshBtn, &QPushButton::clicked, this, &BuilderManagerWidget::onRefresh);
    connect(m_useBtn,     &QPushButton::clicked, this, &BuilderManagerWidget::onUseBuilder);
    connect(m_rmBtn,      &QPushButton::clicked, this, &BuilderManagerWidget::onRemoveBuilder);
    acts->addWidget(m_refreshBtn);
    acts->addWidget(m_useBtn);
    acts->addStretch();
    acts->addWidget(m_rmBtn);
    root->addLayout(acts);

    // Builder table
    m_table = new QTableWidget(0, 4, this);
    m_table->setHorizontalHeaderLabels({"Name", "Driver", "Status", "Endpoint"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet("alternate-background-color: #0d1117;");
    root->addWidget(m_table, 1);

    // Status bar
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("color: #8b949e; font-size: 11px;");
    root->addWidget(m_statusLabel);

    onRefresh();
}

void BuilderManagerWidget::runKforge(const QStringList &args)
{
    if (m_process && m_process->state() != QProcess::NotRunning) return;
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &BuilderManagerWidget::onProcessOutput);
    connect(m_process, qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
            this, &BuilderManagerWidget::onProcessDone);
    m_process->start("kforge", args);
    m_statusLabel->setText("Running: kforge " + args.join(" ") + " …");
}

void BuilderManagerWidget::onRefresh()
{
    // Simulate builder list (replace with actual kforge builder ls parsing)
    m_table->setRowCount(0);

    // In production, parse stdout of `kforge builder ls`
    // For now, show a placeholder row
    auto addRow = [&](const QString &name, const QString &driver,
                      const QString &status, const QString &endpoint) {
        int r = m_table->rowCount();
        m_table->insertRow(r);
        m_table->setItem(r, 0, new QTableWidgetItem(name));
        m_table->setItem(r, 1, new QTableWidgetItem(driver));
        QTableWidgetItem *st = new QTableWidgetItem(status);
        st->setForeground(status == "running" ? QColor("#3fb950") : QColor("#8b949e"));
        m_table->setItem(r, 2, st);
        m_table->setItem(r, 3, new QTableWidgetItem(endpoint));
    };

    addRow("default",    "docker",           "running", "unix:///var/run/docker.sock");
    addRow("multiarch",  "docker-container", "running", "-");

    m_statusLabel->setText("Refreshed at " + QDateTime::currentDateTime().toString("hh:mm:ss"));
}

void BuilderManagerWidget::onCreateBuilder()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) { m_statusLabel->setText("⚠  Builder name required"); return; }

    QStringList args{"builder", "create", "--name", name,
                     "--driver", m_driverBox->currentText()};
    if (!m_endpointEdit->text().isEmpty())
        args << "--endpoint" << m_endpointEdit->text().trimmed();
    runKforge(args);
}

void BuilderManagerWidget::onUseBuilder()
{
    QString name = selectedBuilderName();
    if (name.isEmpty()) return;
    runKforge({"builder", "use", name});
}

void BuilderManagerWidget::onRemoveBuilder()
{
    QString name = selectedBuilderName();
    if (name.isEmpty()) return;
    runKforge({"builder", "rm", name});
}

void BuilderManagerWidget::onProcessOutput()
{
    m_statusLabel->setText(m_process->readAllStandardOutput().trimmed());
}

void BuilderManagerWidget::onProcessDone(int exit, QProcess::ExitStatus)
{
    m_statusLabel->setText(exit == 0 ? "✓ Done" : "✗ Failed");
    onRefresh();
}

QString BuilderManagerWidget::selectedBuilderName() const
{
    auto items = m_table->selectedItems();
    if (items.isEmpty()) return {};
    return m_table->item(items.first()->row(), 0)->text();
}

// ════════════════════════════════════════════════════════════
// BakeWidget
// ════════════════════════════════════════════════════════════

BakeWidget::BakeWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(16);

    QLabel *title = new QLabel("Bake (Declarative Builds)", this);
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #e6edf3;");
    root->addWidget(title);

    QLabel *sub = new QLabel("Run kforge bake from a kforge.hcl or kforge.json config file", this);
    sub->setStyleSheet("color: #8b949e; font-size: 12px;");
    root->addWidget(sub);

    QGroupBox *box = new QGroupBox("Bake Configuration", this);
    QGridLayout *g = new QGridLayout(box);
    g->setColumnStretch(1, 1);

    auto addRow = [&](int row, const QString &lbl, QLineEdit *&field,
                      const QString &placeholder) {
        g->addWidget(new QLabel(lbl, box), row, 0);
        field = new QLineEdit(box);
        field->setPlaceholderText(placeholder);
        g->addWidget(field, row, 1);
    };

    // Bake file row (with browse)
    g->addWidget(new QLabel("Bake File:", box), 0, 0);
    QHBoxLayout *fileRow = new QHBoxLayout();
    m_fileEdit = new QLineEdit(box);
    m_fileEdit->setPlaceholderText("kforge.hcl  (leave empty for auto-detect)");
    QPushButton *browseBtn = new QPushButton("Browse", box);
    connect(browseBtn, &QPushButton::clicked, this, &BakeWidget::onBrowseFile);
    fileRow->addWidget(m_fileEdit, 1);
    fileRow->addWidget(browseBtn);
    g->addLayout(fileRow, 0, 1);

    addRow(1, "Target:",   m_targetEdit, "app  (leave empty for 'default' group)");
    addRow(2, "--set:",    m_setEdit,    "app.platforms=linux/arm64");
    addRow(3, "Env vars:", m_envEdit,    "TAG=1.2.3  KEY=VAL …");

    root->addWidget(box);

    QHBoxLayout *acts = new QHBoxLayout();
    m_bakeBtn = new QPushButton("▶  Run Bake", this);
    m_bakeBtn->setObjectName("primaryBtn");
    m_bakeBtn->setFixedHeight(40);
    connect(m_bakeBtn, &QPushButton::clicked, this, &BakeWidget::onBake);
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("color: #8b949e; font-size: 12px;");
    acts->addWidget(m_bakeBtn);
    acts->addStretch();
    acts->addWidget(m_statusLabel);
    root->addLayout(acts);

    // Log output
    QLabel *logLbl = new QLabel("Build Output", this);
    logLbl->setStyleSheet("color: #8b949e; font-size: 11px; font-weight: 700;");
    root->addWidget(logLbl);

    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet(R"(
        QTextEdit {
            background: #010409;
            border: 1px solid #21262d;
            border-radius: 8px;
            font-family: 'JetBrains Mono','Consolas',monospace;
            font-size: 12px;
            color: #3fb950;
            padding: 12px;
        }
    )");
    root->addWidget(m_logView, 1);
}

void BakeWidget::onBrowseFile()
{
    QString f = QFileDialog::getOpenFileName(this, "Select Bake File", ".",
                                             "Bake files (*.hcl *.json);;All (*)");
    if (!f.isEmpty()) m_fileEdit->setText(f);
}

void BakeWidget::onBake()
{
    m_logView->clear();
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &BakeWidget::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &BakeWidget::onProcessOutput);
    connect(m_process, qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
            this, &BakeWidget::onProcessDone);

    QStringList args{"bake"};
    if (!m_fileEdit->text().isEmpty()) args << "-f" << m_fileEdit->text().trimmed();
    if (!m_targetEdit->text().isEmpty()) args << m_targetEdit->text().trimmed();
    if (!m_setEdit->text().isEmpty())  args << "--set" << m_setEdit->text().trimmed();

    // Pass env vars
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for (const QString &kv : m_envEdit->text().split(' ', Qt::SkipEmptyParts)) {
        QStringList parts = kv.split('=');
        if (parts.size() == 2) env.insert(parts[0], parts[1]);
    }
    m_process->setProcessEnvironment(env);
    m_process->start("kforge", args);

    m_bakeBtn->setEnabled(false);
    m_statusLabel->setText("● Running…");
    m_statusLabel->setStyleSheet("color: #f0883e; font-size: 12px;");
}

void BakeWidget::onProcessOutput()
{
    QString out = m_process->readAllStandardOutput();
    QString err = m_process->readAllStandardError();
    m_logView->append((out + err).trimmed());
}

void BakeWidget::onProcessDone(int exit, QProcess::ExitStatus)
{
    m_bakeBtn->setEnabled(true);
    if (exit == 0) {
        m_statusLabel->setText("✓ Bake complete");
        m_statusLabel->setStyleSheet("color: #3fb950; font-size: 12px;");
    } else {
        m_statusLabel->setText("✗ Bake failed");
        m_statusLabel->setStyleSheet("color: #f85149; font-size: 12px;");
    }
}

// ════════════════════════════════════════════════════════════
// SettingsWidget
// ════════════════════════════════════════════════════════════

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(16);

    QLabel *title = new QLabel("Settings", this);
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #e6edf3;");
    root->addWidget(title);

    QGroupBox *genBox = new QGroupBox("General", this);
    QGridLayout *grid = new QGridLayout(genBox);
    grid->setColumnStretch(1, 1);

    grid->addWidget(new QLabel("kforge binary path:", genBox), 0, 0);
    m_kforgePath = new QLineEdit(genBox);
    m_kforgePath->setPlaceholderText("/usr/local/bin/kforge");
    m_kforgePath->setText("/usr/local/bin/kforge");
    grid->addWidget(m_kforgePath, 0, 1);

    grid->addWidget(new QLabel("Default progress:", genBox), 1, 0);
    m_defaultProgress = new QComboBox(genBox);
    m_defaultProgress->addItems({"auto", "plain", "tty", "spinner", "bar", "dots"});
    grid->addWidget(m_defaultProgress, 1, 1);

    grid->addWidget(new QLabel("Default platform:", genBox), 2, 0);
    m_defaultPlatform = new QComboBox(genBox);
    m_defaultPlatform->addItems({"linux/amd64", "linux/amd64,linux/arm64",
                                  "linux/amd64,linux/arm64,linux/arm/v7"});
    grid->addWidget(m_defaultPlatform, 2, 1);

    grid->addWidget(new QLabel("Default registry:", genBox), 3, 0);
    m_registryEdit = new QLineEdit(genBox);
    m_registryEdit->setPlaceholderText("docker.io");
    grid->addWidget(m_registryEdit, 3, 1);

    root->addWidget(genBox);

    // Setup wizard
    QGroupBox *wizBox = new QGroupBox("Multi-platform Setup Wizard", this);
    QVBoxLayout *wl = new QVBoxLayout(wizBox);
    QLabel *wizInfo = new QLabel(
        "Run 'kforge setup' to configure QEMU emulation or native multi-node builders.\n"
        "Output is shown live below.", wizBox);
    wizInfo->setStyleSheet("color: #8b949e; font-size: 12px;");
    wl->addWidget(wizInfo);

    m_setupBtn = new QPushButton("▶  Launch Setup Wizard", wizBox);
    m_setupBtn->setObjectName("primaryBtn");
    m_setupBtn->setFixedHeight(38);
    connect(m_setupBtn, &QPushButton::clicked, this, &SettingsWidget::onRunSetupWizard);
    wl->addWidget(m_setupBtn);

    m_wizardLog = new QTextEdit(wizBox);
    m_wizardLog->setReadOnly(true);
    m_wizardLog->setFixedHeight(180);
    m_wizardLog->setStyleSheet(R"(
        QTextEdit {
            background: #010409;
            border: 1px solid #21262d;
            border-radius: 8px;
            font-family: 'JetBrains Mono','Consolas',monospace;
            font-size: 11px;
            color: #3fb950;
            padding: 10px;
        }
    )");
    wl->addWidget(m_wizardLog);
    root->addWidget(wizBox);

    QPushButton *saveBtn = new QPushButton("💾  Save Settings", this);
    saveBtn->setObjectName("primaryBtn");
    saveBtn->setFixedHeight(40);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsWidget::onSave);
    root->addWidget(saveBtn);
    root->addStretch();
}

void SettingsWidget::onSave()
{
    // TODO: persist to QSettings
}

void SettingsWidget::onRunSetupWizard()
{
    m_wizardLog->clear();
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &SettingsWidget::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &SettingsWidget::onProcessOutput);
    connect(m_process, qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
            this, &SettingsWidget::onProcessDone);
    m_process->start(m_kforgePath->text(), {"setup"});
    m_setupBtn->setEnabled(false);
}

void SettingsWidget::onProcessOutput()
{
    m_wizardLog->append(m_process->readAll().trimmed());
}

void SettingsWidget::onProcessDone(int, QProcess::ExitStatus)
{
    m_setupBtn->setEnabled(true);
}

// ════════════════════════════════════════════════════════════
// DashboardWidget
// ════════════════════════════════════════════════════════════

DashboardWidget::DashboardWidget(MainWindow *mw, QWidget *parent)
    : QWidget(parent), m_mw(mw)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(20);

    // Header
    QLabel *title = new QLabel("Dashboard", this);
    title->setStyleSheet("font-size: 24px; font-weight: 900; color: #e6edf3;"
                         " letter-spacing: -0.5px;");
    root->addWidget(title);
    QLabel *sub = new QLabel("Session overview — kforge multi-platform build studio", this);
    sub->setStyleSheet("color: #8b949e; font-size: 13px; margin-bottom: 8px;");
    root->addWidget(sub);

    // ── Stat cards row ────────────────────────────
    QHBoxLayout *cards = new QHBoxLayout();
    cards->setSpacing(16);

    auto makeCard = [&](const QString &label, QLabel *&valLabel,
                        const QString &color) -> QWidget* {
        QWidget *card = new QWidget(this);
        card->setStyleSheet(QString(R"(
            QWidget {
                background: #161b22;
                border: 1px solid #30363d;
                border-radius: 12px;
            }
        )"));
        QVBoxLayout *vl = new QVBoxLayout(card);
        vl->setContentsMargins(20, 18, 20, 18);
        vl->setSpacing(4);
        valLabel = new QLabel("0", card);
        valLabel->setStyleSheet(QString("font-size: 40px; font-weight: 900; color: %1;").arg(color));
        QLabel *lbl = new QLabel(label, card);
        lbl->setStyleSheet("color: #8b949e; font-size: 12px; font-weight: 600;");
        vl->addWidget(valLabel);
        vl->addWidget(lbl);
        return card;
    };

    cards->addWidget(makeCard("TOTAL BUILDS",   m_total,   "#e6edf3"), 1);
    cards->addWidget(makeCard("RUNNING",        m_running, "#f0883e"), 1);
    cards->addWidget(makeCard("SUCCEEDED",      m_success, "#3fb950"), 1);
    cards->addWidget(makeCard("FAILED",         m_failed,  "#f85149"), 1);
    root->addLayout(cards);

    // ── Recent jobs ───────────────────────────────
    QLabel *recentTitle = new QLabel("Recent Jobs", this);
    recentTitle->setStyleSheet("font-size: 15px; font-weight: 800; color: #e6edf3;");
    root->addWidget(recentTitle);

    m_recentTree = new QTreeWidget(this);
    m_recentTree->setHeaderLabels({"ID", "Tag", "Platforms", "Status", "Started"});
    m_recentTree->header()->setSectionResizeMode(QHeaderView::Stretch);
    m_recentTree->setRootIsDecorated(false);
    m_recentTree->setAlternatingRowColors(true);
    m_recentTree->setStyleSheet("alternate-background-color: #0d1117;");
    root->addWidget(m_recentTree, 1);

    // Quick start tips
    QGroupBox *tips = new QGroupBox("Quick Start", this);
    QVBoxLayout *tl = new QVBoxLayout(tips);
    QLabel *t1 = new QLabel("1. Go to <b>Build</b> → fill in image tag + context → Launch Build", tips);
    QLabel *t2 = new QLabel("2. Add <b>linux/arm64</b> for multi-platform (Apple Silicon / AWS Graviton)", tips);
    QLabel *t3 = new QLabel("3. Use <b>Bake</b> with a kforge.hcl file for declarative multi-target builds", tips);
    QLabel *t4 = new QLabel("4. Run <b>Settings → Setup Wizard</b> to configure QEMU emulation", tips);
    for (auto *l : {t1, t2, t3, t4}) {
        l->setStyleSheet("color: #8b949e; font-size: 12px; padding: 2px 0;");
        tl->addWidget(l);
    }
    root->addWidget(tips);
}

void DashboardWidget::refresh()
{
    const auto &jobs = m_mw->jobs();
    int running = 0, success = 0, failed = 0;
    for (const auto &j : jobs) {
        if (j.status == "running")  running++;
        if (j.status == "success")  success++;
        if (j.status == "failed")   failed++;
    }
    m_total->setText(QString::number(jobs.size()));
    m_running->setText(QString::number(running));
    m_success->setText(QString::number(success));
    m_failed->setText(QString::number(failed));

    m_recentTree->clear();
    // Build list in reverse order (most recent first)
    QList<BuildJob> jobList = jobs.values();
    for (int i = jobList.size() - 1; i >= 0; --i) {
        const BuildJob &j = jobList[i];
        QTreeWidgetItem *item = new QTreeWidgetItem(m_recentTree);
        item->setText(0, j.id.left(8));
        item->setText(1, j.tag);
        item->setText(2, j.platforms.join(", "));
        item->setText(3, j.status.toUpper());
        item->setText(4, QDateTime::fromMSecsSinceEpoch(j.startedAt)
                          .toString("hh:mm:ss"));
        QColor statusColor =
            j.status == "success"  ? QColor("#3fb950") :
            j.status == "failed"   ? QColor("#f85149") :
            j.status == "running"  ? QColor("#f0883e") : QColor("#8b949e");
        item->setForeground(3, statusColor);
    }
}