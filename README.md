# Smart Energy Meter - ESP32 + PZEM-004T v3.0

A complete smart energy meter solution using ESP32, PZEM-004T v3.0 power meter, and ThingSpeak for cloud visualization.

## 📊 Features

- Real-time monitoring of:
  - **Voltage** (V)
  - **Current** (A)
  - **Power** (W)
  - **Energy** (kWh)
  - **Frequency** (Hz)
  - **Power Factor**
- Cloud data logging via ThingSpeak
- Auto WiFi reconnection
- Serial monitor debugging

## 🔧 Hardware Required

| Component | Quantity |
|-----------|----------|
| ESP32 Dev Board | 1 |
| PZEM-004T v3.0 | 1 |
| CT Coil (100A) | 1 |
| Jumper Wires | 4 |

## 🔌 Wiring Diagram

```
PZEM-004T v3.0          ESP32 Dev Board
================        ===============
     TX  ─────────────►  GPIO16 (RX2)
     RX  ◄─────────────  GPIO17 (TX2)
     5V  ◄─────────────  5V (VIN)
    GND  ◄─────────────  GND

CT Coil: Clamp around LIVE wire only (not neutral!)
```

### Important Wiring Notes:
1. **TX/RX are crossed** - PZEM TX goes to ESP32 RX, and vice versa
2. **Use 5V** for PZEM power, not 3.3V
3. **CT Coil orientation** - Arrow on CT should point towards the load
4. **Only clamp LIVE wire** - Never clamp both wires together

## ⚙️ Setup Instructions

### Step 1: Install Arduino IDE Libraries

Install these libraries via Arduino IDE Library Manager (`Sketch` → `Include Library` → `Manage Libraries`):

1. **PZEM004Tv30** by Jakub Mandula
2. **WiFi** (built-in for ESP32)
3. **HTTPClient** (built-in for ESP32)

### Step 2: Configure ThingSpeak

1. Create a ThingSpeak account at [thingspeak.com](https://thingspeak.com)
2. Create a new Channel with these fields:
   - Field 1: Voltage
   - Field 2: Current
   - Field 3: Power
   - Field 4: Energy
   - Field 5: Frequency
   - Field 6: Power Factor
3. Go to **API Keys** tab and copy your **Write API Key**

### Step 3: Update Code Configuration

Open `smart_energy_meter.ino` and update these lines:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";          // Your WiFi name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";  // Your WiFi password
const char* THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";  // From Step 2
```

### Step 4: Upload to ESP32

1. Select Board: `Tools` → `Board` → `ESP32 Dev Module`
2. Select correct COM port
3. Click Upload

### Step 5: Monitor & Verify

1. Open Serial Monitor (`Tools` → `Serial Monitor`)
2. Set baud rate to **115200**
3. You should see:
   ```
   ========================================
      SMART ENERGY METER - ESP32 + PZEM
   ========================================
   
   [INIT] Initializing PZEM-004T v3.0...
   [INIT] Testing PZEM connection... OK!
   [WIFI] Connecting to YourNetwork... Connected!
   ```

## 🔍 Troubleshooting

### PZEM Not Responding

| Problem | Solution |
|---------|----------|
| No data from PZEM | Check TX/RX wiring (they should be crossed) |
| All readings are 0 | Ensure CT coil is clamped on LIVE wire |
| NaN values | Check 5V power supply to PZEM |
| Intermittent readings | Use shorter wires, add pull-up resistors |

### WiFi Issues

| Problem | Solution |
|---------|----------|
| Can't connect | Verify SSID/password (case-sensitive) |
| Keeps disconnecting | Check router distance, signal strength |
| ThingSpeak fails | Verify API key, check internet connection |

### ThingSpeak Shows No Data

1. Check update interval (minimum 15 seconds for free tier)
2. Verify Write API Key is correct
3. Ensure WiFi is connected (check Serial Monitor)
4. Check ThingSpeak Channel ID matches

## 📈 ThingSpeak Visualization

After data starts flowing, you can:

1. **View Charts**: Go to your channel's Private View
2. **Add Widgets**: Use MATLAB Visualizations for custom charts
3. **Set Alerts**: Create React apps for threshold notifications
4. **Export Data**: Download CSV for analysis

### Recommended MATLAB Visualizations:

- **Daily Energy Usage**: Bar chart of energy per day
- **Power Trend**: Line chart with moving average
- **Cost Calculator**: Custom script to calculate electricity bill

## 🔋 CT Coil Calibration

The PZEM-004T v3.0 with 100A CT coil is pre-calibrated. If readings seem off:

1. Use a known load (e.g., 100W light bulb)
2. Compare PZEM reading vs expected value
3. The PZEM typically has ±1% accuracy

## 📝 License

MIT License - Feel free to modify and use for your projects!

## 🤝 Contributing

Found a bug or have an improvement? Feel free to open an issue or submit a PR!
