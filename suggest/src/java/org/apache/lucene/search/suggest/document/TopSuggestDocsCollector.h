#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest::document
{
class SuggestScoreDocPriorityQueue;
}

namespace org::apache::lucene::analysis
{
class CharArraySet;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::suggest::document
{
class TopSuggestDocs;
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
namespace org::apache::lucene::search::suggest::document
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TopSuggestDocs.SuggestScoreDoc;

/**
 * {@link org.apache.lucene.search.Collector} that collects completion and
 * score, along with document id
 * <p>
 * Non scoring collector that collect completions in order of their
 * pre-computed scores.
 * <p>
 * NOTE: One document can be collected multiple times if a document
 * is matched for multiple unique completions for a given query
 * <p>
 * Subclasses should only override
 * {@link TopSuggestDocsCollector#collect(int, std::wstring, std::wstring,
float)}. GET_CLASS_NAME(es)
 * <p>
 * NOTE: {@link #setScorer(org.apache.lucene.search.Scorer)} and
 * {@link #collect(int)} is not used
 *
 * @lucene.experimental
 */
class TopSuggestDocsCollector : public SimpleCollector
{
  GET_CLASS_NAME(TopSuggestDocsCollector)

private:
  const std::shared_ptr<SuggestScoreDocPriorityQueue> priorityQueue;
  const int num;

  /** Only set if we are deduplicating hits: holds all per-segment hits until
   * the end, when we dedup them */
  const std::deque<std::shared_ptr<SuggestScoreDoc>> pendingResults;

  /** Only set if we are deduplicating hits: holds all surface forms seen so far
   * in the current segment */
public:
  const std::shared_ptr<CharArraySet> seenSurfaceForms;

  /** Document base offset for the current Leaf */
protected:
  int docBase = 0;

  /**
   * Sole constructor
   *
   * Collects at most <code>num</code> completions
   * with corresponding document and weight
   */
public:
  TopSuggestDocsCollector(int num, bool skipDuplicates);

  /** Returns true if duplicates are filtered out */
protected:
  virtual bool doSkipDuplicates();

  /**
   * Returns the number of results to be collected
   */
public:
  virtual int getCountToCollect();

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override;

  /**
   * Called for every matched completion,
   * similar to {@link org.apache.lucene.search.LeafCollector#collect(int)}
   * but for completions.
   *
   * NOTE: collection at the leaf level is guaranteed to be in
   * descending order of score
   */
public:
  virtual void collect(int docID, std::shared_ptr<std::wstring> key,
                       std::shared_ptr<std::wstring> context,
                       float score) ;

  /**
   * Returns at most <code>num</code> Top scoring {@link
   * org.apache.lucene.search.suggest.document.TopSuggestDocs}s
   */
  virtual std::shared_ptr<TopSuggestDocs> get() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<SuggestScoreDoc>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TopSuggestDocsCollector> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TopSuggestDocsCollector> outerInstance);

    int compare(std::shared_ptr<SuggestScoreDoc> a,
                std::shared_ptr<SuggestScoreDoc> b);
  };

  /**
   * Ignored
   */
public:
  void collect(int doc)  override;

  /**
   * Ignored
   */
  bool needsScores() override;

protected:
  std::shared_ptr<TopSuggestDocsCollector> shared_from_this()
  {
    return std::static_pointer_cast<TopSuggestDocsCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
