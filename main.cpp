#include <QQmlContext>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWebView>

#include "controller.h"
#include "existinguser.h"
#include "newuser.h"
#include "reader.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QCoreApplication::setOrganizationName("BUT FIT SU");
    QCoreApplication::setOrganizationDomain("su.fit.vutbr.cz");
    QCoreApplication::setApplicationName("KIS UCP");

    QGuiApplication app(argc, argv);

    // Settings needs to be constructed first so that the Settings::instance is valid
    Settings settings;

    // Other objects
    Controller controller;
    Reader reader;

    qmlRegisterUncreatableType<ExistingUser>("KIS.Users", 1, 0, "ExistingUser", "Objects instantiated in the C++ backend");
    qmlRegisterUncreatableType<NewUser>("KIS.Users", 1, 0, "NewUser", "Object instantiated in the C++ backend");

    app.installEventFilter(&controller);
    QtWebView::initialize();
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("controller", &controller);
    engine.rootContext()->setContextProperty("reader", &reader);
    engine.rootContext()->setContextProperty("settings", &settings);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
