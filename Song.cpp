#include "Song.h"

Song::Song(String id, int fileIndex) {
  this->id = id;
  this->fileIndex = fileIndex;
}

String Song::getId() {
  return id;
}

int Song::getFileIndex() {
  return fileIndex;
}