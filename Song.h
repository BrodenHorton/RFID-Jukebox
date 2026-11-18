#ifndef SONG_H
#define SONG_H

#include <Arduino.h>

class Song {
  public:
    Song(String songId, String songFilePath);
    String getSongId();
    String getSongFilePath();

  private:
    String songId;
    String songFilePath;
};
#endif