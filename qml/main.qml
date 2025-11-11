import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: window
    width: Screen.width
    height: Screen.height
    visible: true
    title: "OpenIGTLink Mobile"
    
    // Set application icon  
    property url iconSource: "qrc:/resources/icon.svg"
    
    Component.onCompleted: {
        // Debug: Check if icon resource is available
        console.log("Icon source:", iconSource)
        
        // Set window icon for desktop platforms
        if (Qt.platform.os !== "ios" && Qt.platform.os !== "android") {
            window.icon = iconSource
            console.log("Desktop platform - icon set to:", iconSource)
        }
        
        // Disable rotation - keep portrait orientation
        if (Qt.platform.os === "ios" || Qt.platform.os === "android") {
            // Mobile-specific orientation locking would be handled at the native level
            console.log("Mobile platform detected - orientation should be locked in native code")
        }
    }
    
    // Remove window decorations for mobile
    flags: Qt.Window | Qt.MaximizeUsingFullscreenGeometryHint

    MainWindow {
        anchors.fill: parent
    }
}