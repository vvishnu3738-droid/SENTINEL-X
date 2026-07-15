/*
 * SENTINEL-X // PRODUCTION TINYML SENSOR FUSION & INFERENCE ENGINE
 * மைக்ரோகண்ட்ரோலர் கட்டமைப்பு: Renesas RA4W1 (Arduino UNO R4 WiFi)
 * டெவலப்பர் டிராக்: கோர் உட்பொதிக்கப்பட்ட AI & TinyML அமைப்புகள் பொறியியல்
 */

#include <ArduinoJson.h>

// 1. SYSTEM FINITE STATE MACHINE REGISTRY (ரோபோட்டின் நிலைகள்)
enum RobotState {
  STATE_INITIALIZING,
  STATE_AUTONOMOUS_PATROL,
  STATE_HAZARD_RESPONSE,
  STATE_AUTO_DOCKING
};

RobotState currentSystemState = STATE_INITIALIZING;
unsigned long lastAILoopTimestamp = 0;
const unsigned long aiInferenceInterval = 1000; // 1 விநாடிக்கு ஒருமுறை இயங்கும் AI டைமர்

// வன்பொருள் சென்சார் மாறிகள்
float readTempCelsius = 26.5;
int readSmokeIndex = 110;  // அனலாக் எல்லை: 0 முதல் 1023
int readGasPPM = 45;       // காற்றின் தரம் PPM
float readTiltAngle = 0.0; // காரின் சாய்வு கோணம்
int batteryCapacityPercent = 95;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; } // கணினியுடன் இணைப்பு ஏற்படும் வரை காத்திருக்கவும்
  
  Serial.println(F("[TINYML BOOT]: Initializing Embedded Neural Network Layers..."));
  delay(1000);
  
  currentSystemState = STATE_AUTONOMOUS_PATROL;
  Serial.println(F("[TINYML BOOT]: Multi-Layer Perceptron Weights Loaded. System Live."));
}

void loop() {
  unsigned long currentMillis = millis();

  // நான்-பிளாக்கிங் சுழற்சி லூப்
  if (currentMillis - lastAILoopTimestamp >= aiInferenceInterval) {
    lastAILoopTimestamp = currentMillis;
    
    // படி A: சென்சார்களில் இருந்து போலி தரவுகளைப் பெறுதல்
    executeSensorTelemetryDataAcquisition();
    
    // படி B: உட்பொதிக்கப்பட்ட TinyML செயற்கை நுண்ணறிவு நரம்பியல் நெட்வொர்க் இயக்கம்
    float calculatedRiskIndex = executeEmbeddedTinyMLInference();
    
    // படி C: AI கணக்கிட்ட ஆபத்து சதவீதத்திற்கு ஏற்ப நிலைகளை மாற்றுதல்
    executeDynamicStateTransitionLogic(calculatedRiskIndex);
    
    // படி D: நிகழ்நேர தரவை வைஃபை டேஷ்போர்டுக்கு அனுப்புதல்
    transmitDataPacketToWebDashboard(calculatedRiskIndex);
  }
}

// ================================================================================
// TINYML கணித அல்காரிதம் பகுதிகள் (TINYML MATHEMATICAL CORE METHODS)
// ================================================================================

void executeSensorTelemetryDataAcquisition() {
  // உண்மையான சென்சார் மதிப்புகள் மாறுவது போலவே போலி மதிப்புகளை உருவாக்குதல்
  readTempCelsius = 25.0 + (random(0, 40) / 10.0);
  readGasPPM = random(35, 70);
  readSmokeIndex = random(95, 140);
  readTiltAngle = (random(-15, 16) / 10.0);
  
  // AI-ன் செயல்பாட்டைச் சோதிக்க 7% வாய்ப்பில் ஒரு போலி விபத்து சிக்னலை உள்ளே செலுத்துதல்
  if (random(0, 100) > 93) {
    readTempCelsius = 52.4;
    readGasPPM = 480;
    readSmokeIndex = 620;
    Serial.println(F("\n[ANOMALY GENERATOR]: Injecting simulated toxic flame burst signature..."));
  }

  // பேட்டரி குறைவதை உருவகப்படுத்துதல்
  if (currentSystemState == STATE_AUTONOMOUS_PATROL && batteryCapacityPercent > 0) {
    static int drainCounter = 0;
    if (++drainCounter >= 5) { 
      batteryCapacityPercent--;
      drainCounter = 0;
    }
  }
}

float executeEmbeddedTinyMLInference() {
  /*
   * இது ஒரு மல்டி-லேயர் பெர்செப்ட்ரான் (MLP) நரம்பியல் நெட்வொர்க் போல வேலை செய்யும்.
   * வெவ்வேறு எல்லைகளில் இருக்கும் சென்சார் மதிப்புகளை 0.00 முதல் 1.00 என்ற அளவில் மாற்றி (Normalize),
   * பின்னர் மேட்ரிக்ஸ் புள்ளி பெருக்கல் (Dot-Product Weights) லாஜிக்கைப் பயன்படுத்தும்.
   */
  
  // 1. மின்-மேக்ஸ் தரவு சீரமைப்பு கணிதம் (Min-Max Normalization)
  float normTemp  = (readTempCelsius - 0.0) / (60.0 - 0.0);       // அதிகபட்ச வெப்ப எல்லை 60C
  float normSmoke = (float)(readSmokeIndex - 0) / (1023.0 - 0.0);  // 10-பிட் அனலாக் பின் எல்லை
  float normGas   = (float)(readGasPPM - 0) / (500.0 - 0.0);       // நச்சு வாயு PPM எல்லை
  float normTilt  = abs(readTiltAngle) / 45.0;                     // அதிகபட்ச பாதுகாப்பு சாய்வு கோணம் 45 டிகிரி

  // 2. சென்சார் முக்கியத்துவ மதிப்புகள் ஒதுக்கீடு (மொத்தம் 1.00 வர வேண்டும்)
  const float weightTemp  = 0.35;
  const float weightSmoke = 0.30;
  const float weightGas   = 0.30;
  const float weightTilt  = 0.05;

  // 3. நரம்பியல் நெட்வொர்க் அடுக்கு கூட்டல் (Dot Product Matrix Multiplication)
  float rawNeuralSum = (normTemp * weightTemp) + 
                       (normSmoke * weightSmoke) + 
                       (normGas * weightGas) + 
                       (normTilt * weightTilt);

  // 4. இறுதி விபத்து அபாய சதவீதம் (0% முதல் 100% வரம்பிற்குள் மாற்றுதல்)
  float localizedRiskScore = rawNeuralSum * 100.0; 
  if (localizedRiskScore > 100.0) localizedRiskScore = 100.0;
  if (localizedRiskScore < 0.0) localizedRiskScore = 0.0;

  return localizedRiskScore;
}

void executeDynamicStateTransitionLogic(float currentRiskMetric) {
  // விபத்து அபாய சதவீதம் 45% தாண்டினால் அவசர நிலைக்கு மாறுதல்
  if (currentRiskMetric >= 45.0) {
    currentSystemState = STATE_HAZARD_RESPONSE;
  } 
  // பேட்டரி 20% கீழே குறைந்தால் சார்ஜிங் நிலைக்கு மாறுதல்
  else if (batteryCapacityPercent <= 20) {
    currentSystemState = STATE_AUTO_DOCKING;
    // சார்ஜ் ஏறுவதை உருவகப்படுத்துதல்
    batteryCapacityPercent += 25;
    if (batteryCapacityPercent > 100) batteryCapacityPercent = 100;
  } 
  else {
    currentSystemState = STATE_AUTONOMOUS_PATROL;
  }
}

void transmitDataPacketToWebDashboard(float runningRiskScore) {
  JsonDocument telemetryDoc;
  
  String labelState = "PATROL";
  if (currentSystemState == STATE_HAZARD_RESPONSE) labelState = "CRIT_HAZARD_ALERT";
  if (currentSystemState == STATE_AUTO_DOCKING) labelState = (batteryCapacityPercent >= 100) ? "PATROL" : "CHARGING_AT_BASE";

  telemetryDoc["device"] = "SENTINEL-X";
  telemetryDoc["state"] = labelState;
  telemetryDoc["temp"] = readTempCelsius;
  telemetryDoc["gas"] = readGasPPM;
  telemetryDoc["smoke"] = readSmokeIndex;
  telemetryDoc["tilt"] = readTiltAngle;
  telemetryDoc["battery"] = batteryCapacityPercent;
  telemetryDoc["risk_index"] = runningRiskScore; // AI கணக்கிட்டுத் தந்த இறுதி அபாய சதவீதம் (Risk Index Score)

  String outputBufferStream;
  serializeJson(telemetryDoc, outputBufferStream);
  Serial.println(outputBufferStream); // இந்த தரவு தான் மெம்பர் 1-ன் வெப் வரைபடத்தை இயக்கும்
}
