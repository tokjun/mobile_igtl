import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: window
    width: Screen.width
    height: Screen.height
    visible: true
    title: "OpenIGTLink Mobile"
    
    // Disable rotation - keep portrait orientation
    Component.onCompleted: {
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