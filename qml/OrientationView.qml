import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GroupBox {
    id: root
    title: "Device Orientation"
    
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
        spacing: 3
        
        
        // Attitude Indicator (Artificial Horizon)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            color: "#2E2E2E"
            radius: 8
            border.color: "#CFD8DC"
            border.width: 1
            
            Item {
                id: attitudeIndicator
                width: Math.min(parent.width - 20, parent.height - 20)
                height: width
                anchors.centerIn: parent
                
                // Convert quaternion to Euler angles - corrected for device vs aircraft coordinates
                property real deviceRoll: Math.atan2(2.0 * (root.rotationW * root.rotationX + root.rotationY * root.rotationZ), 1.0 - 2.0 * (root.rotationX * root.rotationX + root.rotationY * root.rotationY)) * 180.0 / Math.PI
                property real deviceYaw: Math.atan2(2.0 * (root.rotationW * root.rotationZ + root.rotationX * root.rotationY), 1.0 - 2.0 * (root.rotationY * root.rotationY + root.rotationZ * root.rotationZ)) * 180.0 / Math.PI
                
                // Map device axes to aircraft attitude indicator
                property real pitch: deviceRoll    // Aircraft pitch = Device roll (about X)
                property real roll: deviceYaw      // Aircraft roll = Device yaw (about Z)
                
                // Canvas-based attitude indicator with proper circular clipping
                Canvas {
                    id: attitudeCanvas
                    width: parent.width * 0.85
                    height: width
                    anchors.centerIn: parent
                    
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        
                        // Create circular clipping path
                        var radius = width / 2
                        var centerX = width / 2
                        var centerY = height / 2
                        
                        ctx.save()
                        ctx.beginPath()
                        ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI)
                        ctx.clip()
                        
                        // Calculate horizon position and rotation
                        var pitchOffset = attitudeIndicator.pitch * 2
                        var rollAngle = -attitudeIndicator.roll * Math.PI / 180
                        
                        // Save context for horizon rotation
                        ctx.save()
                        ctx.translate(centerX, centerY)
                        ctx.rotate(rollAngle)
                        ctx.translate(-centerX, -centerY + pitchOffset)
                        
                        // Draw sky (blue top)
                        ctx.fillStyle = "#4FC3F7"
                        ctx.fillRect(-width, -height, width * 3, centerY + height)
                        
                        // Draw ground (brown bottom)  
                        ctx.fillStyle = "#8D6E63"
                        ctx.fillRect(-width, centerY, width * 3, height * 2)
                        
                        // Draw horizon line
                        ctx.strokeStyle = "white"
                        ctx.lineWidth = 2
                        ctx.beginPath()
                        ctx.moveTo(-width, centerY)
                        ctx.lineTo(width * 2, centerY)
                        ctx.stroke()
                        
                        // Draw pitch ticks and numbers
                        ctx.strokeStyle = "white"
                        ctx.fillStyle = "white"
                        ctx.lineWidth = 1
                        ctx.font = "12px Arial"
                        ctx.textAlign = "center"
                        
                        // Sky pitch lines (positive)
                        var pitchLines = [10, 20, 30]
                        for (var i = 0; i < pitchLines.length; i++) {
                            var pitch = pitchLines[i]
                            var y = centerY - pitch * 2
                            
                            // Draw tick line
                            ctx.beginPath()
                            ctx.moveTo(centerX - 30, y)
                            ctx.lineTo(centerX + 30, y)
                            ctx.stroke()
                            
                            // Draw numbers on both sides
                            ctx.fillText(pitch.toString(), centerX - 45, y + 4)
                            ctx.fillText(pitch.toString(), centerX + 45, y + 4)
                        }
                        
                        // Ground pitch lines (negative)
                        var groundPitchLines = [10, 20, 30]
                        for (var j = 0; j < groundPitchLines.length; j++) {
                            var groundPitch = groundPitchLines[j]
                            var groundY = centerY + groundPitch * 2
                            
                            // Draw tick line
                            ctx.beginPath()
                            ctx.moveTo(centerX - 30, groundY)
                            ctx.lineTo(centerX + 30, groundY)
                            ctx.stroke()
                            
                            // Draw numbers on both sides
                            ctx.fillText(groundPitch.toString(), centerX - 45, groundY + 4)
                            ctx.fillText(groundPitch.toString(), centerX + 45, groundY + 4)
                        }
                        
                        ctx.restore() // Restore from rotation
                        ctx.restore() // Restore from clipping
                    }
                    
                    // Repaint when attitude changes
                    Connections {
                        target: attitudeIndicator
                        function onPitchChanged() { attitudeCanvas.requestPaint() }
                        function onRollChanged() { attitudeCanvas.requestPaint() }
                    }
                }
                
                // Bank angle markings (roll ticks) around the circle edge
                Repeater {
                    model: [-60, -45, -30, -20, -10, 0, 10, 20, 30, 45, 60]
                    Rectangle {
                        width: modelData % 30 === 0 ? 3 : (modelData % 10 === 0 ? 2 : 1)
                        height: modelData % 30 === 0 ? 15 : (modelData % 10 === 0 ? 10 : 6)
                        color: "white"
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        
                        transform: Rotation {
                            origin.x: width / 2
                            origin.y: attitudeIndicator.height / 2 - 5
                            angle: modelData
                        }
                        
                        // Add numbers for major angles
                        Text {
                            visible: modelData % 30 === 0 && modelData !== 0
                            text: Math.abs(modelData)
                            color: "white"
                            font.pixelSize: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.bottom
                            anchors.topMargin: 2
                        }
                    }
                }
                
                // Bank angle triangle pointer - simple rotation approach
                Item {
                    id: triangleContainer
                    width: parent.width
                    height: parent.height
                    anchors.centerIn: parent
                    
                    transform: Rotation {
                        origin.x: triangleContainer.width / 2
                        origin.y: triangleContainer.height / 2
                        angle: attitudeIndicator.roll
                    }
                    
                    // Triangle positioned at top of circle
                    Canvas {
                        width: 12
                        height: 8
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 18
                        
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            ctx.fillStyle = "white"
                            ctx.beginPath()
                            ctx.moveTo(width / 2, 0)
                            ctx.lineTo(0, height)
                            ctx.lineTo(width, height)
                            ctx.closePath()
                            ctx.fill()
                        }
                    }
                }
                
                // Fixed aircraft symbol (always in center, outside clipping)
                Item {
                    anchors.centerIn: parent
                    z: 10  // Above everything else
                    
                    // Center dot
                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: "white"
                        anchors.centerIn: parent
                    }
                    
                    // Left wing
                    Rectangle {
                        width: 40
                        height: 3
                        color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 3
                    }
                    
                    // Right wing
                    Rectangle {
                        width: 40
                        height: 3
                        color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 3
                    }
                }
                
                // Outer bezel
                Rectangle {
                    width: parent.width
                    height: parent.height
                    radius: width / 2
                    color: "transparent"
                    border.color: "#BDBDBD"
                    border.width: 3
                    anchors.centerIn: parent
                }
            }
            
            // Connection status indicator
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                width: 12
                height: 12
                radius: 6
                color: appController.isConnected ? "#4CAF50" : "#F44336"
                border.color: "white"
                border.width: 1
            }
            
        }
        
        // Heading Indicator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 75
            Layout.topMargin: -8
            color: "#2E2E2E"
            radius: 8
            border.color: "#CFD8DC"
            border.width: 1
            
            Item {
                id: headingIndicator
                anchors.fill: parent
                anchors.margins: 10
                
                // Use device rotation about Y-axis (vertical screen axis) for heading  
                property real deviceRotationY: Math.asin(Math.max(-1.0, Math.min(1.0, 2.0 * (root.rotationW * root.rotationY - root.rotationZ * root.rotationX)))) * 180.0 / Math.PI
                property real initialRotationY: 0.0  // Initial rotation offset for reset
                // Convert to 0-360 compass heading (0 = North) with reset offset
                property real heading: ((deviceRotationY - initialRotationY) + 360) % 360
                
                // Clipping rectangle for the moving scale
                Rectangle {
                    id: headingClip
                    width: parent.width - 40  // Leave space for triangle
                    height: parent.height
                    anchors.centerIn: parent
                    color: "transparent"
                    clip: true
                    
                    
                    // Moving scale background
                    Canvas {
                        id: headingCanvas
                        width: parent.width * 12  // Extra wide canvas for scrolling
                        height: parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        
                        property real pixelsPerDegree: 4
                        x: -headingIndicator.heading * pixelsPerDegree + parent.width / 2 - width / 2  // Move scale based on heading, centered
                        
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            
                            var centerY = height / 2
                            var centerX = width / 2
                            
                            // Draw compass heading scale (0-360 degrees)
                            ctx.strokeStyle = "white"
                            ctx.fillStyle = "white"
                            ctx.font = "12px Arial"
                            ctx.textAlign = "center"
                            
                            // Draw scale across the canvas width
                            for (var heading = -360; heading <= 720; heading += 10) {
                                var normalizedHeading = ((heading % 360) + 360) % 360
                                var x = centerX + heading * headingCanvas.pixelsPerDegree
                                    
                                // Major ticks every 30 degrees
                                if (normalizedHeading % 30 === 0) {
                                    ctx.lineWidth = 2
                                    ctx.beginPath()
                                    ctx.moveTo(x, centerY - 15)
                                    ctx.lineTo(x, centerY + 15)
                                    ctx.stroke()
                                    
                                    // Add compass headings with cardinal directions
                                    var label = ""
                                    if (normalizedHeading === 0) label = "N"
                                    else if (normalizedHeading === 90) label = "E" 
                                    else if (normalizedHeading === 180) label = "S"
                                    else if (normalizedHeading === 270) label = "W"
                                    else label = normalizedHeading.toString()
                                    
                                    ctx.fillText(label, x, centerY + 25)  // Move labels below the line
                                }
                                // Minor ticks every 10 degrees
                                else {
                                    ctx.lineWidth = 1
                                    ctx.beginPath()
                                    ctx.moveTo(x, centerY - 8)
                                    ctx.lineTo(x, centerY + 8)
                                    ctx.stroke()
                                    
                                    // Add numbers for intermediate headings
                                    ctx.font = "10px Arial"
                                    ctx.fillText(normalizedHeading.toString(), x, centerY + 20)
                                    ctx.font = "12px Arial"
                                }
                            }
                        }
                        
                        // Repaint when heading changes
                        Connections {
                            target: headingIndicator
                            function onHeadingChanged() { 
                                headingCanvas.requestPaint()
                            }
                            function onDeviceRotationYChanged() {
                                headingCanvas.requestPaint()
                            }
                        }
                    }
                }
                
                // Fixed triangle pointer at center
                Canvas {
                    width: 16
                    height: 12
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.fillStyle = "white"
                        ctx.beginPath()
                        ctx.moveTo(width / 2, height)
                        ctx.lineTo(0, 0)
                        ctx.lineTo(width, 0)
                        ctx.closePath()
                        ctx.fill()
                    }
                }
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
        
        // Control buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: "Start"
                enabled: appController.isConnected
                Layout.fillWidth: true
                onClicked: appController.startSendingRotation()
            }
            
            Button {
                text: "Stop"
                Layout.fillWidth: true
                onClicked: appController.stopSendingRotation()
            }
            
            Button {
                text: "Reset"
                Layout.fillWidth: true
                onClicked: {
                    appController.resetOrientation()
                    // Reset Z-axis offset
                    root.zOffset = 0.0
                    appController.zAxisOffset = 0.0
                    // Reset slider handle to center
                    sliderHandle.x = sliderTrack.width / 2 - sliderHandle.width / 2
                    // Reset heading to North (0°)
                    headingIndicator.initialRotationY = headingIndicator.deviceRotationY
                }
                
                // Visual feedback
                background: Rectangle {
                    color: parent.pressed ? "#FFC107" : "#FF9800"
                    radius: 4
                    border.width: 1
                    border.color: "#FF8F00"
                }
            }
        }
        
    }
}