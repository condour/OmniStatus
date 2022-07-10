#include "GarageHelper.h"
GarageHelper::GarageHelper(void) {
}
int GarageHelper::getStatus(void) {
  return callGarage(false);
}
int GarageHelper::toggle(void) {
  return callGarage(true);
}
int GarageHelper::callGarage(boolean toggle) {

  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(toggle ? "http://garage.door/toggle" : "http://garage.door/");

  // Send HTTP POST request
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0) {
    payload = http.getString();
    http.end();
    JSONVar resultJSON = JSON.parse(payload);
    if (JSON.typeof(resultJSON) == "undefined") {
      Serial.println("PARSE FAILURE");
      return -1;
    }
    if (toggle) {
      return 1;
    }
    return (JSON.stringify(resultJSON["status"]) == "\"Closed\"");
  }
  http.end();
  return -1;
}
