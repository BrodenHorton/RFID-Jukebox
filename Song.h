#ifndef SONG_H
#define SONG_H

#include <Arduino.h>

class Song {
  public:
    Song(String songId, int fileIndex);
    String getId();
    int getFileIndex();

  private:
    String id;
    int fileIndex;
};
#endif