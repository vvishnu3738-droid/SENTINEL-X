/*
 * SENTINEL-X // PRODUCTION HUMAN-MACHINE INTERFACE (HMI) & VISUAL PERCEPTION ENGINE
 * மைக்ரோகண்ட்ரோலர் கட்டமைப்பு: Renesas RA4W1 (Arduino UNO R4 WiFi)
 * டெவலப்பர் டிராக்: எச்எம்ஐ பாதுகாப்பு இன்ஜினியரிங், ஆப்டிகல் சிக்னலிங் & அலார ஆட்டோமேஷன்
 */

#include <ArduinoJson.h>

// 1. SYSTEM FINITE STATE MACHINE REGISTRY (5 தனித்துவ நிலைகள்)
enum RobotState {
  STATE_INITIALIZING,
  STATE_AUTONOMOUS_PATROL,
  STATE_HAZARD_RESPONSE,
  STATE_AUTO_DOCKING,
  STATE_SYSTEM_FAULT_LOCK
};

RobotState currentSystemState = STATE_INITIALIZING;
unsigned long lastHMILoopTimestamp = 0;
const unsigned long hmiRefreshInterval = 250; // 250ms அதிவேக எச்எம்ஐ சுழற்சி புதுப்பிப்பு டைமர்

// கணினி தரவு மாறிகள்
float readTempCelsius = 26.5;
int readSmokeIndex = 110;  
int readGasPPM = 45;       
int batteryCapacityPercent = 95;
String systemFaultLogCode = "NONE";

// வன்பொருள் பின் ஒதுக்கீடுகள் (நாள் 1 - மிஷன் 02 மேட்ரிக்ஸ் படி இணைக்கப்பட்டது)
const int PIN_RGB_RED   = 10;
const int PIN_RGB_GREEN = 11;
const int PIN_BUZZER    = 12;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  
  // அறிவிப்பு பின்களை அவுட்புட் ஆக மாற்றுதல்
  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  Serial.println(F("[HMI BOOT]: Initializing Optical Drivers and Acoustic Signal Rails..."));
  executeHardwareHMIPowerUpTest(); // தொடக்க வன்பொருள் சோதனை ஓட்டம்
  
  currentSystemState = STATE_AUTONOMOUS_PATROL;
}

void loop() {
  unsigned long currentMillis = millis();

  // நான்-பிளாக்கிங் எச்எம்ஐ கட்டுப்பாட்டு லூப்
  if (currentMillis - lastHMILoopTimestamp >= hmiRefreshInterval) {
    lastHMILoopTimestamp = currentMillis;
    
    // சென்சார் மதிப்புகள் மற்றும் போலி தரவுகளைப் பெறுதல்
    simulateTelemetrySensorBusPull();
    
    // பாதுகாப்பு எல்லை தணிக்கைச் சோதனை
    executeSystemSafetyAuditChecks();
    
    // தற்போதைய நிலைக்கு ஏற்ப எல்இடி விளக்குகள் மற்றும் அலார பஸரை இயக்குதல்
    executeVisualAcousticPerceptionDrivers(currentMillis);
    
    // தரவை வைஃபை டேஷ்போர்டுக்கு அனுப்புதல்
    emitCentralIoTDataPacket();
  }
}

// ================================================================================
// எச்எம்ஐ வன்பொருள் இயக்கப் பகுதி (HMI HARDWARE LAYER DRIVER LOGIC)
// ================================================================================

void executeHardwareHMIPowerUpTest() {
  // ரோபோ ஆன் ஆகும்போது விளக்குகள் மற்றும் பஸர் வேலை செய்கிறதா என்று சோதித்தல்
  digitalWrite(PIN_RGB_RED, HIGH); delay(1000);
  digitalWrite(PIN_RGB_GREEN, HIGH); delay(1000);
  digitalWrite(PIN_RGB_RED, LOW);
  digitalWrite(PIN_RGB_GREEN, LOW);
  digitalWrite(PIN_BUZZER, LOW);
}

void executeVisualAcousticPerceptionDrivers(unsigned long currentMillis) {
  /*
   * நான்-பிளாக்கிங் முறையில் 250ms கிளாக் பல்ஸ் பயன்படுத்தி 
   * வன்பொருள் பின்களை மாற்றி மாற்றி ஆன்/ஆஃப் செய்யும் பகுதி.
   */
  static bool pulseToggleState = false;
  pulseToggleState = !pulseToggleState; // ஒவ்வொரு 250ms-க்கும் நிலையை மாற்றுதல்

  switch (currentSystemState) {
    
    case STATE_AUTONOMOUS_PATROL:
      // குறியீடு 01: நிலையான நியான் பச்சை விளக்கு (பாதுகாப்பான ரோந்து நிலை)
      digitalWrite(PIN_RGB_RED, LOW);
      digitalWrite(PIN_RGB_GREEN, HIGH);
      digitalWrite(PIN_BUZZER, LOW); // அலார சைரன் முற்றிலும் ஆஃப்
      break;

    case STATE_AUTO_DOCKING:
      // குறியீடு 02: விட்டு விட்டு எரியும் மஞ்சள் விளக்கு (சிவப்பு + பச்சை சேர்ந்தால் மஞ்சள்)
      if (pulseToggleState) {
        digitalWrite(PIN_RGB_RED, HIGH);
        digitalWrite(PIN_RGB_GREEN, HIGH); 
      } else {
        digitalWrite(PIN_RGB_RED, LOW);
        digitalWrite(PIN_RGB_GREEN, LOW);
      }
      digitalWrite(PIN_BUZZER, LOW);
      break;

    case STATE_HAZARD_RESPONSE:
      // குறியீடு 03: அதிவேகமாக மின்னும் சிவப்பு விளக்கு + தொடர்ச்சியான அவசர கால அலார சத்தம்
      digitalWrite(PIN_RGB_GREEN, LOW);
      if (pulseToggleState) {
        digitalWrite(PIN_RGB_RED, HIGH);
        digitalWrite(PIN_BUZZER, HIGH); // வெளியேற்ற சைரன் சத்தம் ஓட்டம்
      } else {
        digitalWrite(PIN_RGB_RED, LOW);
        digitalWrite(PIN_BUZZER, LOW);
      }
      break;

    case STATE_SYSTEM_FAULT_LOCK:
      // குறியீடு 04: மெதுவாக 1 விநாடிக்கு ஒருமுறை மின்னும் அலார சத்தம் (பழுது நிலை)
      digitalWrite(PIN_RGB_GREEN, LOW);
      static int staccatoCounter = 0;
      if (++staccatoCounter >= 4) { // 4 * 250ms = 1 விநாடிக்கு ஒருமுறை இயங்கும்
        digitalWrite(PIN_RGB_RED, pulseToggleState ? HIGH : LOW);
        digitalWrite(PIN_BUZZER, pulseToggleState ? HIGH : LOW);
        staccatoCounter = 0;
      }
      break;
  }
}

void simulateTelemetrySensorBusPull() {
  if (currentSystemState == STATE_AUTONOMOUS_PATROL) {
    readTempCelsius = 26.5 + (random(-5, 6) / 10.0);
    readGasPPM = random(40, 65);
    readSmokeIndex = random(95, 120);
    
    // டெஸ்டிங்கிற்காகப் பேட்டரியைக் குறைத்துக் காட்டுதல்
    static int simulationTimeTicks = 0;
    if (++simulationTimeTicks >= 60) { 
      batteryCapacityPercent = 15;
    }
    
    // டெஸ்டிங்கிற்காகப் போலி கேஸ் விபத்து சிக்னலை உள்ளே செலுத்துதல்
    if (random(0, 100) > 97) {
      readGasPPM = 490;
      readSmokeIndex = 580;
    }
  }
}

void executeSystemSafetyAuditChecks() {
  if (readGasPPM > 400 || readSmokeIndex > 500) {
    currentSystemState = STATE_HAZARD_RESPONSE;
  } 
  else if (batteryCapacityPercent <= 20) {
    currentSystemState = STATE_AUTO_DOCKING;
    // சார்ஜ் சுழற்சி உருவகப்படுத்துதல்
    batteryCapacityPercent += 20;
    if (batteryCapacityPercent >= 100) {
      batteryCapacityPercent = 100;
      currentSystemState = STATE_AUTONOMOUS_PATROL;
    }
  }
}

void emitCentralIoTDataPacket() {
  JsonDocument outDoc;
  
  String txtState = "PATROL";
  if (currentSystemState == STATE_HAZARD_RESPONSE) txtState = "HAZARD_CRIT_ALERT";
  if (currentSystemState == STATE_AUTO_DOCKING) txtState = "RETURNING_TO_DOCK_BASE";
  if (currentSystemState == STATE_SYSTEM_FAULT_LOCK) txtState = "HARDWARE_SYSTEM_FAULT";

  outDoc["device"] = "SENTINEL-X";
  outDoc["state"] = txtState;
  outDoc["temp"] = readTempCelsius;
  outDoc["gas"] = readGasPPM;
  outDoc["smoke"] = readSmokeIndex;
  outDoc["battery"] = batteryCapacityPercent;

  String txBuffer;
  serializeJson(outDoc, txBuffer);
  Serial.println(txBuffer); // இந்தத் தரவுத் தொகுப்புகள் மெம்பர் 1-ன் வெப் டேஷ்போர்டை இயக்கும்
}
