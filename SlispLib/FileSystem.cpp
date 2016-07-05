#include <fstream>

#ifdef WIN32
#include <Windows.h>
#endif

#include "FileSystem.h"

using namespace std;

File::File(fstream &&stream, FileSystemInterface::Modes mode):
  Stream(move(stream)),
  Mode(mode),
  ShouldClose(true)
{
}

File::~File() {
  Close();
}

bool File::Close() {
  if (ShouldClose) {
    if (Mode == FileSystemInterface::Modes::Write && Stream.is_open())
      Stream.flush();
    Stream.close();
    ShouldClose = false;
  }
  return true;
}

bool File::ReadLine(string &line) {
  if (getline(Stream, line))
    return true;
  else
    return false;
}

bool File::WriteLine(const string &line) {
  if (Mode == FileSystemInterface::Modes::Write) {
    Stream << line << endl;
    return true;
  }
  else
    return false;
}

bool File::Reset() {
  Stream.clear();
  Stream.seekg(0);
  return true;
}

//=============================================================================

FilePtr FileSystem::Open(const string &path, Modes mode) {
  fstream stream;
  stream.open(path, mode == Modes::Write ? ios::out : ios::in);
  return FilePtr { new File(move(stream), mode) };
}

bool FileSystem::Exists(const string &path) {
  fstream stream;
  stream.open(path, ios::in);
  return stream.is_open();
}

bool FileSystem::Delete(const string &path) {
  #ifdef WIN32
    return ::DeleteFileA(path.c_str()) == TRUE;
  #else
    return false;
  #endif
}
