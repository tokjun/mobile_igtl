#pragma once

#include <QObject>
#include <QTimer>

class QRotationSensor;
class QRotationReading;
class QMagnetometer;
class QMagnetometerReading;
class QAccelerometer;
class QAccelerometerReading;

class RotationSensor : public QObject
{
    Q_OBJECT

public:
    explicit RotationSensor(QObject *parent = nullptr);
    ~RotationSensor();

    void start();
    void stop();
    bool isActive() const;

signals:
    void rotationChanged(double w, double x, double y, double z);

private slots:
    void performSensorFusion();

private:
    QRotationSensor *m_rotationSensor;
    QMagnetometer *m_magnetometer;
    QAccelerometer *m_accelerometer;
    QTimer *m_timer;
    bool m_isActive;
    
    // Sensor fusion helpers
    void quaternionFromEuler(double roll, double pitch, double yaw, double &w, double &x, double &y, double &z);
    double calculateMagneticHeading(double mx, double my, double mz, double roll, double pitch);
};