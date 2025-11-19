# Arduino Home Automation

A web-based home automation system built with Arduino that allows you to control multiple electrical appliances remotely through a responsive web interface. The system includes real-time temperature monitoring and control of up to 5 relay channels for managing various home devices.

![Arduino Home Automation v4.0](https://github.com/jobayerarman/Arduino-Home-Automation/blob/master/screenshot/HomeAutomation-4.0.jpg)

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [System Architecture](#system-architecture)
- [Installation](#installation)
- [Configuration](#configuration)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [API Endpoints](#api-endpoints)
- [Troubleshooting](#troubleshooting)
- [Version History](#version-history)
- [Contributing](#contributing)
- [Credits](#credits)
- [License](#license)

## Features

- **Web-based Control**: Access and control your home appliances from any web browser
- **Real-time Temperature Monitoring**: Displays current temperature in Celsius
- **5-Channel Relay Control**: Independently control up to 5 different appliances
- **Responsive Design**: Mobile-friendly interface that works on smartphones, tablets, and desktops
- **AJAX Communication**: Real-time updates without page refresh
- **SD Card Storage**: Web interface stored on microSD card for easy customization
- **Low Power**: Efficient Arduino-based implementation
- **Room-specific Control**: Pre-configured for different rooms (Living Room, Master Bed, Guest Room, Kitchen, Wash Room)

## Hardware Requirements

### Essential Components

| Component | Specification | Notes |
|-----------|--------------|-------|
| **Arduino Board** | Arduino Uno | Other Arduino boards may work with minor modifications |
| **Ethernet Shield** | Official Arduino Ethernet Shield | Compatible shields should work |
| **MicroSD Card** | 2GB, FAT16 formatted | Stores the web interface (index.htm) |
| **Thermistor** | NTC Thermistor | For temperature sensing (connected to pin 2) |
| **Relay Module** | 5-channel relay board | For controlling AC appliances |
| **Power Supply** | 9V-12V DC | Adequate current rating for relays |

### Pin Configuration

```
Pin 2  : Thermistor (Temperature Sensor)
Pin 4  : SD Card CS (Chip Select)
Pin 5  : Relay 1 - Living Room
Pin 6  : Relay 2 - Master Bed
Pin 7  : Relay 5 - Wash Room
Pin 8  : Relay 4 - Kitchen
Pin 9  : Relay 3 - Guest Room
Pin 10 : Ethernet CS (Chip Select)
Pin 11 : SPI MOSI
Pin 12 : SPI MISO
Pin 13 : SPI SCK
```

## Software Requirements

- **Arduino IDE**: Version 1.0.5 or higher (tested up to 1.0.5)
- **Required Libraries**:
  - `Ethernet.h` - For network communication
  - `SD.h` - For SD card file operations
  - `SPI.h` - For SPI communication
  - `Thermistor.h` - For temperature sensor reading

## System Architecture

### Overview

The system follows a client-server architecture:

```
┌─────────────┐         ┌──────────────────┐         ┌─────────────┐
│   Client    │◄───────►│   Arduino Web    │◄───────►│   Relays    │
│ (Browser)   │  HTTP   │     Server       │  GPIO   │  (Devices)  │
└─────────────┘         └──────────────────┘         └─────────────┘
                               │
                               │ SPI
                               ▼
                        ┌──────────────┐
                        │   SD Card    │
                        │ (index.htm)  │
                        └──────────────┘
                               │
                        ┌──────────────┐
                        │ Thermistor   │
                        │   (Pin 2)    │
                        └──────────────┘
```

### Communication Flow

1. **Initial Request**: Browser requests the main page
2. **HTML Delivery**: Arduino serves `index.htm` from SD card
3. **AJAX Polling**: JavaScript polls server every 1 second
4. **State Update**: Server responds with XML containing relay states and temperature
5. **User Interaction**: Button clicks send state change requests
6. **Relay Control**: Arduino updates GPIO pins based on requests

## Installation

### Step 1: Hardware Setup

1. **Assemble the Stack**:
   - Mount the Ethernet Shield on top of the Arduino Uno
   - Ensure proper alignment of pins

2. **Connect Relays**:
   ```
   Arduino Pin 5 → Relay 1 IN (Living Room)
   Arduino Pin 6 → Relay 2 IN (Master Bed)
   Arduino Pin 7 → Relay 5 IN (Wash Room)
   Arduino Pin 8 → Relay 4 IN (Kitchen)
   Arduino Pin 9 → Relay 3 IN (Guest Room)
   ```

3. **Connect Temperature Sensor**:
   - Connect thermistor to Pin 2 with appropriate pull-up resistor

4. **Prepare SD Card**:
   - Format microSD card as FAT16
   - Copy `website_on_SD/index.htm` to the root of the SD card
   - Insert SD card into the Ethernet Shield

### Step 2: Software Setup

1. **Install Arduino IDE**:
   - Download from [arduino.cc](https://www.arduino.cc/en/software)

2. **Install Required Libraries**:
   - Ethernet library (usually pre-installed)
   - SD library (usually pre-installed)
   - Thermistor library (install via Library Manager)

3. **Upload the Sketch**:
   - Open `webserver_sketch/webserver_sketch.ino`
   - Configure network settings (see Configuration section)
   - Upload to Arduino board

### Step 3: Network Configuration

Edit these lines in the sketch before uploading:

```cpp
// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP address - change to match your network
IPAddress ip(192, 168, 0, 120);
```

## Configuration

### Network Settings

**Static IP Configuration** (default):
- IP Address: `192.168.0.120`
- Server Port: `80` (HTTP)
- MAC Address: Check sticker on your Ethernet shield

**Important**: Ensure the IP address is:
- Within your network's subnet
- Not already in use by another device
- Accessible from your client devices

### Customizing the Web Interface

1. Modify `website_on_SD/index.htm` on your computer
2. Copy the updated file to the SD card
3. Restart the Arduino to load changes

### Relay Assignments

To change room assignments, edit the `SetRELAYs()` function in the sketch:

```cpp
// Example: Change Living Room to Bedroom
if (StrContains(HTTP_req, "RELAY1=1")) {
    RELAY_state[0] = 1;
    digitalWrite(5, HIGH);  // Pin 5 controls this relay
}
```

## How It Works

### Initialization Process

1. **System Boot** (`setup()` function):
   ```cpp
   - Disable Ethernet chip
   - Initialize Serial (9600 baud) for debugging
   - Initialize SD card and verify index.htm exists
   - Configure relay pins (5-9) as OUTPUT
   - Initialize Ethernet with MAC and IP
   - Start HTTP server on port 80
   ```

2. **Ready State**: Arduino listens for incoming connections on port 80

### Request Handling Loop

The `loop()` function continuously:

1. **Client Detection**: Checks for incoming HTTP requests
2. **Request Parsing**: Buffers incoming data (max 60 bytes)
3. **Request Type Identification**:
   - **AJAX Request** (`button_state` in URL): Returns XML
   - **Page Request**: Serves HTML from SD card

### AJAX/XML Communication

**Request Format**:
```
GET /button_state&RELAY1=1&nocache=12345 HTTP/1.1
```

**Response Format**:
```xml
<?xml version = "1.0" ?>
<inputs>
  <temp>25</temp>
  <BUTTON>on</BUTTON>
  <BUTTON>off</BUTTON>
  <BUTTON>off</BUTTON>
  <BUTTON>on</BUTTON>
  <BUTTON>off</BUTTON>
</inputs>
```

### State Management

The system maintains relay states in a boolean array:
```cpp
boolean RELAY_state[BTN_NUM] = {0}; // 5 relays, all initially OFF
```

**State Update Flow**:
1. Browser sends button click: `RELAY1=1`
2. Arduino parses request in `SetRELAYs()`
3. Updates `RELAY_state[0] = 1`
4. Sets GPIO pin: `digitalWrite(5, HIGH)`
5. Returns updated state in XML response
6. JavaScript updates button display

### Temperature Monitoring

```cpp
Thermistor temp(2);  // Initialize on pin 2

// In XML_response():
byte celsius = temp.getTemp();  // Read temperature
cl.print("<temp>");
cl.print(celsius);
cl.print("</temp>");
```

The web interface polls every 1 second and updates the temperature display.

## Project Structure

```
Arduino-Home-Automation/
├── webserver_sketch/
│   └── webserver_sketch.ino    # Main Arduino sketch
├── website_on_SD/
│   └── index.htm                # Web interface (to be copied to SD card)
├── screenshot/
│   ├── HomeAutomation-2.0.png   # Version 2.0 screenshot
│   └── HomeAutomation-4.0.jpg   # Version 4.0 screenshot
├── CHANGELOG.md                 # Detailed version history
└── README.md                    # This file
```

## API Endpoints

### GET /

**Description**: Serves the main HTML interface

**Response**: `index.htm` from SD card

**Content-Type**: `text/html`

---

### GET /button_state

**Description**: AJAX endpoint for real-time updates and control

**Query Parameters**:
- `RELAYn=0|1` (optional): Set relay n to OFF (0) or ON (1)
- `nocache=<random>`: Cache-busting parameter

**Examples**:
```
GET /button_state&nocache=12345
GET /button_state&RELAY1=1&RELAY2=0&nocache=67890
```

**Response**: XML document with current state

**Content-Type**: `text/xml`

## Troubleshooting

### Common Issues

**Problem**: Arduino doesn't respond on network
- **Solution**:
  - Verify IP address is correct for your network
  - Check Ethernet cable connection
  - Ensure MAC address is unique on network
  - Try pinging the Arduino IP address

**Problem**: "ERROR - SD card initialization failed!"
- **Solution**:
  - Check SD card is properly inserted
  - Verify card is formatted as FAT16
  - Try a different SD card (max 2GB recommended)
  - Check pin 4 connection

**Problem**: "ERROR - Can't find index.htm file!"
- **Solution**:
  - Ensure `index.htm` is in root directory of SD card
  - Verify filename is exactly `index.htm` (case-sensitive)
  - Check file is not corrupted

**Problem**: Relays not switching
- **Solution**:
  - Verify relay module is powered
  - Check GPIO pin connections (5-9)
  - Test relay board independently
  - Check relay trigger level (active HIGH/LOW)

**Problem**: Temperature shows incorrect value
- **Solution**:
  - Check thermistor connection to pin 2
  - Verify Thermistor library is installed
  - Calibrate thermistor if needed

### Debugging

Enable Serial Monitor (9600 baud) to see diagnostic messages:
```cpp
Serial.begin(9600);  // Already in setup()
```

Monitor for:
- SD card initialization status
- File existence checks
- Any custom debug messages you add

## Version History

See [CHANGELOG.md](CHANGELOG.md) for detailed version history.

**Current Version**: 0.4.2 (March 28, 2016)

**Major Milestones**:
- **v0.1.0** (Feb 2016): Initial release with 5-relay control
- **v0.2.0** (Feb 2016): Added temperature sensor
- **v0.3.0** (Feb 2016): Improved code with DRY principles
- **v0.4.0** (Feb 2016): Simplified button functions
- **v0.4.2** (Mar 2016): Final refinements

## Screenshots

### Version 2.0
![Version 2.0](https://github.com/jobayerarman/Arduino-Home-Automation/blob/master/screenshot/HomeAutomation-2.0.png)

### Version 4.0 (Current)
![Version 4.0](https://github.com/jobayerarman/Arduino-Home-Automation/blob/master/screenshot/HomeAutomation-4.0.jpg)

## Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Ideas for Contribution

- [ ] Add DHCP support for automatic IP configuration
- [ ] Implement authentication for security
- [ ] Add scheduling/timer functionality
- [ ] Support for more sensors (humidity, motion, etc.)
- [ ] Mobile app integration
- [ ] MQTT support for IoT platforms
- [ ] Energy consumption monitoring

## Credits

### Original Author
**W.A. Smith**
Website: [Starting Electronics](http://startingelectronics.com)

### Enhanced Version
**Jobayer Arman**
GitHub: [@jobayerarman](https://github.com/jobayerarman)
Twitter: [@JobayerArman](https://twitter.com/JobayerArman)
Email: carbonjha@gmail.com

### References
- WebServer example by David A. Mellis and modified by Tom Igoe
- SD card examples by David A. Mellis and Tom Igoe
- [Ethernet Library Documentation](http://arduino.cc/en/Reference/Ethernet)
- [SD Card Library Documentation](http://arduino.cc/en/Reference/SD)

## License

This project is open source and available for educational and personal use.

## Need Help?

Feel free to:
- [Create an issue](https://github.com/jobayerarman/Arduino-Home-Automation/issues)
- [Tweet me](https://twitter.com/JobayerArman)
- [Send an email](mailto:carbonjha@gmail.com)

I'd be glad to help where I can!

---

**Note**: This project involves working with mains voltage (AC power) through relays. Always follow proper electrical safety procedures and consult with a licensed electrician when necessary. The authors are not responsible for any damage or injury resulting from the use of this project.
