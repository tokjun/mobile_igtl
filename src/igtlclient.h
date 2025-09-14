#pragma once

#include <QObject>

#ifdef OPENIGTLINK_FOUND
#include "igtlClientSocket.h"
#else
// Forward declarations for OpenIGTLink
namespace igtl {
    class ClientSocket;
}
#endif

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
#ifdef OPENIGTLINK_FOUND
    igtl::ClientSocket::Pointer m_socket;
#endif
    bool m_isConnected;
};