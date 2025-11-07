#include "rotationsensor.h"
#include <QMagnetometer>
#include <QMagnetometerReading>
#include <QAccelerometer>
#include <QAccelerometerReading>
#include <QGyroscope>
#include <QGyroscopeReading>
#include <QTimer>
#include <QDebug>
#include <cmath>
#include <chrono>

RotationSensor::RotationSensor(QObject *parent)
    : QObject(parent)
    , m_magnetometer(new QMagnetometer(this))
    , m_accelerometer(new QAccelerometer(this))
    , m_gyroscope(new QGyroscope(this))
    , m_timer(new QTimer(this))
    , m_isActive(false)
    , m_initialW(1.0), m_initialX(0.0), m_initialY(0.0), m_initialZ(0.0)
    , m_hasInitialOrientation(false)
    , m_beta(0.1) // Madgwick filter gain
    , m_q0(1.0), m_q1(0.0), m_q2(0.0), m_q3(0.0) // Initial quaternion
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
    if (!m_gyroscope->connectToBackend()) {
        qWarning("Gyroscope is not available on this device");
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
                        m_accelerometer->isConnectedToBackend() ||
                        m_gyroscope->isConnectedToBackend();
    
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
        m_gyroscope->start();
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
        m_gyroscope->stop();
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
    bool hasGyroscope = m_gyroscope->isConnectedToBackend();
    
    if (!hasMagnetometer && !hasAccelerometer && !hasGyroscope) {
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
    double ax = 0.0, ay = 0.0, az = -1.0; // Accelerometer (gravity)
    double mx = 1.0, my = 0.0, mz = 0.0;  // Magnetometer (magnetic north)
    double gyrox = 0.0, gyroy = 0.0, gyroz = 0.0; // Gyroscope (angular velocity)
    
    // Get accelerometer data
    if (hasAccelerometer && m_accelerometer->reading()) {
        QAccelerometerReading *accelReading = m_accelerometer->reading();
        ax = accelReading->x();
        ay = accelReading->y();
        az = accelReading->z();
    }
    
    // Get magnetometer data
    if (hasMagnetometer && m_magnetometer->reading()) {
        QMagnetometerReading *magReading = m_magnetometer->reading();
        mx = magReading->x();
        my = magReading->y();
        mz = magReading->z();
    }
    
    // Get gyroscope data (in rad/s)
    // Try different coordinate mapping for better decoupling
    if (hasGyroscope && m_gyroscope->reading()) {
        QGyroscopeReading *gyroReading = m_gyroscope->reading();
        
        // Original mapping
        double raw_gx = gyroReading->x() * M_PI / 180.0;
        double raw_gy = gyroReading->y() * M_PI / 180.0;
        double raw_gz = gyroReading->z() * M_PI / 180.0;
        
        // Try coordinate system remapping for better decoupling
        // This might need adjustment based on your device orientation
        gyrox = raw_gx;  // X-axis (roll) - seems to work
        gyroy = raw_gy;  // Y-axis (pitch) 
        gyroz = raw_gz;  // Z-axis (yaw)
        
        qDebug() << "Raw gyro - X:" << gyroReading->x() << "Y:" << gyroReading->y() << "Z:" << gyroReading->z() << "deg/s";
        qDebug() << "Mapped gyro - gx:" << gyrox*180.0/M_PI << "gy:" << gyroy*180.0/M_PI << "gz:" << gyroz*180.0/M_PI << "deg/s";
    }
    
    // Simple approach: Use gyroscope if available, fallback to accel+mag
    double w, x, y, z;
    
    // Calculate dt for gyroscope integration and debug output
    static auto lastTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(currentTime - lastTime).count();
    lastTime = currentTime;
    
    // Always use accelerometer + magnetometer for absolute orientation
    // This ensures consistent reference regardless of reset timing
    normalizeVector(ax, ay, az);
    normalizeVector(mx, my, mz);
    quaternionFromTwoVectors(ax, ay, az, mx, my, mz, w, x, y, z);
    
    // If gyroscope is available, use it for smoothing but not primary tracking
    if (hasGyroscope && dt > 0.001 && dt < 0.1) {
        // Apply a small amount of gyroscope-based smoothing to reduce noise
        // but keep accelerometer+magnetometer as the primary source
        double gyro_weight = 0.02; // Very small influence
        if ((abs(gyrox) + abs(gyroy) + abs(gyroz)) > 1e-6) {
            // Slight smoothing based on gyroscope
            double half_dt = dt * 0.5 * gyro_weight;
            double dq0 = -m_q1 * gyrox * half_dt - m_q2 * gyroy * half_dt - m_q3 * gyroz * half_dt;
            double dq1 = m_q0 * gyrox * half_dt + m_q2 * gyroz * half_dt - m_q3 * gyroy * half_dt;
            double dq2 = m_q0 * gyroy * half_dt - m_q1 * gyroz * half_dt + m_q3 * gyrox * half_dt;
            double dq3 = m_q0 * gyroz * half_dt + m_q1 * gyroy * half_dt - m_q2 * gyrox * half_dt;
            
            // Blend with accelerometer+magnetometer result
            w += dq0; x += dq1; y += dq2; z += dq3;
            
            // Normalize
            double norm = sqrt(w*w + x*x + y*y + z*z);
            if (norm > 1e-6) {
                w /= norm; x /= norm; y /= norm; z /= norm;
            }
        }
        
        // Update gyroscope state to current orientation to prevent drift
        m_q0 = w; m_q1 = x; m_q2 = y; m_q3 = z;
    }
    
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
    
    qDebug() << "RotationSensor: Accel - ax=" << ax << "ay=" << ay << "az=" << az;
    qDebug() << "RotationSensor: Mag - mx=" << mx << "my=" << my << "mz=" << mz;
    qDebug() << "RotationSensor: Gyro - gx=" << gyrox*180.0/M_PI << "gy=" << gyroy*180.0/M_PI << "gz=" << gyroz*180.0/M_PI << "deg/s";
    qDebug() << "RotationSensor: dt=" << dt << "s";
    qDebug() << "RotationSensor (absolute): w=" << w << "x=" << x << "y=" << y << "z=" << z;
    qDebug() << "RotationSensor (relative): w=" << relativeW << "x=" << relativeX << "y=" << relativeY << "z=" << relativeZ;
    emit rotationChanged(relativeW, relativeX, relativeY, relativeZ);
}

void RotationSensor::resetOrientation()
{
    qDebug() << "RotationSensor::resetOrientation() called";
    
    // Simply reset the relative orientation tracking
    // Since we use accelerometer+magnetometer as primary source,
    // the next reading will automatically be correct
    m_hasInitialOrientation = false;
    
    qDebug() << "RotationSensor: Reset complete - next reading will be new reference";
}

void RotationSensor::quaternionFromTwoVectors(double gx, double gy, double gz, double mx, double my, double mz, double &w, double &x, double &y, double &z)
{
    // Create orthonormal basis from gravity and magnetic vectors (right-handed)
    // Z-axis: opposite of gravity (up)
    double zx = -gx, zy = -gy, zz = -gz;
    
    // X-axis: cross product of magnetic field and Z (east)
    double xx, xy, xz;
    vectorCross(mx, my, mz, zx, zy, zz, xx, xy, xz);
    normalizeVector(xx, xy, xz);
    
    // Y-axis: cross product of Z and X (north)
    double yx, yy, yz;
    vectorCross(zx, zy, zz, xx, xy, xz, yx, yy, yz);
    normalizeVector(yx, yy, yz);
    
    // Convert rotation matrix to quaternion (right-handed coordinate system)
    // Rotation matrix:
    // [xx, yx, zx]  (X=east, Y=north, Z=up)
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

void RotationSensor::madgwickUpdate(double gx, double gy, double gz, double ax, double ay, double az, double mx, double my, double mz, double dt)
{
    // Safety checks for NaN prevention
    if (!std::isfinite(gx) || !std::isfinite(gy) || !std::isfinite(gz) ||
        !std::isfinite(ax) || !std::isfinite(ay) || !std::isfinite(az) ||
        !std::isfinite(mx) || !std::isfinite(my) || !std::isfinite(mz) ||
        !std::isfinite(dt) || dt <= 0.0) {
        qDebug() << "Madgwick: Invalid input data, skipping update";
        return;
    }
    
    // Check if accelerometer data is valid (not zero vector)
    double accel_norm = sqrt(ax * ax + ay * ay + az * az);
    if (accel_norm < 1e-6) {
        qDebug() << "Madgwick: Invalid accelerometer data, skipping update";
        return;
    }
    
    double recipNorm;
    double s0, s1, s2, s3;
    double qDot1, qDot2, qDot3, qDot4;
    double hx, hy;
    double _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3;
    double q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    // Use IMU algorithm if magnetometer measurement is invalid (avoids NaN in magnetometer normalisation)
    if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
        // IMU algorithm without magnetometer
        recipNorm = 1.0 / sqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Gradient decent algorithm corrective step
        s0 = -2.0 * (2.0 * m_q1 * m_q3 - 2.0 * m_q0 * m_q2 - ax);
        s1 = -2.0 * (2.0 * m_q0 * m_q1 + 2.0 * m_q2 * m_q3 - ay);
        s2 = -2.0 * (1.0 - 2.0 * m_q1 * m_q1 - 2.0 * m_q2 * m_q2 - az);
        s3 = -4.0 * m_q3 * (1.0 - 2.0 * m_q2 * m_q2 - 2.0 * m_q3 * m_q3 - az) + (-4.0) * m_q1 * (2.0 * m_q1 * m_q3 - 2.0 * m_q0 * m_q2 - ax);

        double s_norm = sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        if (s_norm < 1e-12) {
            qDebug() << "Madgwick: Invalid s vector norm, skipping feedback";
            qDot1 = 0.5 * (-m_q1 * gx - m_q2 * gy - m_q3 * gz);
            qDot2 = 0.5 * (m_q0 * gx + m_q2 * gz - m_q3 * gy);
            qDot3 = 0.5 * (m_q0 * gy - m_q1 * gz + m_q3 * gx);
            qDot4 = 0.5 * (m_q0 * gz + m_q1 * gy - m_q2 * gx);
        } else {
            recipNorm = 1.0 / s_norm;
            s0 *= recipNorm;
            s1 *= recipNorm;
            s2 *= recipNorm;
            s3 *= recipNorm;
        }

        // Apply feedback step
        qDot1 = 0.5 * (-m_q1 * gx - m_q2 * gy - m_q3 * gz) - m_beta * s0;
        qDot2 = 0.5 * (m_q0 * gx + m_q2 * gz - m_q3 * gy) - m_beta * s1;
        qDot3 = 0.5 * (m_q0 * gy - m_q1 * gz + m_q3 * gx) - m_beta * s2;
        qDot4 = 0.5 * (m_q0 * gz + m_q1 * gy - m_q2 * gx) - m_beta * s3;
    } else {
        // Full Madgwick algorithm with magnetometer
        
        // Normalise accelerometer measurement
        recipNorm = 1.0 / sqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Normalise magnetometer measurement
        recipNorm = 1.0 / sqrt(mx * mx + my * my + mz * mz);
        mx *= recipNorm;
        my *= recipNorm;
        mz *= recipNorm;

        // Auxiliary variables to avoid repeated arithmetic
        _2q0mx = 2.0 * m_q0 * mx;
        _2q0my = 2.0 * m_q0 * my;
        _2q0mz = 2.0 * m_q0 * mz;
        _2q1mx = 2.0 * m_q1 * mx;
        _2q0 = 2.0 * m_q0;
        _2q1 = 2.0 * m_q1;
        _2q2 = 2.0 * m_q2;
        _2q3 = 2.0 * m_q3;
        _2q0q2 = 2.0 * m_q0 * m_q2;
        _2q2q3 = 2.0 * m_q2 * m_q3;
        q0q0 = m_q0 * m_q0;
        q0q1 = m_q0 * m_q1;
        q0q2 = m_q0 * m_q2;
        q0q3 = m_q0 * m_q3;
        q1q1 = m_q1 * m_q1;
        q1q2 = m_q1 * m_q2;
        q1q3 = m_q1 * m_q3;
        q2q2 = m_q2 * m_q2;
        q2q3 = m_q2 * m_q3;
        q3q3 = m_q3 * m_q3;

        // Reference direction of Earth's magnetic field
        hx = mx * q0q0 - _2q0my * m_q3 + _2q0mz * m_q2 + mx * q1q1 + _2q1 * my * m_q2 + _2q1 * mz * m_q3 - mx * q2q2 - mx * q3q3;
        hy = _2q0mx * m_q3 + my * q0q0 - _2q0mz * m_q1 + _2q1mx * m_q2 - my * q1q1 + my * q2q2 + _2q2 * mz * m_q3 - my * q3q3;
        _2bx = sqrt(hx * hx + hy * hy);
        _2bz = -_2q0mx * m_q2 + _2q0my * m_q1 + mz * q0q0 + _2q1mx * m_q3 - mz * q1q1 + _2q2 * my * m_q3 - mz * q2q2 + mz * q3q3;
        _4bx = 2.0 * _2bx;
        _4bz = 2.0 * _2bz;

        // Gradient descent algorithm corrective step
        s0 = -_2q2 * (2.0 * q1q3 - _2q0q2 - ax) + _2q1 * (2.0 * q0q1 + _2q2q3 - ay) - _2bz * m_q2 * (_2bx * (0.5 - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * m_q3 + _2bz * m_q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * m_q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5 - q1q1 - q2q2) - mz);
        s1 = _2q3 * (2.0 * q1q3 - _2q0q2 - ax) + _2q0 * (2.0 * q0q1 + _2q2q3 - ay) - 4.0 * m_q1 * (1.0 - 2.0 * q1q1 - 2.0 * q2q2 - az) + _2bz * m_q3 * (_2bx * (0.5 - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * m_q2 + _2bz * m_q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * m_q3 - _4bz * m_q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5 - q1q1 - q2q2) - mz);
        s2 = -_2q0 * (2.0 * q1q3 - _2q0q2 - ax) + _2q3 * (2.0 * q0q1 + _2q2q3 - ay) - 4.0 * m_q2 * (1.0 - 2.0 * q1q1 - 2.0 * q2q2 - az) + (-_4bx * m_q2 - _2bz * m_q0) * (_2bx * (0.5 - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * m_q1 + _2bz * m_q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * m_q0 - _4bz * m_q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5 - q1q1 - q2q2) - mz);
        s3 = _2q1 * (2.0 * q1q3 - _2q0q2 - ax) + _2q2 * (2.0 * q0q1 + _2q2q3 - ay) + (-_4bx * m_q3 + _2bz * m_q1) * (_2bx * (0.5 - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * m_q0 + _2bz * m_q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * m_q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5 - q1q1 - q2q2) - mz);
        
        recipNorm = 1.0 / sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); 
        s0 *= recipNorm;
        s1 *= recipNorm;
        s2 *= recipNorm;
        s3 *= recipNorm;

        // Apply feedback step
        qDot1 = 0.5 * (-m_q1 * gx - m_q2 * gy - m_q3 * gz) - m_beta * s0;
        qDot2 = 0.5 * (m_q0 * gx + m_q2 * gz - m_q3 * gy) - m_beta * s1;
        qDot3 = 0.5 * (m_q0 * gy - m_q1 * gz + m_q3 * gx) - m_beta * s2;
        qDot4 = 0.5 * (m_q0 * gz + m_q1 * gy - m_q2 * gx) - m_beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    m_q0 += qDot1 * dt;
    m_q1 += qDot2 * dt;
    m_q2 += qDot3 * dt;
    m_q3 += qDot4 * dt;

    // Normalise quaternion
    double q_norm = sqrt(m_q0 * m_q0 + m_q1 * m_q1 + m_q2 * m_q2 + m_q3 * m_q3);
    if (q_norm < 1e-12) {
        qDebug() << "Madgwick: Quaternion norm too small, resetting to identity";
        m_q0 = 1.0; m_q1 = 0.0; m_q2 = 0.0; m_q3 = 0.0;
    } else {
        recipNorm = 1.0 / q_norm;
        m_q0 *= recipNorm;
        m_q1 *= recipNorm;
        m_q2 *= recipNorm;
        m_q3 *= recipNorm;
    }
    
    // Final validation
    if (!std::isfinite(m_q0) || !std::isfinite(m_q1) || !std::isfinite(m_q2) || !std::isfinite(m_q3)) {
        qDebug() << "Madgwick: NaN detected in quaternion, resetting to identity";
        m_q0 = 1.0; m_q1 = 0.0; m_q2 = 0.0; m_q3 = 0.0;
    }
}