#ifndef SPOTIFYHELPER_H
#define SPOTIFYHELPER_H
#include <HTTPClient.h>
#include "Fetch.h"
#include <Arduino_JSON.h>

extern const char* BASIC_AUTH;
extern const char* refresh_token;

class SpotifyHelper {
  public:
    SpotifyHelper(void);
    String lastsong;
    int songOffset;
    void getNowPlaying();
  private:
    String dequote(String str);
    String access_token;
    String thissong;
    void refreshToken(void);
    String hitAPIWithToken(String token);

};

#endif
