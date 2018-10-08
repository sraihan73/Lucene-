#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::analysis::util
{

/**
 * Some commonly-used stemming functions
 *
 * @lucene.internal
 */
class StemmerUtil : public std::enable_shared_from_this<StemmerUtil>
{
  GET_CLASS_NAME(StemmerUtil)
  /** no instance */
private:
  StemmerUtil();

  /**
   * Returns true if the character array starts with the suffix.
   *
   * @param s Input Buffer
   * @param len length of input buffer
   * @param prefix Prefix string to test
   * @return true if <code>s</code> starts with <code>prefix</code>
   */
public:
  static bool startsWith(std::deque<wchar_t> &s, int len,
                         const std::wstring &prefix);

  /**
   * Returns true if the character array ends with the suffix.
   *
   * @param s Input Buffer
   * @param len length of input buffer
   * @param suffix Suffix string to test
   * @return true if <code>s</code> ends with <code>suffix</code>
   */
  static bool endsWith(std::deque<wchar_t> &s, int len,
                       const std::wstring &suffix);

  /**
   * Returns true if the character array ends with the suffix.
   *
   * @param s Input Buffer
   * @param len length of input buffer
   * @param suffix Suffix string to test
   * @return true if <code>s</code> ends with <code>suffix</code>
   */
  static bool endsWith(std::deque<wchar_t> &s, int len,
                       std::deque<wchar_t> &suffix);

  /**
   * Delete a character in-place
   *
   * @param s Input Buffer
   * @param pos Position of character to delete
   * @param len length of input buffer
   * @return length of input buffer after deletion
   */
  static int delete_(std::deque<wchar_t> &s, int pos, int len);

  /**
   * Delete n characters in-place
   *
   * @param s Input Buffer
   * @param pos Position of character to delete
   * @param len Length of input buffer
   * @param nChars number of characters to delete
   * @return length of input buffer after deletion
   */
  static int deleteN(std::deque<wchar_t> &s, int pos, int len, int nChars);
};

} // namespace org::apache::lucene::analysis::util
