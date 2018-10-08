#pragma once

//----------------------------------------------------------------------------------------
//	Copyright © 2007 - 2018 Tangible Software Solutions, Inc.
//	This class can be used by anyone provided that the copyright notice
//remains intact.
//
//	This class is used to replace some string methods, including
//	conversions to or from strings.
//----------------------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <exception>
#include <sstream>
#include <string>
#include <deque>
#include <boost/hana/concept/comparable.hpp>

#define GET_CLASS_NAME(NAME)                             \
public:                                                  \
  std::wstring getClassName() const { return L###NAME; } \
  static std::wstring _getClassName() { return L###NAME; }


class StringHelper
{
  GET_CLASS_NAME(StringHelper)
public:
  static std::wstring toLower(std::wstring source)
  {
    std::transform(source.begin(), source.end(), source.begin(), std::towlower);
    return source;
  }

  static std::wstring toUpper(std::wstring source)
  {
    std::transform(source.begin(), source.end(), source.begin(), std::towupper);
    return source;
  }

  static std::wstring trimStart(std::wstring source,
                                const std::wstring &trimChars = L" \t\n\r\v\f")
  {
    return source.erase(0, source.find_first_not_of(trimChars));
  }

  static std::wstring trimEnd(std::wstring source,
                              const std::wstring &trimChars = L" \t\n\r\v\f")
  {
    return source.erase(source.find_last_not_of(trimChars) + 1);
  }

  static std::wstring trim(std::wstring source,
                           const std::wstring &trimChars = L" \t\n\r\v\f")
  {
    return trimStart(trimEnd(source, trimChars), trimChars);
  }

  static std::wstring replace(std::wstring source, const std::wstring &find,
                              const std::wstring &replace)
  {
    size_t pos = 0;
    while ((pos = source.find(find, pos)) != std::string::npos) {
      source.replace(pos, find.length(), replace);
      pos += replace.length();
    }
    return source;
  }

  static bool startsWith(const std::wstring &source, const std::wstring &value)
  {
    if (source.length() < value.length())
      return false;
    else
      return source.compare(0, value.length(), value) == 0;
  }

  static bool endsWith(const std::wstring &source, const std::wstring &value)
  {
    if (source.length() < value.length())
      return false;
    else
      return source.compare(source.length() - value.length(), value.length(),
                            value) == 0;
  }

  static std::deque<std::wstring> split(const std::wstring &source,
                                         wchar_t delimiter)
  {
    std::deque<std::wstring> output;
    std::wistringstream ss(source);
    std::wstring nextItem;

    while (std::getline(ss, nextItem, delimiter)) {
      output.push_back(nextItem);
    }

    return output;
  }

  template <typename T>
  static std::wstring toString(const T &subject)
  {
    std::wostringstream ss;
    ss << subject;
    return ss.str();
  }

  template <typename T>
  static T fromString(const std::wstring &subject)
  {
    std::wistringstream ss(subject);
    T target;
    ss >> target;
    return target;
  }

  static bool isEmptyOrWhiteSpace(const std::wstring &source)
  {
    if (source.length() == 0)
      return true;
    else {
      for (int index = 0; index < source.length(); index++) {
        if (!std::isspace(source[index]))
          return false;
      }

      return true;
    }
  }

  template <typename T>
  static std::wstring formatSimple(const std::wstring &input, T arg1)
  {
    std::wostringstream ss;
    int lastFormatChar = -1;
    int percent = -1;
    while ((percent = input.find(L'%', percent + 1)) > -1) {
      if (percent + 1 < input.length()) {
        if (input[percent + 1] == L'%') {
          percent++;
          continue;
        }

        int formatEnd = -1;
        std::wstring index;
        for (int i = percent + 1; i < input.length(); i++) {
          if (input[i] == 's') {
            index = L"1";
            formatEnd = i;
            break;
          } else if (input[i] == '$' && i + 1 < input.length() &&
                     input[i + 1] == 's') {
            index = input.substr(percent + 1, i - percent - 1);
            formatEnd = i + 1;
            break;
          } else if (!std::isdigit(input[i]))
            break;
        }

        if (formatEnd > -1) {
          ss << input.substr(lastFormatChar + 1, percent - lastFormatChar - 1);
          lastFormatChar = formatEnd;

          if (index == L"1")
            ss << arg1;
          else
            throw std::runtime_error(
                "Only simple positional format specifiers are handled by the "
                "'formatSimple' helper method.");
        }
      }
    }

    if (lastFormatChar + 1 < input.length())
      ss << input.substr(lastFormatChar + 1);

    return ss.str();
  }

  template <typename T1, typename T2>
  static std::wstring formatSimple(const std::wstring &input, T1 arg1, T2 arg2)
  {
    std::wostringstream ss;
    int lastFormatChar = -1;
    int percent = -1;
    while ((percent = input.find(L'%', percent + 1)) > -1) {
      if (percent + 1 < input.length()) {
        if (input[percent + 1] == L'%') {
          percent++;
          continue;
        }

        int formatEnd = -1;
        std::wstring index;
        for (int i = percent + 1; i < input.length(); i++) {
          if (input[i] == 's') {
            index = L"1";
            formatEnd = i;
            break;
          } else if (input[i] == '$' && i + 1 < input.length() &&
                     input[i + 1] == 's') {
            index = input.substr(percent + 1, i - percent - 1);
            formatEnd = i + 1;
            break;
          } else if (!std::isdigit(input[i]))
            break;
        }

        if (formatEnd > -1) {
          ss << input.substr(lastFormatChar + 1, percent - lastFormatChar - 1);
          lastFormatChar = formatEnd;

          if (index == L"1")
            ss << arg1;
          else if (index == L"2")
            ss << arg2;
          else
            throw std::runtime_error(
                "Only simple positional format specifiers are handled by the "
                "'formatSimple' helper method.");
        }
      }
    }

    if (lastFormatChar + 1 < input.length())
      ss << input.substr(lastFormatChar + 1);

    return ss.str();
  }

  template <typename T1, typename T2, typename T3>
  static std::wstring formatSimple(const std::wstring &input, T1 arg1, T2 arg2,
                                   T3 arg3)
  {
    std::wostringstream ss;
    int lastFormatChar = -1;
    int percent = -1;
    while ((percent = input.find(L'%', percent + 1)) > -1) {
      if (percent + 1 < input.length()) {
        if (input[percent + 1] == L'%') {
          percent++;
          continue;
        }

        int formatEnd = -1;
        std::wstring index;
        for (int i = percent + 1; i < input.length(); i++) {
          if (input[i] == 's') {
            index = L"1";
            formatEnd = i;
            break;
          } else if (input[i] == '$' && i + 1 < input.length() &&
                     input[i + 1] == 's') {
            index = input.substr(percent + 1, i - percent - 1);
            formatEnd = i + 1;
            break;
          } else if (!std::isdigit(input[i]))
            break;
        }

        if (formatEnd > -1) {
          ss << input.substr(lastFormatChar + 1, percent - lastFormatChar - 1);
          lastFormatChar = formatEnd;

          if (index == L"1")
            ss << arg1;
          else if (index == L"2")
            ss << arg2;
          else if (index == L"3")
            ss << arg3;
          else
            throw std::runtime_error(
                "Only simple positional format specifiers are handled by the "
                "'formatSimple' helper method.");
        }
      }
    }

    if (lastFormatChar + 1 < input.length())
      ss << input.substr(lastFormatChar + 1);

    return ss.str();
  }
};
