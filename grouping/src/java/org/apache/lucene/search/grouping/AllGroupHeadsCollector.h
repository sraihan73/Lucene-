#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::grouping
{
template <typename T>
class GroupHead;
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
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * This collector specializes in collecting the most relevant document (group
 * head) for each group that matches the query.
 *
 * Clients should create new collectors by calling {@link
 * #newCollector(GroupSelector, Sort)}
 *
 * @lucene.experimental
 */
template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) public abstract
// class AllGroupHeadsCollector<T> extends
// org.apache.lucene.search.SimpleCollector
class AllGroupHeadsCollector : public SimpleCollector
{

private:
  const std::shared_ptr<GroupSelector<T>> groupSelector;

protected:
  const std::shared_ptr<Sort> sort;

  std::deque<int> const reversed;
  const int compIDXEnd;

  std::unordered_map<T, GroupHead<T>> heads =
      std::unordered_map<T, GroupHead<T>>();

  std::shared_ptr<LeafReaderContext> context;
  std::shared_ptr<Scorer> scorer;

  /**
   * Create a new AllGroupHeadsCollector based on the type of within-group Sort
   * required
   * @param selector a GroupSelector to define the groups
   * @param sort     the within-group sort to use to choose the group head
   * document
   * @param <T>      the group value type
   */
public:
  template <typename T>
  static std::shared_ptr<AllGroupHeadsCollector<T>>
  newCollector(std::shared_ptr<GroupSelector<T>> selector,
               std::shared_ptr<Sort> sort)
  {
    if (sort->equals(Sort::RELEVANCE)) {
      return std::make_shared<ScoringGroupHeadsCollector<>>(selector, sort);
    }
    return std::make_shared<SortingGroupHeadsCollector<>>(selector, sort);
  }

private:
  AllGroupHeadsCollector(std::shared_ptr<GroupSelector<T>> selector,
                         std::shared_ptr<Sort> sort)
      : groupSelector(selector), sort(sort),
        reversed(std::deque<int>(sort->getSort().size())),
        compIDXEnd(this->reversed.size() - 1)
  {
    std::deque<std::shared_ptr<SortField>> sortFields = sort->getSort();
    for (int i = 0; i < sortFields.size(); i++) {
      reversed[i] = sortFields[i]->getReverse() ? -1 : 1;
    }
  }

  /**
   * @param maxDoc The maxDoc of the top level {@link IndexReader}.
   * @return a {@link FixedBitSet} containing all group heads.
   */
public:
  virtual std::shared_ptr<FixedBitSet> retrieveGroupHeads(int maxDoc)
  {
    std::shared_ptr<FixedBitSet> bitSet = std::make_shared<FixedBitSet>(maxDoc);

    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: java.util.std::deque<? extends GroupHead<T>> groupHeads =
    // getCollectedGroupHeads();
    std::shared_ptr < std::deque < ? extends GroupHead<T>>> groupHeads =
                                         getCollectedGroupHeads();
    for (auto groupHead : groupHeads) {
      bitSet->set(groupHead->doc);
    }

    return bitSet;
  }

  /**
   * @return an int array containing all group heads. The size of the array is
   * equal to number of collected unique groups.
   */
  virtual std::deque<int> retrieveGroupHeads()
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: java.util.std::deque<? extends GroupHead<T>> groupHeads =
    // getCollectedGroupHeads();
    std::shared_ptr < std::deque < ? extends GroupHead<T>>> groupHeads =
                                         getCollectedGroupHeads();
    std::deque<int> docHeads(groupHeads->size());

    int i = 0;
    for (auto groupHead : groupHeads) {
      docHeads[i++] = groupHead->doc;
    }

    return docHeads;
  }

  /**
   * @return the number of group heads found for a query.
   */
  virtual int groupHeadsSize() { return getCollectedGroupHeads()->size(); }

  /**
   * Returns the collected group heads.
   * Subsequent calls should return the same group heads.
   *
   * @return the collected group heads
   */
protected:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: protected java.util.std::deque<? extends GroupHead<T>>
  // getCollectedGroupHeads()
  virtual std::shared_ptr < std::deque < ? extends GroupHead<T>>>
                                               getCollectedGroupHeads()
  {
    return heads.values();
  }

public:
  void collect(int doc)  override
  {
    groupSelector->advanceTo(doc);
    T groupValue = groupSelector->currentValue();
    if (heads.find(groupValue) != heads.end() == false) {
      groupValue = groupSelector->copyValue();
      heads.emplace(groupValue, newGroupHead(doc, groupValue, context, scorer));
      return;
    }

    std::shared_ptr<GroupHead<T>> groupHead = heads[groupValue];
    // Ok now we need to check if the current doc is more relevant than top doc
    // for this group
    for (int compIDX = 0;; compIDX++) {
      constexpr int c = reversed[compIDX] * groupHead->compare(compIDX, doc);
      if (c < 0) {
        // Definitely not competitive. So don't even bother to continue
        return;
      } else if (c > 0) {
        // Definitely competitive.
        break;
      } else if (compIDX == compIDXEnd) {
        // Here c=0. If we're at the last comparator, this doc is not
        // competitive, since docs are visited in doc Id order, which means
        // this doc cannot compete with any other document in the queue.
        return;
      }
    }
    groupHead->updateDocHead(doc);
  }

  bool needsScores() override { return sort->needsScores(); }

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override
  {
    groupSelector->setNextReader(context);
    this->context = context;
    for (auto head : heads) {
      head->second.setNextReader(context);
    }
  }

public:
  void setScorer(std::shared_ptr<Scorer> scorer)  override
  {
    this->scorer = scorer;
    for (auto head : heads) {
      head->second.setScorer(scorer);
    }
  }

  /**
   * Create a new GroupHead for the given group value, initialized with a doc,
   * context and scorer
   */
protected:
  virtual std::shared_ptr<GroupHead<T>>
  newGroupHead(int doc, T value, std::shared_ptr<LeafReaderContext> context,
               std::shared_ptr<Scorer> scorer) = 0;

  /**
   * Represents a group head. A group head is the most relevant document for a
   * particular group. The relevancy is based is usually based on the sort.
   *
   * The group head contains a group value with its associated most relevant
   * document id.
   */
public:
  template <typename T>
  class GroupHead : public std::enable_shared_from_this<GroupHead>
  {
    GET_CLASS_NAME(GroupHead)

  public:
    const T groupValue;
    int doc = 0;

  protected:
    int docBase = 0;

    /**
     * Create a new GroupHead for the given value
     */
    GroupHead(T groupValue, int doc, int docBase) : groupValue(groupValue)
    {
      this->doc = doc + docBase;
      this->docBase = docBase;
    }

    /**
     * Called for each segment
     */
    virtual void
    setNextReader(std::shared_ptr<LeafReaderContext> ctx) 
    {
      this->docBase = ctx->docBase;
    }

    /**
     * Called for each segment
     */
    virtual void setScorer(std::shared_ptr<Scorer> scorer) = 0;

    /**
     * Compares the specified document for a specified comparator against the
     * current most relevant document.
     *
     * @param compIDX The comparator index of the specified comparator.
     * @param doc The specified document.
     * @return -1 if the specified document wasn't competitive against the
     * current most relevant document, 1 if the specified document was
     * competitive against the current most relevant document. Otherwise 0.
     * @throws IOException If I/O related errors occur
     */
    virtual int compare(int compIDX, int doc) = 0;

    /**
     * Updates the current most relevant document with the specified document.
     *
     * @param doc The specified document
     * @throws IOException If I/O related errors occur
     */
    virtual void updateDocHead(int doc) = 0;
  };

  /**
   * General implementation using a {@link FieldComparator} to select the group
   * head
   */
private:
  template <typename T>
  class SortingGroupHeadsCollector : public AllGroupHeadsCollector<T>
  {
    GET_CLASS_NAME(SortingGroupHeadsCollector)

  protected:
    SortingGroupHeadsCollector(std::shared_ptr<GroupSelector<T>> selector,
                               std::shared_ptr<Sort> sort)
        : AllGroupHeadsCollector<T>(selector, sort)
    {
    }

    std::shared_ptr<GroupHead<T>>
    newGroupHead(int doc, T value, std::shared_ptr<LeafReaderContext> ctx,
                 std::shared_ptr<Scorer> scorer)  override
    {
      return std::make_shared<SortingGroupHead<>>(sort, value, doc, ctx,
                                                  scorer);
    }

  protected:
    std::shared_ptr<SortingGroupHeadsCollector> shared_from_this()
    {
      return std::static_pointer_cast<SortingGroupHeadsCollector>(
          AllGroupHeadsCollector<T>::shared_from_this());
    }
  };

private:
  template <typename T>
  class SortingGroupHead : public GroupHead<T>
  {
    GET_CLASS_NAME(SortingGroupHead)

  public:
    std::deque<std::shared_ptr<FieldComparator>> const comparators;
    std::deque<std::shared_ptr<LeafFieldComparator>> const leafComparators;

  protected:
    SortingGroupHead(std::shared_ptr<Sort> sort, T groupValue, int doc,
                     std::shared_ptr<LeafReaderContext> context,
                     std::shared_ptr<Scorer> scorer) 
        : GroupHead<T>(groupValue, doc, context->docBase),
          comparators(std::deque<std::shared_ptr<FieldComparator>>(
              sortFields->length)),
          leafComparators(std::deque<std::shared_ptr<LeafFieldComparator>>(
              sortFields->length))
    {
      std::deque<std::shared_ptr<SortField>> sortFields = sort->getSort();
      for (int i = 0; i < sortFields.size(); i++) {
        comparators[i] = sortFields[i]->getComparator(1, i);
        leafComparators[i] = comparators[i]->getLeafComparator(context);
        leafComparators[i]->setScorer(scorer);
        leafComparators[i]->copy(0, doc);
        leafComparators[i]->setBottom(0);
      }
    }

  public:
    void setNextReader(std::shared_ptr<LeafReaderContext> ctx) throw(
        IOException) override
    {
      GroupHead<T>::setNextReader(ctx);
      for (int i = 0; i < comparators.size(); i++) {
        leafComparators[i] = comparators[i]->getLeafComparator(ctx);
      }
    }

  protected:
    void setScorer(std::shared_ptr<Scorer> scorer)  override
    {
      for (auto c : leafComparators) {
        c->setScorer(scorer);
      }
    }

  public:
    int compare(int compIDX, int doc)  override
    {
      return leafComparators[compIDX]->compareBottom(doc);
    }

    void updateDocHead(int doc)  override
    {
      for (auto comparator : leafComparators) {
        comparator->copy(0, doc);
        comparator->setBottom(0);
      }
      this->doc = doc + docBase;
    }

  protected:
    std::shared_ptr<SortingGroupHead> shared_from_this()
    {
      return std::static_pointer_cast<SortingGroupHead>(
          GroupHead<T>::shared_from_this());
    }
  };

  /**
   * Specialized implementation for sorting by score
   */
private:
  template <typename T>
  class ScoringGroupHeadsCollector : public AllGroupHeadsCollector<T>
  {
    GET_CLASS_NAME(ScoringGroupHeadsCollector)

  protected:
    ScoringGroupHeadsCollector(std::shared_ptr<GroupSelector<T>> selector,
                               std::shared_ptr<Sort> sort)
        : AllGroupHeadsCollector<T>(selector, sort)
    {
    }

    std::shared_ptr<GroupHead<T>>
    newGroupHead(int doc, T value, std::shared_ptr<LeafReaderContext> context,
                 std::shared_ptr<Scorer> scorer)  override
    {
      return std::make_shared<ScoringGroupHead<>>(scorer, value, doc,
                                                  context->docBase);
    }

  protected:
    std::shared_ptr<ScoringGroupHeadsCollector> shared_from_this()
    {
      return std::static_pointer_cast<ScoringGroupHeadsCollector>(
          AllGroupHeadsCollector<T>::shared_from_this());
    }
  };

private:
  template <typename T>
  class ScoringGroupHead : public GroupHead<T>
  {
    GET_CLASS_NAME(ScoringGroupHead)

  private:
    std::shared_ptr<Scorer> scorer;
    float topScore = 0;

  protected:
    ScoringGroupHead(std::shared_ptr<Scorer> scorer, T groupValue, int doc,
                     int docBase) 
        : GroupHead<T>(groupValue, doc, docBase)
    {
      assert(scorer->docID() == doc);
      this->scorer = scorer;
      this->topScore = scorer->score();
    }

    void setScorer(std::shared_ptr<Scorer> scorer) override
    {
      this->scorer = scorer;
    }

    int compare(int compIDX, int doc)  override
    {
      assert(scorer->docID() == doc);
      assert(compIDX == 0);
      float score = scorer->score();
      int c = Float::compare(score, topScore);
      if (c > 0) {
        topScore = score;
      }
      return c;
    }

    void updateDocHead(int doc)  override
    {
      this->doc = doc + docBase;
    }

  protected:
    std::shared_ptr<ScoringGroupHead> shared_from_this()
    {
      return std::static_pointer_cast<ScoringGroupHead>(
          GroupHead<T>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AllGroupHeadsCollector> shared_from_this()
  {
    return std::static_pointer_cast<AllGroupHeadsCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
