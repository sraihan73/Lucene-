#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Query;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDocValuesQueries : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocValuesQueries)

public:
  virtual void testDuelPointRangeSortedNumericRangeQuery() ;

  virtual void
  testDuelPointRangeMultivaluedSortedNumericRangeQuery() ;

  virtual void testDuelPointRangeNumericRangeQuery() ;

private:
  void
  doTestDuelPointRangeNumericRangeQuery(bool sortedNumeric,
                                        int maxValuesPerDoc) ;

  void
  doTestDuelPointRangeSortedRangeQuery(bool sortedSet,
                                       int maxValuesPerDoc) ;

public:
  virtual void testDuelPointRangeSortedSetRangeQuery() ;

  virtual void
  testDuelPointRangeMultivaluedSortedSetRangeQuery() ;

  virtual void testDuelPointRangeSortedRangeQuery() ;

private:
  void assertSameMatches(std::shared_ptr<IndexSearcher> searcher,
                         std::shared_ptr<Query> q1, std::shared_ptr<Query> q2,
                         bool scores) ;

public:
  virtual void testEquals();

  virtual void testToString();

  virtual void testMissingField() ;

  virtual void testSortedNumericNPE() ;

protected:
  std::shared_ptr<TestDocValuesQueries> shared_from_this()
  {
    return std::static_pointer_cast<TestDocValuesQueries>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
