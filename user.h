#ifndef USER_H
#define USER_H

#include <QObject>

class User: public QObject
{
    Q_OBJECT
public:
    explicit User(QObject* parent = nullptr);

    virtual ~User() {}

    virtual void initialize() = 0;
    virtual bool busy() = 0;

signals:
    void busyChanged();
};

#endif // USER_H
