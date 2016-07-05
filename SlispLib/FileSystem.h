#pragma once

#include <string>
#include <fstream>

#include "FileSystemInterface.h"

class File: public FileInterface {
public:
  virtual ~File();
  virtual bool ReadLine(std::string &line) override;
  virtual bool WriteLine(const std::string &line) override;
  virtual bool Reset() override;
  virtual bool Close() override;
private:
  std::fstream Stream;
  FileSystemInterface::Modes Mode;
  bool ShouldClose;

  explicit File(std::fstream &&stream, FileSystemInterface::Modes mode);

  friend class FileSystem;
};

class FileSystem: public FileSystemInterface {
public:
  virtual FilePtr Open(const std::string &path, Modes mode) override;
  virtual bool Exists(const std::string &path) override;
  virtual bool Delete(const std::string &path) override;
};
