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

void IGTLClient::sendRotationData(double w, double x, double y, double z, double zOffset)
{
    if (!m_isConnected) {
        return;
    }

#ifdef OPENIGTLINK_FOUND
    // Create transform message
    igtl::TransformMessage::Pointer transformMsg = igtl::TransformMessage::New();
    transformMsg->SetDeviceName("MobileDevice");
    
    // Create a 4x4 transformation matrix from quaternion
    igtl::Matrix4x4 matrix;
    igtl::IdentityMatrix(matrix);
    
    // Convert quaternion (w, x, y, z) to rotation matrix
    // Normalize quaternion first
    double norm = sqrt(w*w + x*x + y*y + z*z);
    if (norm > 0.0) {
        w /= norm; x /= norm; y /= norm; z /= norm;
        
        qDebug() << "IGTLClient: Normalized quaternion - w=" << w << "x=" << x << "y=" << y << "z=" << z;
        
        // Quaternion to rotation matrix conversion
        matrix[0][0] = 1.0 - 2.0 * (y*y + z*z);
        matrix[0][1] = 2.0 * (x*y - w*z);
        matrix[0][2] = 2.0 * (x*z + w*y);
        matrix[1][0] = 2.0 * (x*y + w*z);
        matrix[1][1] = 1.0 - 2.0 * (x*x + z*z);
        matrix[1][2] = 2.0 * (y*z - w*x);
        matrix[2][0] = 2.0 * (x*z - w*y);
        matrix[2][1] = 2.0 * (y*z + w*x);
        matrix[2][2] = 1.0 - 2.0 * (x*x + y*y);
        
        // Rotate about offset point along device's Z-axis
        // Sequence: T = T_to_point * R * T_back
        // T_back: translate by -zOffset along device Z (move rotation center to origin)
        // R: rotate about origin  
        // T_to_point: translate by +zOffset along device Z (move back to offset point)
        
        // The offset point is at (0, 0, zOffset) in device coordinates
        // After rotation, this becomes the translation component
        
        // For rotation about offset point, the final translation is:
        // translation = rotation_center_offset - rotated(rotation_center_offset)
        // where rotation_center_offset = (0, 0, zOffset) in device coordinates
        
        double offset_x = 0.0;
        double offset_y = 0.0; 
        double offset_z = zOffset;
        
        // Apply rotation to the offset point to see where it ends up
        double rotated_offset_x = matrix[0][0] * offset_x + matrix[0][1] * offset_y + matrix[0][2] * offset_z;
        double rotated_offset_y = matrix[1][0] * offset_x + matrix[1][1] * offset_y + matrix[1][2] * offset_z;
        double rotated_offset_z = matrix[2][0] * offset_x + matrix[2][1] * offset_y + matrix[2][2] * offset_z;
        
        //// Translation = original_offset - rotated_offset
        //matrix[0][3] = -offset_x + 0*rotated_offset_x; // = -rotated_offset_x (since offset_x = 0)
        //matrix[1][3] = -offset_y + 0*rotated_offset_y; // = -rotated_offset_y (since offset_y = 0)
        //matrix[2][3] = -offset_z + 0*rotated_offset_z; // = zOffset - rotated_offset_z
        matrix[0][3] = rotated_offset_x; // = -rotated_offset_x (since offset_x = 0)
        matrix[1][3] = rotated_offset_y; // = -rotated_offset_y (since offset_y = 0)
        matrix[2][3] = rotated_offset_z; // = zOffset - rotated_offset_z

        qDebug() << "IGTLClient: Rotation matrix with Z-offset (" << zOffset << "mm):";
        qDebug() << QString("[%1, %2, %3, %4]").arg(matrix[0][0], 6, 'f', 3).arg(matrix[0][1], 6, 'f', 3).arg(matrix[0][2], 6, 'f', 3).arg(matrix[0][3], 6, 'f', 3);
        qDebug() << QString("[%1, %2, %3, %4]").arg(matrix[1][0], 6, 'f', 3).arg(matrix[1][1], 6, 'f', 3).arg(matrix[1][2], 6, 'f', 3).arg(matrix[1][3], 6, 'f', 3);
        qDebug() << QString("[%1, %2, %3, %4]").arg(matrix[2][0], 6, 'f', 3).arg(matrix[2][1], 6, 'f', 3).arg(matrix[2][2], 6, 'f', 3).arg(matrix[2][3], 6, 'f', 3);
    }
    
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
