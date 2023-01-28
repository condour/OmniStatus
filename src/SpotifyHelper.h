#include <HTTPClient.h>
#include "Fetch.h"
#include <Arduino_JSON.h>
#ifndef SPOTIFY_OBJECT_H
#include "SpotifyObject.h"
#endif
extern const char* BASIC_AUTH;
extern const char* refresh_token;

class SpotifyHelper {
  public:
    SpotifyHelper(void);
    SpotifyObject getNowPlaying(void);
  private:
    String dequote(String str);
    String access_token;
    SpotifyObject currentSpotifyObject;
    void refreshToken(void);
    SpotifyObject hitAPIWithToken(String token);
};

