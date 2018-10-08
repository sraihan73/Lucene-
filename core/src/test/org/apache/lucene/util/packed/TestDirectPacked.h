#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/util/packed/MyRandom.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"

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
namespace org::apache::lucene::util::packed
{

using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDirectPacked : public LuceneTestCase
{
  GET_CLASS_NAME(TestDirectPacked)

  /** simple encode/decode */
public:
  virtual void testSimple() ;

  /** test exception is delivered if you add the wrong number of values */
  virtual void testNotEnoughValues() ;

  virtual void testRandom() ;

  virtual void testRandomWithOffset() ;

private:
  void doTestBpv(std::shared_ptr<Directory> directory, int bpv,
                 int64_t offset) ;

  std::deque<int64_t> randomLongs(std::shared_ptr<MyRandom> random, int bpv);

  // java.util.Random only returns 48bits of randomness in nextLong...
public:
  class MyRandom : public Random
  {
    GET_CLASS_NAME(MyRandom)
  public:
    std::deque<char> buffer = std::deque<char>(8);
    std::shared_ptr<ByteArrayDataInput> input =
        std::make_shared<ByteArrayDataInput>();

    MyRandom(int64_t seed);

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual int64_t nextLong(int bpv);

  protected:
    std::shared_ptr<MyRandom> shared_from_this()
    {
      return std::static_pointer_cast<MyRandom>(
          java.util.Random::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDirectPacked> shared_from_this()
  {
    return std::static_pointer_cast<TestDirectPacked>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
