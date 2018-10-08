#pragma once
#include "../../util/Attribute.h"
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
namespace org::apache::lucene::analysis::tokenattributes
{

using Attribute = org::apache::lucene::util::Attribute;

/**
 * The term text of a Token.
 */
class CharTermAttribute : public Attribute,
                          public std::wstring,
                          public Appendable
{
  GET_CLASS_NAME(CharTermAttribute)

  /** Copies the contents of buffer, starting at offset for
   *  length characters, into the termBuffer array.
   *  @param buffer the buffer to copy
   *  @param offset the index in the buffer of the first character to copy
   *  @param length the number of characters to copy
   */
public:
  virtual void copyBuffer(std::deque<wchar_t> &buffer, int offset,
                          int length) = 0;

  /** Returns the internal termBuffer character array which
   *  you can then directly alter.  If the array is too
   *  small for your token, use {@link
   *  #resizeBuffer(int)} to increase it.  After
   *  altering the buffer be sure to call {@link
   *  #setLength} to record the number of valid
   *  characters that were placed into the termBuffer.
   *  <p>
   *  <b>NOTE</b>: The returned buffer may be larger than
   *  the valid {@link #length()}.
   */
  virtual std::deque<wchar_t> buffer() = 0;

  /** Grows the termBuffer to at least size newSize, preserving the
   *  existing content.
   *  @param newSize minimum size of the new termBuffer
   *  @return newly created termBuffer with {@code length >= newSize}
   */
  virtual std::deque<wchar_t> resizeBuffer(int newSize) = 0;

  /** Set number of valid characters (length of the term) in
   *  the termBuffer array. Use this to truncate the termBuffer
   *  or to synchronize with external manipulation of the termBuffer.
   *  Note: to grow the size of the array,
   *  use {@link #resizeBuffer(int)} first.
   *  @param length the truncated length
   */
  virtual std::shared_ptr<CharTermAttribute> setLength(int length) = 0;

  /** Sets the length of the termBuffer to zero.
   * Use this method before appending contents
   * using the {@link Appendable} interface.
   */
  virtual std::shared_ptr<CharTermAttribute> setEmpty() = 0;

  // the following methods are redefined to get rid of IOException declaration:
  std::shared_ptr<CharTermAttribute> append(std::shared_ptr<std::wstring> csq);
  override = 0;
  std::shared_ptr<CharTermAttribute> append(std::shared_ptr<std::wstring> csq,
                                            int start, int end);
  override = 0;
  std::shared_ptr<CharTermAttribute> append(wchar_t c);
  override = 0;

  /** Appends the specified {@code std::wstring} to this character sequence.
   * <p>The characters of the {@code std::wstring} argument are appended, in order,
   * increasing the length of this sequence by the length of the argument. If
   * argument is {@code null}, then the four characters {@code "null"} are
   * appended.
   */
  virtual std::shared_ptr<CharTermAttribute> append(const std::wstring &s) = 0;

  /** Appends the specified {@code StringBuilder} to this character sequence.
   * <p>The characters of the {@code StringBuilder} argument are appended, in
   * order, increasing the length of this sequence by the length of the
   * argument. If argument is {@code null}, then the four characters {@code
   * "null"} are appended.
   */
  virtual std::shared_ptr<CharTermAttribute>
  append(std::shared_ptr<StringBuilder> sb) = 0;

  /** Appends the contents of the other {@code CharTermAttribute} to this
   * character sequence. <p>The characters of the {@code CharTermAttribute}
   * argument are appended, in order, increasing the length of this sequence by
   * the length of the argument. If argument is {@code null}, then the four
   * characters {@code "null"} are appended.
   */
  virtual std::shared_ptr<CharTermAttribute>
  append(std::shared_ptr<CharTermAttribute> termAtt) = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
