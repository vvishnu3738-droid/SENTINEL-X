/*
 * SENTINEL-X // PRODUCTION SENSOR CALIBRATION & SIGNAL SMOOTHING ENGINE
 * மைக்ரோகண்ட்ரோலர் கட்டமைப்பு: Renesas RA4W1 (Arduino UNO R4 WiFi)
 * டெவலப்பர் டிராக்: சிக்னல் பிராசஸிங், அளவுத்திருத்தம் & டேட்டா ஃபில்டரிங் இன்ஜினியரிங்
 */
#include <Wire.h>             // I2C தகவல் தொடர்பு
#include <Adafruit_GFX.h>     // கிராபிக்ஸ் கோர்கள்
#include <Adafruit_SSD1306.h> // OLED டிஸ்ப்ளே லைப்ரரி
#include <SoftwareSerial.h>

// Software Serial Communication Bus configuration pins for DFPlayer Mini
// Pin D2 connects to DFPlayer TX | Pin D7 connects to DFPlayer RX (via 1K Ohm Resistor)
SoftwareSerial audioSerialBus(2, 7); 

// Core command transmission array packet for the DFPlayer hardware
const byte DFPLAYER_PLAY_TRACK_CMD[] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x01, 0xFE, 0xF7};

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <ArduinoJson.h>

enum RobotState {
  STATE_INITIALIZING,
  STATE_AUTONOMOUS_PATROL,
  STATE_HAZARD_RESPONSE,
  STATE_AUTO_DOCKING
};

RobotState currentSystemState = STATE_INITIALIZING;
unsigned long lastExecutionTimestamp = 0;
const unsigned long executionInterval = 1000; // 1 விநாடிக்கு ஒருமுறை இயங்கும் முதன்மை லூப்

// சென்சார் தரவு மாறிகள்
float readTempCelsius = 26.5;
int rawGasInput = 110;
int filteredGasOutput = 110; // இரைச்சல் நீக்கப்பட்ட இறுதி கேஸ் மதிப்பு
int batteryCapacityPercent = 100;

// அளவுத்திருத்த பதிவேடுகள் (Calibration Constants)
int cleanAirBaselineR0 = 0; // சுத்தமான காற்றின் ஆரம்ப பே特லைன் மதிப்பு
const int GAS_HAZARD_DELTA = 250; // பேஸ்லைனில் இருந்து எவ்வளவு உயர்ந்தால் ஆபத்து என்ற வரம்பு
bool isCalibrationComplete = false;

// நகரும் சராசரி ஃபில்டர் மாறிகள் (Moving Average Filter Buffer - 5 புள்ளிகள்)
const int FILTER_SIZE = 5;
int gasReadingsBuffer[FILTER_SIZE];
int bufferWritePointer = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
    // Initialize the audio transmission matrix link at 9600 baud rate
  audioSerialBus.begin(9600);
  delay(500); // Allow hardware controller to stabilize power
  
  // Trigger system boot vocal alert announcement clip (Track 0001)
  executeDFPlayerHardwareTrigger(1); 

  Serial.println(F("\n=================================================="));
  Serial.println(F(" SENTINEL-X // SENSOR SYSTEM CALIBRATION INTERFACE "));
  Serial.println(F("=================================================="));
    // 0x3C ஐடி மூலமாக OLED திரையை ஆன் செய்தல்
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("[OLED ERROR]: SSD1306 Hardware failed!"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 25);
    display.println(F("SENTINEL-X 2099 CYBER"));
    display.display();
  }

  // 1. ஃபில்டர் பஃபரை ஆரம்பத்தில் பூஜ்ஜியமாக்குதல்
  for (int i = 0; i < FILTER_SIZE; i++) {
    gasReadingsBuffer[i] = 0;
  }
  
  // 2. வன்பொருள் ஆன் ஆனதும் தானியங்கி அளவுத்திருத்தத்தை இயக்குதல் (Auto-Calibration)
  executeAutomaticBaselineCalibration();
  
  currentSystemState = STATE_AUTONOMOUS_PATROL;
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastExecutionTimestamp >= executionInterval) {
    lastExecutionTimestamp = currentMillis;
    
    // படி A: சென்சாரில் இருந்து அனலாக் சிக்னல்களைப் பெறுதல்
    acquireRawHardwareSignals();
    
    // படி B: நகரும் சராசரி அல்காரிதம் மூலம் மின்சார இரைச்சலை நீக்குதல் (Signal Smoothing)
    filteredGasOutput = applyMovingAverageFilter(rawGasInput);
    
    // படி C: அளவுத்திருத்த வரம்புகளுடன் தற்போதைய ஃபில்டர் தரவைச் சரிபார்த்தல்
    executeDynamicThresholdAnalysis();
    
    // படி D: சீராக்கப்பட்ட சுத்தமான தரவை வெப் டேஷ்போர்டுக்கு அனுப்புதல்
    emitCalibratedDataPacket();
  }
}

// ================================================================================
// சிக்னல் பிராசஸிங் மற்றும் அல்காரிதம்கள் (SIGNAL PROCESSING METHODS)
// ================================================================================

void executeAutomaticBaselineCalibration() {
  Serial.println(F("[CALIBRATION]: Robot is stationary. Sampling ambient clean air baseline..."));
  
  long baselineAccumulator = 0;
  // 5 விநாடிகளில் 5 முறை காற்றைச் சுவாசித்து அதன் சராசரியைக் கணக்கிடுதல்
  for (int i = 0; i < 5; i++) {
    int sample = random(105, 125); // தொழிற்சாலையின் ஆரம்ப சுத்தமான காற்று வோல்டேஜ்
    baselineAccumulator += sample;
    Serial.print(F("."));
    delay(1000);
  }
  
  cleanAirBaselineR0 = baselineAccumulator / 5; // பேஸ்லைன் R0 லாக் செய்யப்பட்டது
  isCalibrationComplete = true;
  
  Serial.print(F("\n[CALIBRATION SUCCESS]: Environmental Clean Air Baseline Locked at: "));
  Serial.print(cleanAirBaselineR0);
  Serial.println(F(" Analog Units.\n"));
}

void acquireRawHardwareSignals() {
  readTempCelsius = 26.0 + (random(-5, 6) / 10.0);
  
  // சென்சாரில் இருந்து வரும் சிக்னலில் திடீர் வோல்டேஜ் இரைச்சல் (Spikes) ஏற்படுவதை உருவாக்குதல்
  if (random(0, 100) > 85) {
    rawGasInput = 115 + random(-10, 11) + 25; // இரைச்சலால் திடீரென மாறும் வோல்டேஜ்
  } else {
    rawGasInput = 115 + random(-5, 6);
  }

  // டெஸ்டிங்கிற்காக 8% வாய்ப்பில் ஒரு நிஜ வாயு கசிவு விபத்தை உருவாக்குதல்
  if (random(0, 100) > 92) {
    rawGasInput = 450; // விபத்து சிக்னல் வோல்டேஜ்
    Serial.println(F("\n[REAL HAZARD DETECTED]: Toxic gas concentration crossing safety boundaries!"));
  }
  
  // பேட்டரி சிமுலேஷன்
  if (currentSystemState == STATE_AUTONOMOUS_PATROL && batteryCapacityPercent > 0) {
    static int drainCounter = 0;
    if (++drainCounter >= 8) {
      batteryCapacityPercent--;
      drainCounter = 0;
    }
  }
}

int applyMovingAverageFilter(int rawInput) {
  /*
   * நகரும் சராசரி ஃபில்டர் அல்காரிதம் (Moving Average Filter).
   * இது புதிய சிக்னலை உள்ளே வாங்கி, பஃபரில் உள்ள பழைய 5 எண்களின் 
   * சராசரியைக் கணக்கிட்டு மின்சார இரைச்சலை (Jitter) முற்றிலும் நீக்கிவிடும்.
   */
  gasReadingsBuffer[bufferWritePointer] = rawInput;
  bufferWritePointer = (bufferWritePointer + 1) % FILTER_SIZE; // பஃபர் சுழற்சி
  
  long sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    sum += gasReadingsBuffer[i];
  }
  
  return sum / FILTER_SIZE; // சீராக்கப்பட்ட இறுதி மதிப்பு
}

void executeDynamicThresholdAnalysis() {
  // தற்போதைய சீராக்கப்பட்ட கேஸ் மதிப்பு, அளவுத்திருத்த பேஸ்லைனை விட (Baseline + Delta) தாண்டினால்
  if (filteredGasOutput >= (cleanAirBaselineR0 + GAS_HAZARD_DELTA)) {
    currentSystemState = STATE_HAZARD_RESPONSE;
  } 
  else if (batteryCapacityPercent <= 20) {
    currentSystemState = STATE_AUTO_DOCKING;
    batteryCapacityPercent = 100; // சார்ஜ் ஏறுதல் சிமுலேஷன்
  } 
  else {
    currentSystemState = STATE_AUTONOMOUS_PATROL;
  }
}

void emitCalibratedDataPacket() {
  JsonDocument doc;
  
  String stringState = "PATROL";
  if (currentSystemState == STATE_HAZARD_RESPONSE) stringState = "HAZARD_CRIT_ALERT";
  if (currentSystemState == STATE_AUTO_DOCKING) stringState = "CHARGING_AT_BASE";

  doc["device"] = "SENTINEL-X";
  doc["state"] = stringState;
  doc["temp"] = readTempCelsius;
  doc["raw_gas"] = rawGasInput;       // இரைச்சலுடன் கூடிய அனலாக் சிக்னல்
  doc["filtered_gas"] = filteredGasOutput; // ஃபில்டர் செய்யப்பட்ட தூய்மையான சிக்னல்
  doc["baseline_r0"] = cleanAirBaselineR0;
  doc["battery"] = batteryCapacityPercent;

  String outputStream;
  serializeJson(doc, outputStream);
  Serial.println(outputStream); // இந்த சுத்தமான தரவு மெம்பர் 1-ன் வெப் வரைபடத்தை துல்லியமாக இயக்கும்
}
void refreshOnboardOLEDInterface() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("====================="));
  display.setCursor(10, 8);
  display.println(F("SENTINEL-X // 2099"));
  display.setCursor(0, 16);
  display.println(F("====================="));

  // தற்போதைய நிலையைத் திரையில் காட்டுதல்
  display.setCursor(0, 30);
  display.print(F("STATE: "));
  display.println(currentSystemState == STATE_HAZARD_RESPONSE ? "⚠️ CRIT ALERT" : "ACTIVE PATROL");

  // சென்சார் அளவீடுகள்
  display.setCursor(0, 46);
  display.print(F("T: ")); display.print(readTempCelsius, 1); display.print(F("C | G: ")); display.print(readGasPPM);
  display.setCursor(0, 56);
  display.print(F("BATTERY POWER: ")); display.print(batteryCapacityPercent); display.println(F("%"));
  display.display();
}
void executeDFPlayerHardwareTrigger(int assignedTrackIndex) {
  /*
   * Transmits a standardized 9-byte hex frame down the software serial bus
   * to command the DFPlayer Mini to execute specific file index registers.
   */
  byte commandPacketFrame[9];
  
  // Clone the base operational hardware template package array
  memcpy(commandPacketFrame, DFPLAYER_PLAY_TRACK_CMD, 9);
  
  // Inject the dynamically requested track integer value straight into byte slot 6
  commandPacketFrame[6] = (byte)assignedTrackIndex;
  
  // Recalculate checksum logic corrections dynamically for frame delivery accuracy
  int frameCheckSumCalculated = - (0xFF + 0x06 + 0x03 + 0x00 + 0x00 + assignedTrackIndex);
  commandPacketFrame[7] = (byte)(frameCheckSumCalculated >> 8);   // High byte correction
  commandPacketFrame[8] = (byte)(frameCheckSumCalculated & 0xFF);  // Low byte correction
  
  // Flush compiled hex frame array packets directly into the TX pipeline lane
  audioSerialBus.write(commandPacketFrame, 9);
}
