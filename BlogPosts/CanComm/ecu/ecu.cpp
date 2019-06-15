// Copyright (C) 2019, Burkhard Stubert (DBA Embedded Use)

#include <tuple>
#include <QCanBusFrame>
#include <QRandomGenerator>
#include <QString>
#include "ecu.h"

Ecu::Ecu(int ecuId, QSharedPointer<QCanBusDevice> canBus, QObject *parent)
    : EcuBase{ecuId, canBus, parent}
{
}

Ecu::~Ecu()
{
}

bool Ecu::isReadParameter(const QCanBusFrame &frame) const
{
    return frame.frameId() == 0x18ef0201U && frame.payload()[0] == char(1);
}

void Ecu::sendReadParameter(quint16 pid, quint32 value)
{
    emitReadParameterMessage(QStringLiteral("Ecu/Send"), pid, value);
    enqueueOutgoingFrame(QCanBusFrame(0x18ef0102U, encodedReadParameter(pid, value)));
}

void Ecu::receiveReadParameter(const QCanBusFrame &frame)
{
    quint16 pid = 0U;
    quint32 value = 0U;
    std::tie(pid, value) = decodedReadParameter(frame);
    if (isMissingResponsesEnabled() && pid % 8U == 0U) {
        return;
    }
    emitReadParameterMessage(QStringLiteral("Ecu/Recv"), pid, value);
    if (!isSkipResponseEnabled()) {
        sendReadParameter(pid, QRandomGenerator::global()->generate());
    }
}

bool Ecu::isSkipResponseEnabled() const
{
    return m_skipResponseEnabled;
}

void Ecu::setSkipResponseEnabled(bool enabled)
{
    if (m_skipResponseEnabled != enabled) {
        m_skipResponseEnabled = enabled;
        emit skipResponseEnabledChanged();
        if (m_skipResponseEnabled) {
            setMissingResponsesEnabled(false);
        }
    }
}

bool Ecu::isMissingResponsesEnabled() const
{
    return m_missingResponsesEnabled;
}

void Ecu::setMissingResponsesEnabled(bool enabled)
{
    if (m_missingResponsesEnabled != enabled) {
        m_missingResponsesEnabled = enabled;
        emit missingResponsesEnabledChanged();
        if (m_missingResponsesEnabled) {
            setSkipResponseEnabled(false);
        }
    }
}


