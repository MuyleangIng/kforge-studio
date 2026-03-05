#pragma once
#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class SidebarWidget : public QWidget {
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

signals:
    void navigationChanged(int index);

public slots:
    void setBadge(int index, int count);

private:
    void addNavButton(const QString &icon, const QString &label, int index);
    QButtonGroup  *m_group  = nullptr;
    QVBoxLayout   *m_layout = nullptr;
    QList<QLabel*> m_badges;
};
