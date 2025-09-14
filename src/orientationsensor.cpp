#include "orientationsensor.h"
#include <QRotationSensor>
#include <QRotationReading>
#include <QTimer>

OrientationSensor::OrientationSensor(QObject *parent)
    : QObject(parent)
    , m_rotationSensor(new QRotationSensor(this))
    , m_timer(new QTimer(this))
    , m_isActive(false)
{
    // Configure timer for regular readings (30 FPS)
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, &OrientationSensor::readingSensor);
    
    // Check if rotation sensor is available
    if (!m_rotationSensor->connectToBackend()) {
        qWarning("Rotation sensor is not available on this device");
    }
}

OrientationSensor::~OrientationSensor()
{
    stop();
}

void OrientationSensor::start()
{
    if (!m_rotationSensor->isConnectedToBackend()) {
        return;
    }
    
    if (!m_isActive) {
        m_rotationSensor->start();
        m_timer->start();
        m_isActive = true;
    }
}

void OrientationSensor::stop()
{
    if (m_isActive) {
        m_timer->stop();
        m_rotationSensor->stop();
        m_isActive = false;
    }
}

bool OrientationSensor::isActive() const
{
    return m_isActive;
}

void OrientationSensor::readingSensor()
{
    if (!m_rotationSensor->reading()) {
        return;
    }
    
    QRotationReading *reading = m_rotationSensor->reading();
    
    // Get rotation values (in degrees)
    double x = reading->x();
    double y = reading->y();
    double z = reading->z();
    
    emit orientationChanged(x, y, z);
}