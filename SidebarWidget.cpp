#include "SidebarWidget.h"
#include <QFrame>
#include <QSpacerItem>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(220);
    setObjectName("sidebar");
    setStyleSheet(R"(
        #sidebar {
            background: #010409;
            border-right: 1px solid #21262d;
        }
        QPushButton#navBtn {
            background: transparent;
            border: none;
            border-radius: 8px;
            padding: 10px 14px;
            text-align: left;
            color: #8b949e;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton#navBtn:hover {
            background: #161b22;
            color: #e6edf3;
        }
        QPushButton#navBtn:checked {
            background: #1f6feb22;
            color: #388bfd;
            border-left: 3px solid #388bfd;
        }
        QLabel#logoLabel {
            color: #388bfd;
            font-size: 22px;
            font-weight: 900;
            letter-spacing: -0.5px;
            padding: 20px 16px 8px 16px;
        }
        QLabel#logoSub {
            color: #30363d;
            font-size: 10px;
            letter-spacing: 2px;
            padding: 0 16px 20px 16px;
            text-transform: uppercase;
        }
        QFrame#divider {
            color: #21262d;
            margin: 8px 12px;
        }
        QLabel#badge {
            background: #1f6feb;
            color: white;
            border-radius: 10px;
            font-size: 10px;
            font-weight: 700;
            padding: 1px 7px;
            min-width: 18px;
        }
        QLabel#sectionLabel {
            color: #30363d;
            font-size: 10px;
            letter-spacing: 1.5px;
            padding: 12px 16px 4px 16px;
            font-weight: 700;
        }
    )");

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 0, 8, 16);
    m_layout->setSpacing(2);

    // Logo
    QLabel *logo = new QLabel("⚡ KForge", this);
    logo->setObjectName("logoLabel");
    logo->setFixedHeight(50);
    m_layout->addWidget(logo);

    QLabel *sub = new QLabel("CONTAINER BUILD STUDIO", this);
    sub->setObjectName("logoSub");
    m_layout->addWidget(sub);

    // Divider
    QFrame *div = new QFrame(this);
    div->setObjectName("divider");
    div->setFrameShape(QFrame::HLine);
    m_layout->addWidget(div);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);
    connect(m_group, qOverload<int>(&QButtonGroup::idClicked),
            this, &SidebarWidget::navigationChanged);

    // Section: Main
    QLabel *mainSec = new QLabel("MAIN", this);
    mainSec->setObjectName("sectionLabel");
    m_layout->addWidget(mainSec);

    addNavButton("🏠", "  Dashboard",  0);
    addNavButton("🔨", "  Build",      1);
    addNavButton("📦", "  Bake",       3);

    // Section: Config
    QLabel *confSec = new QLabel("CONFIG", this);
    confSec->setObjectName("sectionLabel");
    m_layout->addWidget(confSec);

    addNavButton("🛠", "  Builders",   2);
    addNavButton("⚙️",  "  Settings",  4);

    m_layout->addStretch();

    // Version label at bottom
    QLabel *ver = new QLabel("v1.0.0  |  kforge", this);
    ver->setAlignment(Qt::AlignCenter);
    ver->setStyleSheet("color: #30363d; font-size: 11px; padding: 8px;");
    m_layout->addWidget(ver);

    // Activate first button
    if (auto *btn = qobject_cast<QPushButton*>(m_group->button(0)))
        btn->setChecked(true);
}

void SidebarWidget::addNavButton(const QString &icon, const QString &label, int index)
{
    QWidget *row = new QWidget(this);
    row->setStyleSheet("background: transparent;");
    QHBoxLayout *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(0);

    QPushButton *btn = new QPushButton(icon + label, row);
    btn->setObjectName("navBtn");
    btn->setCheckable(true);
    btn->setFlat(true);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setFixedHeight(40);
    m_group->addButton(btn, index);
    hl->addWidget(btn, 1);

    QLabel *badge = new QLabel("", row);
    badge->setObjectName("badge");
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedSize(22, 18);
    badge->hide();
    hl->addWidget(badge);
    hl->addSpacing(4);

    while (m_badges.size() <= index) m_badges.append(nullptr);
    m_badges[index] = badge;

    m_layout->addWidget(row);
}

void SidebarWidget::setBadge(int index, int count)
{
    if (index >= m_badges.size() || !m_badges[index]) return;
    QLabel *badge = m_badges[index];
    if (count <= 0) {
        badge->hide();
    } else {
        badge->setText(QString::number(count));
        badge->show();
    }
}
