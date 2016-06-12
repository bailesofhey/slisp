#pragma once

#include <string>
#include <memory>

class FileInterface {
public:
  virtual ~FileInterface();
  virtual bool ReadLine(std::string &line) = 0;
  virtual bool WriteLine(const std::string &line) = 0;
  virtual bool Reset() = 0;
  virtual bool Close() = 0;
};

using FilePtr = std::unique_ptr<FileInterface>;

class FileSystemInterface {
public:
  enum Modes {
    Read,
    Write
  };

  virtual ~FileSystemInterface();
  virtual FilePtr Open(const std::string &path, Modes mode) = 0;
  virtual bool Exists(const std::string &path) = 0;
  virtual bool Delete(const std::string &path) = 0;
};