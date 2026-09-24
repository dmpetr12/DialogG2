#pragma once

#include "engine/AppConfig.h"

#include <QObject>
#include <QString>

#include <memory>

QString defaultStatePath();
QString defaultLogPath();
QString defaultAppConfigPath();

class EngineRuntime : public QObject
{
public:
    EngineRuntime(DialogG2::AppConfig config, QString statePath, bool demoMode, QObject *parent = nullptr);
    ~EngineRuntime() override;

    void start();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
