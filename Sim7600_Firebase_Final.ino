#define TINY_GSM_MODEM_SIM7600
#define SerialMon Serial  
#define SerialAT Serial2 
#define TINY_GSM_DEBUG SerialMon
#define GSM_PIN "" 

// APN configuration for GPRS connection
const char apn[] = "YOUR_APN"; // SIM APN
const char gprsUser[] = "";   // GPRS username (if needed)
const char gprsPass[] = "";   // GPRS password (if needed)

// Time-related definitions
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for microseconds to seconds
#define TIME_TO_SLEEP 600          // Deep sleep duration (in seconds)

// Pin configuration for the modem
#define MODEM_TX 16  
#define MODEM_RX 17  
#define PKEY 5       
#define RST 4       

#include <Arduino.h>
#include <TinyGsmClient.h>
#include <FirebaseClient.h>
#include <ESP_SSLClient.h>

// Firebase configuration
#define API_KEY "YOUR_API_KEY_HERE"              // Firebase API key
#define USER_EMAIL "YOUR_EMAIL_HERE"             // Firebase user email
#define USER_PASSWORD "YOUR_PASSWORD_HERE"       // Firebase user password
#define DATABASE_URL "YOUR_DATABASE_URL_HERE"    // Firebase Realtime Database URL

// Modem and client configuration
TinyGsm modem(SerialAT);
TinyGsmClient gsm_client1(modem, 0);
TinyGsmClient gsm_client2(modem, 1);

ESP_SSLClient ssl_client1, ssl_client2;
GSMNetwork gsm_network(&modem, GSM_PIN, apn, gprsUser, gprsPass);
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;

// Async client setup for Firebase communication
using AsyncClient = AsyncClientClass;
AsyncClient aClient1(ssl_client1, getNetwork(gsm_network)), aClient2(ssl_client2, getNetwork(gsm_network));

// Function prototypes
void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);

// Firebase Realtime Database instance
RealtimeDatabase Database;
unsigned long ms = 0;  // Timer variable for periodic tasks

void setup() {
  SerialMon.begin(115200);  // Start the serial monitor
  delay(10);
  SerialMon.println("Wait ...");

  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);  // Initialize modem serial interface
  delay(3000);
  SerialMon.println("Initializing modem ...");
  modem.restart();  // Restart the modem

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // Check SIM status and unlock if necessary
  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }

  // Wait for network connection
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  // Connect to GPRS
  SerialMon.print("Connecting to APN: ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    ESP.restart();  // Restart on failure
  }
  SerialMon.println(" OK");
  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
  delay(100);

  // Firebase client setup
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  // SSL client configuration for secure communication
  ssl_client1.setInsecure();
  ssl_client1.setDebugLevel(1);
  ssl_client1.setBufferSizes(2048, 1024);
  ssl_client1.setClient(&gsm_client1);

  ssl_client2.setInsecure();
  ssl_client2.setDebugLevel(1);
  ssl_client2.setBufferSizes(2048, 1024);
  ssl_client2.setClient(&gsm_client2);

  // Initialize Firebase app
  Serial.println("Initializing app...");
  initializeApp(aClient1, app, getAuth(user_auth), asyncCB, "authTask");
  app.getApp<RealtimeDatabase>(Database);  // Retrieve Realtime Database instance
  Database.url(DATABASE_URL);
  Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");  // Set database event filters
}

void loop() {
  app.loop();  // Run Firebase app loop
  Database.loop();  // Run database loop

  // Send data every 20 seconds if the app is ready
  if (millis() - ms > 20000 && app.ready()) {
    Serial.println("Sending hardcoded data...");

    const char *temp_str = "28.99";  // Hardcoded temperature data
    const char *humi_str = "79.54";  // Hardcoded humidity data

    Serial.print("Temperature = ");
    Serial.println(temp_str);
    Serial.print("Humidity = ");
    Serial.println(humi_str);

    ms = millis();  // Update timestamp for the next iteration
    JsonWriter writer;
    object_t json, obj1, obj2;
    writer.create(obj1, "temp", temp_str);
    writer.create(obj2, "humi", humi_str);
    writer.join(json, 2, obj1, obj2);
    Database.set<object_t>(aClient1, "/sensor-data/", json, asyncCB, "setTask");  // Send data to Firebase
  }
}

void asyncCB(AsyncResult &aResult) {
  printResult(aResult);  // Callback function for asynchronous tasks
}

void printResult(AsyncResult &aResult) {
  if (aResult.isEvent()) {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug()) {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available()) {
    RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
    if (RTDB.isStream()) {
      Serial.println("----------------------------");
      Firebase.printf("task: %s\n", aResult.uid().c_str());
      Firebase.printf("event: %s\n", RTDB.event().c_str());
      Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
      Firebase.printf("data: %s\n", RTDB.to<const char *>());
      Firebase.printf("type: %d\n", RTDB.type());
    } else {
      Serial.println("----------------------------");
      Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
    Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
  }
}
