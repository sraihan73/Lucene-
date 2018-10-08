#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class TestHit;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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

using LeafReader = org::apache::lucene::index::LeafReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTermScorer : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermScorer)
protected:
  std::shared_ptr<Directory> directory;

private:
  static const std::wstring FIELD;

protected:
  std::deque<std::wstring> values = std::deque<std::wstring>{
      L"all", L"dogs dogs", L"like", L"playing", L"fetch", L"all"};
  std::shared_ptr<IndexSearcher> indexSearcher;
  std::shared_ptr<LeafReader> indexReader;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void test() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestTermScorer> outerInstance;

    std::shared_ptr<LeafReaderContext> context;
    std::deque<std::shared_ptr<TestHit>> docs;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestTermScorer> outerInstance,
        std::shared_ptr<LeafReaderContext> context,
        std::deque<std::shared_ptr<TestHit>> &docs);

  private:
    int base = 0;
    std::shared_ptr<Scorer> scorer;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;

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
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testNext() ;

  virtual void testAdvance() ;

private:
  class TestHit : public std::enable_shared_from_this<TestHit>
  {
    GET_CLASS_NAME(TestHit)
  public:
    int doc = 0;
    float score = 0;

    TestHit(int doc, float score);

    virtual std::wstring toString();
  };

public:
  virtual void testDoesNotLoadNorms() ;

private:
  class FilterLeafReaderAnonymousInnerClass : public FilterLeafReader
  {
    GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
  private:
    std::shared_ptr<TestTermScorer> outerInstance;

  public:
    FilterLeafReaderAnonymousInnerClass(
        std::shared_ptr<TestTermScorer> outerInstance,
        std::shared_ptr<LeafReader> indexReader);

    std::shared_ptr<NumericDocValues>
    getNormValues(const std::wstring &field)  override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<FilterLeafReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass>(
          org.apache.lucene.index.FilterLeafReader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestTermScorer> shared_from_this()
  {
    return std::static_pointer_cast<TestTermScorer>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
