# Deployment Guide for OpenIGTLink Mobile

This guide covers how to deploy your Qt-based OpenIGTLink mobile application to iOS and Android devices and distribute through app stores.

## Prerequisites

### General Requirements
- Qt 6.5+ with mobile components installed
- CMake 3.20+
- OpenIGTLink library compiled for mobile platforms

### iOS Development
- macOS development machine
- Xcode (latest version from App Store)
- Apple Developer Account ($99/year for App Store distribution)
- iOS device for testing

### Android Development  
- Android Studio or Android SDK/NDK
- Java Development Kit (JDK 8 or later)
- Android device or emulator for testing
- Google Play Console account ($25 one-time fee)

## Platform-Specific Setup

### iOS Deployment

1. **Configure Qt Creator for iOS**
   ```bash
   # Ensure iOS kit is available in Qt Creator
   # Go to Tools > Options > Kits
   # Verify iOS kit is configured with proper device/simulator
   ```

2. **Build OpenIGTLink for iOS**
   ```bash
   git clone https://github.com/openigtlink/OpenIGTLink.git third_party-ios/openigtlink
   cd third_party-ios/openigtlink
   mkdir build && cd build
   qt-cmake .. -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES="arm64"    
   ```
Then, use the following steps to build the code using Xcode.
  - Open the generated `.xcodeproj` in Xcode
  - Choose ALL_BUILD and Any iOS Device (arm64)
  - Build
   
3. **Build for iOS**
   ```bash
   mkdir build-ios && cd build-ios
   qt-cmake .. -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES="arm64"
   ```

4. **Xcode Configuration**
   - Open the generated `.xcodeproj` (under thrid_party-ios/openigtlink/build) in Xcode
   - Configure signing & capabilities
   - Set bundle identifier (e.g., `com.yourcompany.openigtlinkmobile`)
   - Configure deployment target (iOS 12.0+)

5. **Icon Setup**
To setup an icon for the app launcher, the asset folder should appear under `(Project Name)` > `Resources` in the Project Navigator. If not, add the asset folder as follows:
   - Right-click `Resources` in the Project Navigator.
   - Choose `Add Files to "OpenIGTLinkMobile"...`
   - Choose the `ios/Assets.xcassets` folder, and in `Add to targes:`, check `OpenIGTLinkMobile`
   - `Assets.xcassets` should appear under `Resources` in the Project Navigator.


### Android Deployment

1. **Configure Android SDK/NDK paths in Qt Creator**
   - Tools > Options > Devices > Android
   - Set Android SDK location
   - Set Android NDK location 
   - Set JDK location

2. **Build for Android**
   ```bash
   mkdir build-android && cd build-android
   cmake .. -DQt6_DIR=/path/to/qt6/android -DANDROID_ABI=arm64-v8a
   cmake --build . --config Release
   ```

3. **Generate APK/AAB**
   ```bash
   # For development APK
   androiddeployqt --input android-deployment-settings.json --output android-build --apk
   
   # For Play Store AAB (recommended)
   androiddeployqt --input android-deployment-settings.json --output android-build --aab
   ```

## App Store Distribution

### iOS App Store

#### Distribution Methods (2025):
1. **Public App Store** - Global distribution (unlimited users)
2. **TestFlight** - Beta testing (up to 10,000 external testers)  
3. **Ad Hoc** - Direct installation (up to 100 devices)
4. **Custom Apps** - Private business distribution
5. **Unlisted Apps** - Hidden App Store with private links
6. **Enterprise Program** - Internal distribution (restricted)

#### Steps:
1. **Prepare App Store Connect**
   - Create app record in App Store Connect
   - Configure app information, screenshots, description
   - Set pricing and availability

2. **Archive and Upload**
   ```bash
   # In Xcode:
   # Product > Archive
   # Use Organizer to validate and upload to App Store Connect
   ```

3. **App Review Process**
   - Submit for review (typically 24-48 hours)
   - Address any feedback from Apple
   - Release when approved

### Google Play Store

#### Distribution Options:
- **Production** - Public release
- **Open Testing** - Public beta (no approval needed)
- **Closed Testing** - Private beta groups
- **Internal Testing** - Team testing

#### Steps:
1. **Create Google Play Console Account**
   - Pay $25 one-time registration fee
   - Verify developer identity

2. **Prepare Store Listing**
   - App name, description, screenshots
   - Content rating questionnaire
   - Privacy policy (required)

3. **Upload App Bundle (AAB)**
   ```bash
   # Build AAB (recommended over APK)
   androiddeployqt --input android-deployment-settings.json --output android-build --aab --release
   ```

4. **Release Management**
   - Upload AAB to desired track (Internal/Closed/Open/Production)
   - Review and publish

## Platform-Specific Configurations

### iOS Configuration Files

Create `ios/Info.plist` additions:
```xml
<key>NSMotionUsageDescription</key>
<string>This app needs access to motion sensors to track device orientation.</string>
<key>NSLocationWhenInUseUsageDescription</key>
<string>This app may need location services for sensor calibration.</string>
```

### Android Configuration Files

Create `android/AndroidManifest.xml`:
```xml
<uses-permission android:name="android.permission.INTERNET" />
<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
<uses-permission android:name="android.permission.ACCESS_WIFI_STATE" />

<application
    android:name="org.qtproject.qt.android.bindings.QtApplication"
    android:label="@string/app_name"
    android:theme="@style/AppTheme">
    
    <activity android:name="org.qtproject.qt.android.bindings.QtActivity"
              android:exported="true"
              android:screenOrientation="unspecified"
              android:configChanges="orientation|uiMode|screenLayout|screenSize|smallestScreenSize|layoutDirection|locale|fontScale|keyboard|keyboardHidden|navigation|mcc|mnc|density"
              android:launchMode="singleTop">
        <intent-filter>
            <action android:name="android.intent.action.MAIN"/>
            <category android:name="android.intent.category.LAUNCHER"/>
        </intent-filter>
    </activity>
</application>
```

## Testing and Quality Assurance

### Pre-submission Checklist
- [ ] Test on multiple device types and screen sizes
- [ ] Verify OpenIGTLink connectivity on cellular and WiFi
- [ ] Test orientation accuracy across device orientations  
- [ ] Verify UI responds properly to device rotation
- [ ] Check battery usage during extended operation
- [ ] Test network interruption handling
- [ ] Validate privacy policy compliance
- [ ] Ensure proper app icon and metadata

### Beta Testing
- **iOS**: Use TestFlight for external beta testing
- **Android**: Use Google Play Internal Testing or Closed Testing tracks

## Build Automation (Optional)

Consider using CI/CD solutions for automated builds:
- **GitHub Actions** with Qt installation
- **Azure DevOps** with mobile app templates
- **GitLab CI** with custom Qt containers

Example GitHub Actions workflow snippet:
```yaml
- name: Setup Qt
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.5.0'
    target: 'android'
    
- name: Build Android AAB
  run: |
    cmake --build . --config Release
    androiddeployqt --aab
```

## Common Issues and Solutions

### iOS
- **Provisioning Profile Issues**: Ensure bundle ID matches exactly
- **Sensor Permissions**: Add required usage descriptions in Info.plist
- **Architecture Mismatch**: Use `arm64` for device builds

### Android  
- **APK Size**: Use AAB for Play Store to optimize downloads
- **NDK Version**: Ensure NDK compatibility with Qt version
- **Permissions**: Declare all required permissions in manifest

## Resources

- [Qt for iOS Documentation](https://doc.qt.io/qt-6/ios.html)
- [Qt Android Deployment](https://doc.qt.io/qt-6/deployment-android.html)  
- [Apple App Store Guidelines](https://developer.apple.com/app-store/review/guidelines/)
- [Google Play Policy Center](https://play.google.com/about/developer-content-policy/)
- [Qt Mobile Best Practices](https://doc.qt.io/qt-6/mobiledevelopment.html)
