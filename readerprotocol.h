#ifndef READERPROTOCOL_H
#define READERPROTOCOL_H

#include <cstdint>

struct ProtocolPacket {
    uint8_t messageType;
    uint8_t checkByteOne;
    uint8_t data[32];
    uint8_t checkByteTwo;
} __attribute__((packed));

struct IdData {
    uint8_t uid_length;
    uint8_t uid[7];
    uint8_t padding[8];
    uint8_t card0[16];
} __attribute__((packed));

static const uint8_t kProtocolCheckByteOne = 0x6E;
static const uint8_t kProtocolCheckByteTwo = 0xE6;

// APP->READER
enum class MessageA2R : uint8_t {
    Ping = 0x0,
    AutoReadBegin = 0x01,
    SingleRead = 0x02,
    Idle = 0x03,
    SingleAuthenticateId = 0x04,
    SingleAuthenticateKey = 0x05,
    PrintTextDual = 0x06,
};

// APP<-READER
enum class MessageR2A : uint8_t{
    Pong = 0x08,
    AutoId = 0x09,
    SingleId = 0x0A,
    SingleIdSendKey = 0x0B,
    VerificationCode = 0x0C,
};

#endif // READERPROTOCOL_H
