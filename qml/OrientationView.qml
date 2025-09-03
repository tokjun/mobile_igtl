import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GroupBox {
    id: root
    title: "Device Orientation"
    
    property real orientationX: 0.0
    property real orientationY: 0.0
    property real orientationZ: 0.0
    
    // Connect to orientation data signals
    Connections {
        target: appController
        function onOrientationDataSent(x, y, z) {
            root.orientationX = x
            root.orientationY = y
            root.orientationZ = z
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 15
        
        // Orientation display
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 20
            rowSpacing: 10
            
            Label {
                text: "X (Roll):"
                font.bold: true
            }
            Label {
                text: root.orientationX.toFixed(2) + "°"
                font.family: "monospace"
            }
            
            Label {
                text: "Y (Pitch):"
                font.bold: true
            }
            Label {
                text: root.orientationY.toFixed(2) + "°"
                font.family: "monospace"
            }
            
            Label {
                text: "Z (Yaw):"
                font.bold: true
            }
            Label {
                text: root.orientationZ.toFixed(2) + "°"
                font.family: "monospace"
            }
        }
        
        // Visual representation
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#ECEFF1"
            radius: 8
            border.color: "#CFD8DC"
            border.width: 1
            
            Rectangle {
                id: deviceRepresentation
                width: 60
                height: 100
                anchors.centerIn: parent
                color: "#607D8B"
                radius: 8
                
                // Apply rotation based on orientation
                transform: [
                    Rotation {
                        axis.x: 1
                        axis.y: 0
                        axis.z: 0
                        angle: root.orientationX
                    },
                    Rotation {
                        axis.x: 0
                        axis.y: 1
                        axis.z: 0
                        angle: root.orientationY
                    },
                    Rotation {
                        axis.x: 0
                        axis.y: 0
                        axis.z: 1
                        angle: root.orientationZ
                    }
                ]
                
                // Device screen indicator
                Rectangle {
                    width: parent.width * 0.8
                    height: parent.height * 0.6
                    anchors.centerIn: parent
                    color: "#37474F"
                    radius: 4
                    
                    Rectangle {
                        width: parent.width * 0.9
                        height: parent.height * 0.9
                        anchors.centerIn: parent
                        color: "#263238"
                        radius: 2
                    }
                }
            }
            
            Label {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.margins: 8
                text: "Device Orientation Preview"
                font.pixelSize: 12
                color: "#546E7A"
            }
        }
        
        // Control buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: "Start Sending"
                enabled: appController.isConnected
                Layout.fillWidth: true
                onClicked: appController.startSendingOrientation()
            }
            
            Button {
                text: "Stop Sending"
                Layout.fillWidth: true
                onClicked: appController.stopSendingOrientation()
            }
        }
        
        // Instructions
        Label {
            Layout.fillWidth: true
            text: appController.isConnected ? 
                  "Connected! Tap 'Start Sending' to transmit orientation data." :
                  "Please connect to a server first to send orientation data."
            wrapMode: Text.WordWrap
            color: "#666"
            font.italic: true
        }
    }
}