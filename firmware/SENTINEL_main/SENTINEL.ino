/*
 * SENTINEL-X // PRODUCTION DIAGNOSTIC ENGINE & SYSTEM FAIL-SAFE FIRMWARE
 * மைக்ரோகண்ட்ரோலர் கட்டமைப்பு: Renesas RA4W1 (Arduino UNO R4 WiFi)
 * டெவலப்பர் டிராக்: சிஸ்டம்ஸ் பாதுகாப்பு, நம்பகத்தன்மை & ஃபால்ட்-டாலரண்ட் இன்ஜினியரிங்
 */

#include <ArduinoJson.h>

// 1. SYSTEM FINITE STATE MACHINE REGISTRY (மேம்படுத்தப்பட்ட 5 நிலைகள்)
enum RobotState {
  STATE_INITIALIZING,
  STATE_AUTONOMOUS_PATROL,
  STATE_HAZARD_RESPONSE,
  STATE_AUTO_DOCKING,
  STATE_SYSTEM_FAULT_LOCK  // பழுது ஏற்பட்டால் லாக் ஆகும் புதிய அவசர நிலை
};

RobotState currentSystemState = STATE_INITIALIZING;
unsigned long lastDiagnosticTimestamp = 0;
const unsigned long diagnosticInterval = 1000; // 1 விநாடிக்கு ஒருமுறை நடக்கும் பாதுகாப்பு தணிக்கை

// வன்பொருள் சென்சார் மற்றும் பழுது கண்காணிப்பு மாறிகள் (Hardware & Fault Registers)
float readTempCelsius = 26.5;
int readSmokeIndex = 110;  
int readGasPPM = 45;       
float readTiltAngle = 0.0; 
int batteryCapacityPercent = 95;

// சென்சார் ஆரோக்கியக் கண்காணிப்பாளர்கள் (Health Status Registers)
bool isTempSensorHealthy = true;
bool isGasSensorHealthy = true;
bool isSmokeSensorHealthy = true;
String activeFaultDescription = "NONE";

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; } 
  
  Serial.println(F("[SAFETY BOOT]: Securing system core buses..."));
  delay(1000);
  
  currentSystemState = STATE_AUTONOMOUS_PATROL;
  Serial.println(F("[SAFETY BOOT]: Hardware Diagnostic Monitors Online. System Guard Active."));
}

void loop() {
  unsigned long currentMillis = millis();

  // நான்-பிளாக்கிங் பாதுகாப்பு தணிக்கை சுழற்சி
  if (currentMillis - lastDiagnosticTimestamp >= diagnosticInterval) {
    lastDiagnosticTimestamp = currentMillis;
    
    // படி A: சென்சார் தரவுகளைப் பெறுதல்
    executeSensorDataAcquisition();
    
    // படி B: வன்பொருள் ஆரோக்கிய தணிக்கை (Hardware Health Audit)
    executeHardwareHealthAudit();
    
    // படி C: TinyML இன்டர்ன்ஸ் மற்றும் தோல்வி-காப்பு லாஜிக் இயக்கம்
    if (currentSystemState != STATE_SYSTEM_FAULT_LOCK) {
      float calculatedRisk = executeTinyMLSensorFusionInference();
      executeDynamicStateEngine(calculatedRisk);
    } else {
      executeEmergencySafeHoldBrake(); // பழுது ஏற்பட்டால் மோட்டார்களை பிரேக் அடித்தல்
    }
    
    // படி D: பாதுகாப்பு லாக் விபரங்களை டேஷ்போர்டுக்கு அனுப்புதல்
    transmitSafetyTelemetryPacket();
  }
}

// ================================================================================
// பாதுகாப்பு தணிக்கை மற்றும் தோல்வி-காப்பு பகுதிகள் (SAFETY AUDIT METHODS)
// ================================================================================

void executeSensorDataAcquisition() {
  // சென்சார் மதிப்புகள் மாறுவதை உருவாக்குதல்
  readTempCelsius = 25.0 + (random(0, 30) / 10.0);
  readGasPPM = random(35, 60);
  readSmokeIndex = random(95, 130);
  readTiltAngle = (random(-10, 11) / 10.0);
  
  // டெஸ்டிங்கிற்காக 5% வாய்ப்பில் ஒரு சென்சார் ஒயர் அறுந்து போவதை உருவாக்குதல் (Hardware Fault Injection)
  if (random(0, 100) > 95) {
    readTempCelsius = -999.0; // DHT22 ஒயர் கழன்றதைக் குறிக்கும் போலி எண்
    Serial.println(F("\n[FAULT INJECTOR]: Simulating physical wire rupture on DHT22 Bus..."));
  }
}

void executeHardwareHealthAudit() {
  /*
   * வன்பொருள் எல்லைகளைத் தணிக்கை செய்தல்.
   * எண்கள் எல்லையைத் தாண்டினால் உடனடியாகப் பாதுகாப்புப் பொறியை ஆன் செய்யும்.
   */
  
  // 1. வெப்பநிலை சென்சார் தணிக்கை (-10C க்குக் கீழ் அல்லது 75C க்கு மேல் போனால் பழுது)
  if (readTempCelsius < -10.0 || readTempCelsius > 75.0) {
    isTempSensorHealthy = false;
    currentSystemState = STATE_SYSTEM_FAULT_LOCK;
    activeFaultDescription = "DHT22_TEMPERATURE_SENSOR_DISCONNECTED_FAULT";
  }
  
  // 2. கேஸ் சென்சார் தணிக்கை (PPM மதிப்பு எதிர்மறையாகப் போனால் பழுது)
  if (readGasPPM < 0 || readGasPPM > 1000) {
    isGasSensorHealthy = false;
    currentSystemState = STATE_SYSTEM_FAULT_LOCK;
    activeFaultDescription = "MQ135_GAS_SENSOR_HARDWARE_FAILURE";
  }
}

float executeTinyMLSensorFusionInference() {
  // சென்சார்கள் ஆரோக்கியமாக இருந்தால் மட்டும் AI கணக்கீட்டைச் செய்தல்
  float normTemp  = (readTempCelsius - 0.0) / 60.0;
  float normSmoke = (float)readSmokeIndex / 1023.0;
  float normGas   = (float)readGasPPM / 500.0;
  
  float riskScore = (normTemp * 0.35) + (normSmoke * 0.35) + (normGas * 0.30);
  return (riskScore * 100.0);
}

void executeDynamicStateEngine(float riskMetric) {
  if (riskMetric >= 45.0) {
    currentSystemState = STATE_HAZARD_RESPONSE;
  } else if (batteryCapacityPercent <= 20) {
    currentSystemState = STATE_AUTO_DOCKING;
    batteryCapacityPercent = 100; // சார்ஜ் ஏறுவதை உருவகப்படுத்துதல்
  } else {
    currentSystemState = STATE_AUTONOMOUS_PATROL;
  }
}

void executeEmergencySafeHoldBrake() {
  // மோட்டார்களை பிரேக் அடித்து நிறுத்துதல் (Brake Actuators)
  // நிஜப் பொருட்கள் வரும்போது இங்கு டிஜிட்டல் பின்கள் LOW ஆக்கப்படும்
  Serial.print(F("[⚠️ CRITICAL FAIL-SAFE ACTIVATED]: "));
  Serial.println(activeFaultDescription);
  Serial.println(F("[⚠️ CRITICAL FAIL-SAFE]: Drivetrain Locked. Actuators Braked. Awaiting Maintenance."));
}

void transmitSafetyTelemetryPacket() {
  JsonDocument safetyDoc;
  
  String stringState = "PATROL";
  if (currentSystemState == STATE_HAZARD_RESPONSE) stringState = "CRIT_HAZARD_ALERT";
  if (currentSystemState == STATE_AUTO_DOCKING) stringState = "CHARGING_AT_BASE";
  if (currentSystemState == STATE_SYSTEM_FAULT_LOCK) stringState = "SYSTEM_FAULT_LOCK"; // பழுது சிக்னல்

  safetyDoc["device"] = "SENTINEL-X";
  safetyDoc["state"] = stringState;
  safetyDoc["temp"] = (readTempCelsius == -999.0) ? 0.0 : readTempCelsius; // எர்ரர் எண்ணை மறைத்து 0 காட்டுதல்
  safetyDoc["gas"] = readGasPPM;
  safetyDoc["battery"] = batteryCapacityPercent;
  safetyDoc["fault_log"] = activeFaultDescription; // வெப் பக்கத்தில் எந்த ஒயர் கழன்றது என்று காட்டும்

  String outputBuffer;
  serializeJson(safetyDoc, outputBuffer);
  Serial.println(outputBuffer); // இது மெம்பர் 1-ன் வெப் அலாரத்தை ஆன் செய்யும்
}
