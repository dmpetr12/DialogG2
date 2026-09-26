#pragma once

#include "CabinetSnapshot.h"

#include <QVector>

namespace DialogG2 {

struct StateLogEvent
{
    QString message;
    bool warning = false;
};

class StateChangeTracker
{
public:
    QVector<StateLogEvent> update(const CabinetSnapshot &snapshot);

private:
    bool m_initialized = false;
    CabinetSnapshot m_previous;
};

} // namespace DialogG2
