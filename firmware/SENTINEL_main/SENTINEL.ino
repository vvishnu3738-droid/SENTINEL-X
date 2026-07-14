/*
 * SENTINEL-X // CORE EMULATED TELEMETRY FIRMWARE MODULE
 * மைக்ரோகண்ட்ரோலர் கட்டமைப்பு: Renesas RA4W1 (Arduino UNO R4 WiFi)
 * மெய்நிகர் புரோட்டோடைப்பிங் சோதனைக்காக வடிவமைக்கப்பட்டது (Day 1 - Mission 05)
 */

#include <ArduinoJson.h> // தரவுகளை JSON ஸ்ட்ரிங்காக மாற்ற உதவும் லைப்ரரி

// டெலிமெட்ரி தரவு அமைப்பு (Data Structure)
struct EnvironmentalSignals {
  float ambientTemperature;
  float relativeHumidity;
  int airQualityPPM;
  int rawSmokeIndex;
  int lightIntensityLux;
  float chassisTiltAngle;
  int targetObstacleDistance;
  int virtualBatteryPercentage;
};

EnvironmentalSignals currentRobotState;

void setup() {
  Serial.begin(115200); // அதிவேக வன்பொருள் சீரியல் தகவல் தொடர்பைத் தொடங்குதல்
  while (!Serial) { ; } // கணினியுடன் இணைப்பு ஏற்படும் வரை காத்திருக்கவும்
  
  Serial.println(F("[SYSTEM]: SENTINEL-X Firmware Base Setup Engaged. Running Virtual Sensors."));
}

void loop() {
  // மெய்நிகர் சென்சார்களில் இருந்து தரவுகளைப் பெறுதல்
  acquireVirtualSensorTelemetry();
  
  // தரவுகளை நிலையான தொழில்முறை JSON ஸ்ட்ரிங்காக மாற்றுதல்
  String publishedPayload = compileJSONPacketString();
  
  // கணினித் திரைக்கு (Serial Monitor) தரவை அனுப்புதல்
  Serial.println(publishedPayload);
  
  delay(2000); // ஒவ்வொரு 2 விநாடிக்கும் இந்த சுழற்சி தொடரும்
}

void acquireVirtualSensorTelemetry() {
  // உண்மையான சென்சார் மதிப்புகள் மாறுவது போலவே கணித அல்காரிதம் மூலம் போலி மதிப்புகளை உருவாக்குதல்
  currentRobotState.ambientTemperature = 26.5 + (random(-10, 11) / 10.0);
  currentRobotState.relativeHumidity = 50.0 + (random(-20, 21) / 10.0);
  currentRobotState.airQualityPPM = random(35, 65);
  currentRobotState.rawSmokeIndex = random(90, 130);
  currentRobotState.lightIntensityLux = random(400, 500);
  currentRobotState.chassisTiltAngle = (random(-15, 16) / 10.0);
  currentRobotState.targetObstacleDistance = random(120, 200);
  currentRobotState.virtualBatteryPercentage = 94; // நிலையான பேட்டரி அளவு
}

String compileJSONPacketString() {
  JsonDocument jsonDoc; // நினைவகத்தை ஒதுக்குதல்
  
  // சேகரித்த மதிப்புகளை JSON கீகளாக (Keys) மாற்றுதல்
  jsonDoc["device"] = "SENTINEL-X";
  jsonDoc["temp"] = currentRobotState.ambientTemperature;
  jsonDoc["humidity"] = currentRobotState.relativeHumidity;
  jsonDoc["aqi"] = currentRobotState.airQualityPPM;
  jsonDoc["smoke"] = currentRobotState.rawSmokeIndex;
  jsonDoc["lux"] = currentRobotState.lightIntensityLux;
  jsonDoc["tilt"] = currentRobotState.chassisTiltAngle;
  jsonDoc["obstacle"] = currentRobotState.targetObstacleDistance;
  jsonDoc["battery"] = currentRobotState.virtualBatteryPercentage;
  
  String transmissionBuffer;
  serializeJson(jsonDoc, transmissionBuffer); // நினைவகப் பொருட்களை ASCII ஸ்ட்ரிங்காக மாற்றுதல்
  return transmissionBuffer;
}
