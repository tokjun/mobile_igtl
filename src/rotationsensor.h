#pragma once

#include <QObject>
#include <QTimer>

class QRotationSensor;
class QRotationReading;

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
    void readingSensor();

private:
    QRotationSensor *m_rotationSensor;
    QTimer *m_timer;
    bool m_isActive;
};