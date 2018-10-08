#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class BaseTestRangeFilter : public LuceneTestCase
{
  GET_CLASS_NAME(BaseTestRangeFilter)

public:
  static constexpr bool F = false;
  static constexpr bool T = true;

  /**
   * Collation interacts badly with hyphens -- collation produces different
   * ordering than Unicode code-point ordering -- so two indexes are created:
   * one which can't have negative random integers, for testing collated ranges,
   * and the other which can have negative random integers, for all other tests.
   */
public:
  class TestIndex : public std::enable_shared_from_this<TestIndex>
  {
    GET_CLASS_NAME(TestIndex)
  public:
    int maxR = 0;
    int minR = 0;
    bool allowNegativeRandomInts = false;
    std::shared_ptr<Directory> index;

    TestIndex(std::shared_ptr<Random> random, int minR, int maxR,
              bool allowNegativeRandomInts);
  };

public:
  static std::shared_ptr<IndexReader> signedIndexReader;
  static std::shared_ptr<IndexReader> unsignedIndexReader;

  static std::shared_ptr<TestIndex> signedIndexDir;
  static std::shared_ptr<TestIndex> unsignedIndexDir;

  static int minId;
  static int maxId;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  static const int intLength =
      Integer::toString(std::numeric_limits<int>::max())->length();

  /**
   * a simple padding function that should work with any int
   */
  static std::wstring pad(int n);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void
  // beforeClassBaseTestRangeFilter() throws Exception
  static void beforeClassBaseTestRangeFilter() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void
  // afterClassBaseTestRangeFilter() throws Exception
  static void afterClassBaseTestRangeFilter() ;

private:
  static std::shared_ptr<IndexReader>
  build(std::shared_ptr<Random> random,
        std::shared_ptr<TestIndex> index) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPad()
  virtual void testPad();

protected:
  std::shared_ptr<BaseTestRangeFilter> shared_from_this()
  {
    return std::static_pointer_cast<BaseTestRangeFilter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
