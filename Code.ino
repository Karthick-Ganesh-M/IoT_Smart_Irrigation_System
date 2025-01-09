#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// WiFi credentials
const char* ssid = "Karthick";     // Replace with your Wi-Fi SSID
const char* password = "12345678"; // Replace with your Wi-Fi password

// Firebase credentials
#define FIREBASE_HOST "https://sih-hardware-202d6-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "AIzaSyAHAOZM_W6CNmRbxBa1qCWIA4Gza9ysfa0"

// Soil moisture sensor pin
#define SOIL_MOISTURE_SENSOR_PIN 33  // Change as per your connection
#define RELAY_PIN 4                   // Relay pin for controlling the motor
#define BUTTON_PIN 2                  // Button pin for manual control

// LCD I2C wiring
const int I2C_SDA_PIN = 21;       // SDA pin connected to GPIO 21
const int I2C_SCL_PIN = 22;       // SCL pin connected to GPIO 22
const int I2C_ADDR = 0x27;        // I2C address of the LCD
const int LCD_COLUMNS = 16;       // Number of columns on the LCD
const int LCD_ROWS = 2;           // Number of rows on the LCD

// Create instances for the LCD and Firebase
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_ROWS);
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

// Variable to store moisture value
int moistureValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Soil Moisture and Relay Control");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set Firebase config
  config.host = FIREBASE_HOST;
  config.api_key = FIREBASE_AUTH;

  // Set Firebase authentication (if needed)
  // Uncomment if you're using email/password authentication
   auth.user.email = "agventure06@gmail.com";  
   auth.user.password = "agventure06";  

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize the relay and button pin
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input with internal pull-up resistor
  digitalWrite(RELAY_PIN, HIGH); // Initially turn off the relay

  // Initialize the LCD
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();

  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("Soil Moisture");
  delay(3000);  // Display message for 3 seconds
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moisture:");
}

void loop() {
  // Read soil moisture value
  moistureValue = analogRead(SOIL_MOISTURE_SENSOR_PIN);
  moistureValue = map(moistureValue, 0, 4095, 0, 100); // Map to percentage
  moistureValue = (moistureValue - 100) * -1; // Adjust value

  // Ensure moisture value is non-negative
  if (moistureValue < 0) {
    moistureValue = 0;
  }

  // Print moisture value to the serial monitor
  Serial.print("Moisture: ");
  Serial.print(moistureValue);
  Serial.println("%");

  // Display moisture value on the LCD
  lcd.setCursor(0, 1);
  lcd.print("  "); // Clear previous value
  lcd.setCursor(0, 1);
  lcd.print(moistureValue);
  lcd.print("%");

  // Prepare JSON data
  String path = "/moisture"; // Firebase path to store moisture value
  FirebaseJson json;
  json.set("moistureValue", moistureValue);

  // Send data to Firebase
  if (Firebase.updateNode(firebaseData, path, json)) {
    Serial.println("Data updated successfully");
  } else {
    Serial.println("Failed to update data");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

  // Control the relay based on the button input
  if (digitalRead(BUTTON_PIN) == LOW) {  // If button is pressed
    digitalWrite(RELAY_PIN, LOW); // Turn on the relay
    lcd.setCursor(0, 1);
    lcd.print("Relay ON     ");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Turn off the relay
    lcd.setCursor(0, 1);
    lcd.print("Relay OFF    ");
  }

  delay(1000);  // Read every second
}