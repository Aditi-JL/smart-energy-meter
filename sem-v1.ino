/*
 * Smart Energy Meter using ESP32 + PZEM-004T v3.0 + OLED Display
 * Data visualization via ThingSpeak + Local OLED
 * 
 * Hardware Connections:
 * PZEM-004T TX -> ESP32 GPIO16 (RX2)
 * PZEM-004T RX -> ESP32 GPIO17 (TX2)
 * PZEM-004T VCC -> 5V
 * PZEM-004T GND -> GND
 * 
 * OLED Display (I2C):
 * OLED SDA -> ESP32 GPIO21
 * OLED SCL -> ESP32 GPIO22
 * OLED VCC -> 3.3V
 * OLED GND -> GND
 * 
 * CT Coil: Clamp around the LIVE wire (not neutral)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <PZEM004Tv30.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== OLED CONFIGURATION ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Reset pin not used
#define OLED_ADDRESS 0x3C  // Common I2C address (try 0x3D if this doesn't work)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==================== CONFIGURATION ====================
// WiFi Credentials
const char* WIFI_SSID = "wifi_id";
const char* WIFI_PASSWORD = "wifi_pp";

// ThingSpeak Settings
const char* THINGSPEAK_API_KEY = "thinspeak_api_key";
const char* THINGSPEAK_SERVER = "api.thingspeak.com";
const unsigned long CHANNEL_ID = 0; // Your channel ID (optional, for reference)

// PZEM Serial Configuration
#define PZEM_RX_PIN 16  // ESP32 RX2 <- PZEM TX
#define PZEM_TX_PIN 17  // ESP32 TX2 -> PZEM RX
#define PZEM_SERIAL Serial2

// Timing Configuration
const unsigned long THINGSPEAK_INTERVAL = 20000; // 20 seconds
const unsigned long SERIAL_PRINT_INTERVAL = 2000; // Print to serial every 2 seconds
const unsigned long OLED_UPDATE_INTERVAL = 500;   // Update OLED every 500ms
const unsigned long OLED_PAGE_INTERVAL = 3000;    // Switch OLED page every 3 seconds

// ==================== GLOBAL VARIABLES ====================
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

// Timing variables
unsigned long lastThingSpeakUpdate = 0;
unsigned long lastSerialPrint = 0;
unsigned long lastOLEDUpdate = 0;
unsigned long lastPageSwitch = 0;

// OLED page (0 = main readings, 1 = detailed info)
int oledPage = 0;
bool oledInitialized = false;

// Energy readings
float voltage = 0;
float current = 0;
float power = 0;
float energy = 0;
float frequency = 0;
float powerFactor = 0;

// WiFi connection status
bool wifiConnected = false;

// ==================== FUNCTION DECLARATIONS ====================
void connectToWiFi();
bool readPZEMData();
void printReadings();
void sendToThingSpeak();
void initOLED();
void updateOLED();
void drawMainPage();
void drawDetailPage();
void drawHeader();

// ==================== SETUP ====================
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("   SMART ENERGY METER - ESP32 + PZEM");
  Serial.println("========================================");
  Serial.println();
  
  // Initialize OLED
  initOLED();
  
  // Show boot screen
  if (oledInitialized) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 20);
    display.println("SMART ENERGY");
    display.setCursor(35, 35);
    display.println("METER");
    display.setCursor(25, 50);
    display.println("Initializing...");
    display.display();
  }
  
  // Initialize PZEM Serial
  Serial.println("[INIT] Initializing PZEM-004T v3.0...");
  delay(1000);
  
  // Test PZEM connection
  Serial.print("[INIT] Testing PZEM connection... ");
  float testVoltage = pzem.voltage();
  if (!isnan(testVoltage)) {
    Serial.println("OK!");
    Serial.printf("[INIT] Initial voltage reading: %.1f V\n", testVoltage);
  } else {
    Serial.println("FAILED!");
    Serial.println("[WARN] PZEM not responding. Check wiring:");
    Serial.println("       - PZEM TX -> ESP32 GPIO16");
    Serial.println("       - PZEM RX -> ESP32 GPIO17");
    Serial.println("       - Ensure CT coil is clamped on LIVE wire");
    Serial.println("       - Check 5V power to PZEM module");
  }
  Serial.println();
  
  // Update OLED with WiFi connecting message
  if (oledInitialized) {
    display.clearDisplay();
    display.setCursor(20, 25);
    display.println("Connecting to");
    display.setCursor(35, 40);
    display.println("WiFi...");
    display.display();
  }
  
  // Connect to WiFi
  connectToWiFi();
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("        SETUP COMPLETE - RUNNING");
  Serial.println("========================================");
  Serial.println();
}

// ==================== OLED INITIALIZATION ====================
void initOLED() {
  Serial.print("[INIT] Initializing OLED display... ");
  
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    oledInitialized = true;
    Serial.println("OK!");
    display.clearDisplay();
    display.display();
  } else {
    oledInitialized = false;
    Serial.println("FAILED!");
    Serial.println("[WARN] OLED not found. Check wiring:");
    Serial.println("       - OLED SDA -> ESP32 GPIO21");
    Serial.println("       - OLED SCL -> ESP32 GPIO22");
    Serial.println("       - Try I2C address 0x3D if 0x3C fails");
  }
}

// ==================== MAIN LOOP ====================
void loop() {
  unsigned long currentMillis = millis();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("[WIFI] Connection lost! Reconnecting...");
      wifiConnected = false;
    }
    connectToWiFi();
  }
  
  // Read PZEM data
  readPZEMData();
  
  // Update OLED display
  if (currentMillis - lastOLEDUpdate >= OLED_UPDATE_INTERVAL) {
    lastOLEDUpdate = currentMillis;
    updateOLED();
  }
  
  // Switch OLED page
  if (currentMillis - lastPageSwitch >= OLED_PAGE_INTERVAL) {
    lastPageSwitch = currentMillis;
    oledPage = (oledPage + 1) % 2;  // Toggle between 0 and 1
  }
  
  // Print to Serial periodically
  if (currentMillis - lastSerialPrint >= SERIAL_PRINT_INTERVAL) {
    lastSerialPrint = currentMillis;
    printReadings();
  }
  
  // Send to ThingSpeak periodically
  if (currentMillis - lastThingSpeakUpdate >= THINGSPEAK_INTERVAL) {
    lastThingSpeakUpdate = currentMillis;
    if (wifiConnected) {
      sendToThingSpeak();
    } else {
      Serial.println("[THINGSPEAK] Skipped - WiFi not connected");
    }
  }
  
  // Small delay to prevent overwhelming the PZEM
  delay(100);
}

// ==================== OLED UPDATE ====================
void updateOLED() {
  if (!oledInitialized) return;
  
  display.clearDisplay();
  
  if (oledPage == 0) {
    drawMainPage();
  } else {
    drawDetailPage();
  }
  
  display.display();
}

// ==================== OLED MAIN PAGE ====================
void drawMainPage() {
  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("ENERGY METER");
  
  // WiFi indicator
  display.setCursor(100, 0);
  display.print(wifiConnected ? "WiFi" : "----");
  
  // Horizontal line
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  
  // Large Power reading
  display.setTextSize(2);
  display.setCursor(0, 15);
  display.printf("%.1fW", power);
  
  // Voltage and Current
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.printf("V: %.1fV", voltage);
  display.setCursor(70, 35);
  display.printf("I: %.2fA", current);
  
  // Energy consumption
  display.setCursor(0, 48);
  display.printf("Energy: %.3f kWh", energy);
  
  // Bottom status bar
  display.drawLine(0, 58, 127, 58, SSD1306_WHITE);
  display.setCursor(0, 60);
  display.setTextSize(1);
  
  // Page indicator
  display.print("Page 1/2");
  display.setCursor(80, 60);
  display.printf("%.1fHz", frequency);
}

// ==================== OLED DETAIL PAGE ====================
void drawDetailPage() {
  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("DETAILED INFO");
  
  // WiFi indicator
  display.setCursor(100, 0);
  display.print(wifiConnected ? "WiFi" : "----");
  
  // Horizontal line
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  
  // All readings in list format
  display.setCursor(0, 14);
  display.printf("Voltage:  %.1f V", voltage);
  
  display.setCursor(0, 24);
  display.printf("Current:  %.3f A", current);
  
  display.setCursor(0, 34);
  display.printf("Power:    %.1f W", power);
  
  display.setCursor(0, 44);
  display.printf("PF:       %.2f", powerFactor);
  
  // Bottom status bar
  display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print("Page 2/2");
  display.setCursor(60, 56);
  display.printf("%.3f kWh", energy);
}

// ==================== WIFI CONNECTION ====================
void connectToWiFi() {
  Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println(" Connected!");
    Serial.printf("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WIFI] Signal Strength: %d dBm\n", WiFi.RSSI());
  } else {
    wifiConnected = false;
    Serial.println(" FAILED!");
    Serial.println("[WIFI] Could not connect. Will retry later.");
  }
}

// ==================== READ PZEM DATA ====================
bool readPZEMData() {
  // Read all values from PZEM
  float v = pzem.voltage();
  float c = pzem.current();
  float p = pzem.power();
  float e = pzem.energy();
  float f = pzem.frequency();
  float pf = pzem.pf();
  
  // Check if readings are valid
  if (isnan(v) || isnan(c) || isnan(p)) {
    // Keep previous values if reading fails
    return false;
  }
  
  // Update global variables
  voltage = v;
  current = c;
  power = p;
  energy = e;
  frequency = f;
  powerFactor = pf;
  
  return true;
}

// ==================== PRINT READINGS ====================
void printReadings() {
  Serial.println("----------------------------------------");
  Serial.println("          ENERGY METER READINGS");
  Serial.println("----------------------------------------");
  
  if (!isnan(voltage)) {
    Serial.printf("  Voltage:      %.1f V\n", voltage);
    Serial.printf("  Current:      %.3f A\n", current);
    Serial.printf("  Power:        %.1f W\n", power);
    Serial.printf("  Energy:       %.3f kWh\n", energy);
    Serial.printf("  Frequency:    %.1f Hz\n", frequency);
    Serial.printf("  Power Factor: %.2f\n", powerFactor);
  } else {
    Serial.println("  [ERROR] No data from PZEM-004T");
    Serial.println("  Check connections and CT coil placement");
  }
  
  Serial.printf("  WiFi Status:  %s\n", wifiConnected ? "Connected" : "Disconnected");
  Serial.printf("  OLED Status:  %s\n", oledInitialized ? "OK" : "Not Found");
  Serial.println("----------------------------------------");
  Serial.println();
}

// ==================== SEND TO THINGSPEAK ====================
void sendToThingSpeak() {
  if (isnan(voltage)) {
    Serial.println("[THINGSPEAK] Skipped - No valid PZEM data");
    return;
  }
  
  Serial.println("[THINGSPEAK] Sending data...");
  
  HTTPClient http;
  
  // Build the URL with all fields
  String url = "http://";
  url += THINGSPEAK_SERVER;
  url += "/update?api_key=";
  url += THINGSPEAK_API_KEY;
  url += "&field1=" + String(voltage, 1);      // Voltage
  url += "&field2=" + String(current, 3);      // Current
  url += "&field3=" + String(power, 1);        // Power
  url += "&field4=" + String(energy, 3);       // Energy
  url += "&field5=" + String(frequency, 1);    // Frequency
  url += "&field6=" + String(powerFactor, 2);  // Power Factor
  
  http.begin(url);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("[THINGSPEAK] Success! Entry ID: %s\n", response.c_str());
  } else {
    Serial.printf("[THINGSPEAK] Error: %d\n", httpResponseCode);
  }
  
  http.end();
}

// ==================== UTILITY FUNCTIONS ====================

// Reset energy counter (call this if needed)
void resetEnergy() {
  Serial.println("[PZEM] Resetting energy counter...");
  if (pzem.resetEnergy()) {
    Serial.println("[PZEM] Energy counter reset successful!");
  } else {
    Serial.println("[PZEM] Energy counter reset failed!");
  }
}

// Get PZEM address (for debugging)
void getPZEMAddress() {
  uint8_t addr = pzem.getAddress();
  Serial.printf("[PZEM] Current address: 0x%02X\n", addr);
}
