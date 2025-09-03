#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "applicationcontroller.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Set application metadata
    QGuiApplication::setApplicationName("OpenIGTLink Mobile");
    QGuiApplication::setApplicationVersion("1.0.0");
    QGuiApplication::setOrganizationName("OpenIGTLink");
    QGuiApplication::setOrganizationDomain("openigtlink.org");

    // Create the application controller
    ApplicationController controller;

    QQmlApplicationEngine engine;
    
    // Register the controller with QML
    engine.rootContext()->setContextProperty("appController", &controller);
    
    // Load main QML file
    const QUrl url(u"qrc:/OpenIGTLinkMobile/qml/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(url);

    return app.exec();
}