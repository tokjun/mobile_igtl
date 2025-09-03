#pragma once

#include <QObject>
#include <QTimer>

class QRotationSensor;
class QRotationReading;

class OrientationSensor : public QObject
{
    Q_OBJECT

public:
    explicit OrientationSensor(QObject *parent = nullptr);
    ~OrientationSensor();

    void start();
    void stop();
    bool isActive() const;

signals:
    void orientationChanged(double x, double y, double z);

private slots:
    void readingSensor();

private:
    QRotationSensor *m_rotationSensor;
    QTimer *m_timer;
    bool m_isActive;
};