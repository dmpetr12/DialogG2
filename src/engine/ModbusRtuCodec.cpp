#include "ModbusRtuCodec.h"

namespace DialogG2 {

static quint8 byteAt(const QByteArray &data, int index)
{
    return static_cast<quint8>(data.at(index));
}

QByteArray ModbusRtuCodec::readRequest(int slaveAddress, quint8 function, int start, int count)
{
    QByteArray frame;
    frame.append(char(slaveAddress & 0xFF));
    frame.append(char(function));
    frame.append(char((start >> 8) & 0xFF));
    frame.append(char(start & 0xFF));
    frame.append(char((count >> 8) & 0xFF));
    frame.append(char(count & 0xFF));
    appendCrc(&frame);
    return frame;
}

bool ModbusRtuCodec::validateCrc(const QByteArray &frame)
{
    if (frame.size() < 5)
        return false;

    const QByteArray payload = frame.left(frame.size() - 2);
    const quint16 expected = crc(payload);
    const quint16 received = static_cast<quint16>(byteAt(frame, frame.size() - 2)
        | (byteAt(frame, frame.size() - 1) << 8));
    return expected == received;
}

QVector<quint16> ModbusRtuCodec::registersFromReadResponse(const QByteArray &frame)
{
    QVector<quint16> values;
    if (frame.size() < 5)
        return values;

    const int byteCount = byteAt(frame, 2);
    values.reserve(byteCount / 2);
    for (int i = 0; i + 1 < byteCount; i += 2) {
        const int offset = 3 + i;
        values.append(static_cast<quint16>((byteAt(frame, offset) << 8) | byteAt(frame, offset + 1)));
    }
    return values;
}

quint8 ModbusRtuCodec::bitsFromReadResponse(const QByteArray &frame)
{
    if (frame.size() < 4)
        return 0;
    return byteAt(frame, 3);
}

int ModbusRtuCodec::expectedReadResponseSize(int count, bool bits)
{
    return 3 + (bits ? ((count + 7) / 8) : (count * 2)) + 2;
}

quint16 ModbusRtuCodec::crc(const QByteArray &bytes)
{
    quint16 result = 0xFFFF;
    for (char ch : bytes) {
        result ^= static_cast<quint8>(ch);
        for (int i = 0; i < 8; ++i) {
            if (result & 0x0001)
                result = static_cast<quint16>((result >> 1) ^ 0xA001);
            else
                result >>= 1;
        }
    }
    return result;
}

void ModbusRtuCodec::appendCrc(QByteArray *frame)
{
    if (!frame)
        return;

    const quint16 value = crc(*frame);
    frame->append(char(value & 0xFF));
    frame->append(char((value >> 8) & 0xFF));
}

} // namespace DialogG2
