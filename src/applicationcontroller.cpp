#include "applicationcontroller.h"
#include "rotationsensor.h"
#include "networkmanager.h"
#include <QDebug>
#include <QSettings>

ApplicationController::ApplicationController(QObject *parent)
    : QObject(parent)
    , m_rotationSensor(new RotationSensor(this))
    , m_networkManager(new NetworkManager(this))
    , m_isConnected(false)
    , m_isSendingRotation(false)
    , m_connectionStatus("Disconnected")
    , m_zAxisOffset(0.0)
{
    // Load saved settings
    loadSettings();
    // Connect signals
    connect(m_networkManager, &NetworkManager::connectionStateChanged,
            this, &ApplicationController::onConnectionStateChanged);
    
    connect(m_networkManager, &NetworkManager::connectionError,
            this, [this](const QString &error) {
                qDebug() << "Connection error:" << error;
                m_connectionStatus = "Error: " + error;
                emit connectionStatusChanged();
            });
    
    connect(m_rotationSensor, &RotationSensor::rotationChanged,
            this, &ApplicationController::onRotationChanged);
}

ApplicationController::~ApplicationController()
{
}

bool ApplicationController::isConnected() const
{
    return m_isConnected;
}

bool ApplicationController::isSendingRotation() const
{
    return m_isSendingRotation;
}

QString ApplicationController::serverHost() const
{
    return m_serverHost;
}

void ApplicationController::setServerHost(const QString &host)
{
    if (m_serverHost != host) {
        m_serverHost = host;
        saveSettings();
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
        saveSettings();
        emit serverPortChanged();
    }
}

QString ApplicationController::connectionStatus() const
{
    return m_connectionStatus;
}

double ApplicationController::zAxisOffset() const
{
    return m_zAxisOffset;
}

void ApplicationController::setZAxisOffset(double offset)
{
    if (m_zAxisOffset != offset) {
        m_zAxisOffset = offset;
        emit zAxisOffsetChanged();
    }
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
    stopSendingRotation();
    m_networkManager->disconnectFromServer();
}

void ApplicationController::startSendingRotation()
{
    qDebug() << "ApplicationController::startSendingRotation() called";
    qDebug() << "Connected:" << m_isConnected << "Already sending:" << m_isSendingRotation;
    
    if (m_isConnected && !m_isSendingRotation) {
        qDebug() << "Starting rotation sensor...";
        m_rotationSensor->start();
        m_isSendingRotation = true;
        emit sendingStatusChanged();
        qDebug() << "Rotation sending started";
    } else {
        qDebug() << "Cannot start sending - not connected or already sending";
    }
}

void ApplicationController::stopSendingRotation()
{
    if (m_isSendingRotation) {
        m_rotationSensor->stop();
        m_isSendingRotation = false;
        emit sendingStatusChanged();
    }
}

void ApplicationController::onConnectionStateChanged()
{
    bool connected = m_networkManager->isConnected();
    if (m_isConnected != connected) {
        m_isConnected = connected;
        
        if (!connected) {
            stopSendingRotation();
        }
        
        m_connectionStatus = connected ? "Connected" : "Disconnected";
        
        emit connectionChanged();
        emit connectionStatusChanged();
    }
}

void ApplicationController::onRotationChanged(double w, double x, double y, double z)
{
    qDebug() << "ApplicationController::onRotationChanged:" << w << x << y << z;
    
    if (m_isConnected && m_isSendingRotation) {
        qDebug() << "Sending rotation data to network with Z-offset:" << m_zAxisOffset;
        m_networkManager->sendRotationData(w, x, y, z, m_zAxisOffset);
        emit rotationDataSent(w, x, y, z);
    } else {
        qDebug() << "Not sending - connected:" << m_isConnected << "sending:" << m_isSendingRotation;
    }
}

void ApplicationController::resetOrientation()
{
    qDebug() << "ApplicationController::resetOrientation() called";
    m_rotationSensor->resetOrientation();
}

void ApplicationController::loadSettings()
{
    QSettings settings;
    m_serverHost = settings.value("connection/serverHost", "localhost").toString();
    m_serverPort = settings.value("connection/serverPort", 18944).toInt();
    qDebug() << "Loaded settings - Host:" << m_serverHost << "Port:" << m_serverPort;
}

void ApplicationController::saveSettings()
{
    QSettings settings;
    settings.setValue("connection/serverHost", m_serverHost);
    settings.setValue("connection/serverPort", m_serverPort);
    qDebug() << "Saved settings - Host:" << m_serverHost << "Port:" << m_serverPort;
}