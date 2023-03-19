#include <Arduino.h>
#ifndef SPOTIFY_OBJECT_H
#include "SpotifyObject.h"
#endif

class DisplayHelper {
  public:
    DisplayHelper(void);
    void setSpotifyInfo(SpotifyObject spotifyObject);
    void setGarageStatus(bool garageStatus);
    void init(void);
  private:
    void render(bool doFull);
    SpotifyObject currentSpotifyObject;
    bool isClosed;
    void printLine(String str, bool shouldClear);
    void breakStringAndCenter(String str);
    void printDivider(void);
    int xCurs;
    int yCurs;
};
