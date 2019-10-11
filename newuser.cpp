#include "newuser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

#include "controller.h"
#include "reader.h"
#include "settings.h"

NewUser::NewUser(QString session, QObject *parent) : User(parent), mSession(session), mInfoReply(nullptr), mSubmitReply(nullptr), mUserConsent(false)
{
    connect(Reader::instance, &Reader::rfidRead, this, &NewUser::updateRfid);
}

void NewUser::initialize()
{
    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/auth/eduid/register");
    requestUrl.setQuery(QString("session_id=%1").arg(mSession));

    if (mInfoReply) {
        mInfoReply->deleteLater();
    }
    mInfoReply = Controller::instance->network().get(QNetworkRequest(requestUrl));
    connect(mInfoReply, &QNetworkReply::finished, this, &NewUser::infoReceived);

    emit busyChanged();
}

bool NewUser::busy()
{
    return mInfoReply;
}

bool NewUser::initialized()
{
    return !mUserEmail.isEmpty();
}

QString NewUser::name()
{
    return mUserName;
}

QString NewUser::email()
{
    return mUserEmail;
}

QString NewUser::textRfid()
{
    if (mUserRfid.isEmpty()) {
        return tr("unassigned");
    }
    QString decoded = QString::fromLatin1(mUserRfid.toHex(':'));

    return QString("%1__:__:__:__:%2__:__:__:__:__:__:__:__:__").arg(decoded.left(1*3)).arg(decoded.mid(5*3, 2*3));
}

bool NewUser::rfidAssigned()
{
    return !mUserRfid.isEmpty();
}

void NewUser::updateNickname(QString newNickname)
{
    mUserNickname = newNickname;
}

void NewUser::updateConsent(bool newConsent)
{
    mUserConsent = newConsent;
}

void NewUser::updateRfid(QByteArray newRfid)
{
    mUserRfid = newRfid;
    emit rfidAssignedChanged();
    emit textRfidChanged();
}

void NewUser::submit()
{
    Reader::instance->setIdle();

    QJsonObject jsonRequest;
    jsonRequest["gamification_consent"] = mUserConsent;
    if (!mUserRfid.isEmpty()) {
        jsonRequest["rfid"] = QString::fromLatin1(mUserRfid.toBase64());
    }
    if (!mUserNickname.isEmpty()) {
        jsonRequest["nickname"] = mUserNickname;
    }

    QUrl requestUrl(Settings::instance->apiBaseUrl());
    requestUrl.setPath(requestUrl.path() + "/auth/eduid/register");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("session_id", mSession);
    requestUrl.setQuery(urlQuery);

    QNetworkRequest netRequest(requestUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (mSubmitReply) {
        mSubmitReply->deleteLater();
    }
    mSubmitReply = Controller::instance->network().post(netRequest, QJsonDocument(jsonRequest).toJson());
    connect(mSubmitReply, &QNetworkReply::finished, this, &NewUser::registerReplyReceived);

    emit busyChanged();
}

void NewUser::infoReceived()
{
    // check if the reply object is still valid
    if (!mInfoReply) {
        emit busyChanged();
        return;
    }

    if (mInfoReply->error() != QNetworkReply::NetworkError::NoError) {
        // display error page
        QString description;
        switch (mInfoReply->error()) {
        case QNetworkReply::ContentNotFoundError:
            description = tr("The eduID session has expired. Please prepare the required information and try again.");
            break;
        case QNetworkReply::ContentConflictError:
            description = tr("The user is already registered. To update your preferences please login instead.");
            break;
        default:
            description = mInfoReply->errorString();
        }

        Controller::instance->triggerFatalError(tr("Failed to proceed with registration"), description);

        mInfoReply->deleteLater();
        mInfoReply = nullptr;
        emit busyChanged();

        return;
    }

    // suppose we have the data, we need to parse it (JSON)
    QJsonDocument reply = QJsonDocument::fromJson(mInfoReply->readAll());
    mInfoReply->deleteLater();
    mInfoReply = nullptr;
    emit busyChanged();

    if (!reply.isObject()) {
        // display error page
        Controller::instance->triggerFatalError(tr("Failed to proceed with registration"), tr("Unable to parse the server response"));

        return;
    }

    mUserName = reply["name"].toString();
    mUserEmail = reply["email"].toString();

    emit infoChanged();
    emit initializedChanged();
    Reader::instance->readRfid();
}

void NewUser::registerReplyReceived()
{
    // check if aborted earlier and do nothing in that case
    if (!mSubmitReply) {
        emit busyChanged();
        return;
    }

    if (mSubmitReply->error() != QNetworkReply::NetworkError::NoError) {
        // display error page
        QString description;
        switch (mSubmitReply->error()) {
        case QNetworkReply::ContentNotFoundError:
            description = tr("The eduID session has expired. Please prepare the required information and try again.");
            break;
        case QNetworkReply::ContentConflictError:
            description = tr("A user with the same email, nickname, or RFID already exists. If you haven't registered before try to change your nickname.");
            break;
        default:
            description = mSubmitReply->errorString();
        }

        if (mSubmitReply->error() == QNetworkReply::ContentNotFoundError) {
            Controller::instance->triggerFatalError(tr("Failed to register"), description);
        } else {
            Controller::instance->triggerRecoverableError(tr("Failed to register"), description);
        }

        mSubmitReply->deleteLater();
        mSubmitReply = nullptr;
        emit busyChanged();

        return;
    }

    mSubmitReply->deleteLater();
    mSubmitReply = nullptr;
    emit busyChanged();

    Controller::instance->registerFinished();
}
