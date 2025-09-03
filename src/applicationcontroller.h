#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

class OrientationSensor;
class NetworkManager;

class ApplicationController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString serverHost READ serverHost WRITE setServerHost NOTIFY serverHostChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)

public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();

    bool isConnected() const;
    QString serverHost() const;
    void setServerHost(const QString &host);
    int serverPort() const;
    void setServerPort(int port);
    QString connectionStatus() const;

public slots:
    void connectToServer();
    void disconnectFromServer();
    void startSendingOrientation();
    void stopSendingOrientation();

signals:
    void connectionChanged();
    void serverHostChanged();
    void serverPortChanged();
    void connectionStatusChanged();
    void orientationDataSent(double x, double y, double z);

private slots:
    void onConnectionStateChanged();
    void onOrientationChanged(double x, double y, double z);

private:
    OrientationSensor *m_orientationSensor;
    NetworkManager *m_networkManager;
    QString m_serverHost;
    int m_serverPort;
    bool m_isConnected;
    bool m_isSendingOrientation;
    QString m_connectionStatus;
};