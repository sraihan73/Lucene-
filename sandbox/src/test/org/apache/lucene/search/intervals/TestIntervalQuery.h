#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::store
{
class Directory;
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

namespace org::apache::lucene::search::intervals
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIntervalQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestIntervalQuery)

private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> directory;

public:
  static const std::wstring field;

  void setUp()  override;

  void tearDown()  override;

private:
  std::deque<std::wstring> docFields = {L"w1 w2 w3 w4 w5",
                                         L"w1 w3 w2 w3",
                                         L"w1 xx w2 w4 yy w3",
                                         L"w1 w3 xx w2 yy w3",
                                         L"w2 w1",
                                         L"w2 w1 w3 w2 w4",
                                         L"coordinate genome mapping research",
                                         L"coordinate genome research"};

  void checkHits(std::shared_ptr<Query> query,
                 std::deque<int> &results) ;

public:
  virtual void testPhraseQuery() ;

  virtual void testOrderedNearQueryWidth3() ;

  virtual void testOrderedNearQueryWidth4() ;

  virtual void testNestedOrderedNearQuery() ;

  virtual void testUnorderedQuery() ;

  virtual void testNonOverlappingQuery() ;

  virtual void testNotWithinQuery() ;

  virtual void testNotContainingQuery() ;

  virtual void testContainingQuery() ;

  virtual void testContainedByQuery() ;

  virtual void testNotContainedByQuery() ;

  virtual void testNonExistentTerms() ;

  // The Vigna paper doesn't deal with prefix disjunctions.  For now, we keep
  // the same logic as detailed in the paper, but we may want to address it in
  // future so that tests like the one below will pass
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testNestedOr() throws
  // java.io.IOException
  virtual void testNestedOr() ;

  virtual void testUnordered() ;

protected:
  std::shared_ptr<TestIntervalQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestIntervalQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals
