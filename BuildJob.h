#pragma once
#include <QString>
#include <QStringList>
#include <QProcess>

struct BuildJob {
    QString id;
    QString tag;
    QString context;
    QStringList platforms;
    QString progress;
    QProcess *process = nullptr;
    QString status;
    qint64  startedAt = 0;
    QString logOutput;
};