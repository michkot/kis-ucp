#include "existinguser.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>

#include "controller.h"
#include "settings.h"

template<typename LiteralType>
static inline QByteArray encodeJsonLiteral(const LiteralType& value)
{
    return QJsonDocument(
                    QJsonArray() << QJsonValue(value)
                ).toJson(QJsonDocument::Compact).mid(1).chopped(1);
}

ExistingUser::ExistingUser(QString session, QObject *parent) : User(parent), mSession(session), mInitialized(false), mLoginReply(nullptr), mUserDataReply(nullptr)
{
    mAuthTimer.setSingleShot(true);
    connect(&mAuthTimer, &QTimer::timeout, this, &ExistingUser::refreshAuthToken);
}

void ExistingUser::initialize()
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/auth/eduid/login");
    requestUrl.setQuery(QString("session=%1").arg(mSession));

    if (mLoginReply) {
        mLoginReply->deleteLater();
    }
    mLoginReply = Controller::instance->network().get(QNetworkRequest(requestUrl));
    connect(mLoginReply, &QNetworkReply::finished, this, &ExistingUser::loginInfoReceived);

    emit busyChanged();
}

bool ExistingUser::busy()
{
    return mLoginReply || mUserDataReply;
}

bool ExistingUser::initialized()
{
    return mInitialized;
}

QString ExistingUser::textUser()
{
    return tr("Logged in as %1 (%2)").arg(mUserData["name"].toString()).arg(mUserData["email"].toString());
}

QString ExistingUser::textMembership()
{
    if (mUserData["is_member"].toBool()) {
        QDateTime expiration = QDateTime::fromString(mUserData["member_until"].toString(), Qt::ISODate).toLocalTime();
        return tr("valid until %1").arg(expiration.toString());
    } else {
        if (mUserData["roles"].toArray().contains(QJsonValue("sympathizing_member"))) {
            return tr("expried (make a contribution to regain membership)");
        } else {
            return tr("invalid (please contact an administrator for details)");
        }
    }
}

QString ExistingUser::textOrders()
{
    return tr("%1 CZK in the past 30 days\n%2 CZK in the past 365 days")
            .arg(mUserData["orders"].toObject()["month"].toDouble(), 0, 'f', 2)
            .arg(mUserData["orders"].toObject()["year"].toDouble(), 0, 'f', 2);
}

QString ExistingUser::textContributions()
{
    return tr("%1 CZK in the past 30 days\n%2 CZK in the past 365 days")
            .arg(mUserData["contributions"].toObject()["month"].toDouble(), 0, 'f', 2)
            .arg(mUserData["contributions"].toObject()["year"].toDouble(), 0, 'f', 2);
}

QString ExistingUser::textPrestige()
{
    return QString("%1").arg(mUserData["prestige"].toDouble(), 0, 'f', 2);
}

QString ExistingUser::nickname()
{
    return mUserData["nickname"].toString();
}

bool ExistingUser::consent()
{
    return mUserData["gamification_consent"].toBool();
}

QString ExistingUser::pin()
{
    return mUserData["pin"].toString();
}

bool ExistingUser::pinVisible()
{
    return mUserData["roles"].toArray().contains(QJsonValue("regular_member")) ||
            mUserData["roles"].toArray().contains(QJsonValue("operator"));
}

void ExistingUser::setNickname(QString newNickname)
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/users/me/nickname");

    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(mAuthToken).toLatin1());

    if (mUserDataReply) {
        mUserDataReply->deleteLater();
    }
    mUserDataReply = Controller::instance->network().put(request, encodeJsonLiteral(newNickname));
    connect(mUserDataReply, &QNetworkReply::finished, [this]{ updateReplyReceived(UpdateChoice::kNickname); });

    emit busyChanged();
}

void ExistingUser::setConsent(bool newConsentStatus)
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/users/me/gamification_consent");

    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(mAuthToken).toLatin1());

    if (mUserDataReply) {
        mUserDataReply->deleteLater();
    }
    mUserDataReply = Controller::instance->network().put(request, encodeJsonLiteral(newConsentStatus));
    connect(mUserDataReply, &QNetworkReply::finished, [this]{ updateReplyReceived(UpdateChoice::kConsent); });

    emit busyChanged();
}

void ExistingUser::setPin(QString newPin)
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/users/me/pin");

    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(mAuthToken).toLatin1());

    if (mUserDataReply) {
        mUserDataReply->deleteLater();
    }
    if (newPin.length())
        mUserDataReply = Controller::instance->network().put(request, encodeJsonLiteral(newPin));
    else
        mUserDataReply = Controller::instance->network().put(request, encodeJsonLiteral(QJsonValue::Null));
    connect(mUserDataReply, &QNetworkReply::finished, [this]{ updateReplyReceived(UpdateChoice::kPin); });

    emit busyChanged();
}

void ExistingUser::refresh()
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/users/me");

    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(mAuthToken).toLatin1());

    if (mUserDataReply) {
        mUserDataReply->deleteLater();
    }
    mUserDataReply = Controller::instance->network().get(request);
    connect(mUserDataReply, &QNetworkReply::finished, this, &ExistingUser::userInfoReceived);

    emit busyChanged();
}

void ExistingUser::loginInfoReceived()
{
    // check if the reply object is still valid
    if (!mLoginReply) {
        emit busyChanged();
        return;
    }

    if (mLoginReply->error() != QNetworkReply::NoError) {
        // display error page
        QString description;
        switch (mLoginReply->error()) {
        case QNetworkReply::ContentNotFoundError:
            description = tr("The user does not exist. Please register first.");
            break;
        case QNetworkReply::ContentAccessDenied:
            description = tr("The user is disabled. Please contact a staff member for assistance.");
            break;
        default:
            description = mLoginReply->errorString();
        }

        Controller::instance->triggerFatalError(tr("Failed to authenticate"), description);

        mLoginReply->deleteLater();
        mLoginReply = nullptr;
        emit busyChanged();

        return;
    }

    // suppose we have the data, we need to parse it (JSON)
    QJsonDocument reply = QJsonDocument::fromJson(mLoginReply->readAll());
    mLoginReply->deleteLater();
    mLoginReply = nullptr;
    emit busyChanged();

    if (!reply.isObject()) {
        // display error page
        Controller::instance->triggerFatalError(tr("Failed to authenticate"), tr("Unable to parse the server response"));

        return;
    }

    // set the state
    if (reply.object().contains("refresh_token")) {
        mRefreshToken = reply["refresh_token"].toString();
        // TODO refresh
    }

    mAuthToken = reply["auth_token"].toString();
    QDateTime expires = QDateTime::fromString(reply["expires_at"].toString(), Qt::ISODate).addSecs(-60);
    mAuthTimer.start(int(QDateTime::currentDateTime().msecsTo(expires)));

    if (reply.object().contains("refresh_token")) {
        // this was the initial request, force refresh
        refresh();
    }
}

void ExistingUser::refreshAuthToken()
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/auth/fresh_token");
    requestUrl.setQuery(QString("refresh_token=%1").arg(mRefreshToken));

    mLoginReply = Controller::instance->network().get(QNetworkRequest(requestUrl));
    connect(mLoginReply, &QNetworkReply::finished, this, &ExistingUser::loginInfoReceived);

    emit busyChanged();
}

void ExistingUser::userInfoReceived()
{
    // check if the reply object is still valid
    if (!mUserDataReply) {
        emit busyChanged();
        return;
    }

    if (mUserDataReply->error() != QNetworkReply::NetworkError::NoError) {
        // display error page
        Controller::instance->triggerFatalError(tr("Failed to obtain user information"), mUserDataReply->errorString());

        mUserDataReply->deleteLater();
        mUserDataReply = nullptr;
        emit busyChanged();

        return;
    }

    // suppose we have the data, we need to parse it (JSON)
    QJsonDocument reply = QJsonDocument::fromJson(mUserDataReply->readAll());
    mUserDataReply->deleteLater();
    mUserDataReply = nullptr;
    emit busyChanged();

    if (!reply.isObject()) {
        // display error page
        Controller::instance->triggerFatalError(tr("Failed to obtain user information"), tr("Unable to parse the server response"));

        return;
    }

    mUserData = reply.object();
    emit userChanged();
    emit nicknameChanged();
    emit consentChanged();
    emit pinChanged();

    if (!mInitialized) {
        mInitialized = true;
        emit initializedChanged();
    }
}

void ExistingUser::updateReplyReceived(UpdateChoice choice)
{
    // check if the reply object is still valid
    if (!mUserDataReply) {
        emit busyChanged();
        return;
    }

    if (mUserDataReply->error() != QNetworkReply::NetworkError::NoError) {
        // display error page
        QString description = mUserDataReply->errorString();
        if (mUserDataReply->error() == QNetworkReply::ContentConflictError) {
            description = tr("The chosen nickname is already taken. Please choose a different one.");
        } else {
            QVariant status_code = mUserDataReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (status_code.isValid() && status_code.toInt() == 429) {
                description = tr("You have reached the limit for the number of nickname changes.");
            }
        }

        Controller::instance->triggerRecoverableError(tr("Failed to update your preferences"), description);

        mUserDataReply->deleteLater();
        mUserDataReply = nullptr;
        emit busyChanged();

        // fix back consent if it's the updated choice
        if (choice == UpdateChoice::kConsent) {
            emit consentChanged();
        }

        return;
    }

    // suppose we have the data, we need to parse it (JSON)
    QJsonDocument reply = QJsonDocument::fromJson(mUserDataReply->readAll());
    mUserDataReply->deleteLater();
    mUserDataReply = nullptr;
    emit busyChanged();

    if (!reply.isObject()) {
        // display error page
        Controller::instance->triggerRecoverableError(tr("Failed to update your preferences"), tr("Unable to parse the server response"));

        // fix back consent if it's the updated choice
        if (choice == UpdateChoice::kConsent) {
            emit consentChanged();
        }

        return;
    }

    // perform a selective update
    switch (choice) {
    case UpdateChoice::kPin:
        mUserData["pin"] = reply["pin"];
        emit pinChanged();
        break;
    case UpdateChoice::kConsent:
        mUserData["gamification_consent"] = reply["gamification_consent"];
        emit consentChanged();
        break;
    case UpdateChoice::kNickname:
        mUserData["nickname"] = reply["nickname"];
        emit nicknameChanged();
        break;
    }
}
