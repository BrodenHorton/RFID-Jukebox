#include "Song.h"

Song::Song(String songId, String songFilePath) {
  this->songId = songId;
  this->songFilePath = songFilePath;
}

String Song::getSongId() {
  return songId;
}

String Song::getSongFilePath() {
  return songFilePath;
}