#ifndef READER_H
#define READER_H

#include <QObject>
#include <QWebSocket>

class Reader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool waiting READ waiting NOTIFY waitingChanged)

public:
    explicit Reader(QObject *parent = nullptr);
    static Reader* instance;

    QString state();
    bool connected();
    bool waiting();

signals:
    void stateChanged();
    void connectedChanged();
    void waitingChanged();

    void rfidRead(QByteArray);

public slots:
    void connect(QUrl address);
    void disconnect();

    void readRfid();
    void setIdle();

private slots:
    void socketStateChanged(QAbstractSocket::SocketState);
    void socketError();
    void processResponse(QByteArray data);

private:
    QWebSocket mReaderSocket;
    bool mWaiting;
};

#endif // READER_H
