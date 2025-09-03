#pragma once

#include <QObject>
#include <QString>

class IGTLClient;

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    void connectToServer(const QString &hostname, int port);
    void disconnectFromServer();
    bool isConnected() const;
    
    void sendOrientationData(double x, double y, double z);

signals:
    void connectionStateChanged();
    void connectionError(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onConnectionError(const QString &error);

private:
    IGTLClient *m_igtlClient;
    bool m_isConnected;
};