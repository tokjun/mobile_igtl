#include "rotationsensor.h"
#include <QRotationSensor>
#include <QRotationReading>
#include <QMagnetometer>
#include <QMagnetometerReading>
#include <QAccelerometer>
#include <QAccelerometerReading>
#include <QTimer>
#include <QDebug>
#include <cmath>

RotationSensor::RotationSensor(QObject *parent)
    : QObject(parent)
    , m_rotationSensor(new QRotationSensor(this))
    , m_magnetometer(new QMagnetometer(this))
    , m_accelerometer(new QAccelerometer(this))
    , m_timer(new QTimer(this))
    , m_isActive(false)
{
    // Configure timer for regular readings (30 FPS)
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, &RotationSensor::performSensorFusion);
    
    // Check if sensors are available
    if (!m_rotationSensor->connectToBackend()) {
        qWarning("Rotation sensor is not available on this device");
    }
    if (!m_magnetometer->connectToBackend()) {
        qWarning("Magnetometer is not available on this device");
    }
    if (!m_accelerometer->connectToBackend()) {
        qWarning("Accelerometer is not available on this device");
    }
}

RotationSensor::~RotationSensor()
{
    stop();
}

void RotationSensor::start()
{
    qDebug() << "RotationSensor::start() called";
    
    bool hasAnyBackend = m_rotationSensor->isConnectedToBackend() || 
                        m_magnetometer->isConnectedToBackend() || 
                        m_accelerometer->isConnectedToBackend();
    
    if (!hasAnyBackend) {
        qDebug() << "RotationSensor: No backends connected, using simulated data";
        // Fallback to simulated data for desktop testing
        if (!m_isActive) {
            m_timer->start();
            m_isActive = true;
            qDebug() << "RotationSensor: Started with simulated data";
        }
        return;
    }
    
    if (!m_isActive) {
        qDebug() << "RotationSensor: Starting real sensors and timer";
        m_rotationSensor->start();
        m_magnetometer->start();
        m_accelerometer->start();
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
        m_magnetometer->stop();
        m_accelerometer->stop();
        m_isActive = false;
    }
}

bool RotationSensor::isActive() const
{
    return m_isActive;
}

void RotationSensor::performSensorFusion()
{
    bool hasRotation = m_rotationSensor->isConnectedToBackend();
    bool hasMagnetometer = m_magnetometer->isConnectedToBackend();
    bool hasAccelerometer = m_accelerometer->isConnectedToBackend();
    
    if (!hasRotation && !hasMagnetometer && !hasAccelerometer) {
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
    
    double roll = 0.0, pitch = 0.0, yaw = 0.0;
    
    // Get accelerometer data for roll and pitch
    if (hasAccelerometer && m_accelerometer->reading()) {
        QAccelerometerReading *accelReading = m_accelerometer->reading();
        double ax = accelReading->x();
        double ay = accelReading->y();
        double az = accelReading->z();
        
        // Calculate roll and pitch from accelerometer
        roll = atan2(ay, sqrt(ax*ax + az*az));
        pitch = atan2(-ax, sqrt(ay*ay + az*az));
    }
    
    // Get magnetometer data for yaw (heading)
    if (hasMagnetometer && m_magnetometer->reading()) {
        QMagnetometerReading *magReading = m_magnetometer->reading();
        double mx = magReading->x();
        double my = magReading->y();
        double mz = magReading->z();
        
        yaw = calculateMagneticHeading(mx, my, mz, roll, pitch);
    }
    
    // If rotation sensor is available, use it as a fallback or for validation
    if (hasRotation && m_rotationSensor->reading()) {
        QRotationReading *rotReading = m_rotationSensor->reading();
        
        // Use rotation sensor data if magnetometer is not available
        if (!hasMagnetometer) {
            roll = rotReading->x() * M_PI / 180.0;
            pitch = rotReading->y() * M_PI / 180.0;
            yaw = rotReading->z() * M_PI / 180.0;
        }
    }
    
    // Convert fused Euler angles to quaternion
    double w, x, y, z;
    quaternionFromEuler(roll, pitch, yaw, w, x, y, z);
    
    qDebug() << "RotationSensor (fused): roll=" << roll*180.0/M_PI << "pitch=" << pitch*180.0/M_PI << "yaw=" << yaw*180.0/M_PI;
    qDebug() << "RotationSensor (fused): w=" << w << "x=" << x << "y=" << y << "z=" << z;
    emit rotationChanged(w, x, y, z);
}

void RotationSensor::quaternionFromEuler(double roll, double pitch, double yaw, double &w, double &x, double &y, double &z)
{
    // Convert Euler angles to quaternion (ZYX rotation order)
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    
    w = cr * cp * cy + sr * sp * sy;
    x = sr * cp * cy - cr * sp * sy;
    y = cr * sp * cy + sr * cp * sy;
    z = cr * cp * sy - sr * sp * cy;
}

double RotationSensor::calculateMagneticHeading(double mx, double my, double mz, double roll, double pitch)
{
    // Compensate magnetometer readings for tilt
    // This is a simplified tilt compensation
    double cosRoll = cos(roll);
    double sinRoll = sin(roll);
    double cosPitch = cos(pitch);
    double sinPitch = sin(pitch);
    
    // Tilt-compensated magnetic field components
    double magX = mx * cosPitch + mz * sinPitch;
    double magY = mx * sinRoll * sinPitch + my * cosRoll - mz * sinRoll * cosPitch;
    
    // Calculate heading (yaw) from tilt-compensated magnetometer
    double heading = atan2(-magY, magX);
    
    // Normalize to 0-2π range
    if (heading < 0) {
        heading += 2.0 * M_PI;
    }
    
    return heading;
}