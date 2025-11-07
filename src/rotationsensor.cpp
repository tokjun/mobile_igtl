#include "rotationsensor.h"
#include <QMagnetometer>
#include <QMagnetometerReading>
#include <QAccelerometer>
#include <QAccelerometerReading>
#include <QTimer>
#include <QDebug>
#include <cmath>

RotationSensor::RotationSensor(QObject *parent)
    : QObject(parent)
    , m_magnetometer(new QMagnetometer(this))
    , m_accelerometer(new QAccelerometer(this))
    , m_timer(new QTimer(this))
    , m_isActive(false)
    , m_initialW(1.0), m_initialX(0.0), m_initialY(0.0), m_initialZ(0.0)
    , m_hasInitialOrientation(false)
{
    // Configure timer for regular readings (30 FPS)
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, &RotationSensor::performSensorFusion);
    
    // Check if sensors are available
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
    
    bool hasAnyBackend = m_magnetometer->isConnectedToBackend() || 
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
    bool hasMagnetometer = m_magnetometer->isConnectedToBackend();
    bool hasAccelerometer = m_accelerometer->isConnectedToBackend();
    
    if (!hasMagnetometer && !hasAccelerometer) {
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
        
        // Apply relative rotation
        if (m_hasInitialOrientation) {
            double relativeW, relativeX, relativeY, relativeZ;
            double initialConjW, initialConjX, initialConjY, initialConjZ;
            quaternionConjugate(m_initialW, m_initialX, m_initialY, m_initialZ, initialConjW, initialConjX, initialConjY, initialConjZ);
            quaternionMultiply(w, x, y, z, initialConjW, initialConjX, initialConjY, initialConjZ, relativeW, relativeX, relativeY, relativeZ);
            emit rotationChanged(relativeW, relativeX, relativeY, relativeZ);
        } else {
            m_initialW = w; m_initialX = x; m_initialY = y; m_initialZ = z;
            m_hasInitialOrientation = true;
            emit rotationChanged(1.0, 0.0, 0.0, 0.0); // Identity quaternion for initial
        }
        return;
    }
    
    // Get raw sensor readings
    double gx = 0.0, gy = 0.0, gz = -1.0; // Default gravity pointing down
    double mx = 1.0, my = 0.0, mz = 0.0;  // Default magnetic north pointing forward
    
    // Get accelerometer data (gravity vector)
    if (hasAccelerometer && m_accelerometer->reading()) {
        QAccelerometerReading *accelReading = m_accelerometer->reading();
        gx = accelReading->x();
        gy = accelReading->y();
        gz = accelReading->z();
        normalizeVector(gx, gy, gz);
    }
    
    // Get magnetometer data (magnetic north vector)
    if (hasMagnetometer && m_magnetometer->reading()) {
        QMagnetometerReading *magReading = m_magnetometer->reading();
        mx = magReading->x();
        my = magReading->y();
        mz = magReading->z();
        normalizeVector(mx, my, mz);
    }
    
    // Calculate absolute orientation quaternion from two reference vectors
    double w, x, y, z;
    quaternionFromTwoVectors(gx, gy, gz, mx, my, mz, w, x, y, z);
    
    // If we don't have an initial orientation, set it now
    if (!m_hasInitialOrientation) {
        m_initialW = w;
        m_initialX = x;
        m_initialY = y;
        m_initialZ = z;
        m_hasInitialOrientation = true;
        qDebug() << "RotationSensor: Set initial orientation - w=" << m_initialW << "x=" << m_initialX << "y=" << m_initialY << "z=" << m_initialZ;
        
        // Emit identity quaternion for initial orientation
        emit rotationChanged(1.0, 0.0, 0.0, 0.0);
        return;
    }
    
    // Calculate relative rotation (current * inverse(initial))
    double relativeW, relativeX, relativeY, relativeZ;
    double initialConjW, initialConjX, initialConjY, initialConjZ;
    quaternionConjugate(m_initialW, m_initialX, m_initialY, m_initialZ, initialConjW, initialConjX, initialConjY, initialConjZ);
    quaternionMultiply(w, x, y, z, initialConjW, initialConjX, initialConjY, initialConjZ, relativeW, relativeX, relativeY, relativeZ);
    
    qDebug() << "RotationSensor (absolute): w=" << w << "x=" << x << "y=" << y << "z=" << z;
    qDebug() << "RotationSensor (relative): w=" << relativeW << "x=" << relativeX << "y=" << relativeY << "z=" << relativeZ;
    emit rotationChanged(relativeW, relativeX, relativeY, relativeZ);
}

void RotationSensor::resetOrientation()
{
    qDebug() << "RotationSensor::resetOrientation() called";
    m_hasInitialOrientation = false;
    // The next reading will set the new initial orientation
}

void RotationSensor::quaternionFromTwoVectors(double gx, double gy, double gz, double mx, double my, double mz, double &w, double &x, double &y, double &z)
{
    // Create orthonormal basis from gravity and magnetic vectors
    // Z-axis: opposite of gravity (up)
    double zx = -gx, zy = -gy, zz = -gz;
    
    // Y-axis: cross product of Z and magnetic field (east)
    double yx, yy, yz;
    vectorCross(zx, zy, zz, mx, my, mz, yx, yy, yz);
    normalizeVector(yx, yy, yz);
    
    // X-axis: cross product of Y and Z (north)
    double xx, xy, xz;
    vectorCross(yx, yy, yz, zx, zy, zz, xx, xy, xz);
    normalizeVector(xx, xy, xz);
    
    // Convert rotation matrix to quaternion
    // Rotation matrix:
    // [xx, yx, zx]
    // [xy, yy, zy]
    // [xz, yz, zz]
    
    double trace = xx + yy + zz;
    
    if (trace > 0.0) {
        double s = sqrt(trace + 1.0) * 2.0; // s = 4 * qw
        w = 0.25 * s;
        x = (yz - zy) / s;
        y = (zx - xz) / s;
        z = (xy - yx) / s;
    } else if ((xx > yy) && (xx > zz)) {
        double s = sqrt(1.0 + xx - yy - zz) * 2.0; // s = 4 * qx
        w = (yz - zy) / s;
        x = 0.25 * s;
        y = (yx + xy) / s;
        z = (zx + xz) / s;
    } else if (yy > zz) {
        double s = sqrt(1.0 + yy - xx - zz) * 2.0; // s = 4 * qy
        w = (zx - xz) / s;
        x = (yx + xy) / s;
        y = 0.25 * s;
        z = (zy + yz) / s;
    } else {
        double s = sqrt(1.0 + zz - xx - yy) * 2.0; // s = 4 * qz
        w = (xy - yx) / s;
        x = (zx + xz) / s;
        y = (zy + yz) / s;
        z = 0.25 * s;
    }
}

void RotationSensor::normalizeVector(double &x, double &y, double &z)
{
    double length = sqrt(x*x + y*y + z*z);
    if (length > 0.0) {
        x /= length;
        y /= length;
        z /= length;
    }
}

double RotationSensor::vectorDot(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return x1*x2 + y1*y2 + z1*z2;
}

void RotationSensor::vectorCross(double x1, double y1, double z1, double x2, double y2, double z2, double &x, double &y, double &z)
{
    x = y1*z2 - z1*y2;
    y = z1*x2 - x1*z2;
    z = x1*y2 - y1*x2;
}

void RotationSensor::quaternionMultiply(double q1w, double q1x, double q1y, double q1z, 
                                       double q2w, double q2x, double q2y, double q2z, 
                                       double &qw, double &qx, double &qy, double &qz)
{
    qw = q1w * q2w - q1x * q2x - q1y * q2y - q1z * q2z;
    qx = q1w * q2x + q1x * q2w + q1y * q2z - q1z * q2y;
    qy = q1w * q2y - q1x * q2z + q1y * q2w + q1z * q2x;
    qz = q1w * q2z + q1x * q2y - q1y * q2x + q1z * q2w;
}

void RotationSensor::quaternionConjugate(double qw, double qx, double qy, double qz, 
                                         double &conjW, double &conjX, double &conjY, double &conjZ)
{
    conjW = qw;
    conjX = -qx;
    conjY = -qy;
    conjZ = -qz;
}