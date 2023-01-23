#include <Arduino.h>

class DisplayHelper {
  public:
    DisplayHelper(void);
    void showSpotifyInfo(String str);
    void init(void);
  private:
    int status;
};
