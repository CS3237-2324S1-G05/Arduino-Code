const char* ssid = "WIFINAME";
const char* pass = "WIFIPASS";
const char* server = "mqtt://<IP>:1883";

const float fltDetectionDistanceCm = 10;
const String strHostname = "ESP-EXIT1";
const String exitAliveTopic = "status/gantry/exit";
const String exitEventTopic = "event/gantry/exit";
const String exitCarplateTopic = "status/exit/carplate";
const String exitParkingFeeTopic = "status/exit/parking-fee";
const String exitNumLotsTopic = "status/exit/number-of-available-lots";
const int intCarDetectionTime = 5000;
const int intGantryTimeOut = 30000;