#pragma once

#include <QByteArray>
#include <QVector>

namespace DialogG2 {

class ModbusRtuCodec
{
public:
    static QByteArray readRequest(int slaveAddress, quint8 function, int start, int count);
    static bool validateCrc(const QByteArray &frame);
    static QVector<quint16> registersFromReadResponse(const QByteArray &frame);
    static quint8 bitsFromReadResponse(const QByteArray &frame);
    static int expectedReadResponseSize(int count, bool bits);

private:
    static quint16 crc(const QByteArray &bytes);
    static void appendCrc(QByteArray *frame);
};

} // namespace DialogG2
