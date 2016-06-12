#include "FileSystemInterface.h"

FileInterface::~FileInterface() {
  Close();
}

bool FileInterface::Close() {
  return false;
}

//=============================================================================

FileSystemInterface::~FileSystemInterface() {
}
