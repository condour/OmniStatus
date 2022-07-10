#include "SpotifyHelper.h"

SpotifyHelper::SpotifyHelper(void) {
  lastsong = thissong = "";
  songOffset = 0;
}

String SpotifyHelper::dequote(String str) {
  return str.substring(1, str.length() - 1);
}

String SpotifyHelper::hitAPIWithToken(String token) {
  RequestOptions options;
  options.method = "GET";
  options.headers["Authorization"] = "Bearer " + token;
  Response response = fetch("https://api.spotify.com/v1/me/player/currently-playing", options);
  int status = response.status;
  if (status == 200) {
    Serial.println(status);
    JSONVar resultJSON = JSON.parse(response.text());
    JSONVar item = resultJSON["item"];
    String song = dequote(JSON.stringify(item["name"]));
    String album = dequote(JSON.stringify(item["album"]["name"]));
    String artists = "";
    for (int i = 0; i < item["artists"].length(); i++) {
      if(i != 0 && i != item["artists"].length() - 1) {
        artists = artists + ", ";
      }
      artists = artists + dequote(JSON.stringify(item["artists"][i]["name"]));
    }
    return album + " - " + song + " - " + artists;
  } else {
    throw (-1);
  }
}
void SpotifyHelper::refreshToken() {
  RequestOptions options;
  options.method = "GET";
  options.headers["Content-Type"] = "application/x-www-form-urlencoded";
  options.headers["Authorization"] = BASIC_AUTH;
  options.method = "POST";

  options.body = refresh_token;
  options.headers["Content-Length"] = options.body.text().length();
  Serial.println("going to fetch");
  Response response = fetch("https://accounts.spotify.com/api/token", options);
  JSONVar resultJSON = JSON.parse(response.text());
  access_token = resultJSON["access_token"];
}

void SpotifyHelper::getNowPlaying() {
  for (;;) {
    // use token to hit api
    try {
      if (access_token.length() > 0) {
        thissong = hitAPIWithToken(access_token);
        if (thissong != lastsong) {
          songOffset = 0;
          lastsong = thissong;
        }
      } else {
        refreshToken();
      }

    } catch (int whichError) {
      Serial.println("i'm in the catch");
      Serial.println(whichError);
      // if token fails, refresh token then reuse
      refreshToken();
    }
    // display song

    vTaskDelay(15000);
  }
}
