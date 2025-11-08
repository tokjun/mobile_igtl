import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GroupBox {
    id: root
    title: "Device Rotation (Quaternion)"
    
    property real rotationW: 1.0
    property real rotationX: 0.0
    property real rotationY: 0.0
    property real rotationZ: 0.0
    property real zOffset: 0.0
    
    // Connect to orientation data signals
    Connections {
        target: appController
        function onRotationDataSent(w, x, y, z) {
            root.rotationW = w
            root.rotationX = x
            root.rotationY = y
            root.rotationZ = z
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 15
        
        // Orientation display
        GridLayout {
            Layout.fillWidth: true
            columns: 4
            columnSpacing: 15
            rowSpacing: 8
            
            Label {
                text: "W:"
                font.bold: true
            }
            Label {
                text: root.rotationW.toFixed(3)
                font.family: "monospace"
            }
            
            Label {
                text: "X:"
                font.bold: true
            }
            Label {
                text: root.rotationX.toFixed(3)
                font.family: "monospace"
            }
            
            Label {
                text: "Y:"
                font.bold: true
            }
            Label {
                text: root.rotationY.toFixed(3)
                font.family: "monospace"
            }
            
            Label {
                text: "Z:"
                font.bold: true
            }
            Label {
                text: root.rotationZ.toFixed(3)
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
                
                // Apply rotation based on quaternion
                // Convert quaternion to Euler angles for visualization
                property real eulerX: Math.atan2(2.0 * (root.rotationW * root.rotationX + root.rotationY * root.rotationZ), 1.0 - 2.0 * (root.rotationX * root.rotationX + root.rotationY * root.rotationY)) * 180.0 / Math.PI
                property real eulerY: Math.asin(Math.max(-1.0, Math.min(1.0, 2.0 * (root.rotationW * root.rotationY - root.rotationZ * root.rotationX)))) * 180.0 / Math.PI
                property real eulerZ: Math.atan2(2.0 * (root.rotationW * root.rotationZ + root.rotationX * root.rotationY), 1.0 - 2.0 * (root.rotationY * root.rotationY + root.rotationZ * root.rotationZ)) * 180.0 / Math.PI
                
                transform: [
                    Rotation {
                        axis.x: 1
                        axis.y: 0
                        axis.z: 0
                        angle: deviceRepresentation.eulerX
                    },
                    Rotation {
                        axis.x: 0
                        axis.y: 1
                        axis.z: 0
                        angle: deviceRepresentation.eulerY
                    },
                    Rotation {
                        axis.x: 0
                        axis.y: 0
                        axis.z: 1
                        angle: deviceRepresentation.eulerZ
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
                text: "Device Rotation Preview"
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
                onClicked: appController.startSendingRotation()
            }
            
            Button {
                text: "Stop Sending"
                Layout.fillWidth: true
                onClicked: appController.stopSendingRotation()
            }
        }
        
        // Reset orientation button
        Button {
            text: "Reset Orientation"
            Layout.fillWidth: true
            onClicked: appController.resetOrientation()
            
            // Visual feedback
            background: Rectangle {
                color: parent.pressed ? "#FFC107" : "#FF9800"
                radius: 4
                border.width: 1
                border.color: "#FF8F00"
            }
        }
        
        // Z-axis offset slider
        Column {
            Layout.fillWidth: true
            spacing: 8
            
            Label {
                text: "Z-axis Offset: " + root.zOffset.toFixed(3) + " mm"
                font.bold: true
            }
            
            Rectangle {
                id: sliderTrack
                width: parent.width
                height: 40
                color: "#ECEFF1"
                radius: 20
                border.color: "#CFD8DC"
                border.width: 1
                
                Rectangle {
                    id: sliderHandle
                    width: 30
                    height: 30
                    radius: 15
                    color: "#2196F3"
                    y: (parent.height - height) / 2
                    x: parent.width / 2 - width / 2 // Center initially
                    
                    MouseArea {
                        anchors.fill: parent
                        drag.target: parent
                        drag.axis: Drag.XAxis
                        drag.minimumX: 0
                        drag.maximumX: sliderTrack.width - sliderHandle.width
                        
                        onPositionChanged: {
                            if (drag.active) {
                                // Convert slider position to offset value
                                // Center = 0, full left = -500mm, full right = +500mm
                                var center = (sliderTrack.width - sliderHandle.width) / 2
                                var position = sliderHandle.x - center
                                var maxRange = center
                                root.zOffset = (position / maxRange) * 500
                                
                                // Update application controller
                                appController.zAxisOffset = root.zOffset
                            }
                        }
                    }
                    
                    // Visual feedback
                    Rectangle {
                        anchors.centerIn: parent
                        width: 10
                        height: 10
                        radius: 5
                        color: "#FFFFFF"
                    }
                }
            }
        }
        
        // Instructions
        Label {
            Layout.fillWidth: true
            text: appController.isConnected ? 
                  "Connected! Tap 'Start Sending' to transmit rotation data." :
                  "Please connect to a server first to send rotation data."
            wrapMode: Text.WordWrap
            color: "#666"
            font.italic: true
        }
    }
}