import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    
    ColumnLayout {
        width: root.width
        spacing: 20
        
        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "#2196F3"
            
            Label {
                anchors.centerIn: parent
                text: "OpenIGTLink Mobile"
                font.pixelSize: 24
                font.bold: true
                color: "white"
            }
        }
        
        // Connection Panel
        ConnectionPanel {
            Layout.fillWidth: true
            Layout.margins: 20
        }
        
        // Orientation View
        OrientationView {
            Layout.fillWidth: true
            Layout.margins: 20
        }
    }
}