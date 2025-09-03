#pragma once

#include <QObject>
#include <memory>

// Forward declarations for OpenIGTLink
namespace igtl {
    class ClientSocket;
    class OrientationMessage;
}

class IGTLClient : public QObject
{
    Q_OBJECT

public:
    explicit IGTLClient(QObject *parent = nullptr);
    ~IGTLClient();

    bool connectToServer(const QString &hostname, int port);
    void disconnectFromServer();
    bool isConnected() const;
    
    void sendOrientationData(double x, double y, double z);

signals:
    void connected();
    void disconnected();
    void connectionError(const QString &error);

private:
    std::unique_ptr<igtl::ClientSocket> m_socket;
    bool m_isConnected;
};