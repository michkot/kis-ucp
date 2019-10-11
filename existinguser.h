#ifndef EXISTINGUSER_H
#define EXISTINGUSER_H

#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

#include "user.h"

class ExistingUser : public User
{
    Q_OBJECT

    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)

    Q_PROPERTY(QString textUser READ textUser NOTIFY userChanged)
    Q_PROPERTY(QString textContributions READ textContributions NOTIFY userChanged)
    Q_PROPERTY(QString textPrestige READ textPrestige NOTIFY userChanged)
    Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
    Q_PROPERTY(bool consent READ consent WRITE setConsent NOTIFY consentChanged)
    Q_PROPERTY(QString pin READ pin WRITE setPin NOTIFY pinChanged)
    Q_PROPERTY(bool pinVisible READ pinVisible NOTIFY userChanged)

    enum class UpdateChoice {
        kConsent,
        kNickname,
        kPin
    };

public:
    explicit ExistingUser(QString session, QObject *parent = nullptr);

    void initialize() override;
    bool busy() override;

    bool initialized();
    QString textUser();
    QString textContributions();
    QString textPrestige();
    QString nickname();
    bool consent();
    QString pin();
    bool pinVisible();

signals:
    void initializedChanged();
    void userChanged();
    void nicknameChanged();
    void consentChanged();
    void pinChanged();

public slots:
    void setNickname(QString);
    void setConsent(bool);
    void setPin(QString);

    void refresh();

private slots:
    void loginInfoReceived();
    void refreshAuthToken();
    void userInfoReceived();
    void updateReplyReceived(UpdateChoice);

private:
    QString mSession;
    bool mInitialized;

    QNetworkReply* mLoginReply;
    QNetworkReply* mUserDataReply;

    QString mRefreshToken;
    QString mAuthToken;

    QTimer mAuthTimer;

    QJsonObject mUserData;
};

#endif // EXISTINGUSER_H
