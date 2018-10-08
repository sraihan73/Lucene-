#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}
namespace org::apache::lucene::index
{
class Term;
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
namespace org::apache::lucene::search::spans
{

using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertFinished;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.spans.SpanTestUtil.assertNext;

class TestFieldMaskingSpanQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestFieldMaskingSpanQuery)

protected:
  static std::shared_ptr<Document>
  doc(std::deque<std::shared_ptr<Field>> &fields);

  static std::shared_ptr<Field> field(const std::wstring &name,
                                      const std::wstring &value);

  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<Directory> directory;
  static std::shared_ptr<IndexReader> reader;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

protected:
  virtual void check(std::shared_ptr<SpanQuery> q,
                     std::deque<int> &docs) ;

public:
  virtual void testRewrite0() ;

  virtual void testRewrite1() ;

private:
  class SpanTermQueryAnonymousInnerClass : public SpanTermQuery
  {
    GET_CLASS_NAME(SpanTermQueryAnonymousInnerClass)
  private:
    std::shared_ptr<TestFieldMaskingSpanQuery> outerInstance;

  public:
    SpanTermQueryAnonymousInnerClass(
        std::shared_ptr<TestFieldMaskingSpanQuery> outerInstance,
        std::shared_ptr<Term> org);

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader) override;

  protected:
    std::shared_ptr<SpanTermQueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SpanTermQueryAnonymousInnerClass>(
          SpanTermQuery::shared_from_this());
    }
  };

public:
  virtual void testRewrite2() ;

  virtual void testEquality1();

  virtual void testNoop0() ;
  virtual void testNoop1() ;

  virtual void testSimple1() ;

  virtual void testSimple2() ;

  virtual void testSpans0() ;

  virtual void testSpans1() ;

  virtual void testSpans2() ;

  virtual std::wstring s(int doc, int start, int end);

protected:
  std::shared_ptr<TestFieldMaskingSpanQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestFieldMaskingSpanQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
