# ESP32 and SIM7600 4G Module Project

This project demonstrates using an ESP32 and SIM7600 4G module to connect to a GPRS network and interact with a Firebase Realtime Database. 

## Features
- Connects ESP32 to the internet using the SIM7600 4G module.
- Utilizes Firebase for cloud storage and data management.
- Supports asynchronous data communication.
- Sends hardcoded sensor data (temperature and humidity) to Firebase.

## Requirements
- **ESP32** development board
- **SIM7600 4G module**
- **Arduino IDE** with the following libraries installed:
  - [TinyGsm](https://github.com/vshymanskyy/TinyGSM)
  - [FirebaseClient](https://github.com/mobizt/ESP_SSLClient)
  - [ESP_SSLClient](https://github.com/mobizt/ESP_SSLClient)

## Pin Configuration
| ESP32 GPIO | SIM7600 Pin |
|------------|-------------|
| GPIO16     | TXD         |
| GPIO17     | RXD         |
| GPIO5      | PWRKEY      |
| GPIO4      | RESET       |

## Setup Instructions

1. **Hardware Setup:**
   - Connect the ESP32 to the SIM7600 module using the pin configuration mentioned above.
   - Power the SIM7600 module and ensure it has a working SIM card with internet access.

2. **Software Setup:**
   - Install the Arduino IDE if not already installed.
   - Add the ESP32 board to the Arduino IDE.
   - Install the required libraries listed in the Requirements section.
   - Open the project code in the Arduino IDE.

3. **Configuration:**
   - Update the APN, API Key, User Email, User Password, and Firebase Database URL in the code:
     ```cpp
     const char apn[] = "your_apn";
     #define API_KEY "your_api_key"
     #define USER_EMAIL "your_email"
     #define USER_PASSWORD "your_password"
     #define DATABASE_URL "your_firebase_database_url"
     ```

4. **Upload the Code:**
   - Connect the ESP32 to your computer via USB.
   - Select the appropriate board and port from the Tools menu.
   - Upload the code to the ESP32.

5. **Monitor the Serial Output:**
   - Open the Serial Monitor (set the baud rate to 115200).
   - The Serial Monitor will display logs about the modem initialization, network connection status, and Firebase interactions.

## Notes
- Make sure your SIM card has internet access and the correct APN settings.
- Firebase authentication is required for the project to interact with the database.


