#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QUrl>

class Settings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl apiBaseUrl READ apiBaseUrl WRITE setApiBaseUrl NOTIFY apiBaseUrlChanged)
    Q_PROPERTY(QUrl rfidReaderUrl READ rfidReaderUrl WRITE setRfidReaderUrl NOTIFY rfidReaderUrlChanged)
    Q_PROPERTY(int eduidSessionTimeout READ eduidSessionTimeout WRITE setEduidSessionTimeout NOTIFY eduidSessionTimeoutChanged)
    Q_PROPERTY(int loginTimeout READ loginTimeout WRITE setLoginTimeout NOTIFY loginTimeoutChanged)

public:
    explicit Settings(QObject *parent = nullptr);
    static Settings* instance;

    QUrl apiBaseUrl();
    QUrl rfidReaderUrl();
    int eduidSessionTimeout();
    int loginTimeout();

signals:
    void apiBaseUrlChanged();
    void rfidReaderUrlChanged();
    void eduidSessionTimeoutChanged();
    void loginTimeoutChanged();

public slots:
    void setApiBaseUrl(QUrl);
    void setRfidReaderUrl(QUrl);
    void setEduidSessionTimeout(int);
    void setLoginTimeout(int);

private:
    QSettings mSettings;
};

#endif // SETTINGS_H
