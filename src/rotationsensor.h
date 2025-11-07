#pragma once

#include <QObject>
#include <QTimer>

class QMagnetometer;
class QMagnetometerReading;
class QAccelerometer;
class QAccelerometerReading;
class QGyroscope;
class QGyroscopeReading;

class RotationSensor : public QObject
{
    Q_OBJECT

public:
    explicit RotationSensor(QObject *parent = nullptr);
    ~RotationSensor();

    void start();
    void stop();
    bool isActive() const;
    
    void resetOrientation();

signals:
    void rotationChanged(double w, double x, double y, double z);

private slots:
    void performSensorFusion();

private:
    QMagnetometer *m_magnetometer;
    QAccelerometer *m_accelerometer;
    QGyroscope *m_gyroscope;
    QTimer *m_timer;
    bool m_isActive;
    
    // Initial orientation for relative calculations
    double m_initialW, m_initialX, m_initialY, m_initialZ;
    bool m_hasInitialOrientation;
    
    // Sensor fusion helpers
    void quaternionFromTwoVectors(double gx, double gy, double gz, double mx, double my, double mz, double &w, double &x, double &y, double &z);
    void normalizeVector(double &x, double &y, double &z);
    double vectorDot(double x1, double y1, double z1, double x2, double y2, double z2);
    void vectorCross(double x1, double y1, double z1, double x2, double y2, double z2, double &x, double &y, double &z);
    
    // Madgwick filter for IMU fusion
    void madgwickUpdate(double gx, double gy, double gz, double ax, double ay, double az, double mx, double my, double mz, double dt);
    double m_beta; // Madgwick filter gain
    double m_q0, m_q1, m_q2, m_q3; // Madgwick quaternion state
    void quaternionMultiply(double q1w, double q1x, double q1y, double q1z, double q2w, double q2x, double q2y, double q2z, double &qw, double &qx, double &qy, double &qz);
    void quaternionConjugate(double qw, double qx, double qy, double qz, double &conjW, double &conjX, double &conjY, double &conjZ);
};