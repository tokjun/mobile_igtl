#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

class RotationSensor;
class NetworkManager;

class ApplicationController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString serverHost READ serverHost WRITE setServerHost NOTIFY serverHostChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(double zAxisOffset READ zAxisOffset WRITE setZAxisOffset NOTIFY zAxisOffsetChanged)

public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();

    bool isConnected() const;
    QString serverHost() const;
    void setServerHost(const QString &host);
    int serverPort() const;
    void setServerPort(int port);
    QString connectionStatus() const;
    double zAxisOffset() const;
    void setZAxisOffset(double offset);

public slots:
    void connectToServer();
    void disconnectFromServer();
    void startSendingRotation();
    void stopSendingRotation();
    void resetOrientation();

signals:
    void connectionChanged();
    void serverHostChanged();
    void serverPortChanged();
    void connectionStatusChanged();
    void zAxisOffsetChanged();
    void rotationDataSent(double w, double x, double y, double z);

private slots:
    void onConnectionStateChanged();
    void onRotationChanged(double w, double x, double y, double z);

private:
    void loadSettings();
    void saveSettings();
    
    RotationSensor *m_rotationSensor;
    NetworkManager *m_networkManager;
    QString m_serverHost;
    int m_serverPort;
    bool m_isConnected;
    bool m_isSendingRotation;
    QString m_connectionStatus;
    double m_zAxisOffset;
};