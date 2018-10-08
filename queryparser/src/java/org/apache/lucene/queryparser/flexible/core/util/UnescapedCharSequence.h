#pragma once
#include "stringbuilder.h"
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
namespace org::apache::lucene::queryparser::flexible::core::util
{

/**
 * CharsSequence with escaped chars information.
 */
class UnescapedCharSequence final
    : public std::enable_shared_from_this<UnescapedCharSequence>,
      public std::wstring
{
  GET_CLASS_NAME(UnescapedCharSequence)
private:
  std::deque<wchar_t> chars;

  // C++ NOTE: Fields cannot have the same name as methods:
  std::deque<bool> wasEscaped_;

  /**
   * Create a escaped std::wstring
   */
public:
  UnescapedCharSequence(std::deque<wchar_t> &chars,
                        std::deque<bool> &wasEscaped, int offset, int length);

  /**
   * Create a non-escaped std::wstring
   */
  UnescapedCharSequence(std::shared_ptr<std::wstring> text);

  /**
   * Create a copy of an existent UnescapedCharSequence
   */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private
  // UnescapedCharSequence(UnescapedCharSequence text)
  UnescapedCharSequence(std::shared_ptr<UnescapedCharSequence> text);

public:
  wchar_t charAt(int index) override;

  int length() override;

  std::shared_ptr<std::wstring> subSequence(int start, int end) override;

  virtual std::wstring toString();

  /**
   * Return a escaped std::wstring
   *
   * @return a escaped std::wstring
   */
  std::wstring toStringEscaped();

  /**
   * Return a escaped std::wstring
   *
   * @param enabledChars
   *          - array of chars to be escaped
   * @return a escaped std::wstring
   */
  std::wstring toStringEscaped(std::deque<wchar_t> &enabledChars);

  bool wasEscaped(int index);

  static bool wasEscaped(std::shared_ptr<std::wstring> text, int index);

  static std::shared_ptr<std::wstring>
  toLowerCase(std::shared_ptr<std::wstring> text,
              std::shared_ptr<Locale> locale);
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/util/
