#include "igtlclient.h"
#include <QDebug>

// OpenIGTLink includes
#ifdef OPENIGTLINK_FOUND
#include "igtlClientSocket.h"
#include "igtlTransformMessage.h"
#include "igtlTimeStamp.h"
#include "igtlMath.h"
#include <cmath>
#else
// Stub implementations when OpenIGTLink is not available
namespace igtl {
    class ClientSocket {
    public:
        int ConnectToServer(const char*, int) { return 0; }
        int CloseSocket() { return 0; }
        int Send(void*, int) { return 0; }
        bool GetConnected() { return false; }
    };
    
    class TransformMessage {
    public:
        typedef void* Pointer;
        static Pointer New() { return nullptr; }
        void SetDeviceName(const char*) {}
        void SetMatrix(void*) {}
        void Pack() {}
        void* GetPackPointer() { return nullptr; }
        int GetPackSize() { return 0; }
    };
    
    class TimeStamp {
    public:
        static void GetTime(void*) {}
    };
}
#endif

IGTLClient::IGTLClient(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
{
#ifdef OPENIGTLINK_FOUND
    m_socket = igtl::ClientSocket::New();
#endif
}

IGTLClient::~IGTLClient()
{
    disconnectFromServer();
}

bool IGTLClient::connectToServer(const QString &hostname, int port)
{
    qDebug() << "IGTLClient: connectToServer called with" << hostname << ":" << port;
    
#ifdef OPENIGTLINK_FOUND
    qDebug() << "OPENIGTLINK_FOUND is defined";
#else
    qDebug() << "OPENIGTLINK_FOUND is NOT defined";
#endif
    
    if (m_isConnected) {
        qDebug() << "IGTLClient: Already connected";
        return true;
    }

#ifdef OPENIGTLINK_FOUND
    qDebug() << "IGTLClient: Using real OpenIGTLink library";
    int result = m_socket->ConnectToServer(hostname.toLocal8Bit().data(), port);
    qDebug() << "IGTLClient: ConnectToServer returned:" << result;
    if (result == 0) {
        m_isConnected = true;
        emit connected();
        return true;
    } else {
        emit connectionError("Failed to connect to server");
        return false;
    }
#else
    // Stub implementation - always fail gracefully
    qDebug() << "IGTLClient: Using stub implementation";
    emit connectionError("OpenIGTLink library not available");
    return false;
#endif
}

void IGTLClient::disconnectFromServer()
{
    if (m_isConnected) {
        m_socket->CloseSocket();
        m_isConnected = false;
        emit disconnected();
    }
}

bool IGTLClient::isConnected() const
{
    return m_isConnected;
}

void IGTLClient::sendOrientationData(double x, double y, double z)
{
    if (!m_isConnected) {
        return;
    }

#ifdef OPENIGTLINK_FOUND
    // Create transform message
    igtl::TransformMessage::Pointer transformMsg = igtl::TransformMessage::New();
    transformMsg->SetDeviceName("MobileDevice");
    
    // Create a 4x4 transformation matrix representing rotation
    igtl::Matrix4x4 matrix;
    igtl::IdentityMatrix(matrix);
    
    // Convert Euler angles to rotation matrix (simplified rotation around Z-axis)
    double radZ = z * M_PI / 180.0; // Convert to radians
    matrix[0][0] = cos(radZ);
    matrix[0][1] = -sin(radZ);
    matrix[1][0] = sin(radZ);
    matrix[1][1] = cos(radZ);
    
    transformMsg->SetMatrix(matrix);
    
    // Set timestamp
    igtl::TimeStamp::Pointer ts = igtl::TimeStamp::New();
    ts->GetTime();
    transformMsg->SetTimeStamp(ts);
    
    // Pack and send
    transformMsg->Pack();
    m_socket->Send(transformMsg->GetPackPointer(), transformMsg->GetPackSize());
#endif
}