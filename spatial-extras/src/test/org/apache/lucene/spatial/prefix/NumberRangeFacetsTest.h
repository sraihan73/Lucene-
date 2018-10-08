#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class DateRangePrefixTree;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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
namespace org::apache::lucene::spatial::prefix
{

using Query = org::apache::lucene::search::Query;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using DateRangePrefixTree =
    org::apache::lucene::spatial::prefix::tree::DateRangePrefixTree;
using Bits = org::apache::lucene::util::Bits;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

class NumberRangeFacetsTest : public StrategyTestCase
{
  GET_CLASS_NAME(NumberRangeFacetsTest)

public:
  std::shared_ptr<DateRangePrefixTree> tree;

  int randomCalWindowField = 0;
  int64_t randomCalWindowMs = 0;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = 20) @Test public void test() throws
  // java.io.IOException
  virtual void test() ;

private:
  std::shared_ptr<Bits>
  searchForDocBits(std::shared_ptr<Query> query) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<NumberRangeFacetsTest> outerInstance;

    std::shared_ptr<FixedBitSet> bitSet;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<NumberRangeFacetsTest> outerInstance,
        std::shared_ptr<FixedBitSet> bitSet);

    int leafDocBase = 0;
    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

private:
  void preQueryHavoc();

protected:
  virtual std::shared_ptr<Shape> randomShape();

private:
  std::shared_ptr<Calendar> randomCalendar();

protected:
  std::shared_ptr<NumberRangeFacetsTest> shared_from_this()
  {
    return std::static_pointer_cast<NumberRangeFacetsTest>(
        org.apache.lucene.spatial.StrategyTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
