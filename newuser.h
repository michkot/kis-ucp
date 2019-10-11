#ifndef NEWUSER_H
#define NEWUSER_H

#include <QNetworkReply>
#include <QObject>

#include "user.h"

class NewUser : public User
{
    Q_OBJECT

    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)

    Q_PROPERTY(QString name READ name NOTIFY infoChanged)
    Q_PROPERTY(QString email READ email NOTIFY infoChanged)
    Q_PROPERTY(QString textRfid READ textRfid NOTIFY textRfidChanged)
    Q_PROPERTY(bool rfidAssigned READ rfidAssigned NOTIFY rfidAssignedChanged)

public:
    explicit NewUser(QString session, QObject *parent = nullptr);

    void initialize() override;
    bool busy() override;

    bool initialized();
    QString name();
    QString email();
    QString textRfid();
    bool rfidAssigned();

signals:
    void initializedChanged();
    void infoChanged();
    void textRfidChanged();
    void rfidAssignedChanged();

public slots:
    void updateNickname(QString);
    void updateConsent(bool);
    void updateRfid(QByteArray);
    void submit();

private slots:
    void infoReceived();
    void registerReplyReceived();

private:
    QString mSession;
    QNetworkReply* mInfoReply;
    QNetworkReply* mSubmitReply;

    QString mUserName;
    QString mUserEmail;
    bool mUserConsent;

    QString mUserNickname;
    QByteArray mUserRfid;
};

#endif // NEWUSER_H
