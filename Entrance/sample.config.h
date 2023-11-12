// WiFI Configuration
const char* ssid = "WIFINAME";
const char* pass = "WIFIPASS";
const char* server = "mqtt://<IP>:1883";

// MQTT Configuration
const String strHostname = "ESP-ENTRANCE1";
const String entranceAliveTopic = "status/gantry/entrance";
const String entranceEventTopic = "event/gantry/entrance";
const String entranceHumanStatusTopic = "status/entrance/human-presence";
const String entranceCarplateTopic = "status/entrance/carplate";
const String entranceNearestLotTopic = "status/entrance/nearest-lot";
const String entranceNumLotsTopic = "status/entrance/number-of-available-lots";

// Fine-tune
const float fltDetectionDistanceCm = 5; // in cm
const int intCarDetectionTime = 3000; // in ms
const int intGantryTimeOut = 30000; // in ms