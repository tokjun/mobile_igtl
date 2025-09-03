#include "igtlclient.h"

// OpenIGTLink includes
#ifdef OPENIGTLINK_FOUND
#include "igtlClientSocket.h"
#include "igtlOrientationMessage.h"
#include "igtlTimeStamp.h"
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
    
    class OrientationMessage {
    public:
        static Pointer New() { return nullptr; }
        void SetDeviceName(const char*) {}
        void SetOrientation(float, float, float, float) {}
        void Pack() {}
        void* GetPackPointer() { return nullptr; }
        int GetPackSize() { return 0; }
        typedef void* Pointer;
    };
    
    class TimeStamp {
    public:
        static void GetTime(void*) {}
    };
}
#endif

IGTLClient::IGTLClient(QObject *parent)
    : QObject(parent)
    , m_socket(std::make_unique<igtl::ClientSocket>())
    , m_isConnected(false)
{
}

IGTLClient::~IGTLClient()
{
    disconnectFromServer();
}

bool IGTLClient::connectToServer(const QString &hostname, int port)
{
    if (m_isConnected) {
        return true;
    }

#ifdef OPENIGTLINK_FOUND
    int result = m_socket->ConnectToServer(hostname.toLocal8Bit().data(), port);
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
    // Create orientation message
    igtl::OrientationMessage::Pointer orientMsg = igtl::OrientationMessage::New();
    orientMsg->SetDeviceName("MobileDevice");
    
    // Convert Euler angles to quaternion (simplified - you may want to use proper conversion)
    // For now, just sending the raw values as quaternion components
    orientMsg->SetOrientation(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0f);
    
    // Set timestamp
    igtl::TimeStamp::Pointer ts = igtl::TimeStamp::New();
    igtl::TimeStamp::GetTime(ts);
    orientMsg->SetTimeStamp(ts);
    
    // Pack and send
    orientMsg->Pack();
    m_socket->Send(orientMsg->GetPackPointer(), orientMsg->GetPackSize());
#endif
}