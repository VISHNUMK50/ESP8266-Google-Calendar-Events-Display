# ESP8266 Google Calendar Events Display

This project displays the current time, date, and upcoming Google Calendar events of the day on an SSD1306 OLED display using an ESP8266. The display scrolls through events fetched from Google Calendar, showing the event title and time in a clear and readable format.

## Features

- **Wi-Fi Connectivity:** Connects to a specified Wi-Fi network.
- **NTP Time Synchronization:** Automatically syncs time with an NTP server.
- **Google Calendar Integration:** Fetches upcoming events from a Google Calendar.
- **Scrolling Text Display:** Smoothly scrolls through events on the OLED screen.
- **Automatic Updates:** Refreshes the events list every minute.

## Hardware Requirements

- **ESP8266 (e.g., NodeMCU, Wemos D1 Mini):** The microcontroller used to run the code and manage the Wi-Fi connection.
- **SSD1306 128x64 OLED Display:** The screen used to display the time, date, and events.
- **Breadboard and Jumper Wires:** For making connections between the ESP8266 and the OLED display.

## Libraries Used

The following libraries are required to run this project:

- [ESP8266WiFi](https://github.com/esp8266/Arduino): To handle Wi-Fi connectivity.
- [ESP8266HTTPClient](https://github.com/esp8266/Arduino): For making HTTP requests.
- [WiFiClientSecure](https://github.com/esp8266/Arduino): To handle secure (HTTPS) connections.
- [ArduinoJson](https://arduinojson.org/): To parse JSON responses from Google Calendar API.
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library): For handling graphics on the OLED display.
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306): To interface with the SSD1306 OLED display.

You can install these libraries using the Arduino Library Manager or by downloading them from GitHub.

## Circuit Diagram

Here’s how you can connect the components:

- **OLED VCC** to **3.3V** on ESP8266
- **OLED GND** to **GND** on ESP8266
- **OLED SCL** to **D1** on ESP8266 (GPIO 5)
- **OLED SDA** to **D2** on ESP8266 (GPIO 4)

## Software Setup

1. **Clone or Download the Repository:**

   ```bash
   git clone https://github.com/your-username/esp8266-google-calendar-display.git
   ```
2. **Configure Wi-Fi Credentials:**
  In the .ino file, replace the placeholders with your Wi-Fi SSID and password:
    
    ```C++
    const char* ssid = "Your_SSID";
    const char* password = "Your_Password";
    ```
3. **Google Calendar API Setup:**

    - Create a project on the Google Cloud Console.
    - Enable the Google Calendar API.
    - Generate an API key and replace the placeholder in the code:
    
    ```C++
    String calendarUrl = "https://www.googleapis.com/calendar/v3/calendars/your-calendar-id/events?timeMin=" + currentTime + "&timeMax=" + getEndOfDayTimeRFC3339() + "&orderBy=startTime&singleEvents=true&key=YOUR_API_KEY";
    ```
    - Replace **"YOUR_API_KEY"** with your API KEY.
    - Replace **"your-calendar-id"** with your calendar id.
5. **Upload the Code:**

    - Open the .ino file in the Arduino IDE.
    - Select the correct board (NodeMCU 1.0 or Wemos D1 Mini).
    - Upload the code to your ESP8266.
  
6. **Monitor the Serial Output:**

   - Open the Serial Monitor in Arduino IDE (baud rate: 115200) to check connection status and debug messages.
# Usage
  - Once powered on, the ESP8266 will connect to the specified Wi-Fi network, synchronize the time using NTP, and then fetch upcoming events from your Google Calendar. The events will be displayed on the OLED screen, scrolling from right to left.

# Troubleshooting
  - Connection Issues: Ensure that the SSID and password are correct and that the ESP8266 is within range of the Wi-Fi network.
  - Display Not Working: Double-check the wiring between the ESP8266 and the OLED display.
  - No Events Displayed: Ensure that the Google Calendar API key is valid and that there are events scheduled in your calendar.
# License
  - This project is licensed under the MIT License - see the LICENSE file for details.
