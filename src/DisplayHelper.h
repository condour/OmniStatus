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
    void render(void);
    SpotifyObject currentSpotifyObject;
    bool isClosed;
    void printLine(String str);
    int xCurs;
    int yCurs;
};
