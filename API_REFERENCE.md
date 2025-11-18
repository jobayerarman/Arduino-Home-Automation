# Smart Home API Reference

Complete REST API documentation for the Arduino Smart Home system.

---

## 📋 Overview

The Smart Home API provides RESTful endpoints for controlling and monitoring your home automation system. All endpoints return JSON responses unless otherwise specified.

**Base URL:** `http://192.168.0.120` (or your configured IP)

**Protocol:** HTTP/1.1

**Response Format:** JSON

---

## 🔌 Endpoints

### 1. GET /api/status

Get complete system status including all relays, temperature, and system information.

**Request:**
```http
GET /api/status HTTP/1.1
Host: 192.168.0.120
```

**Response:**
```json
{
  "temperature": 25,
  "relays": [
    {
      "id": 0,
      "name": "Living Room",
      "state": true,
      "timer": 1800
    },
    {
      "id": 1,
      "name": "Master Bed",
      "state": false
    }
  ],
  "system": {
    "uptime": 3600,
    "free_ram": 1024,
    "automation_enabled": true
  }
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `temperature` | integer | Current temperature in Celsius |
| `relays` | array | Array of relay objects |
| `relays[].id` | integer | Relay ID (0-4) |
| `relays[].name` | string | Relay name |
| `relays[].state` | boolean | Current state (true=ON, false=OFF) |
| `relays[].timer` | integer | Remaining timer seconds (optional) |
| `system.uptime` | integer | System uptime in seconds |
| `system.free_ram` | integer | Available SRAM in bytes |
| `system.automation_enabled` | boolean | Automation system status |

---

### 2. GET /api/relay

Control individual relay state.

**Request:**
```http
GET /api/relay?id=0&state=1 HTTP/1.1
Host: 192.168.0.120
```

**Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `id` | integer | Yes | Relay ID (0-4) |
| `state` | integer | Yes | Desired state (0=OFF, 1=ON) |

**Response (Success):**
```json
{
  "status": "ok",
  "relay": 0,
  "state": true
}
```

**Response (Error):**
```json
{
  "status": "error",
  "message": "Invalid relay ID"
}
```

**Example Usage:**

```bash
# Turn ON Living Room (relay 0)
curl "http://192.168.0.120/api/relay?id=0&state=1"

# Turn OFF Master Bed (relay 1)
curl "http://192.168.0.120/api/relay?id=1&state=0"
```

---

### 3. GET /api/scene

Activate predefined scenes or list available scenes.

**Request (Activate Scene):**
```http
GET /api/scene?name=good_night HTTP/1.1
Host: 192.168.0.120
```

**Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `name` | string | No | Scene name to activate |

**Response (Activate):**
```json
{
  "status": "ok",
  "scene": "good_night"
}
```

**Request (List Scenes):**
```http
GET /api/scene HTTP/1.1
Host: 192.168.0.120
```

**Response (List):**
```json
{
  "status": "ok",
  "scenes": [
    {"id": 0, "name": "good_morning"},
    {"id": 1, "name": "good_night"},
    {"id": 2, "name": "away"},
    {"id": 3, "name": "party"},
    {"id": 4, "name": "movie_night"},
    {"id": 5, "name": "all_on"},
    {"id": 6, "name": "all_off"}
  ]
}
```

**Available Scenes:**

| Scene Name | Description |
|------------|-------------|
| `good_morning` | Living Room + Kitchen ON, bedrooms OFF |
| `good_night` | All lights OFF except Wash Room (night light) |
| `away` | All devices OFF (security mode) |
| `party` | All lights ON |
| `movie_night` | Only Living Room ON |
| `all_on` | Turn all devices ON |
| `all_off` | Turn all devices OFF |

**Example Usage:**

```bash
# Activate Good Night scene
curl "http://192.168.0.120/api/scene?name=good_night"

# List all scenes
curl "http://192.168.0.120/api/scene"
```

---

### 4. GET /api/timer

Set countdown timers for relays.

**Request:**
```http
GET /api/timer?relay=2&duration=3600 HTTP/1.1
Host: 192.168.0.120
```

**Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `relay` | integer | Yes | Relay ID (0-4) |
| `duration` | integer | Yes | Duration in seconds |

**Response:**
```json
{
  "status": "ok",
  "relay": 2,
  "duration": 3600
}
```

**Behavior:**
- Timer starts immediately
- When timer expires, relay turns OFF
- Only one timer per relay (setting new timer cancels old one)
- Timer survives relay manual state changes
- Maximum 5 active timers system-wide

**Example Usage:**

```bash
# Turn OFF Guest Room in 30 minutes (1800 seconds)
curl "http://192.168.0.120/api/timer?relay=2&duration=1800"

# Turn OFF Kitchen in 1 hour (3600 seconds)
curl "http://192.168.0.120/api/timer?relay=3&duration=3600"
```

**Common Durations:**

| Duration | Seconds |
|----------|---------|
| 5 minutes | 300 |
| 15 minutes | 900 |
| 30 minutes | 1800 |
| 1 hour | 3600 |
| 2 hours | 7200 |
| 4 hours | 14400 |

---

### 5. GET /api/stats

Get usage statistics for all devices.

**Request:**
```http
GET /api/stats HTTP/1.1
Host: 192.168.0.120
```

**Response:**
```json
{
  "stats": [
    {
      "name": "Living Room",
      "on_time": 7200,
      "trigger_count": 15
    },
    {
      "name": "Master Bed",
      "on_time": 3600,
      "trigger_count": 8
    }
  ],
  "temperature": {
    "current": 25,
    "min": 20,
    "max": 30
  }
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `stats[].name` | string | Relay name |
| `stats[].on_time` | integer | Total ON time in seconds since boot |
| `stats[].trigger_count` | integer | Number of state changes |
| `temperature.current` | integer | Current temperature (°C) |
| `temperature.min` | integer | Minimum temperature since boot |
| `temperature.max` | integer | Maximum temperature since boot |

**Example Usage:**

```bash
# Get statistics
curl "http://192.168.0.120/api/stats"
```

---

## 🔧 Advanced Usage

### JavaScript Fetch API

```javascript
// Get status
fetch('http://192.168.0.120/api/status')
  .then(response => response.json())
  .then(data => {
    console.log('Temperature:', data.temperature);
    console.log('Relays:', data.relays);
  });

// Toggle relay
function toggleRelay(id, state) {
  fetch(`http://192.168.0.120/api/relay?id=${id}&state=${state ? 1 : 0}`)
    .then(response => response.json())
    .then(data => {
      if (data.status === 'ok') {
        console.log('Relay toggled successfully');
      }
    });
}

// Activate scene
function activateScene(name) {
  fetch(`http://192.168.0.120/api/scene?name=${name}`)
    .then(response => response.json())
    .then(data => console.log('Scene activated:', data.scene));
}
```

### Python Requests

```python
import requests

BASE_URL = "http://192.168.0.120"

# Get status
response = requests.get(f"{BASE_URL}/api/status")
data = response.json()
print(f"Temperature: {data['temperature']}°C")

# Turn ON relay
requests.get(f"{BASE_URL}/api/relay", params={"id": 0, "state": 1})

# Activate scene
requests.get(f"{BASE_URL}/api/scene", params={"name": "good_night"})

# Set timer (turn OFF in 30 minutes)
requests.get(f"{BASE_URL}/api/timer", params={"relay": 2, "duration": 1800})
```

### cURL Examples

```bash
# Complete status
curl "http://192.168.0.120/api/status" | jq

# Turn all lights ON
for i in 0 1 2 3 4; do
  curl "http://192.168.0.120/api/relay?id=$i&state=1"
done

# Turn all lights OFF
for i in 0 1 2 3 4; do
  curl "http://192.168.0.120/api/relay?id=$i&state=0"
done

# Set timer for all devices (turn OFF in 1 hour)
for i in 0 1 2 3 4; do
  curl "http://192.168.0.120/api/timer?relay=$i&duration=3600"
done
```

---

## 🤖 Home Assistant Integration

Add to `configuration.yaml`:

```yaml
# REST Switches
switch:
  - platform: rest
    name: "Living Room"
    resource: "http://192.168.0.120/api/relay?id=0&state=1"
    body_on: ""
    body_off: ""
    is_on_template: '{{ value_json.state }}'

# Temperature Sensor
sensor:
  - platform: rest
    name: "Home Temperature"
    resource: "http://192.168.0.120/api/status"
    value_template: '{{ value_json.temperature }}'
    unit_of_measurement: "°C"

# Scenes
script:
  good_night:
    sequence:
      - service: rest_command.activate_scene
        data:
          scene_name: "good_night"

rest_command:
  activate_scene:
    url: "http://192.168.0.120/api/scene?name={{ scene_name }}"
```

---

## 📱 Mobile App Integration (iOS Shortcuts)

```
GET http://192.168.0.120/api/scene?name=good_morning

Shortcut: "Good Morning"
Trigger: Time of Day (7:00 AM)
Action: Get Contents of URL
```

---

## ⚡ Rate Limiting & Performance

### Recommended Polling Intervals

| Endpoint | Recommended Interval | Notes |
|----------|---------------------|-------|
| `/api/status` | 2-5 seconds | Real-time updates |
| `/api/stats` | 30-60 seconds | Slow-changing data |
| `/api/scene` | On-demand | User action only |
| `/api/relay` | On-demand | User action only |
| `/api/timer` | On-demand | User action only |

### Concurrent Requests

- **Maximum:** 1 simultaneous HTTP connection
- **Queue:** Requests are processed sequentially
- **Timeout:** 5 seconds per request
- **Best Practice:** Wait for response before sending next request

---

## 🔒 Security Considerations

⚠️ **Current Implementation:** No authentication

**Planned Features:**
- Basic HTTP authentication
- API key authentication
- HTTPS support (with ESP8266/ESP32)

**Current Recommendations:**
1. Use on trusted local network only
2. Do not expose to internet without firewall
3. Consider VPN for remote access
4. Implement router-level access controls

---

## 🐛 Error Handling

### HTTP Status Codes

| Code | Meaning |
|------|---------|
| 200 | Success |
| 400 | Bad Request (invalid parameters) |
| 404 | Endpoint not found |
| 500 | Internal server error |

### Error Response Format

```json
{
  "status": "error",
  "message": "Descriptive error message"
}
```

### Common Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `Invalid relay ID` | Relay ID < 0 or > 4 | Use valid relay ID (0-4) |
| `Missing parameters` | Required parameter not provided | Check API documentation |
| `Scene not found` | Scene name doesn't exist | Check available scenes |
| `Failed to add timer` | Too many active timers | Cancel existing timers |

---

## 📊 Response Time Benchmarks

Typical response times on Arduino Uno with W5100 Ethernet shield:

| Endpoint | Avg Response Time |
|----------|------------------|
| `/api/status` | 50-100 ms |
| `/api/relay` | 30-50 ms |
| `/api/scene` | 100-150 ms |
| `/api/timer` | 40-60 ms |
| `/api/stats` | 60-100 ms |
| `/` (HTML page) | 500-1000 ms |

---

## 🔄 WebSocket Support (Future)

**Status:** Not yet implemented

**Planned:**
```javascript
const ws = new WebSocket('ws://192.168.0.120:81');

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log('Real-time update:', data);
};
```

---

## 📚 Additional Resources

- **Installation Guide:** `INSTALLATION_GUIDE.md`
- **User Guide:** `USER_GUIDE.md`
- **GitHub Issues:** https://github.com/jobayerarman/Arduino-Home-Automation/issues
- **Examples:** See `/examples` directory

---

## 📝 API Changelog

### Version 3.0 (Current)
- ✅ Added `/api/status` - Complete system status
- ✅ Added `/api/scene` - Scene activation
- ✅ Added `/api/timer` - Countdown timers
- ✅ Added `/api/stats` - Usage statistics
- ✅ Improved JSON responses
- ✅ Added system information

### Version 2.0 (Legacy)
- `/button_state` - XML-based relay control (deprecated)

---

**Questions or Issues?**

- Create issue: https://github.com/jobayerarman/Arduino-Home-Automation/issues
- Email: carbonjha@gmail.com
- Twitter: @JobayerArman
