#include "SpotifyHelper.h"
SpotifyHelper::SpotifyHelper(void) {
  currentSpotifyObject = {"","",""};
}

String SpotifyHelper::dequote(String str) {
  return str.substring(1, str.length() - 1);
}

SpotifyObject SpotifyHelper::hitAPIWithToken(String token) {
  RequestOptions options;
  options.method = "GET";
  options.headers["Authorization"] = "Bearer " + token;
  SpotifyObject spotifyObject;
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
    Serial.println("about to assign in spotifyhelper hitAPi");
    spotifyObject.song = song;
    spotifyObject.artists = artists;
    spotifyObject.album = album;
    Serial.println("going to return soon");
    Serial.println(album);
    return spotifyObject;
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
  Serial.print("going to fetch");
  Response response = fetch("https://accounts.spotify.com/api/token", options);
  JSONVar resultJSON = JSON.parse(response.text());
  Serial.print(response.text());
  access_token = resultJSON["access_token"];
}

SpotifyObject SpotifyHelper::getNowPlaying() {
    // use token to hit api
    try {
      if (access_token.length() > 0) {
        currentSpotifyObject = hitAPIWithToken(access_token);
        Serial.println("assigned spotifyObject in spotify helper");
      } else {
        refreshToken();
      }

    } catch (int whichError) {
      Serial.print("i'm in the catch");
      Serial.print(whichError);
      // if token fails, refresh token then reuse
      refreshToken();
    }
    // display song
    return currentSpotifyObject;
}
