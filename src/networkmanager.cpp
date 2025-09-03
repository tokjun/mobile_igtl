#include "networkmanager.h"
#include "igtlclient.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_igtlClient(new IGTLClient(this))
    , m_isConnected(false)
{
    // Connect IGTL client signals
    connect(m_igtlClient, &IGTLClient::connected,
            this, &NetworkManager::onConnected);
    
    connect(m_igtlClient, &IGTLClient::disconnected,
            this, &NetworkManager::onDisconnected);
    
    connect(m_igtlClient, &IGTLClient::connectionError,
            this, &NetworkManager::onConnectionError);
}

NetworkManager::~NetworkManager()
{
    disconnectFromServer();
}

void NetworkManager::connectToServer(const QString &hostname, int port)
{
    if (!m_isConnected) {
        m_igtlClient->connectToServer(hostname, port);
    }
}

void NetworkManager::disconnectFromServer()
{
    if (m_isConnected) {
        m_igtlClient->disconnectFromServer();
    }
}

bool NetworkManager::isConnected() const
{
    return m_isConnected;
}

void NetworkManager::sendOrientationData(double x, double y, double z)
{
    if (m_isConnected) {
        m_igtlClient->sendOrientationData(x, y, z);
    }
}

void NetworkManager::onConnected()
{
    m_isConnected = true;
    emit connectionStateChanged();
}

void NetworkManager::onDisconnected()
{
    m_isConnected = false;
    emit connectionStateChanged();
}

void NetworkManager::onConnectionError(const QString &error)
{
    m_isConnected = false;
    emit connectionError(error);
    emit connectionStateChanged();
}