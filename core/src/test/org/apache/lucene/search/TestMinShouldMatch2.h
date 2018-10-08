#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::search
{
class BooleanWeight;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** tests BooleanScorer2's minShouldMatch */
class TestMinShouldMatch2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestMinShouldMatch2)
public:
  static std::shared_ptr<Directory> dir;
  static std::shared_ptr<DirectoryReader> r;
  static std::shared_ptr<LeafReader> reader;
  static std::shared_ptr<IndexSearcher> searcher;

  static std::deque<std::wstring> const alwaysTerms;
  static std::deque<std::wstring> const commonTerms;
  static std::deque<std::wstring> const mediumTerms;
  static std::deque<std::wstring> const rareTerms;

public:
  enum class Mode { GET_CLASS_NAME(Mode) SCORER, BULK_SCORER, DOC_VALUES };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

private:
  static void addSome(std::shared_ptr<Document> doc,
                      std::deque<std::wstring> &values);

  std::shared_ptr<Scorer> scorer(std::deque<std::wstring> &values,
                                 int minShouldMatch,
                                 Mode mode) ;

  void assertNext(std::shared_ptr<Scorer> expected,
                  std::shared_ptr<Scorer> actual) ;

  void assertAdvance(std::shared_ptr<Scorer> expected,
                     std::shared_ptr<Scorer> actual,
                     int amount) ;

  /** simple test for next(): minShouldMatch=2 on 3 terms (one common, one
   * medium, one rare) */
public:
  virtual void testNextCMR2() ;

  /** simple test for advance(): minShouldMatch=2 on 3 terms (one common, one
   * medium, one rare) */
  virtual void testAdvanceCMR2() ;

  /** test next with giant bq of all terms with varying minShouldMatch */
  virtual void testNextAllTerms() ;

  /** test advance with giant bq of all terms with varying minShouldMatch */
  virtual void testAdvanceAllTerms() ;

  /** test next with varying numbers of terms with varying minShouldMatch */
  virtual void testNextVaryingNumberOfTerms() ;

  /** test advance with varying numbers of terms with varying minShouldMatch */
  virtual void testAdvanceVaryingNumberOfTerms() ;

  // TODO: more tests

  // a slow min-should match scorer that uses a docvalues field.
  // later, we can make debugging easier as it can record the set of ords it
  // currently matched and e.g. print out their values and so on for the
  // document
public:
  class SlowMinShouldMatchScorer : public Scorer
  {
    GET_CLASS_NAME(SlowMinShouldMatchScorer)
  public:
    int currentDoc = -1;     // current docid
    int currentMatched = -1; // current number of terms matched

    const std::shared_ptr<SortedSetDocValues> dv;
    const int maxDoc;

    const std::shared_ptr<Set<int64_t>> ords =
        std::unordered_set<int64_t>();
    std::deque<std::shared_ptr<SimScorer>> const sims;
    const int minNrShouldMatch;

    // C++ NOTE: Fields cannot have the same name as methods:
    double score_ = NAN;

    SlowMinShouldMatchScorer(
        std::shared_ptr<BooleanWeight> weight,
        std::shared_ptr<LeafReader> reader,
        std::shared_ptr<IndexSearcher> searcher) ;

    float score()  override;

    int docID() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

  private:
    class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
    {
      GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<SlowMinShouldMatchScorer> outerInstance;

    public:
      DocIdSetIteratorAnonymousInnerClass(
          std::shared_ptr<SlowMinShouldMatchScorer> outerInstance);

      int nextDoc()  override;

      int advance(int target)  override;

      int64_t cost() override;

      int docID() override;

    protected:
      std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
            DocIdSetIterator::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SlowMinShouldMatchScorer> shared_from_this()
    {
      return std::static_pointer_cast<SlowMinShouldMatchScorer>(
          Scorer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMinShouldMatch2> shared_from_this()
  {
    return std::static_pointer_cast<TestMinShouldMatch2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
