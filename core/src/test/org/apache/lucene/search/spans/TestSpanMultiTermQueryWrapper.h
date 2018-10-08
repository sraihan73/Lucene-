#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class MultiTermQuery;
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
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests for {@link SpanMultiTermQueryWrapper}, wrapping a few MultiTermQueries.
 */
class TestSpanMultiTermQueryWrapper : public LuceneTestCase
{
  GET_CLASS_NAME(TestSpanMultiTermQueryWrapper)
private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testWildcard() ;

  virtual void testPrefix() ;

  virtual void testFuzzy() ;

  virtual void testFuzzy2() ;
  virtual void testNoSuchMultiTermsInNear() ;

  virtual void testNoSuchMultiTermsInNotNear() ;

  virtual void testNoSuchMultiTermsInOr() ;

  virtual void testNoSuchMultiTermsInSpanFirst() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWrappedQueryIsNotModified()
  virtual void testWrappedQueryIsNotModified();

private:
  class SpanRewriteMethodAnonymousInnerClass
      : public SpanMultiTermQueryWrapper::SpanRewriteMethod
  {
    GET_CLASS_NAME(SpanRewriteMethodAnonymousInnerClass)
  private:
    std::shared_ptr<TestSpanMultiTermQueryWrapper> outerInstance;

  public:
    SpanRewriteMethodAnonymousInnerClass(
        std::shared_ptr<TestSpanMultiTermQueryWrapper> outerInstance);

    std::shared_ptr<SpanQuery>
    rewrite(std::shared_ptr<IndexReader> reader,
            std::shared_ptr<MultiTermQuery> query)  override;

  protected:
    std::shared_ptr<SpanRewriteMethodAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SpanRewriteMethodAnonymousInnerClass>(
          SpanMultiTermQueryWrapper.SpanRewriteMethod::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSpanMultiTermQueryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<TestSpanMultiTermQueryWrapper>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
