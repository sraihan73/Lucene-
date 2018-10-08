#pragma once
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharacterBuffer;
}

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
namespace org::apache::lucene::analysis
{

/**
 * Utility class to write tokenizers or token filters.
 * @lucene.internal
 */
class CharacterUtils final : public std::enable_shared_from_this<CharacterUtils>
{
  GET_CLASS_NAME(CharacterUtils)

private:
  CharacterUtils();

  /**
   * Creates a new {@link CharacterBuffer} and allocates a <code>char[]</code>
   * of the given bufferSize.
   *
   * @param bufferSize
   *          the internal char buffer size, must be <code>&gt;= 2</code>
   * @return a new {@link CharacterBuffer} instance.
   */
public:
  static std::shared_ptr<CharacterBuffer>
  newCharacterBuffer(int const bufferSize);

  /**
   * Converts each unicode codepoint to lowerCase via {@link
   * Character#toLowerCase(int)} starting at the given offset.
   * @param buffer the char buffer to lowercase
   * @param offset the offset to start at
   * @param limit the max char in the buffer to lower case
   */
  static void toLowerCase(std::deque<wchar_t> &buffer, int const offset,
                          int const limit);

  /**
   * Converts each unicode codepoint to UpperCase via {@link
   * Character#toUpperCase(int)} starting at the given offset.
   * @param buffer the char buffer to UPPERCASE
   * @param offset the offset to start at
   * @param limit the max char in the buffer to lower case
   */
  static void toUpperCase(std::deque<wchar_t> &buffer, int const offset,
                          int const limit);

  /** Converts a sequence of Java characters to a sequence of unicode code
   * points.
   *  @return the number of code points written to the destination buffer */
  static int toCodePoints(std::deque<wchar_t> &src, int srcOff, int srcLen,
                          std::deque<int> &dest, int destOff);

  /** Converts a sequence of unicode code points to a sequence of Java
   * characters.
   *  @return the number of chars written to the destination buffer */
  static int toChars(std::deque<int> &src, int srcOff, int srcLen,
                     std::deque<wchar_t> &dest, int destOff);

  /**
   * Fills the {@link CharacterBuffer} with characters read from the given
   * reader {@link Reader}. This method tries to read <code>numChars</code>
   * characters into the {@link CharacterBuffer}, each call to fill will start
   * filling the buffer from offset <code>0</code> up to <code>numChars</code>.
   * In case code points can span across 2 java characters, this method may
   * only fill <code>numChars - 1</code> characters in order not to split in
   * the middle of a surrogate pair, even if there are remaining characters in
   * the {@link Reader}.
   * <p>
   * This method guarantees
   * that the given {@link CharacterBuffer} will never contain a high surrogate
   * character as the last element in the buffer unless it is the last available
   * character in the reader. In other words, high and low surrogate pairs will
   * always be preserved across buffer boarders.
   * </p>
   * <p>
   * A return value of <code>false</code> means that this method call exhausted
   * the reader, but there may be some bytes which have been read, which can be
   * verified by checking whether <code>buffer.getLength() &gt; 0</code>.
   * </p>
   *
   * @param buffer
   *          the buffer to fill.
   * @param reader
   *          the reader to read characters from.
   * @param numChars
   *          the number of chars to read
   * @return <code>false</code> if and only if reader.read returned -1 while
   * trying to fill the buffer
   * @throws IOException
   *           if the reader throws an {@link IOException}.
   */
  static bool fill(std::shared_ptr<CharacterBuffer> buffer,
                   std::shared_ptr<Reader> reader,
                   int numChars) ;

  /** Convenience method which calls <code>fill(buffer, reader,
   * buffer.buffer.length)</code>. */
  static bool fill(std::shared_ptr<CharacterBuffer> buffer,
                   std::shared_ptr<Reader> reader) ;

  static int readFully(std::shared_ptr<Reader> reader,
                       std::deque<wchar_t> &dest, int offset,
                       int len) ;

  /**
   * A simple IO buffer to use with
   * {@link CharacterUtils#fill(CharacterBuffer, Reader)}.
   */
public:
  class CharacterBuffer final
      : public std::enable_shared_from_this<CharacterBuffer>
  {
    GET_CLASS_NAME(CharacterBuffer)

  private:
    std::deque<wchar_t> const buffer;
    int offset = 0;
    int length = 0;
    // NOTE: not private so outer class can access without
    // $access methods:
  public:
    wchar_t lastTrailingHighSurrogate = L'\0';

    CharacterBuffer(std::deque<wchar_t> &buffer, int offset, int length);

    /**
     * Returns the internal buffer
     *
     * @return the buffer
     */
    std::deque<wchar_t> getBuffer();

    /**
     * Returns the data offset in the internal buffer.
     *
     * @return the offset
     */
    int getOffset();

    /**
     * Return the length of the data in the internal buffer starting at
     * {@link #getOffset()}
     *
     * @return the length
     */
    int getLength();

    /**
     * Resets the CharacterBuffer. All internals are reset to its default
     * values.
     */
    void reset();
  };
};

} // namespace org::apache::lucene::analysis
