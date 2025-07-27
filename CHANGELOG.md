# CHANGELOG

## [1.2.0] - 2025-07-12

### 🎯 Major Features
- **Modular Display System**: Complete rewrite of display system with user-configurable layouts
- **Web-Based Configuration**: Real-time configuration of display panels and indicators via web interface
- **Dynamic Panel Layout**: Support for 8 configurable data panels with custom positioning
- **Smart Indicator System**: 8 configurable status indicators with position control

### 🔧 Technical Improvements
- **Modular Architecture**: Refactored from single `.ino` file to modular `.h/.cpp` structure
- **Memory Management**: Improved EEPROM usage for configuration persistence
- **Performance Optimization**: Reduced display flicker with smart redraw algorithms
- **Type Safety**: Enhanced type checking and error handling throughout codebase

### 🖥️ Display Enhancements
- **Layout Collision Fix**: Resolved TPS/ADV panel collision with RPM bar
- **Optimized Positioning**: Improved panel spacing and positioning logic
- **Better Visual Hierarchy**: Clear separation between data panels and indicators
- **Responsive Design**: Adaptive layout that works within display constraints

### 🌐 Web Interface
- **Live Configuration**: Real-time display configuration through web browser
- **Panel Management**: Enable/disable individual data panels
- **Indicator Control**: Configure which indicators are shown
- **Configuration Persistence**: Save/load/reset display configurations
- **User-Friendly Interface**: Intuitive web UI for configuration management

### 📊 Data Management
- **Enhanced Data Types**: Support for multiple data types (int, float, uint)
- **Color Coding**: Smart color coding based on data values and thresholds
- **Decimal Precision**: Configurable decimal places for float values
- **Data Validation**: Improved data validation and error handling

### 🔄 Code Quality
- **Function Cleanup**: Removed unused functions (`itemDraw`) and dead code
- **Better Documentation**: Comprehensive inline documentation and comments
- **Consistent Naming**: Standardized function and variable naming conventions
- **Error Handling**: Improved error detection and recovery mechanisms

### 🛠️ Developer Experience
- **Modular Headers**: Clean separation of concerns across modules
- **Type Definitions**: Centralized type definitions in `DataTypes.h`
- **Configuration Management**: Centralized configuration in `Config.h`
- **Build System**: Optimized PlatformIO build configuration

### 🐛 Bug Fixes
- Fixed display panel overlap with RPM bar
- Resolved font rendering issues and overlap problems
- Fixed memory leaks in sprite operations
- Corrected EEPROM address conflicts
- Fixed communication mode indicator flickering

### 🎨 User Interface
- **Configurable Layouts**: Users can now customize which data is displayed where
- **Visual Feedback**: Clear indication of active/inactive states
- **Responsive Controls**: Web interface works on mobile and desktop
- **Real-time Updates**: Configuration changes reflected immediately

### 🔧 System Architecture
- **DisplayConfig System**: New configuration management system
- **DisplayManager**: Centralized display rendering and management
- **WebServerHandler**: Enhanced web server with configuration endpoints
- **CANHandler**: Improved CAN communication handling
- **GlobalVariables**: Centralized variable management

### 📱 Compatibility
- **ESP32 Optimized**: Optimized for ESP32 DevKit C v4
- **ILI9488 Display**: Full support for 480x320 TFT displays
- **CAN Protocol**: Compatible with Haltech CAN ECU broadcast protocol
- **Web Standards**: Standards-compliant web interface

### ⚡ Performance
- **Reduced CPU Usage**: Optimized rendering algorithms
- **Memory Efficiency**: Better memory management and cleanup
- **Faster Updates**: Improved update frequency and responsiveness
- **Battery Friendly**: Optimized power consumption

### 🔐 Stability
- **Error Recovery**: Better error handling and recovery mechanisms
- **Memory Safety**: Improved memory management and bounds checking
- **Configuration Validation**: Robust configuration validation
- **Failsafe Modes**: Fallback to default configurations when needed

### 📝 Documentation
- **Comprehensive README**: Updated project documentation
- **API Documentation**: Function and module documentation
- **Configuration Guide**: User guide for web-based configuration
- **Developer Notes**: Technical documentation for contributors

### 🚀 Migration Notes
- **Backward Compatibility**: Existing configurations will be migrated automatically
- **New Features**: All new features are optional and don't break existing functionality
- **Configuration Reset**: Use web interface to reset to default layout if needed
- **EEPROM Usage**: New configuration uses EEPROM address 10+ (existing settings preserved)

### 🎯 Future Roadmap
- Drag-and-drop configuration interface
- Custom data source integration
- Advanced theming and color schemes
- Data logging and export capabilities
- Multi-language support

---

## [1.2.1] - 2025-07-12

### 🚀 Performance Optimizations
- **Improved FPS**: Optimized display refresh rates from 15-25fps to 30-40fps target
- **Reduced Serial Communication**: Increased serial update interval from 10ms to 20ms
- **Smart Display Updates**: Added time-based update throttling for display components
- **Efficient Memory Usage**: Optimized sprite operations and memory allocations

### 🔧 WiFi & WebServer Improvements
- **Fixed WiFi Timeout**: Corrected WiFi shutdown logic after 1 minute of inactivity
- **Power Management**: Proper WiFi and Bluetooth shutdown for power saving
- **Manual WiFi Restart**: Added 'w' command to restart WiFi/WebServer via serial
- **Reduced Debug Overhead**: Optimized debug print frequencies to reduce serial load

### 🖥️ Display Performance
- **Throttled Updates**: 
  - RPM display updates: every 100ms max
  - Panel updates: every 50ms max  
  - Indicator updates: every 100ms max
  - RPM bar updates: every 75ms max
- **Selective Redraws**: Only update display elements when values actually change
- **Optimized Debug Display**: Reduced debug info update frequency to 1000ms

### 🎨 Splash Screen Improvements
- **Eliminated Slide Animations**: Replaced risky slide animations with safe fade effects
- **Fade-In Title**: Smooth fade-in animation for main title
- **Fade-In Website**: Cyan fade-in effect for website text
- **No Ghosting**: Completely eliminated text ghosting issues

### 🛠️ System Optimizations
- **Reduced CPU Load**: Optimized main loop with yield() calls and reduced polling
- **Better Multitasking**: Improved ESP32 task scheduling with strategic delays
- **Enhanced Debug Info**: Added WiFi status and connection info to debug output
- **Memory Efficiency**: Maintained stable memory usage at 15.9% RAM, 87.5% Flash

### 📋 New Serial Commands
- **'w' Command**: Restart WiFi and WebServer manually
- **Enhanced Help**: Updated help system with all available commands
- **Improved Debug**: Better debug information display

---

### 💡 Highlights
This version represents a major leap forward in functionality and user experience. The modular architecture makes the code more maintainable, while the web-based configuration system gives users unprecedented control over their display layout.

### 🙏 Acknowledgments
Special thanks to the community for feedback and testing that made this release possible.

### 📞 Support
- **Website**: [www.mazduino.com](https://www.mazduino.com)
- **Purchase**: [Buy on Tokopedia](https://tk.tokopedia.com/ZSkFr6LX6/)
- **Documentation**: See README.md for setup instructions
