#include <string>
#include <initializer_list>
#include "gtest\gtest.h"

#include "FileSystem.h"

using namespace std;

class FileSystemTest: public testing::Test {
protected:
  FileSystem FS;
  vector<string> CreatedFiles;

  virtual void TearDown() override;

  void BasicExistsDeleteTest();
  void BasicReadWriteTest();
  void OverwriteTest();
  void InvalidWriteTest();

  const string& RegisterFile(const string &path);
  void CreateFile(const string &path, initializer_list<string> &&lines);
};

void FileSystemTest::TearDown() {
  for (auto &createdFile : CreatedFiles)
    FS.Delete(createdFile);
}

const string& FileSystemTest::RegisterFile(const string &path) {
  CreatedFiles.push_back(path);
  return path;
}

void FileSystemTest::CreateFile(const string &path, initializer_list<string> &&lines) {
  ASSERT_FALSE(FS.Exists(path));
  FilePtr newFile = FS.Open(path, FileSystemInterface::Modes::Write);
  ASSERT_TRUE(newFile.operator bool());
  for (auto &line : lines)
    ASSERT_TRUE(newFile->WriteLine(line));
  ASSERT_TRUE(newFile->Close());
  ASSERT_TRUE(FS.Exists(path));
}

void FileSystemTest::BasicExistsDeleteTest() {
  const string fileName = RegisterFile("TestExistsDelete.txt");
  ASSERT_FALSE(FS.Exists(fileName));
  FilePtr newFile = FS.Open(fileName, FileSystemInterface::Write);
  ASSERT_TRUE(newFile.operator bool());
  ASSERT_TRUE(newFile->Close());
  ASSERT_TRUE(FS.Exists(fileName));
  ASSERT_TRUE(FS.Delete(fileName));
  ASSERT_FALSE(FS.Exists(fileName));
  ASSERT_FALSE(FS.Delete(fileName));
  ASSERT_FALSE(FS.Exists(fileName));
}

TEST_F(FileSystemTest, TestExists) {
  ASSERT_NO_FATAL_FAILURE(BasicExistsDeleteTest());
}

void FileSystemTest::BasicReadWriteTest() {
  const string fileName = RegisterFile("BasicReadWriteTest.txt");
  {
    ASSERT_FALSE(FS.Exists(fileName));
    FilePtr newFile = FS.Open(fileName, FileSystemInterface::Write);
    ASSERT_TRUE(newFile.operator bool());
    ASSERT_TRUE(newFile->WriteLine("line 0"));
    ASSERT_TRUE(newFile->WriteLine("line 1"));
    ASSERT_TRUE(newFile->WriteLine("line 2"));
    ASSERT_TRUE(newFile->Close());
    ASSERT_TRUE(FS.Exists(fileName));
  }
  {
    string currLine;
    int currLineNum = 0;
    FilePtr existingFile = FS.Open(fileName, FileSystemInterface::Read);
    ASSERT_TRUE(existingFile.operator bool());
    while (existingFile->ReadLine(currLine)) {
      ASSERT_EQ("line " + to_string(currLineNum), currLine);
      ++currLineNum;
    }
    ASSERT_EQ(3, currLineNum);
  }
}

void FileSystemTest::OverwriteTest() {
  const string fileName = RegisterFile("OverwriteFile.txt");
  {
    ASSERT_NO_FATAL_FAILURE(CreateFile(fileName, {"old first line", "old second line"}));
    FilePtr overwrittenFile = FS.Open(fileName, FileSystemInterface::Write);
    ASSERT_TRUE(overwrittenFile.operator bool());
    ASSERT_TRUE(overwrittenFile->WriteLine("new first line"));
  }
  {
    FilePtr overwrittenFile = FS.Open(fileName, FileSystemInterface::Read);
    string currLine;
    ASSERT_TRUE(overwrittenFile.operator bool());
    ASSERT_TRUE(overwrittenFile->ReadLine(currLine));
    ASSERT_EQ(string("new first line"), currLine);
    ASSERT_FALSE(overwrittenFile->ReadLine(currLine));
  }
}

void FileSystemTest::InvalidWriteTest() {
  const string fileName = RegisterFile("InvalidWriteTest.txt");
  ASSERT_NO_FATAL_FAILURE(CreateFile(fileName, {"foobar"}));
  {
    FilePtr readFile = FS.Open(fileName, FileSystemInterface::Read);
    ASSERT_TRUE(readFile.operator bool());
    ASSERT_FALSE(readFile->WriteLine("qux"));
  }
  {
    FilePtr writeFile = FS.Open(fileName, FileSystemInterface::Write);
    ASSERT_TRUE(writeFile.operator bool());
    ASSERT_TRUE(writeFile->WriteLine("qux"));
  }
}

TEST_F(FileSystemTest, TestOpenWrite) {
  ASSERT_NO_FATAL_FAILURE(BasicReadWriteTest());
  EXPECT_NO_FATAL_FAILURE(OverwriteTest());
  EXPECT_NO_FATAL_FAILURE(InvalidWriteTest());
}

TEST_F(FileSystemTest, TestOpenRead) {
  ASSERT_NO_FATAL_FAILURE(BasicReadWriteTest());
  {
    const string fileName = RegisterFile("EmptyFile.txt");
    string currLine;
    ASSERT_NO_FATAL_FAILURE(CreateFile(fileName, {}));
    FilePtr file = FS.Open(fileName, FileSystemInterface::Read);
    ASSERT_TRUE(file.operator bool());
    ASSERT_FALSE(file->ReadLine(currLine));
    ASSERT_TRUE(currLine.empty());
  }
  {
    const string fileName = RegisterFile("SingleBlankLine.txt");
    string currLine;
    ASSERT_NO_FATAL_FAILURE(CreateFile(fileName, {""}));
    FilePtr file = FS.Open(fileName, FileSystemInterface::Read);
    ASSERT_TRUE(file.operator bool());
    ASSERT_TRUE(file->ReadLine(currLine));
    ASSERT_TRUE(currLine.empty());
    ASSERT_FALSE(file->ReadLine(currLine));
  }
}

TEST_F(FileSystemTest, TestDelete) {
  ASSERT_NO_FATAL_FAILURE(BasicExistsDeleteTest());
}

TEST_F(FileSystemTest, TestClose) {
  {
    const string fileName = RegisterFile("TestExplicitClose.txt");
    FilePtr newFile = FS.Open(fileName, FileSystemInterface::Modes::Write);
    ASSERT_TRUE(newFile.operator bool());
    ASSERT_TRUE(FS.Exists(fileName));
    ASSERT_FALSE(FS.Delete(fileName));
    ASSERT_TRUE(newFile->Close());
    ASSERT_TRUE(FS.Delete(fileName));
  }
  {
    const string fileName = RegisterFile("TestImplicitClose.txt");
    {
      FilePtr newFile = FS.Open(fileName, FileSystemInterface::Modes::Write);
      ASSERT_TRUE(newFile.operator bool());
      ASSERT_TRUE(FS.Exists(fileName));
      ASSERT_FALSE(FS.Delete(fileName));
    }
    // Close() calls by dtor
    ASSERT_TRUE(FS.Delete(fileName));
  }
}

TEST_F(FileSystemTest, TestReset) {
  const string fileName = RegisterFile("TestReset.txt");
  string currLine;
  ASSERT_NO_FATAL_FAILURE(CreateFile(fileName, {"hello, world!", "second line"}));
  FilePtr file = FS.Open(fileName, FileSystemInterface::Modes::Read);
  ASSERT_TRUE(file.operator bool());

  // First time
  ASSERT_TRUE(file->ReadLine(currLine));
  ASSERT_EQ(string("hello, world!"), currLine);
  ASSERT_TRUE(file->ReadLine(currLine));
  ASSERT_EQ(string("second line"), currLine);
  currLine.clear();
  ASSERT_FALSE(file->ReadLine(currLine));
  ASSERT_TRUE(currLine.empty());

  ASSERT_TRUE(file->Reset());

  // Second time
  ASSERT_TRUE(file->ReadLine(currLine));
  ASSERT_EQ(string("hello, world!"), currLine);
  ASSERT_TRUE(file->ReadLine(currLine));
  ASSERT_EQ(string("second line"), currLine);
  currLine.clear();
  ASSERT_FALSE(file->ReadLine(currLine));
  ASSERT_TRUE(currLine.empty());
}
