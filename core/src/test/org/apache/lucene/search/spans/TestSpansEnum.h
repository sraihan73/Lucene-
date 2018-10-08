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
namespace org::apache::lucene::search::spans
{
class SpanQuery;
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
namespace org::apache::lucene::search::spans
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

using namespace org::apache::lucene::search::spans;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;

/**
 * Tests Spans (v2)
 *
 */
class TestSpansEnum : public LuceneTestCase
{
  GET_CLASS_NAME(TestSpansEnum)
private:
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<Directory> directory;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

private:
  void checkHits(std::shared_ptr<Query> query,
                 std::deque<int> &results) ;

public:
  virtual void testSpansEnumOr1() ;

  virtual void testSpansEnumOr2() ;

  virtual void testSpansEnumOr3() ;

  virtual std::shared_ptr<SpanQuery> spanTQ(const std::wstring &s);

  virtual void testSpansEnumOrNot1() ;

  virtual void testSpansEnumNotBeforeAfter1() ;

  virtual void testSpansEnumNotBeforeAfter2() ;

  virtual void testSpansEnumNotBeforeAfter3() ;

protected:
  std::shared_ptr<TestSpansEnum> shared_from_this()
  {
    return std::static_pointer_cast<TestSpansEnum>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
