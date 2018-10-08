#pragma once
#include "stringhelper.h"
#include <memory>
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

/** Acts like a forever growing char[] as you read
 *  characters into it from the provided reader, but
 *  internally it uses a circular buffer to only hold the
 *  characters that haven't been freed yet.  This is like a
 *  PushbackReader, except you don't have to specify
 *  up-front the max size of the buffer, but you do have to
 *  periodically call {@link #freeBefore}. */

class RollingCharBuffer final
    : public std::enable_shared_from_this<RollingCharBuffer>
{
  GET_CLASS_NAME(RollingCharBuffer)

private:
  std::shared_ptr<Reader> reader;

  std::deque<wchar_t> buffer = std::deque<wchar_t>(512);

  // Next array index to write to in buffer:
  int nextWrite = 0;

  // Next absolute position to read from reader:
  int nextPos = 0;

  // How many valid chars (wrapped) are in the buffer:
  int count = 0;

  // True if we hit EOF
  bool end = false;

  /** Clear array and switch to new reader. */
public:
  void reset(std::shared_ptr<Reader> reader);

  /* Absolute position read.  NOTE: pos must not jump
   * ahead by more than 1!  Ie, it's OK to read arbitarily
   * far back (just not prior to the last {@link
   * #freeBefore}), but NOT ok to read arbitrarily far
   * ahead.  Returns -1 if you hit EOF. */
  int get(int pos) ;

  // For assert:
private:
  bool inBounds(int pos);

  int getIndex(int pos);

public:
  std::deque<wchar_t> get(int posStart, int length);

  /** Call this to notify us that no chars before this
   *  absolute position are needed anymore. */
  void freeBefore(int pos);
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
