#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QUrl>

#include "user.h"

class Controller : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString timeoutLabel READ timeoutLabel NOTIFY timeoutLabelChanged)
    Q_PROPERTY(QString timeoutValue READ timeoutValue NOTIFY timeoutValueChanged)
    Q_PROPERTY(QUrl loginWayfUrl READ loginWayfUrl NOTIFY loginWayfUrlChanged)
    Q_PROPERTY(QUrl loginCaptureUrl READ loginCaptureUrl NOTIFY loginCaptureUrlChanged)
    Q_PROPERTY(QString errorTitle MEMBER mErrorTitle NOTIFY errorTitleChanged)
    Q_PROPERTY(QString errorDescription MEMBER mErrorDescription NOTIFY errorDescriptionChanged)

public:
    explicit Controller(QObject *parent = nullptr);
    static Controller* instance;
    ~Controller();

    bool busy();
    QString timeoutLabel();
    QString timeoutValue();
    QUrl loginWayfUrl();
    QUrl loginCaptureUrl();

    QNetworkAccessManager& network();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

signals:
    void busyChanged();
    void timeoutLabelChanged();
    void timeoutValueChanged();
    void loginWayfUrlChanged();
    void loginCaptureUrlChanged();
    void errorTitleChanged();
    void errorDescriptionChanged();

    void replaceStackView(QString url);
    void pushStackView(QString url);
    void replaceWithUserProperty(QString url, QObject* user);
    void resetAndPushStackView(QString url);

public slots:
    void loginClicked();
    void registerClicked();

    void loginComplete();
    void loginFailed();

    void backClicked(int newDepth);

    void triggerRecoverableError(QString, QString = QString());
    void triggerFatalError(QString, QString = QString());
    void registerFinished();

private slots:
    void obtainEduidLoginUrl();
    void eduidLoginUrlReplyReceived();
    void eduidSessionExpired();
    void inactivityTimerTriggered();

private:
    QString mErrorTitle;
    QString mErrorDescription;

    bool mRegisterOnly;

    QNetworkAccessManager mNetwork;
    QNetworkReply* mEduidLoginUrlReply;

    QUrl mEduidLoginUrl;
    QString mEduidSessionId;

    QTimer mUpdateTimer;
    QTimer mEduidSessionTimer;
    QTimer mInactiveLoginTimer;

    User* mUser;
};

#endif // CONTROLLER_H
