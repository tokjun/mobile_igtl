import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GroupBox {
    id: root
    title: "Server Connection"
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        // Status indicator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: appController.isConnected ? "#4CAF50" : "#F44336"
            radius: 5
            
            Label {
                anchors.centerIn: parent
                text: appController.connectionStatus
                color: "white"
                font.bold: true
            }
        }
        
        // Server settings
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 10
            
            Label {
                text: "Host:"
            }
            
            TextField {
                id: hostField
                Layout.fillWidth: true
                text: appController.serverHost
                placeholderText: "e.g., 192.168.1.100"
                onTextChanged: appController.serverHost = text
            }
            
            Label {
                text: "Port:"
            }
            
            TextField {
                id: portField
                Layout.preferredWidth: 100
                text: appController.serverPort.toString()
                validator: IntValidator { bottom: 1; top: 65535 }
                onTextChanged: {
                    if (text.length > 0) {
                        appController.serverPort = parseInt(text)
                    }
                }
            }
        }
        
        // Connection buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: "Connect"
                enabled: !appController.isConnected && hostField.text.length > 0
                Layout.fillWidth: true
                onClicked: appController.connectToServer()
            }
            
            Button {
                text: "Disconnect"
                enabled: appController.isConnected
                Layout.fillWidth: true
                onClicked: appController.disconnectFromServer()
            }
        }
    }
}