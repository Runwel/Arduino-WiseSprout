#include <WiFi.h>
#include <WiFiSSLClient.h> // For HTTPS
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.google.com", 8 * 3600, 60000); // UTC+8 for Philippine Time

// WiFi credentials and server URLs
const char* ssid = "ZTE_2.4G_zqY3JX"; // Replace with your WiFi SSID
const char* password = "chikoyuni123"; // Replace with your WiFi Password

const char* serverHost = "www.fjworkforce.com";
const int serverPort = 443; // HTTPS uses port 443

// Certificate for fjworkforce.com
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFBTCCAu2gAwIBAgIQS6hSk/eaL6JzBkuoBI110DANBgkqhkiG9w0BAQsFADBP\n" \
"MQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFy\n" \
"Y2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMTAeFw0yNDAzMTMwMDAwMDBa\n" \
"Fw0yNzAzMTIyMzU5NTlaMDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBF\n" \
"bmNyeXB0MQwwCgYDVQQDEwNSMTAwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
"AoIBAQDPV+XmxFQS7bRH/sknWHZGUCiMHT6I3wWd1bUYKb3dtVq/+vbOo76vACFL\n" \
"YlpaPAEvxVgD9on/jhFD68G14BQHlo9vH9fnuoE5CXVlt8KvGFs3Jijno/QHK20a\n" \
"/6tYvJWuQP/py1fEtVt/eA0YYbwX51TGu0mRzW4Y0YCF7qZlNrx06rxQTOr8IfM4\n" \
"FpOUurDTazgGzRYSespSdcitdrLCnF2YRVxvYXvGLe48E1KGAdlX5jgc3421H5KR\n" \
"mudKHMxFqHJV8LDmowfs/acbZp4/SItxhHFYyTr6717yW0QrPHTnj7JHwQdqzZq3\n" \
"DZb3EoEmUVQK7GH29/Xi8orIlQ2NAgMBAAGjgfgwgfUwDgYDVR0PAQH/BAQDAgGG\n" \
"MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/\n" \
"AgEAMB0GA1UdDgQWBBS7vMNHpeS8qcbDpHIMEI2iNeHI6DAfBgNVHSMEGDAWgBR5\n" \
"tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKG\n" \
"Fmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYD\n" \
"VR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0B\n" \
"AQsFAAOCAgEAkrHnQTfreZ2B5s3iJeE6IOmQRJWjgVzPw139vaBw1bGWKCIL0vIo\n" \
"zwzn1OZDjCQiHcFCktEJr59L9MhwTyAWsVrdAfYf+B9haxQnsHKNY67u4s5Lzzfd\n" \
"u6PUzeetUK29v+PsPmI2cJkxp+iN3epi4hKu9ZzUPSwMqtCceb7qPVxEbpYxY1p9\n" \
"1n5PJKBLBX9eb9LU6l8zSxPWV7bK3lG4XaMJgnT9x3ies7msFtpKK5bDtotij/l0\n" \
"GaKeA97pb5uwD9KgWvaFXMIEt8jVTjLEvwRdvCn294GPDF08U8lAkIv7tghluaQh\n" \
"1QnlE4SEN4LOECj8dsIGJXpGUk3aU3KkJz9icKy+aUgA+2cP21uh6NcDIS3XyfaZ\n" \
"QjmDQ993ChII8SXWupQZVBiIpcWO4RqZk3lr7Bz5MUCwzDIA359e57SSq5CCkY0N\n" \
"4B6Vulk7LktfwrdGNVI5BsC9qqxSwSKgRJeZ9wygIaehbHFHFhcBaMDKpiZlBHyz\n" \
"rsnnlFXCb5s8HKn5LsUgGvB24L7sGNZP2CX7dhHov+YhD+jozLW2p9W4959Bz2Ei\n" \
"RmqDtmiXLnzqTpXbI+suyCsohKRg6Un0RC47+cpiVwHiXZAW+cn8eiNIjqbVgXLx\n" \
"KPpdzvvtTnOPlC7SQZSYmdunr3Bf9b77AiC/ZidstK36dRILKz7OA54=\n" \
"-----END CERTIFICATE-----\n";

const int soilSensorPin = A0;
const int relayEnable = 3;
const int thresholdMax = 1020;
const int thresholdMin = 30;
const long interval = 3000; // Check every 10 seconds
unsigned long previousMillis = 0;

// Watering state variables
unsigned long lastWateringTime = 0;
unsigned long wateringTime = 0;
bool isWatering = false;
int currentScheduleId = -1; // To keep track of the current schedule ID

// Create WiFiSSLClient
WiFiSSLClient sslClient;

void setup() {
    Serial.begin(9600);
    pinMode(relayEnable, OUTPUT);
    digitalWrite(relayEnable, HIGH); // Start with relay off

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");

    timeClient.begin();
    timeClient.update();
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        checkSchedule();
        sendSoilData();  // Send soil data every interval
    }

    if (isWatering) {
        monitorWatering();
        checkWateringStatus(); // Check the status of watering if it's ongoing
    }
}

void checkSchedule() {
    timeClient.update();
    String currentTime = timeClient.getFormattedTime();
    String currentDay = getCurrentDayOfWeek(timeClient.getDay());

    if (WiFi.status() == WL_CONNECTED) {
        HttpClient http(sslClient, serverHost, serverPort);  // Using the serverHost and serverPort variables

        http.get("/get-schedule.php?user_id=18");
        int httpResponseCode = http.responseStatusCode();

        if (httpResponseCode == 200) {
            String payload = http.responseBody();
            DynamicJsonDocument doc(2048);
            auto error = deserializeJson(doc, payload);

            if (!error && doc["status"] == "success") {
                JsonArray schedules = doc["schedules"].as<JsonArray>();
                bool scheduleFound = false;
                for (JsonObject schedule : schedules) {
                    int scheduleId = schedule["id"];
                    String startTime = schedule["start_time"].as<String>();
                    int duration = schedule["duration"];
                    String days = schedule["days"].as<String>();

                    // Check if today's date and time match the schedule
                    if (isScheduleTodayAndTime(currentDay, currentTime, days, startTime, duration) && currentScheduleId != scheduleId) {
                        currentScheduleId = scheduleId; // Store the current schedule ID
                        updateScheduleStatus(scheduleId, "Ongoing");
                        startWatering(duration);
                        scheduleFound = true;
                    }
                }
                if (!scheduleFound && !isWatering) {
                    sendSoilData(); // If no schedule found and not watering, send soil data
                }
            }
        }
        http.stop();
    }
}

void sendSoilData() {
    int soilMoisture = analogRead(soilSensorPin);
    String currentTime = timeClient.getFormattedTime();
    int moisturePercentage = map(soilMoisture, thresholdMax, thresholdMin, 0, 100);

    String soilCondition = "Adequate";
    if (moisturePercentage < 30) {
        soilCondition = "Dry";
    } else if (moisturePercentage > 70) {
        soilCondition = "Wet";
    }

    String deviceStatus = "Online";  // Arduino device status
    
    if (WiFi.status() == WL_CONNECTED) {
        HttpClient http(sslClient, serverHost, serverPort);  // Using the serverHost and serverPort variables

        String postData = "moisture_level=" + String(moisturePercentage) + 
                          "&timestamp=" + currentTime + 
                          "&condition=" + soilCondition + 
                          "&status=" + deviceStatus;  // Add status to the POST data
                          
        http.post("/soil-data.php?user_id=18", "application/x-www-form-urlencoded", postData);
        
        int httpResponseCode = http.responseStatusCode();
        if (httpResponseCode == 200) {
            Serial.println("Soil data sent successfully.");
        } else {
            Serial.print("Failed to send soil data. Error code: ");
            Serial.println(httpResponseCode);
        }

        http.stop();
    }
}

void startWatering(int duration) {
    isWatering = true;
    wateringTime = duration * 1000;  // Convert duration to milliseconds
    lastWateringTime = millis();  // Store the start time of watering
    digitalWrite(relayEnable, LOW); // Turn on the relay
    Serial.println("Relay ON - Watering started.");
}

void monitorWatering() {
    // Monitor if the watering duration has passed
    if (millis() - lastWateringTime >= wateringTime) {
        stopWatering();
    }
}

void stopWatering() {
    digitalWrite(relayEnable, HIGH); // Turn off the relay
    Serial.println("Relay OFF - Watering done.");
    isWatering = false;  // Stop the watering process
    // Update the schedule status to "Ended" after watering completes
    updateScheduleStatus(currentScheduleId, "Ended");
    currentScheduleId = -1; // Reset the current schedule ID after watering
}

void checkWateringStatus() {
    // Check for "Stopped" status from the server
    if (WiFi.status() == WL_CONNECTED && currentScheduleId != -1) {
        HttpClient http(sslClient, serverHost, serverPort);
        String checkStatusUrl = "/get-schedule-status.php?schedule_id=" + String(currentScheduleId);
        http.get(checkStatusUrl);

        int httpResponseCode = http.responseStatusCode();
        Serial.print("HTTP Response Code (Stopped Status): ");
        Serial.println(httpResponseCode);

        if (httpResponseCode == 200) {
            String response = http.responseBody();
            // If the status is "Stopped", stop watering
            if (response == "Stopped" && currentScheduleId != -1) {
                digitalWrite(relayEnable, HIGH); // Turn off the relay
                Serial.println("Relay OFF - Watering done.");
                updateScheduleStatus(currentScheduleId, "Stopped");  // Update status to "Stopped"
                Serial.println("Status is Stopped for Schedule ID: " + String(currentScheduleId)); // Log stopped status
                currentScheduleId = -1;
                isWatering = false;  // Ensure watering is marked as stopped
            }
        } else {
            Serial.println("Failed to check schedule status, Response Code: " + String(httpResponseCode));
        }

        http.stop();
    }
}

void updateScheduleStatus(int scheduleId, String status) {
    if (WiFi.status() == WL_CONNECTED) {
        HttpClient http(sslClient, serverHost, serverPort);
        
        // Prepare the POST data as a URL-encoded string
        String postData = "user_id=18&schedule_id=" + String(scheduleId) + "&status=" + status;

        // Send the POST request to the server
        http.post("/update-schedule.php", "application/x-www-form-urlencoded", postData);

        // Get the HTTP response code
        int httpResponseCode = http.responseStatusCode();
        if (httpResponseCode == 200) {
            Serial.println("Schedule status updated to: " + status);
        } else {
            Serial.print("Failed to update schedule status. Error code: ");
            Serial.println(httpResponseCode);
        }

        // Stop the HTTP client
        http.stop();
    }
}

String getCurrentDayOfWeek(int dayIndex) {
    const char* daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return String(daysOfWeek[dayIndex]);
}

bool isScheduleTodayAndTime(String currentDay, String currentTime, String days, String startTime, int duration) {
    bool isDayMatch = days.indexOf(currentDay) != -1;

    int startHour = startTime.substring(0, 2).toInt();
    int startMinute = startTime.substring(3, 5).toInt();
    int startTotalMinutes = startHour * 60 + startMinute;

    int currentHour = currentTime.substring(0, 2).toInt();
    int currentMinute = currentTime.substring(3, 5).toInt();
    int currentTotalMinutes = currentHour * 60 + currentMinute;

    int endTotalMinutes = startTotalMinutes + (duration / 60);
    return isDayMatch && (currentTotalMinutes >= startTotalMinutes && currentTotalMinutes < endTotalMinutes);
}
