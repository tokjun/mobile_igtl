#include "networkmanager.h"
#include "igtlclient.h"
#include <QDebug>

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
    qDebug() << "NetworkManager: Connecting to" << hostname << ":" << port;
    if (!m_isConnected) {
        bool result = m_igtlClient->connectToServer(hostname, port);
        qDebug() << "NetworkManager: Connection result:" << result;
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

void NetworkManager::sendRotationData(double w, double x, double y, double z, double zOffset)
{
    if (m_isConnected) {
        m_igtlClient->sendRotationData(w, x, y, z, zOffset);
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