#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::util::automaton
{
class Builder;
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
namespace org::apache::lucene::util::automaton
{

// TODO
//   - do we really need the .bits...?  if not we can make util in UnicodeUtil
//   to convert 1 char into a BytesRef

/**
 * Converts UTF-32 automata to the equivalent UTF-8 representation.
 * @lucene.internal
 */
class UTF32ToUTF8 final : public std::enable_shared_from_this<UTF32ToUTF8>
{
  GET_CLASS_NAME(UTF32ToUTF8)

  // Unicode boundaries for UTF8 bytes 1,2,3,4
private:
  static std::deque<int> const startCodes;
  static std::deque<int> const endCodes;

public:
  static std::deque<int> MASKS;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static UTF32ToUTF8::StaticConstructor staticConstructor;

  // Represents one of the N utf8 bytes that (in sequence)
  // define a code point.  value is the byte value; bits is
  // how many bits are "used" by utf8 at that byte
private:
  class UTF8Byte : public std::enable_shared_from_this<UTF8Byte>
  {
    GET_CLASS_NAME(UTF8Byte)
  public:
    int value = 0; // TODO: change to byte
    char bits = 0;
  };

  // Holds a single code point, as a sequence of 1-4 utf8 bytes:
  // TODO: maybe move to UnicodeUtil?
private:
  class UTF8Sequence : public std::enable_shared_from_this<UTF8Sequence>
  {
    GET_CLASS_NAME(UTF8Sequence)
  private:
    std::deque<std::shared_ptr<UTF8Byte>> const bytes;
    int len = 0;

  public:
    UTF8Sequence();

    virtual int byteAt(int idx);

    virtual int numBits(int idx);

  private:
    void set(int code);

    void setRest(int code, int numBytes);

  public:
    virtual std::wstring toString();
  };

  /** Sole constructor. */
public:
  UTF32ToUTF8();

private:
  const std::shared_ptr<UTF8Sequence> startUTF8 =
      std::make_shared<UTF8Sequence>();
  const std::shared_ptr<UTF8Sequence> endUTF8 =
      std::make_shared<UTF8Sequence>();

  const std::shared_ptr<UTF8Sequence> tmpUTF8a =
      std::make_shared<UTF8Sequence>();
  const std::shared_ptr<UTF8Sequence> tmpUTF8b =
      std::make_shared<UTF8Sequence>();

  // Builds necessary utf8 edges between start & end
public:
  void convertOneEdge(int start, int end, int startCodePoint, int endCodePoint);

private:
  void build(int start, int end, std::shared_ptr<UTF8Sequence> startUTF8,
             std::shared_ptr<UTF8Sequence> endUTF8, int upto);

  void start(int start, int end, std::shared_ptr<UTF8Sequence> startUTF8,
             int upto, bool doAll);

  void end(int start, int end, std::shared_ptr<UTF8Sequence> endUTF8, int upto,
           bool doAll);

  void all(int start, int end, int startCode, int endCode, int left);

public:
  std::shared_ptr<Automaton::Builder> utf8;

  /** Converts an incoming utf32 automaton to an equivalent
   *  utf8 one.  The incoming automaton need not be
   *  deterministic.  Note that the returned automaton will
   *  not in general be deterministic, so you must
   *  determinize it if that's needed. */
  std::shared_ptr<Automaton> convert(std::shared_ptr<Automaton> utf32);
};

} // namespace org::apache::lucene::util::automaton
