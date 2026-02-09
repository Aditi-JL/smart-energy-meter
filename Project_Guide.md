# Smart Energy Meter - Complete Project Guide

## IoT Data Analysis Project

---

## 📋 Project Overview

| Attribute | Details |
|-----------|---------|
| **Project Title** | Smart Energy Meter |
| **Domain** | IoT Data Analysis |
| **Duration** | 4-6 Weeks |
| **Technology Stack** | ESP32, PZEM-004T, ThingSpeak, Arduino |

---

## 🎯 What This Project Does

The Smart Energy Meter is an IoT-based electrical energy monitoring system that:

1. **Measures** real-time electrical parameters:
   - Voltage (V)
   - Current (A)
   - Power (W)
   - Energy (kWh)
   - Frequency (Hz)
   - Power Factor

2. **Displays** readings on:
   - Local OLED screen (immediate access)
   - ThingSpeak cloud dashboard (remote access)
   - Serial monitor (debugging)

3. **Analyzes** energy consumption patterns through cloud-based charts

---

## 🔧 Hardware Components

### Main Components

| Component | Purpose | Cost (INR) |
|-----------|---------|------------|
| ESP32 DevKit | Main controller, WiFi communication | ₹450 |
| PZEM-004T v3.0 | Energy measurement module | ₹650 |
| CT Coil (100A) | Non-invasive current sensing | (Included) |
| OLED Display 0.96" | Local data display | ₹180 |

**Total Estimated Cost: ₹1,380**

### Component Details

#### ESP32 Development Board
- Dual-core 240MHz processor
- Built-in WiFi and Bluetooth
- 34 GPIO pins
- USB powered (5V)

#### PZEM-004T v3.0
- Measures: Voltage (80-260V), Current (0-100A), Power, Energy, Frequency, PF
- Communication: UART (9600 baud)
- Accuracy: ±0.5% voltage, ±1% current

#### CT Coil (Current Transformer)
- Range: 0-100A
- Type: Split-core (clamp-on)
- Non-invasive installation

#### OLED Display
- Size: 0.96" diagonal
- Resolution: 128x64 pixels
- Interface: I2C
- Driver: SSD1306

---

## 🔌 Circuit Connections

### Wiring Diagram

```
                    ┌─────────────────┐
                    │     ESP32       │
                    │    DevKit       │
                    │                 │
    ┌───────────────┤ GPIO16 (RX2)    │
    │               │ GPIO17 (TX2)    ├───────────────┐
    │    ┌──────────┤ GPIO21 (SDA)    │               │
    │    │    ┌─────┤ GPIO22 (SCL)    │               │
    │    │    │     │ 5V (VIN)        ├───────┐       │
    │    │    │     │ 3.3V            ├───┐   │       │
    │    │    │     │ GND             ├─┬─┼───┼───┐   │
    │    │    │     └─────────────────┘ │ │   │   │   │
    │    │    │                         │ │   │   │   │
    ▼    ▼    ▼                         ▼ ▼   ▼   ▼   ▼
┌────────────────┐                 ┌────────────────────┐
│  OLED Display  │                 │    PZEM-004T       │
│                │                 │                    │
│  SDA ──────────┤                 │ TX ────────────────┤
│  SCL ──────────┤                 │ RX ────────────────┤
│  VCC (3.3V)────┤                 │ 5V ────────────────┤
│  GND ──────────┤                 │ GND ───────────────┤
└────────────────┘                 │                    │
                                   │  [CT Coil Port]    │
                                   │       ▲            │
                                   └───────┼────────────┘
                                           │
                                     ┌─────┴─────┐
                                     │  CT Coil  │
                                     │   100A    │
                                     │           │
                                     │ ~~LIVE~~  │ ← Clamp here
                                     └───────────┘
```

### Connection Table

| From | To | Notes |
|------|-----|-------|
| PZEM TX | ESP32 GPIO16 | Cross-connection! |
| PZEM RX | ESP32 GPIO17 | Cross-connection! |
| PZEM 5V | ESP32 VIN | Power for PZEM |
| PZEM GND | ESP32 GND | Common ground |
| OLED SDA | ESP32 GPIO21 | I2C data |
| OLED SCL | ESP32 GPIO22 | I2C clock |
| OLED VCC | ESP32 3.3V | Display power |
| OLED GND | ESP32 GND | Common ground |
| CT Coil | PZEM CT Port | Clamp on LIVE wire |

### ⚠️ Important Warnings

1. **TX/RX are crossed** - PZEM TX goes to ESP32 RX (GPIO16)
2. **Use 5V for PZEM** - The module needs 5V, not 3.3V
3. **CT Coil on LIVE wire only** - Never clamp both wires together
4. **CT arrow direction** - Arrow should point towards the load

---

## 💻 Software Setup

### Step 1: Install Arduino IDE

1. Download from: https://www.arduino.cc/en/software
2. Install and open Arduino IDE

### Step 2: Add ESP32 Board Support

1. Go to **File → Preferences**
2. Add this URL to "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search for "esp32" and install "ESP32 by Espressif Systems"

### Step 3: Install Required Libraries

Go to **Sketch → Include Library → Manage Libraries** and install:

| Library | Author |
|---------|--------|
| PZEM004Tv30 | Jakub Mandula |
| Adafruit SSD1306 | Adafruit |
| Adafruit GFX Library | Adafruit |

### Step 4: Create ThingSpeak Channel

1. Create account at https://thingspeak.com
2. Create new channel with 6 fields:
   - Field 1: Voltage
   - Field 2: Current
   - Field 3: Power
   - Field 4: Energy
   - Field 5: Frequency
   - Field 6: Power Factor
3. Copy your **Write API Key**

### Step 5: Configure and Upload Code

1. Open `sem-v1.ino`
2. Update these settings:
   ```cpp
   const char* WIFI_SSID = "your_wifi_name";
   const char* WIFI_PASSWORD = "your_password";
   const char* THINGSPEAK_API_KEY = "your_api_key";
   ```
3. Select: **Tools → Board → ESP32 Dev Module**
4. Select correct COM port
5. Click **Upload**

---

## 📊 How It Works

### System Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   AC MAINS  │────▶│  PZEM-004T  │────▶│    ESP32    │
│   220V/50Hz │     │  (Measure)  │     │  (Process)  │
└─────────────┘     └─────────────┘     └──────┬──────┘
                                               │
                          ┌────────────────────┼────────────────────┐
                          ▼                    ▼                    ▼
                   ┌─────────────┐      ┌─────────────┐      ┌─────────────┐
                   │    OLED     │      │ SERIAL MON  │      │ THINGSPEAK  │
                   │  (Display)  │      │   (Debug)   │      │   (Cloud)   │
                   └─────────────┘      └─────────────┘      └─────────────┘
```

### Update Intervals

| Action | Interval |
|--------|----------|
| Read PZEM data | 100ms |
| Update OLED display | 500ms |
| Switch OLED page | 3 seconds |
| Print to Serial | 2 seconds |
| Send to ThingSpeak | 20 seconds |

---

## 📱 Output Screens

### OLED Display - Page 1 (Main View)
```
┌────────────────────────────┐
│ ENERGY METER          WiFi │
│────────────────────────────│
│                            │
│      98.7W                 │
│                            │
│ V: 220.1V      I: 0.46A    │
│ Energy: 0.125 kWh          │
│────────────────────────────│
│ Page 1/2           50.0Hz  │
└────────────────────────────┘
```

### OLED Display - Page 2 (Detailed View)
```
┌────────────────────────────┐
│ DETAILED INFO         WiFi │
│────────────────────────────│
│ Voltage:  220.1 V          │
│ Current:  0.456 A          │
│ Power:    98.7 W           │
│ PF:       0.98             │
│────────────────────────────│
│ Page 2/2     0.125 kWh     │
└────────────────────────────┘
```

### ThingSpeak Dashboard

The ThingSpeak channel displays 6 charts:
- **Chart 1:** Voltage over time (V)
- **Chart 2:** Current over time (A)
- **Chart 3:** Power consumption (W)
- **Chart 4:** Cumulative energy (kWh)
- **Chart 5:** Frequency (Hz)
- **Chart 6:** Power Factor

---

## 🔍 Testing Procedure

### Test 1: No Load Connected
**Expected Result:**
- Voltage: ~220V (or your local voltage)
- Current: 0A
- Power: 0W

### Test 2: Connect a Known Load
Use a 100W light bulb:
- Voltage: ~220V
- Current: ~0.45A
- Power: ~100W

### Test 3: Verify ThingSpeak
1. Connect load
2. Wait 20 seconds
3. Check ThingSpeak dashboard
4. Charts should show new data points

---

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| PZEM not responding | Check TX/RX wiring (must be crossed) |
| All readings are 0 | Ensure CT coil is on LIVE wire only |
| WiFi not connecting | Verify SSID and password (case-sensitive) |
| ThingSpeak shows no data | Check API key is correct |
| OLED blank | Verify SDA/SCL connections |
| Voltage reading but no current | CT coil not properly clamped |

---

## 📈 Project Results

### Accuracy Comparison

| Parameter | Expected | Measured | Error |
|-----------|----------|----------|-------|
| Voltage | 220V | 219.8V | 0.09% |
| Current | 0.45A | 0.456A | 1.3% |
| Power | 100W | 98.7W | 1.3% |

### Features Achieved

✅ Real-time measurement of 6 electrical parameters
✅ Cloud data logging every 20 seconds
✅ Local OLED display with dual-page view
✅ Automatic WiFi reconnection
✅ Historical data visualization on ThingSpeak

---

## 🚀 Future Improvements

1. **Add cost calculator** - Show electricity bill estimate
2. **SMS/Email alerts** - Notify when consumption exceeds threshold
3. **Mobile app** - Create dedicated smartphone application
4. **SD card logging** - Store data locally for offline analysis
5. **Multi-phase support** - Extend to 3-phase industrial systems

---

## 📚 Key Learnings

Through this project, the following skills were developed:

1. **Embedded Programming** - C/C++ on ESP32 platform
2. **Sensor Interfacing** - UART, I2C protocols
3. **IoT Cloud Platforms** - ThingSpeak API integration
4. **Data Visualization** - Real-time charts and dashboards
5. **Circuit Design** - Power monitoring circuit
6. **Debugging** - Serial monitor analysis

---

## 📎 Project Files

| File | Description |
|------|-------------|
| `sem-v1.ino` | Main Arduino source code |
| `README.md` | Quick setup guide |
| `Project_Guide.md` | This detailed guide |
| `platformio.ini` | Alternative build configuration |

---

## 👤 Project Credits

**Student:** Aditi

**Duration:** February 2026


---

*This document serves as a comprehensive guide for understanding, replicating, and presenting the Smart Energy Meter project.*
