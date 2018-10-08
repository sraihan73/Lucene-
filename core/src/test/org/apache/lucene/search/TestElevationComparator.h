#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class LeafFieldComparator;
}
namespace org::apache::lucene::search
{
class Scorer;
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

using Document = org::apache::lucene::document::Document;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestElevationComparator : public LuceneTestCase
{
  GET_CLASS_NAME(TestElevationComparator)

private:
  const std::unordered_map<std::shared_ptr<BytesRef>, int> priority =
      std::unordered_map<std::shared_ptr<BytesRef>, int>();

  //@Test
public:
  virtual void testSorting() ;

private:
  void runTest(std::shared_ptr<IndexSearcher> searcher,
               bool reversed) ;

  std::shared_ptr<Query> getElevatedQuery(std::deque<std::wstring> &vals);

  std::shared_ptr<Document> adoc(std::deque<std::wstring> &vals);

protected:
  std::shared_ptr<TestElevationComparator> shared_from_this()
  {
    return std::static_pointer_cast<TestElevationComparator>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

class ElevationComparatorSource : public FieldComparatorSource
{
  GET_CLASS_NAME(ElevationComparatorSource)
private:
  const std::unordered_map<std::shared_ptr<BytesRef>, int> priority;

public:
  ElevationComparatorSource(
      std::unordered_map<std::shared_ptr<BytesRef>, int> &boosts);

  std::shared_ptr<FieldComparator<int>>
  newComparator(const std::wstring &fieldname, int const numHits, int sortPos,
                bool reversed) override;

private:
  class FieldComparatorAnonymousInnerClass : public FieldComparator<int>
  {
    GET_CLASS_NAME(FieldComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ElevationComparatorSource> outerInstance;

    std::wstring fieldname;
    int numHits = 0;

  public:
    FieldComparatorAnonymousInnerClass(
        std::shared_ptr<ElevationComparatorSource> outerInstance,
        const std::wstring &fieldname, int numHits);

  private:
    std::deque<int> const values;

  public:
    int bottomVal = 0;

    std::shared_ptr<LeafFieldComparator> getLeafComparator(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class LeafFieldComparatorAnonymousInnerClass
        : public std::enable_shared_from_this<
              LeafFieldComparatorAnonymousInnerClass>,
          public LeafFieldComparator
    {
      GET_CLASS_NAME(LeafFieldComparatorAnonymousInnerClass)
    private:
      std::shared_ptr<FieldComparatorAnonymousInnerClass> outerInstance;

      std::shared_ptr<LeafReaderContext> context;

    public:
      LeafFieldComparatorAnonymousInnerClass(
          std::shared_ptr<FieldComparatorAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReaderContext> context);

      void setBottom(int slot) override;

      int compareTop(int doc) override;

    private:
      int docVal(int doc) ;

    public:
      int compareBottom(int doc)  override;

      void copy(int slot, int doc)  override;

      void setScorer(std::shared_ptr<Scorer> scorer) override;
    };

  public:
    int compare(int slot1, int slot2) override;

    void setTopValue(std::optional<int> &value) override;

    std::optional<int> value(int slot) override;

  protected:
    std::shared_ptr<FieldComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FieldComparatorAnonymousInnerClass>(
          FieldComparator<int>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ElevationComparatorSource> shared_from_this()
  {
    return std::static_pointer_cast<ElevationComparatorSource>(
        FieldComparatorSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
