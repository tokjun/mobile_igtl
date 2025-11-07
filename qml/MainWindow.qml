import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Header - reduced height
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2196F3"
            
            Label {
                anchors.centerIn: parent
                text: "OpenIGTLink Mobile"
                font.pixelSize: 18
                font.bold: true
                color: "white"
            }
        }
        
        // Connection Panel
        ConnectionPanel {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        // Orientation View
        OrientationView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}