#ifndef GARAGEHELPER_H
#define GARAGEHELPER_H
#include <HTTPClient.h>
#include "Fetch.h"
#include <Arduino_JSON.h>

extern const char* BASIC_AUTH;
extern const char* refresh_token;

class GarageHelper {
  private:
    int status;
    int callGarage(boolean toggle);
  public:
    GarageHelper(void);
    int getStatus(void);
    int toggle(void);
};


#endif
