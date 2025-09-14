#include "orientationsensor.h"
#include <QRotationSensor>
#include <QRotationReading>
#include <QTimer>
#include <QDebug>
#include <cmath>

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
    qDebug() << "OrientationSensor::start() called";
    
    if (!m_rotationSensor->isConnectedToBackend()) {
        qDebug() << "OrientationSensor: Not connected to backend, using simulated data";
        // Fallback to simulated data for desktop testing
        if (!m_isActive) {
            m_timer->start();
            m_isActive = true;
            qDebug() << "OrientationSensor: Started with simulated data";
        }
        return;
    }
    
    if (!m_isActive) {
        qDebug() << "OrientationSensor: Starting real sensor and timer";
        m_rotationSensor->start();
        m_timer->start();
        m_isActive = true;
    } else {
        qDebug() << "OrientationSensor: Already active";
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
    if (!m_rotationSensor->isConnectedToBackend()) {
        // Generate simulated orientation data for desktop testing
        static double angle = 0.0;
        angle += 1.0; // Increment by 1 degree each time
        if (angle >= 360.0) angle = 0.0;
        
        double x = sin(angle * M_PI / 180.0) * 10.0; // Simulate roll
        double y = cos(angle * M_PI / 180.0) * 5.0;  // Simulate pitch
        double z = angle; // Simulate yaw
        
        qDebug() << "OrientationSensor (simulated): x=" << x << "y=" << y << "z=" << z;
        emit orientationChanged(x, y, z);
        return;
    }
    
    if (!m_rotationSensor->reading()) {
        qDebug() << "OrientationSensor: No reading available";
        return;
    }
    
    QRotationReading *reading = m_rotationSensor->reading();
    
    // Get rotation values (in degrees)
    double x = reading->x();
    double y = reading->y();
    double z = reading->z();
    
    qDebug() << "OrientationSensor (real): x=" << x << "y=" << y << "z=" << z;
    emit orientationChanged(x, y, z);
}