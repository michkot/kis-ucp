#include "controller.h"

#include <QEvent>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QQmlComponent>
#include <QQmlContext>

#include "existinguser.h"
#include "newuser.h"
#include "reader.h"
#include "settings.h"

Controller* Controller::instance = nullptr;

Controller::Controller(QObject *parent) : QObject(parent), mEduidLoginUrlReply(nullptr), mUser(nullptr)
{
    instance = this;

    connect(Settings::instance, &Settings::apiBaseUrlChanged, this, &Controller::loginCaptureUrlChanged);

    mEduidSessionTimer.setSingleShot(true);
    connect(&mEduidSessionTimer, &QTimer::timeout, this, &Controller::eduidSessionExpired);

    mInactiveLoginTimer.setSingleShot(true);
    connect(&mInactiveLoginTimer, &QTimer::timeout, this, &Controller::inactivityTimerTriggered);

    connect(&mUpdateTimer, &QTimer::timeout, this, &Controller::timeoutLabelChanged);
    connect(&mUpdateTimer, &QTimer::timeout, this, &Controller::timeoutValueChanged);
    mUpdateTimer.start(1000);
}

Controller::~Controller()
{
    if (mUser) {
        User* tmp = mUser;
        mUser = nullptr;
        tmp->deleteLater();
    }
}

bool Controller::busy()
{
    return mEduidLoginUrlReply || (mUser && mUser->busy());
}

QString Controller::timeoutLabel()
{
    if (mEduidSessionTimer.isActive()) {
        return tr("Session expiration:");
    }
    if (mInactiveLoginTimer.isActive()) {
        return tr("Automatic logout:");
    }
    return "";
}

QString Controller::timeoutValue()
{
    if (mEduidSessionTimer.isActive()) {
        int ms = mEduidSessionTimer.remainingTime();
        if (ms < 10000) {
            return tr("a few seconds");
        }
        if (ms < 60000) {
            return tr("< 1 minute");
        }
        int min = ms / 60000;
        if (ms % 60000 >= 30000) {
            min++;
        }
        return QString("~ %1 %2").arg(min).arg(min > 1 ? tr("minutes") : tr("minute"));
    }
    if (mInactiveLoginTimer.isActive()) {
        int ms = mInactiveLoginTimer.remainingTime();
        return QString("%1 %2").arg(ms/1000).arg(ms/1000 == 1 ? tr("second") : tr("seconds"));
    }
    return "";
}

QUrl Controller::loginWayfUrl()
{
    return mEduidWayfUrl;
}

QUrl Controller::loginCaptureUrl()
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/auth/eduid/assertion");

    return requestUrl;
}

QNetworkAccessManager &Controller::network()
{
    return mNetwork;
}

bool Controller::eventFilter(QObject*, QEvent* event)
{
    if (!mInactiveLoginTimer.isActive()) {
        return false;
    }
    if (event->type() >= QEvent::MouseButtonPress && event->type() <= QEvent::KeyRelease) {
        // all usual mouse and keyboard events restarts the inactive timer
        mInactiveLoginTimer.start(Settings::instance->loginTimeout() * 1000);
    }
    return false;
}

void Controller::loginClicked()
{
    mRegisterOnly = false;
    obtainEduidLoginUrl();
}

void Controller::registerClicked()
{
    mRegisterOnly = true;
    obtainEduidLoginUrl();
}

void Controller::loginComplete()
{
    // based on the requested page (login vs register) we construct the corresponding object
    if (mRegisterOnly) {
        mUser = new NewUser(mEduidSession);
        emit replaceWithUserProperty("qrc:/regpanel.qml", mUser);
    } else {
        mUser = new ExistingUser(mEduidSession);
        emit replaceWithUserProperty("qrc:/ctrlpanel.qml", mUser);

        // In this case, the session timer loses its importance
        mEduidSessionTimer.stop();
        // We track inactive time instead
        mInactiveLoginTimer.start(Settings::instance->loginTimeout() * 1000);
    }

    connect(mUser, &User::busyChanged, this, &Controller::busyChanged);
    mUser->initialize();
}

void Controller::loginFailed()
{
    mEduidSessionTimer.stop();
    // replace the login page with error
    triggerFatalError(tr("An error occurred during the login"), tr("Unexpected error occurred during the login operation. Please try to start from the beginning."));
}

void Controller::backClicked(int newDepth)
{
    // StackView is in the state *after* the pop.
    if (newDepth < 2) {
        // we are on the home page again, cancel all pending timers and timeouts
        mEduidSessionTimer.stop();
        mInactiveLoginTimer.stop();
        disconnect(&mUpdateTimer, &QTimer::timeout, this, &Controller::eduidSessionExpired);

        // also destroy a user, if exists
        if (mUser) {
            User* tmp = mUser;
            mUser = nullptr;
            tmp->deleteLater();
        }
    }
    Reader::instance->setIdle();
}

void Controller::triggerRecoverableError(QString title, QString description)
{
    mErrorTitle = title;
    mErrorDescription = description;
    emit errorTitleChanged();
    emit errorDescriptionChanged();

    emit pushStackView("qrc:/error.qml");
}

void Controller::triggerFatalError(QString title, QString description)
{
    mErrorTitle = title;
    mErrorDescription = description;
    emit errorTitleChanged();
    emit errorDescriptionChanged();
    Reader::instance->setIdle();

    emit replaceStackView("qrc:/error.qml");
}

void Controller::registerFinished()
{
    emit replaceStackView("qrc:/regsuccess.qml");
}

void Controller::obtainEduidLoginUrl()
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/auth/eduid");

    mEduidLoginUrlReply = mNetwork.get(QNetworkRequest(requestUrl));
    connect(mEduidLoginUrlReply, &QNetworkReply::finished, this, &Controller::eduidLoginUrlReplyReceived);

    emit busyChanged();
}

void Controller::eduidLoginUrlReplyReceived()
{
    // check if the reply object is still valid
    if (!mEduidLoginUrlReply) {
        emit busyChanged(); // just to be safe the GUI is not stuck
        return;
    }

    if (mEduidLoginUrlReply->error() != QNetworkReply::NetworkError::NoError) {
        // display error page on top of the home
        triggerRecoverableError(tr("Failed to obtain authentication URL from the server"), mEduidLoginUrlReply->errorString());

        mEduidLoginUrlReply->deleteLater();
        mEduidLoginUrlReply = nullptr;
        emit busyChanged();

        return;
    }

    // suppose we have the data, we need to parse it (JSON)
    QJsonDocument reply = QJsonDocument::fromJson(mEduidLoginUrlReply->readAll());
    mEduidLoginUrlReply->deleteLater();
    mEduidLoginUrlReply = nullptr;
    emit busyChanged();

    if (!reply.isObject()) {
        // display error page on top of the home
        triggerRecoverableError(tr("Failed to obtain authentication URL from the server"), tr("Unable to parse the server response"));

        return;
    }

    // set the required state
    mEduidSession = reply["session"].toString();
    mEduidWayfUrl = reply["wayf_url"].toString();
    emit loginWayfUrlChanged();

    // set up the timer
    mEduidSessionTimer.start(Settings::instance->eduidSessionTimeout() * 60 * 1000);

    emit pushStackView("qrc:/login.qml");
}

void Controller::eduidSessionExpired()
{
    if (busy()) {
        // postpone this task by appending it to the update task
        connect(&mUpdateTimer, &QTimer::timeout, this, &Controller::eduidSessionExpired, Qt::UniqueConnection);
        return;
    }

    // disconnect is safe, it will return false if no connection exists
    disconnect(&mUpdateTimer, &QTimer::timeout, this, &Controller::eduidSessionExpired);

    // this may happen on the login page, register page, error or settings page on top of register page...
    mErrorTitle = tr("Session expired");
    mErrorDescription = tr("The eduID session used to login and / or register has expired and it is not possible to proceed. Please prepare the required information and try again.");
    emit errorTitleChanged();
    emit errorDescriptionChanged();
    Reader::instance->setIdle();

    emit resetAndPushStackView("qrc:/error.qml");
}

void Controller::inactivityTimerTriggered()
{
    // multiple items can be pushed on top of the stack view, deal with it
    mErrorTitle = tr("Logged out automatically");
    mErrorDescription = tr("The logged in user was logged out due to inactivity.");
    emit errorTitleChanged();
    emit errorDescriptionChanged();

    emit resetAndPushStackView("qrc:/error.qml");
}
