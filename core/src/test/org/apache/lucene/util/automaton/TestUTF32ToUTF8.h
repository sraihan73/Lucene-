#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class ByteRunAutomaton;
}

namespace org::apache::lucene::util::automaton
{
class Automaton;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestUTF32ToUTF8 : public LuceneTestCase
{
  GET_CLASS_NAME(TestUTF32ToUTF8)

public:
  void setUp()  override;

private:
  static constexpr int MAX_UNICODE = 0x10FFFF;

  bool matches(std::shared_ptr<ByteRunAutomaton> a, int code);

  void testOne(std::shared_ptr<Random> r, std::shared_ptr<ByteRunAutomaton> a,
               int startCode, int endCode, int iters);

  // Evenly picks random code point from the 4 "buckets"
  // (bucket = same #bytes when encoded to utf8)
  int getCodeStart(std::shared_ptr<Random> r);

  static bool isSurrogate(int code);

public:
  virtual void testRandomRanges() ;

  virtual void testSpecialCase();

  virtual void testSpecialCase2() ;

  virtual void testSpecialCase3() ;

  virtual void testRandomRegexes() ;

  virtual void testSingleton() ;

private:
  void assertAutomaton(std::shared_ptr<Automaton> automaton) throw(
      std::runtime_error);

protected:
  std::shared_ptr<TestUTF32ToUTF8> shared_from_this()
  {
    return std::static_pointer_cast<TestUTF32ToUTF8>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
