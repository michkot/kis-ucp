#include "reader.h"

#include "controller.h"
#include "readerprotocol.h"

Reader* Reader::instance = nullptr;

Reader::Reader(QObject *parent) : QObject(parent), mWaiting(false)
{
    instance = this;

    QSslConfiguration sslConfig = mReaderSocket.sslConfiguration();
    const auto sslCas = QSslCertificate::fromPath("certs/*.crt", QSsl::Pem, QRegExp::Wildcard);
    sslConfig.setCaCertificates(sslCas);
    mReaderSocket.setSslConfiguration(sslConfig);

    QObject::connect(&mReaderSocket, &QWebSocket::stateChanged, this, &Reader::socketStateChanged);
    QObject::connect(&mReaderSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &Reader::socketError);
    QObject::connect(&mReaderSocket, &QWebSocket::binaryMessageReceived, this, &Reader::processResponse);
}

QString Reader::state()
{
    switch (mReaderSocket.state()) {
    case QAbstractSocket::UnconnectedState:
    case QAbstractSocket::ClosingState:
        return tr("disconnected");
    case QAbstractSocket::HostLookupState:
        return tr("lookup");
    case QAbstractSocket::ConnectingState:
        return tr("connecting");
    case QAbstractSocket::ConnectedState:
        return tr("connected");
    default:
        return tr("unknown");
    }
}

bool Reader::connected()
{
    return mReaderSocket.state() == QAbstractSocket::ConnectedState;
}

bool Reader::waiting()
{
    return mWaiting;
}

void Reader::connect(QUrl url)
{
    if (connected()) {
        mReaderSocket.close();
    }
    mReaderSocket.open(url);
}

void Reader::disconnect()
{
    mReaderSocket.close();
}

void Reader::readRfid()
{
    if (!connected()) {
        return;
    }

    // ask for a single ID
    ProtocolPacket packet;
    packet.messageType = static_cast<std::underlying_type<MessageA2R>::type>(MessageA2R::SingleRead);
    packet.checkByteOne = kProtocolCheckByteOne;
    packet.checkByteTwo = kProtocolCheckByteTwo;
    QByteArray message(reinterpret_cast<const char*>(&packet), sizeof(packet));
    mReaderSocket.sendBinaryMessage(message);

    // display a message
    packet.messageType = static_cast<std::underlying_type<MessageA2R>::type>(MessageA2R::PrintTextDual);
    std::copy_n("Touch the card                  ", 32, packet.data);
    message = QByteArray(reinterpret_cast<const char*>(&packet), sizeof(packet));
    mReaderSocket.sendBinaryMessage(message);

    mWaiting = true;
    emit waitingChanged();
}

void Reader::setIdle()
{
    if (!mWaiting) {
        return;
    }

    mWaiting = false;
    emit waitingChanged();

    // set idle status
    ProtocolPacket packet;
    packet.messageType = static_cast<std::underlying_type<MessageA2R>::type>(MessageA2R::Idle);
    packet.checkByteOne = kProtocolCheckByteOne;
    packet.checkByteTwo = kProtocolCheckByteTwo;
    QByteArray message(reinterpret_cast<const char*>(&packet), sizeof(packet));
    mReaderSocket.sendBinaryMessage(message);

    // clear the display
    packet.messageType = static_cast<std::underlying_type<MessageA2R>::type>(MessageA2R::PrintTextDual);
    packet.checkByteOne = kProtocolCheckByteOne;
    packet.checkByteTwo = kProtocolCheckByteTwo;
    std::copy_n("                                ", 32, packet.data);
    message = QByteArray(reinterpret_cast<const char*>(&packet), sizeof(packet));
    mReaderSocket.sendBinaryMessage(message);
}

void Reader::socketStateChanged(QAbstractSocket::SocketState state)
{
    if (state != QAbstractSocket::ConnectedState && mWaiting) {
        mWaiting = false;
        emit waitingChanged();
    }
    emit connectedChanged();
    emit stateChanged();
}

void Reader::socketError()
{
    Controller::instance->triggerRecoverableError(tr("RFID reader connection failed"), mReaderSocket.errorString());
}

void Reader::processResponse(QByteArray data)
{
    mWaiting = false;
    emit waitingChanged();

    // clear the display
    ProtocolPacket packet;
    packet.messageType = static_cast<std::underlying_type<MessageA2R>::type>(MessageA2R::PrintTextDual);
    packet.checkByteOne = kProtocolCheckByteOne;
    packet.checkByteTwo = kProtocolCheckByteTwo;
    std::copy_n("                                ", 32, packet.data);
    QByteArray message(reinterpret_cast<const char*>(&packet), sizeof(packet));
    mReaderSocket.sendBinaryMessage(message);

    // process the reply
    if (data.length() == sizeof(ProtocolPacket)) {
        const ProtocolPacket *packet = reinterpret_cast<const ProtocolPacket *>(data.data());
        if (packet->messageType == static_cast<std::underlying_type<MessageR2A>::type>(MessageR2A::SingleId) &&
                packet->checkByteOne == kProtocolCheckByteOne && packet->checkByteTwo == kProtocolCheckByteTwo) {
            const IdData *message = reinterpret_cast<const IdData *>(packet->data);

            // successful read, hopefully
            emit rfidRead(QByteArray(reinterpret_cast<const char*>(message->card0), sizeof(message->card0)));
            return;
        }
    }

    // display error page
    Controller::instance->triggerRecoverableError(tr("Reader error"), tr("Unexpected message from the RFID reader received"));
    return;
}
