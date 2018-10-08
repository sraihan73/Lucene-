#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Sort;
}

namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
namespace org::apache::lucene::search
{
class LeafFieldComparator;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search::grouping
{
class GroupQueue;
}
namespace org::apache::lucene::search::grouping
{
template <typename T>
class TopGroups;
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
namespace org::apache::lucene::search::grouping
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using Weight = org::apache::lucene::search::Weight;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

// TODO: this sentence is too long for the class summary.
/** BlockGroupingCollector performs grouping with a
 *  single pass collector, as long as you are grouping by a
 *  doc block field, ie all documents sharing a given group
 *  value were indexed as a doc block using the atomic
 *  {@link IndexWriter#addDocuments IndexWriter.addDocuments()}
 *  or {@link IndexWriter#updateDocuments IndexWriter.updateDocuments()}
 *  API.
 *
 *  <p>This results in faster performance (~25% faster QPS)
 *  than the two-pass grouping collectors, with the tradeoff
 *  being that the documents in each group must always be
 *  indexed as a block.  This collector also fills in
 *  TopGroups.totalGroupCount without requiring the separate
 *  {@link org.apache.lucene.search.grouping.AllGroupsCollector}.  However, this
 * collector does not fill in the groupValue of each group; this field will
 * always be null.
 *
 *  <p><b>NOTE</b>: this collector makes no effort to verify
 *  the docs were in fact indexed as a block, so it's up to
 *  you to ensure this was the case.
 *
 *  <p>See {@link org.apache.lucene.search.grouping} for more
 *  details including a full code example.</p>
 *
 * @lucene.experimental
 */

class BlockGroupingCollector : public SimpleCollector
{
  GET_CLASS_NAME(BlockGroupingCollector)

private:
  std::deque<int> pendingSubDocs;
  std::deque<float> pendingSubScores;
  int subDocUpto = 0;

  const std::shared_ptr<Sort> groupSort;
  const int topNGroups;
  const std::shared_ptr<Weight> lastDocPerGroup;

  // TODO: specialize into 2 classes, static "create" method:
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool needsScores_;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final org.apache.lucene.search.FieldComparator<?>[]
  // comparators;
  std::deque < FieldComparator < ? >> const comparators;
  std::deque<std::shared_ptr<LeafFieldComparator>> const leafComparators;
  std::deque<int> const reversed;
  const int compIDXEnd;
  int bottomSlot = 0;
  bool queueFull = false;
  std::shared_ptr<LeafReaderContext> currentReaderContext;

  int topGroupDoc = 0;
  int totalHitCount = 0;
  int totalGroupCount = 0;
  int docBase = 0;
  int groupEndDocID = 0;
  std::shared_ptr<DocIdSetIterator> lastDocPerGroupBits;
  std::shared_ptr<Scorer> scorer;
  const std::shared_ptr<GroupQueue> groupQueue;
  bool groupCompetes = false;

private:
  class OneGroup final : public std::enable_shared_from_this<OneGroup>
  {
    GET_CLASS_NAME(OneGroup)
  public:
    std::shared_ptr<LeafReaderContext> readerContext;
    // int groupOrd;
    int topGroupDoc = 0;
    std::deque<int> docs;
    std::deque<float> scores;
    int count = 0;
    int comparatorSlot = 0;
  };

  // Sorts by groupSort.  Not static -- uses comparators, reversed
private:
  class GroupQueue final : public PriorityQueue<std::shared_ptr<OneGroup>>
  {
    GET_CLASS_NAME(GroupQueue)
  private:
    std::shared_ptr<BlockGroupingCollector> outerInstance;

  public:
    GroupQueue(std::shared_ptr<BlockGroupingCollector> outerInstance, int size);

  protected:
    bool lessThan(std::shared_ptr<OneGroup> group1,
                  std::shared_ptr<OneGroup> group2) override;

  protected:
    std::shared_ptr<GroupQueue> shared_from_this()
    {
      return std::static_pointer_cast<GroupQueue>(
          org.apache.lucene.util.PriorityQueue<OneGroup>::shared_from_this());
    }
  };

  // Called when we transition to another group; if the
  // group is competitive we insert into the group queue
private:
  void processGroup() ;

  /**
   * Create the single pass collector.
   *
   *  @param groupSort The {@link Sort} used to sort the
   *    groups.  The top sorted document within each group
   *    according to groupSort, determines how that group
   *    sorts against other groups.  This must be non-null,
   *    ie, if you want to groupSort by relevance use
   *    Sort.RELEVANCE.
   *  @param topNGroups How many top groups to keep.
   *  @param needsScores true if the collected documents
   *    require scores, either because relevance is included
   *    in the withinGroupSort or because you plan to pass true
   *    for either getSscores or getMaxScores to {@link
   *    #getTopGroups}
   *  @param lastDocPerGroup a {@link Weight} that marks the
   *    last document in each group.
   */
public:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: public BlockGroupingCollector(org.apache.lucene.search.Sort
  // groupSort, int topNGroups, bool needsScores,
  // org.apache.lucene.search.Weight lastDocPerGroup)
  BlockGroupingCollector(std::shared_ptr<Sort> groupSort, int topNGroups,
                         bool needsScores,
                         std::shared_ptr<Weight> lastDocPerGroup);

  // TODO: maybe allow no sort on retrieving groups?  app
  // may want to simply process docs in the group itself?
  // typically they will be presented as a "single" result
  // in the UI?

  /** Returns the grouped results.  Returns null if the
   *  number of groups collected is &lt;= groupOffset.
   *
   *  <p><b>NOTE</b>: This collector is unable to compute
   *  the groupValue per group so it will always be null.
   *  This is normally not a problem, as you can obtain the
   *  value just like you obtain other values for each
   *  matching document (eg, via stored fields, via
   *  DocValues, etc.)
   *
   *  @param withinGroupSort The {@link Sort} used to sort
   *    documents within each group.
   *  @param groupOffset Which group to start from
   *  @param withinGroupOffset Which document to start from
   *    within each group
   *  @param maxDocsPerGroup How many top documents to keep
   *     within each group.
   *  @param fillSortFields If true then the Comparable
   *     values for the sort fields will be set
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: public TopGroups<?>
  // getTopGroups(org.apache.lucene.search.Sort withinGroupSort, int groupOffset,
  // int withinGroupOffset, int maxDocsPerGroup, bool fillSortFields) throws
  // java.io.IOException
  virtual std::shared_ptr < TopGroups <
      ? >> getTopGroups(std::shared_ptr<Sort> withinGroupSort, int groupOffset,
                        int withinGroupOffset, int maxDocsPerGroup,
                        bool fillSortFields) ;

  void setScorer(std::shared_ptr<Scorer> scorer)  override;

  void collect(int doc)  override;

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> readerContext) throw(
      IOException) override;

public:
  bool needsScores() override;

protected:
  std::shared_ptr<BlockGroupingCollector> shared_from_this()
  {
    return std::static_pointer_cast<BlockGroupingCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::grouping
