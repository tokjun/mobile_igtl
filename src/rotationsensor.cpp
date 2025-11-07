#include "rotationsensor.h"
#include <QRotationSensor>
#include <QRotationReading>
#include <QTimer>
#include <QDebug>
#include <cmath>

RotationSensor::RotationSensor(QObject *parent)
    : QObject(parent)
    , m_rotationSensor(new QRotationSensor(this))
    , m_timer(new QTimer(this))
    , m_isActive(false)
{
    // Configure timer for regular readings (30 FPS)
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, &RotationSensor::readingSensor);
    
    // Check if rotation sensor is available
    if (!m_rotationSensor->connectToBackend()) {
        qWarning("Rotation sensor is not available on this device");
    }
}

RotationSensor::~RotationSensor()
{
    stop();
}

void RotationSensor::start()
{
    qDebug() << "RotationSensor::start() called";
    
    if (!m_rotationSensor->isConnectedToBackend()) {
        qDebug() << "RotationSensor: Not connected to backend, using simulated data";
        // Fallback to simulated data for desktop testing
        if (!m_isActive) {
            m_timer->start();
            m_isActive = true;
            qDebug() << "RotationSensor: Started with simulated data";
        }
        return;
    }
    
    if (!m_isActive) {
        qDebug() << "RotationSensor: Starting real sensor and timer";
        m_rotationSensor->start();
        m_timer->start();
        m_isActive = true;
    } else {
        qDebug() << "RotationSensor: Already active";
    }
}

void RotationSensor::stop()
{
    if (m_isActive) {
        m_timer->stop();
        m_rotationSensor->stop();
        m_isActive = false;
    }
}

bool RotationSensor::isActive() const
{
    return m_isActive;
}

void RotationSensor::readingSensor()
{
    if (!m_rotationSensor->isConnectedToBackend()) {
        // Generate simulated quaternion data for desktop testing
        static double angle = 0.0;
        angle += 1.0; // Increment by 1 degree each time
        if (angle >= 360.0) angle = 0.0;
        
        // Create a simple rotation around Z axis for simulation
        double radians = angle * M_PI / 180.0;
        double w = cos(radians / 2.0);
        double x = 0.0;
        double y = 0.0;
        double z = sin(radians / 2.0);
        
        qDebug() << "RotationSensor (simulated): w=" << w << "x=" << x << "y=" << y << "z=" << z;
        emit rotationChanged(w, x, y, z);
        return;
    }
    
    if (!m_rotationSensor->reading()) {
        qDebug() << "RotationSensor: No reading available";
        return;
    }
    
    QRotationReading *reading = m_rotationSensor->reading();
    
    // Get Euler angles from the sensor (in degrees)
    double eulerX = reading->x();
    double eulerY = reading->y();
    double eulerZ = reading->z();
    
    // Convert Euler angles (degrees) to quaternion
    // Convert to radians first
    double roll = eulerX * M_PI / 180.0;
    double pitch = eulerY * M_PI / 180.0;
    double yaw = eulerZ * M_PI / 180.0;
    
    // Convert Euler angles to quaternion (ZYX rotation order)
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    
    double w = cr * cp * cy + sr * sp * sy;
    double x = sr * cp * cy - cr * sp * sy;
    double y = cr * sp * cy + sr * cp * sy;
    double z = cr * cp * sy - sr * sp * cy;
    
    qDebug() << "RotationSensor (real): w=" << w << "x=" << x << "y=" << y << "z=" << z;
    emit rotationChanged(w, x, y, z);
}