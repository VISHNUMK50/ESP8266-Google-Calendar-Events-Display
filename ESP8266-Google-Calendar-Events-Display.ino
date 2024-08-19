#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

// Define OLED display dimensions and reset pin
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Wi-Fi credentials
const char* ssid = "Rotech_2.4G";
const char* password = "rotech@AI";

// NTP server for time synchronization
const char* ntpServer = "pool.ntp.org";

// Time zone settings for IST (UTC + 5:30)
const long gmtOffset_sec = 5 * 3600 + 1800;  // IST is UTC + 5:30
const int daylightOffset_sec = 0;           // No daylight saving in India

WiFiClientSecure client;  // Secure WiFi client for HTTPS requests
String eventsText = "";   // Stores the concatenated event details
int scrollIndex = 0;      // Scroll index for the events display

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  // Halt the program if display initialization fails
  }
  
  // Set display rotation and clear any previous content
  display.setRotation(2);
  display.clearDisplay();

  // Display a message indicating Wi-Fi connection status
  displayTimeAndMessage("Connecting to WiFi...");

  // Connect to the Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");

    // Update display while waiting for Wi-Fi connection
    displayTimeAndMessage("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");

  // Initialize and synchronize time using NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Fetch and display Google Calendar events
  updateCalendarEvents();
}

// Function to display the current time and a message on the OLED
void displayTimeAndMessage(String message) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    display.clearDisplay();

    // Display current time in HH:MM AM/PM format
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 15);
    char timeStr[16];
    strftime(timeStr, sizeof(timeStr), "%I:%M %p", &timeinfo);  // 12-hour format with AM/PM
    display.print(timeStr);

    // Display current date in YYYY-MM-DD format
    display.setTextSize(1);
    display.setCursor(30, 35);
    char dateStr[16];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    display.print(dateStr);

    // Set text size to 2 for the events text and start scrolling
    display.setTextSize(2);
    int16_t x = SCREEN_WIDTH - scrollIndex; // Start just off the right side of the screen

    // Render each character manually and ensure no wrapping occurs
    for (int i = 0; i < eventsText.length(); i++) {
      if (x >= 0 && x <= SCREEN_WIDTH - 12) {  // Ensure the character is within the screen bounds
        display.setCursor(x, 45);
        display.print(eventsText[i]);
      }
      x += 12;  // Move cursor by the width of one character (12 pixels for text size 2)
    }

    // Scroll the text smoothly
    scrollIndex++;
    if (scrollIndex > eventsText.length() * 12) {
      scrollIndex = 0;  // Reset scrolling after the entire text has scrolled off the screen
    }

    // Update the display with new content
    display.display();
  } else {
    Serial.println("Failed to obtain time");
  }
}

// Function to fetch and update Google Calendar events
void updateCalendarEvents() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Get current time in RFC3339 format for the API request
    String currentTime = getCurrentTimeRFC3339();
    Serial.println("Current time in RFC3339 format: " + currentTime);

    // Construct the calendar URL with dynamic timeMin and timeMax
    String calendarUrl = "https://www.googleapis.com/calendar/v3/calendars/rotechedukerala@gmail.com/events?timeMin=" + currentTime + "&timeMax=" + getEndOfDayTimeRFC3339() + "&orderBy=startTime&singleEvents=true&key=AIzaSyDMcqZqkTAXMOlMM1WfnDtQfaNoNflwSbs";

    Serial.println(calendarUrl);

    client.setInsecure();  // Optionally bypass certificate validation for testing
    http.begin(client, calendarUrl);  // Use secure client for HTTPS
    int httpCode = http.GET();  // Send the GET request
    
    Serial.println("updateCalendarEvents executed");
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    // Check if the HTTP request was successful
    if (httpCode > 0) {
      String payload = http.getString();  // Get the response payload
      Serial.println("Payload received:");
      Serial.println(payload);

      // Check if the payload is empty
      if (payload.length() == 0) {
        Serial.println("Received empty payload!");
        return;
      }

      // Parse JSON response from Google Calendar API
      DynamicJsonDocument doc(4096);  // Increase buffer size if needed
      DeserializationError error = deserializeJson(doc, payload);
      
      // Check for errors in JSON parsing
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        displayTimeAndMessage(error.f_str());

        return;
      }

      // Prepare the events text for display
      eventsText = "";
      int eventCount = 1;
      for (JsonObject event : doc["items"].as<JsonArray>()) {
        String summary = event["summary"].as<String>();  // Get event title
        String startTime = extractTimeOnly(event["start"]["dateTime"].as<String>());  // Get event start time
        eventsText += "(" + String(eventCount) + ") " + summary + " at " + startTime + "  ";
        eventCount++;
      }
      if (eventsText == "") {
        eventsText = "No upcoming events";  // Display message if no events are found
      }

      scrollIndex = 0; // Reset scroll index when new events are fetched
    } else {
      // Print HTTP request error
      Serial.print("HTTP request failed, error: ");
      Serial.println(http.errorToString(httpCode).c_str());
    }
    
    http.end();  // End the HTTP request
  } else {
    Serial.println("WiFi Disconnected");
  }
}

// Function to get current time in RFC3339 format (UTC)
String getCurrentTimeRFC3339() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }

  // Convert local time (IST) to UTC
  timeinfo.tm_hour -= 5;   // IST is UTC + 5:30
  timeinfo.tm_min -= 30;

  // Normalize the time (adjust if minutes or hours are negative)
  if (timeinfo.tm_min < 0) {
    timeinfo.tm_min += 60;
    timeinfo.tm_hour -= 1;
  }
  if (timeinfo.tm_hour < 0) {
    timeinfo.tm_hour += 24;
    timeinfo.tm_mday -= 1; // Adjust day if hours wrap around
  }

  // Format time in RFC3339 format
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buffer);
}

// Function to get end of day time in RFC3339 format (UTC)
String getEndOfDayTimeRFC3339() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }

  // Set time to end of the day (23:59:59)
  timeinfo.tm_hour = 23;
  timeinfo.tm_min = 59;
  timeinfo.tm_sec = 59;

  // Convert local time (IST) to UTC
  timeinfo.tm_hour -= 5;   // IST is UTC + 5:30
  timeinfo.tm_min -= 30;

  // Normalize the time
  if (timeinfo.tm_min < 0) {
    timeinfo.tm_min += 60;
    timeinfo.tm_hour -= 1;
  }
  if (timeinfo.tm_hour < 0) {
    timeinfo.tm_hour += 24;
    timeinfo.tm_mday -= 1; // Adjust day if hours wrap around
  }

  // Format time in RFC3339 format
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buffer);
}

// Function to extract time in HH:MM AM/PM format from a full datetime string
String extractTimeOnly(const String& dateTime) {
  Serial.print("Raw dateTime: ");
  Serial.println(dateTime);

  int start = dateTime.indexOf('T') + 1;  // Start after 'T'
  int end = dateTime.indexOf('+');        // End before '+'
  if (start != -1 && end != -1 && start < end) {
    String timePart = dateTime.substring(start, end);
    int colonIndex = timePart.indexOf(':');
    if (colonIndex != -1) {
      String hoursMinutes = timePart.substring(0, colonIndex + 3); // Extract hours and minutes (HH:MM)
      Serial.print("Extracted time (HH:MM): ");
      Serial.println(hoursMinutes);
      
      // Convert to 12-hour format with AM/PM
      int hours = hoursMinutes.substring(0, 2).toInt();
      String ampm = (hours >= 12) ? "PM" : "AM";
      if (hours > 12) hours -= 12;
      if (hours == 0) hours = 12;
      return String(hours) + ":" + hoursMinutes.substring(3, 5) + " " + ampm;
    }
  }

  Serial.println("Failed to extract time, invalid format.");
  return "Invalid time";
}

void loop() {
  // Display the scrolling events text
  displayTimeAndMessage(eventsText);

  // Update events every minute
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 60000) {  // 60,000 milliseconds = 1 minute
    updateCalendarEvents();
    lastUpdate = millis();
  }
}
