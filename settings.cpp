#include "settings.h"

Settings* Settings::instance = nullptr;

Settings::Settings(QObject *parent) : QObject(parent)
{
    instance = this;

    // set up with sane defaults
    if (!mSettings.contains("api_url")) {
        mSettings.setValue("api_url", QUrl("https://su-int.fit.vutbr.cz/kis/api"));
    }
    if (!mSettings.contains("reader_url")) {
        mSettings.setValue("reader_url", QUrl("wss://1779d2.local/"));
    }
    if (!mSettings.contains("eduid_timeout")) {
        mSettings.setValue("eduid_timeout", 15);
    }
    if (!mSettings.contains("login_timeout")) {
        mSettings.setValue("login_timeout", 30);
    }
}

QUrl Settings::apiBaseUrl()
{
    return mSettings.value("api_url").toUrl();
}

QUrl Settings::rfidReaderUrl()
{
    return mSettings.value("reader_url").toUrl();
}

int Settings::eduidSessionTimeout()
{
    return mSettings.value("eduid_timeout").toInt();
}

int Settings::loginTimeout()
{
    return mSettings.value("login_timeout").toInt();
}

void Settings::setApiBaseUrl(QUrl newUlr)
{
    mSettings.setValue("api_url", newUlr);
    emit apiBaseUrlChanged();
}

void Settings::setRfidReaderUrl(QUrl newUrl)
{
    mSettings.setValue("reader_url", newUrl);
    emit rfidReaderUrlChanged();
}

void Settings::setEduidSessionTimeout(int newTimeout)
{
    if (newTimeout < 1) {
        return;
    }
    mSettings.setValue("eduid_timeout", newTimeout);
    emit eduidSessionTimeoutChanged();
}

void Settings::setLoginTimeout(int newTimeout)
{
    if (newTimeout < 1) {
        return;
    }
    mSettings.setValue("login_timeout", newTimeout);
    emit loginTimeoutChanged();
}
