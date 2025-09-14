#include "applicationcontroller.h"
#include "orientationsensor.h"
#include "networkmanager.h"
#include <QDebug>

ApplicationController::ApplicationController(QObject *parent)
    : QObject(parent)
    , m_orientationSensor(new OrientationSensor(this))
    , m_networkManager(new NetworkManager(this))
    , m_serverHost("localhost")
    , m_serverPort(18944)
    , m_isConnected(false)
    , m_isSendingOrientation(false)
    , m_connectionStatus("Disconnected")
{
    // Connect signals
    connect(m_networkManager, &NetworkManager::connectionStateChanged,
            this, &ApplicationController::onConnectionStateChanged);
    
    connect(m_networkManager, &NetworkManager::connectionError,
            this, [this](const QString &error) {
                qDebug() << "Connection error:" << error;
                m_connectionStatus = "Error: " + error;
                emit connectionStatusChanged();
            });
    
    connect(m_orientationSensor, &OrientationSensor::orientationChanged,
            this, &ApplicationController::onOrientationChanged);
}

ApplicationController::~ApplicationController()
{
}

bool ApplicationController::isConnected() const
{
    return m_isConnected;
}

QString ApplicationController::serverHost() const
{
    return m_serverHost;
}

void ApplicationController::setServerHost(const QString &host)
{
    if (m_serverHost != host) {
        m_serverHost = host;
        emit serverHostChanged();
    }
}

int ApplicationController::serverPort() const
{
    return m_serverPort;
}

void ApplicationController::setServerPort(int port)
{
    if (m_serverPort != port) {
        m_serverPort = port;
        emit serverPortChanged();
    }
}

QString ApplicationController::connectionStatus() const
{
    return m_connectionStatus;
}

void ApplicationController::connectToServer()
{
    qDebug() << "Attempting to connect to" << m_serverHost << ":" << m_serverPort;
    m_connectionStatus = "Connecting...";
    emit connectionStatusChanged();
    m_networkManager->connectToServer(m_serverHost, m_serverPort);
}

void ApplicationController::disconnectFromServer()
{
    stopSendingOrientation();
    m_networkManager->disconnectFromServer();
}

void ApplicationController::startSendingOrientation()
{
    if (m_isConnected && !m_isSendingOrientation) {
        m_orientationSensor->start();
        m_isSendingOrientation = true;
    }
}

void ApplicationController::stopSendingOrientation()
{
    if (m_isSendingOrientation) {
        m_orientationSensor->stop();
        m_isSendingOrientation = false;
    }
}

void ApplicationController::onConnectionStateChanged()
{
    bool connected = m_networkManager->isConnected();
    if (m_isConnected != connected) {
        m_isConnected = connected;
        
        if (!connected) {
            stopSendingOrientation();
        }
        
        m_connectionStatus = connected ? "Connected" : "Disconnected";
        
        emit connectionChanged();
        emit connectionStatusChanged();
    }
}

void ApplicationController::onOrientationChanged(double x, double y, double z)
{
    if (m_isConnected && m_isSendingOrientation) {
        m_networkManager->sendOrientationData(x, y, z);
        emit orientationDataSent(x, y, z);
    }
}