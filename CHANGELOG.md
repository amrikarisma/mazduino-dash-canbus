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

### 💡 Highlights
This version represents a major leap forward in functionality and user experience. The modular architecture makes the code more maintainable, while the web-based configuration system gives users unprecedented control over their display layout.

### 🙏 Acknowledgments
Special thanks to the community for feedback and testing that made this release possible.

### 📞 Support
- **Website**: [www.mazduino.com](https://www.mazduino.com)
- **Purchase**: [Buy on Tokopedia](https://tk.tokopedia.com/ZSkFr6LX6/)
- **Documentation**: See README.md for setup instructions
