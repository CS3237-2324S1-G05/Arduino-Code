const char* ssid = "WIFINAME";
const char* pass = "WIFIPASS";
const char* server = "mqtt://<IP>:1883";

// CONFIGURATION
#ifdef ENTRANCE
const String strHostname = "ESPCAM-ENTRANCE1";
const String gantryCameraTopic = "trigger/camera/gantry/entrance";
const String strApiRoute = "backend/carplate-recognition/entrance";
const String strLotId = "1";
#elif defined(EXIT)
const String strHostname = "ESPCAM-EXIT1";
const String gantryCameraTopic = "trigger/camera/gantry/exit";
const String strApiRoute = "backend/carplate-recognition/exit";
const String strLotId = "1";
#elif defined(LOT1)
const String strHostname = "ESPCAM-LOT1";
const String gantryCameraTopic = "trigger/camera/lot/1";
const String strApiRoute = "/backend/carplate-recognition/lot";
const String strLotId = "1";
#elif defined(HUMAN)
const String strHostname = "ESPCAM-HUMAN1";
const String gantryCameraTopic = "trigger/camera/carpark";
const String strApiRoute = "backend/human-recognition";
const String strLotId = "1";
#endif

// Venus Computer
const char* backendHost = "192.168.43.186";
const int backendPort = 7000;

// Set to 0 if u dont want the flash to fire while taking a photo
// max is 255
const int intFlashPower = 50;