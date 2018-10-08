#pragma once
#include "stringhelper.h"
#pragma once

//----------------------------------------------------------------------------------------
//	Copyright © 2007 - 2018 Tangible Software Solutions, Inc.
//	This class can be used by anyone provided that the copyright notice
//remains intact.
//
//	This class is used to replace some Java file and folder method calls
//	with std::filesystem method calls.
//----------------------------------------------------------------------------------------
#include <filesystem>
#include <string>

class FileSystem
{
  GET_CLASS_NAME(FileSystem)
public:
  static void createDirectory(const std::wstring &path)
  {
    std::filesystem::create_directory(pathFromString(path));
  }

  static bool pathExists(const std::wstring &path)
  {
    return std::filesystem::exists(pathFromString(path));
  }

  static bool fileExists(const std::wstring &path)
  {
    return std::filesystem::is_regular_file(pathFromString(path));
  }

  static bool directoryExists(const std::wstring &path)
  {
    return std::filesystem::is_directory(pathFromString(path));
  }

  static std::wstring getFullPath(const std::wstring &path)
  {
    return std::filesystem::absolute(pathFromString(path)).generic_wstring();
  }

  static std::wstring getFileName(const std::wstring &path)
  {
    return std::filesystem::path(pathFromString(path))
        .filename()
        .generic_wstring();
  }

  static std::wstring getDirectoryName(const std::wstring &path)
  {
    return std::filesystem::path(pathFromString(path))
        .parent_path()
        .generic_wstring();
  }

  static wchar_t preferredSeparator()
  {
    return std::filesystem::path::preferred_separator;
  }

private:
  static std::filesystem::path pathFromString(const std::wstring &path)
  {
    return std::filesystem::path(&path[0]);
  }
};
