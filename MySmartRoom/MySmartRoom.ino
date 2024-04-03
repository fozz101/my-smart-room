#include <WiFiManager.h>
#include <time.h>
#include <WebServer.h>
#include <WiFi.h>

// Define GPIO pins for relay control
const int relayPinUp = 25;    // Relay control pin for UP command
const int relayPinDown = 26;  // Relay control pin for DOWN command

// NTP server details
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;   // Enter the offset in seconds for your timezone
const int daylightOffset_sec = 0;  // Enter the daylight offset in seconds

// Time to trigger the actions
int upHour = -69;    // Set the hour to move the blinds up (24-hour format)
int downHour;        // Set the hour to move the blinds down (24-hour format)
int upMinute = -69;  // Set the minute to move the blinds up
int downMinute;      // Set the minute to move the blinds down

bool isUpActive = false;


WebServer server(80);

void handleRoot() {

  struct tm timeInfo;

  getLocalTime(&timeInfo);

  int minuteLimit = upMinute + 20;

  String html = "<!DOCTYPE html><html><head><title>MySmartRoom</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>";
  html += "<div class='container'>";

  html += "<h1>MySmartRoom</h1>";

  html += "<form action='/up' method='GET'>";
  html += "<input type='submit' value='UP'>";
  html += "</form>";


  html += "<form action='/down' method='GET'>";
  html += "<input type='submit' value='DOWN'>";
  html += "</form>";

  html += "<form action='/stop' method='GET'>";
  html += "<input type='submit' value='RESET RELAY'>";
  html += "</form>";




  if (isUpActive) {
    html += "<div class='status'>";
    html += "<h3>Status:</h3>";
    html += "<p>";
    html += "Blinds are scheduled to MOVE UP at " + (String(upHour).length() == 1 ? "0"+String(upHour) : String(upHour)) + ":" + (String(upMinute).length() == 1 ? "0"+String(upMinute) : String(upMinute)) + " !";
    html += "<form action='/stopUpSchedule' method='GET'>";
    html += "<input type='submit' value='Stop Schedule'>";
    html += "</form>";
  } else if (!isUpActive && upMinute != -69 && upHour != -69) {
    if (minuteLimit < 60) {
      if (timeInfo.tm_min < minuteLimit && timeInfo.tm_hour == upHour) {
        html += "<h1> WAKE THE FUCK UP ! </h1>";
      }
    } else {
      if (((timeInfo.tm_min > minuteLimit % 60) && timeInfo.tm_hour == upHour) || (((timeInfo.tm_min < minuteLimit % 60) && timeInfo.tm_hour == upHour + 1))) {
        html += "<h1> WAKE THE FUCK UP ! </h1>";
      }
    }
    html += "<div class='form-container'>";
    html += "<h2>Set Blinds Schedule</h2>";
    html += "<form action='/scheduleUp' method='GET'>";
    html += "<label for='upHour'>Up Hour:</label>";
    html += "<input type='number' id='upHour' name='upHour' min='0' max='23' required><br>";
    html += "<label for='upMinute'>Up Minute:</label>";
    html += "<input type='number' id='upMinute' name='upMinute' min='1' max='59' required><br>";
    html += "<input type='submit' value='Set Schedule'>";
    html += "</form>";
    html += "</div>";
    html += "<div class='status'>";
    html += "<h3>Status:</h3>";
    html += "<p>";
    html += "Blinds are not scheduled";
  } else {
    html += "<div class='form-container'>";
    html += "<h2>Set Blinds Schedule</h2>";
    html += "<form action='/scheduleUp' method='GET'>";
    html += "<label for='upHour'>Up Hour:</label>";
    html += "<input type='number' id='upHour' name='upHour' min='0' max='23' required><br>";
    html += "<label for='upMinute'>Up Minute:</label>";
    html += "<input type='number' id='upMinute' name='upMinute' min='1' max='59' required><br>";
    html += "<input type='submit' value='Set Schedule'>";
    html += "</form>";
    html += "</div>";
    html += "<div class='status'>";
    html += "<h3>Status:</h3>";
    html += "<p>";
    html += "Blinds are not scheduled";
  }
  html += "</p>";
  html += "</div>";

  html += "<form action='/disconnect' method='GET'>";
  html += "<input type='submit' value='Disconnect from WiFi'>";
  html += "</form>";

  html += "</div>";
  html += "<style>";
  html += "h1 {text-align: center;}";
  html += "body {font-family: Arial, sans-serif; margin: 0; padding: 0;}";
  html += ".container {width: 80%; margin: auto; padding-top: 20px;}";
  html += ".form-container {background-color: #f2f2f2; padding: 20px; border-radius: 10px;}";
  html += "input[type='number'] {width: 100%; padding: 10px; margin: 5px 0; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;}";
  html += "input[type='submit'] {width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer;}";
  html += "input[type='submit']:hover {background-color: #45a049;}";
  html += ".status {margin-top: 20px;}";
  html += "</style>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}



void handleStopUpSchedule() {
  isUpActive = false;
  String html = "<!DOCTYPE html><html><head><title>UP Schedule DISABLED</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>";
  html += "<div class='container'>";
  html += "<h2>UP Schedule DISABLED</h2>";
  html += "<p>Schedule to move up is DISABLED</p>";
  html += "<a href='/'>Back</a>";
  html += "</div>";
  html += "<style>";
  html += "body {font-family: Arial, sans-serif; margin: 0; padding: 0;}";
  html += ".container {width: 80%; margin: auto; padding-top: 20px;}";
  html += "h2 {text-align: center;}";
  html += "p {text-align: center;}";
  html += "a {display: block; width: 100px; margin: 20px auto; text-align: center; padding: 10px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 5px;}";
  html += "a:hover {background-color: #45a049;}";
  html += "</style>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleUpSchedule() {
  moveBlindsDown();
  upHour = server.arg("upHour").toInt();
  upMinute = server.arg("upMinute").toInt();
  isUpActive = true;
  String html = "<!DOCTYPE html><html><head><title>UP Schedule ACTIVATED</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>";
  html += "<div class='container'>";
  html += "<h2>UP Schedule ACTIVATED</h2>";
  html += "<p>Schedule set: Up Hour - " + (String(upHour).length() == 1 ? "0"+String(upHour) : String(upHour))  + ", Up Minute - " + (String(upMinute).length() == 1 ? "0"+String(upMinute) : String(upMinute)) + "</p>";
  html += "<a href='/'>Back</a>";
  html += "</div>";
  html += "<style>";
  html += "body {font-family: Arial, sans-serif; margin: 0; padding: 0;}";
  html += ".container {width: 80%; margin: auto; padding-top: 20px;}";
  html += "h2, p {text-align: center;}";
  html += "a {display: block; width: 100px; margin: 20px auto; text-align: center; padding: 10px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 5px;}";
  html += "a:hover {background-color: #45a049;}";
  html += "</style>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}


void handleUp() {
  Serial.println("UPPP");
  moveBlindsUp();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void handleDown() {

  struct tm timeInfo;

  getLocalTime(&timeInfo);

  int minuteLimit = upMinute + 20;

  if (minuteLimit < 60) {

    if (timeInfo.tm_min >= minuteLimit && timeInfo.tm_hour >= upHour) {
      Serial.println("DOWNNN");
      moveBlindsDown();
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "");
    }

  } else {
    if ((timeInfo.tm_min >= minuteLimit % 60) && timeInfo.tm_hour >= (upHour + 1)) {
      Serial.println("DOWNNN");
      moveBlindsDown();
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "");
    }
  }

  /*
  if (timeInfo.tm_min > upMinute+10 || timeInfo.tm_min < upMinute - 1 ){
      Serial.println("DOWNNN");
      moveBlindsDown();
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "");
  }
  */
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void handleStop() {
  Serial.println("STOPP");
  stopBlinds();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void handleDisconnectFromWiFi() {
  server.send(200, "text/plain", "Resetting WiFi settings... Please reconnect on MySmartRoom AP (na7chi fih.) TODO!");
}


void setupServer() {
  server.on("/", handleRoot);
  server.on("/scheduleUp", handleUpSchedule);
  server.on("/stopUpSchedule", handleStopUpSchedule);
  server.on("/up", handleUp);
  server.on("/down", handleDown);
  server.on("/stop", handleStop);

  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  Serial.begin(115200);


  pinMode(relayPinUp, OUTPUT);
  pinMode(relayPinDown, OUTPUT);
  stopBlinds();  // Ensure blinds are stopped initially

  WiFiManager wm;
  //wm.resetSettings();

  bool res;
  res = wm.autoConnect("MySmartRoom", "0123456789@");

  server.on("/disconnect", handleDisconnectFromWiFi);

  if (!res) {
    Serial.println("Failed to connect");
    //ESP.restart();
  } else {
    Serial.println("Connected to WiFi");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    setupServer();
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  server.handleClient();
  // Get current time
  struct tm timeInfo;



  if (!getLocalTime(&timeInfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  if (isUpActive) {
    Serial.println("UP Schedule is Active");
    if (timeInfo.tm_hour == upHour && timeInfo.tm_min == upMinute) {
      Serial.println("Windows is Opening !");
      moveBlindsUp();
      isUpActive = false;
    }
    // Put a short delay to prevent hammering the NTP server
    delay(1000);
  }
}

void moveBlindsUp() {
  stopBlinds();
  digitalWrite(relayPinDown, LOW);  // Ensure the DOWN relay is OFF
  digitalWrite(relayPinUp, HIGH);   // Turn the UP relay ON
}

void moveBlindsDown() {
  stopBlinds();
  digitalWrite(relayPinUp, LOW);     // Ensure the UP relay is OFF
  digitalWrite(relayPinDown, HIGH);  // Turn the DOWN relay ON
}

void stopBlinds() {
  // Turn both relays OFF to stop the blinds
  digitalWrite(relayPinUp, HIGH);
  digitalWrite(relayPinDown, HIGH);
  delay(500);
}
