# Changelog

All notable changes to the Arduino Home Automation project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.2] - 2016-03-28

### Changed
- Updated footer copyright information
- Minor bug fixes and improvements

## [0.4.1] - 2016-02-16

### Added
- USB debugging support in Arduino sketch for easier troubleshooting
- New screenshot of the updated website interface (HomeAutomation-4.0.jpg)

### Changed
- Updated README with better documentation

## [0.4.0] - 2016-02-16

### Changed
- Simplified button function implementation
- Removed individual per-button functions in favor of a unified approach
- Improved code maintainability and reduced redundancy

## [0.3.0] - 2016-02-14

### Changed
- Refactored XML response generation to use for-loop (DRY principle)
- Improved code efficiency by eliminating redundant XML response code
- Enhanced JavaScript structure for better maintainability

## [0.2.1] - 2016-02-10

### Added
- Revamped web UI with cleaner design
- Optimized Bootstrap CSS using UnCSS for faster loading

### Removed
- Clock feature from the web interface
- Unnecessary CSS bloat from Bootstrap

### Changed
- Refactored XML response function for better code organization
- Removed unused variables from index.htm

## [0.2.0] - 2016-02-04

### Added
- Temperature sensor integration (Thermistor library)
- Real-time temperature display in Celsius on web interface
- New screenshot showcasing the updated UI (HomeAutomation-2.0.png)

### Changed
- Completely redesigned website interface with modern look
- Improved user experience with better visual feedback

### Fixed
- Various minor bugs in the web server implementation

## [0.1.0] - 2016-02-04

### Added
- Initial release of Arduino Home Automation system
- Web server implementation on Arduino Uno with Ethernet shield
- SD card support for hosting web interface (index.htm)
- Control of 4 relay outputs via web interface
- Real-time button state feedback using AJAX/XML
- Support for 5 relay channels (pins 5-9):
  - Living Room (pin 5)
  - Master Bed (pin 6)
  - Guest Room (pin 9)
  - Kitchen (pin 8)
  - Wash Room (pin 7)
- Responsive web design with Bootstrap framework
- HTTP server running on port 80
- Static IP configuration (192.168.0.120)

### Technical Details
- Compatible with Arduino 1.0+
- Uses Ethernet library for network communication
- Uses SD library for file storage
- Buffer size optimization for HTTP requests (60 bytes)
- XML-based asynchronous communication

---

## Version History Summary

| Version | Date       | Key Features                                    |
|---------|------------|-------------------------------------------------|
| 0.4.2   | 2016-03-28 | Footer update, minor fixes                      |
| 0.4.1   | 2016-02-16 | USB debugging, documentation improvements       |
| 0.4.0   | 2016-02-16 | Simplified button functions                     |
| 0.3.0   | 2016-02-14 | DRY code implementation for XML responses       |
| 0.2.1   | 2016-02-10 | UI revamp, code refactoring                     |
| 0.2.0   | 2016-02-04 | Temperature sensor, redesigned interface        |
| 0.1.0   | 2016-02-04 | Initial release with 5-relay control            |

---

## Original Author
**W.A. Smith** - Initial work from [Starting Electronics](http://startingelectronics.com)

## Maintainer
**Jobayer Arman** - Enhanced version with temperature monitoring and improved UI
