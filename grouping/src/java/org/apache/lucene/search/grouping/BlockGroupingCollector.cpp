using namespace std;

#include "BlockGroupingCollector.h"

namespace org::apache::lucene::search::grouping
{
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using TopDocsCollector = org::apache::lucene::search::TopDocsCollector;
using TopFieldCollector = org::apache::lucene::search::TopFieldCollector;
using TopScoreDocCollector = org::apache::lucene::search::TopScoreDocCollector;
using Weight = org::apache::lucene::search::Weight;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

BlockGroupingCollector::GroupQueue::GroupQueue(
    shared_ptr<BlockGroupingCollector> outerInstance, int size)
    : org::apache::lucene::util::PriorityQueue<OneGroup>(size),
      outerInstance(outerInstance)
{
}

bool BlockGroupingCollector::GroupQueue::lessThan(shared_ptr<OneGroup> group1,
                                                  shared_ptr<OneGroup> group2)
{

  // System.out.println("    ltcheck");
  assert(group1 != group2);
  assert(group1->comparatorSlot != group2->comparatorSlot);

  constexpr int numComparators = outerInstance->comparators.size();
  for (int compIDX = 0; compIDX < numComparators; compIDX++) {
    constexpr int c = outerInstance->reversed[compIDX] *
                      outerInstance->comparators[compIDX]->compare(
                          group1->comparatorSlot, group2->comparatorSlot);
    if (c != 0) {
      // Short circuit
      return c > 0;
    }
  }

  // Break ties by docID; lower docID is always sorted first
  return group1->topGroupDoc > group2->topGroupDoc;
}

void BlockGroupingCollector::processGroup() 
{
  totalGroupCount++;
  // System.out.println("    processGroup ord=" + lastGroupOrd + " competes=" +
  // groupCompetes + " count=" + subDocUpto + " groupDoc=" + topGroupDoc);
  if (groupCompetes) {
    if (!queueFull) {
      // Startup transient: always add a new OneGroup
      shared_ptr<OneGroup> *const og = make_shared<OneGroup>();
      og->count = subDocUpto;
      og->topGroupDoc = docBase + topGroupDoc;
      og->docs = pendingSubDocs;
      pendingSubDocs = std::deque<int>(10);
      if (needsScores_) {
        og->scores = pendingSubScores;
        pendingSubScores = std::deque<float>(10);
      }
      og->readerContext = currentReaderContext;
      // og.groupOrd = lastGroupOrd;
      og->comparatorSlot = bottomSlot;
      shared_ptr<OneGroup> *const bottomGroup = groupQueue->push_back(og);
      // System.out.println("      ADD group=" + getGroupString(lastGroupOrd) +
      // " newBottom=" + getGroupString(bottomGroup.groupOrd));
      queueFull = groupQueue->size() == topNGroups;
      if (queueFull) {
        // Queue just became full; now set the real bottom
        // in the comparators:
        bottomSlot = bottomGroup->comparatorSlot;
        // System.out.println("    set bottom=" + bottomSlot);
        for (int i = 0; i < comparators.size(); i++) {
          leafComparators[i]->setBottom(bottomSlot);
        }
        // System.out.println("     QUEUE FULL");
      } else {
        // Queue not full yet -- just advance bottomSlot:
        bottomSlot = groupQueue->size();
      }
    } else {
      // Replace bottom element in PQ and then updateTop
      shared_ptr<OneGroup> *const og = groupQueue->top();
      assert(og != nullptr);
      og->count = subDocUpto;
      og->topGroupDoc = docBase + topGroupDoc;
      // Swap pending docs
      const std::deque<int> savDocs = og->docs;
      og->docs = pendingSubDocs;
      pendingSubDocs = savDocs;
      if (needsScores_) {
        // Swap pending scores
        const std::deque<float> savScores = og->scores;
        og->scores = pendingSubScores;
        pendingSubScores = savScores;
      }
      og->readerContext = currentReaderContext;
      // og.groupOrd = lastGroupOrd;
      bottomSlot = groupQueue->updateTop()->comparatorSlot;

      // System.out.println("    set bottom=" + bottomSlot);
      for (int i = 0; i < comparators.size(); i++) {
        leafComparators[i]->setBottom(bottomSlot);
      }
    }
  }
  subDocUpto = 0;
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: public BlockGroupingCollector(org.apache.lucene.search.Sort
// groupSort, int topNGroups, bool needsScores,
// org.apache.lucene.search.Weight lastDocPerGroup)
    BlockGroupingCollector::BlockGroupingCollector(shared_ptr<Sort> groupSort, int topNGroups, bool needsScores, shared_ptr<Weight> lastDocPerGroup) : groupSort(groupSort), topNGroups(topNGroups), lastDocPerGroup(lastDocPerGroup), needsScores(needsScores), comparators(std::deque<FieldComparator<?>>(sortFields->length)), leafComparators(std::deque<std::shared_ptr<LeafFieldComparator>>(sortFields->length)), reversed(std::deque<int>(sortFields->length)), compIDXEnd(comparators.size() - 1), groupQueue(make_shared<GroupQueue>(shared_from_this(), topNGroups))
    {

      if (topNGroups < 1) {
        throw invalid_argument(L"topNGroups must be >= 1 (got " +
                               to_wstring(topNGroups) + L")");
      }

      pendingSubDocs = std::deque<int>(10);
      if (needsScores) {
        pendingSubScores = std::deque<float>(10);
      }

      std::deque<std::shared_ptr<SortField>> sortFields = groupSort->getSort();
      for (int i = 0; i < sortFields.size(); i++) {
        shared_ptr<SortField> *const sortField = sortFields[i];
        comparators[i] = sortField->getComparator(topNGroups, i);
        reversed[i] = sortField->getReverse() ? -1 : 1;
      }
    }

    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: public TopGroups<?>
    // getTopGroups(org.apache.lucene.search.Sort withinGroupSort, int
    // groupOffset, int withinGroupOffset, int maxDocsPerGroup, bool
    // fillSortFields) throws java.io.IOException
    shared_ptr < TopGroups < ? >> BlockGroupingCollector::getTopGroups(
                                      shared_ptr<Sort> withinGroupSort,
                                      int groupOffset, int withinGroupOffset,
                                      int maxDocsPerGroup,
                                      bool fillSortFields) 
    {

      // if (queueFull) {
      // System.out.println("getTopGroups groupOffset=" + groupOffset + "
      // topNGroups=" + topNGroups);
      //}
      if (subDocUpto != 0) {
        processGroup();
      }
      if (groupOffset >= groupQueue->size()) {
        return nullptr;
      }
      int totalGroupedHitCount = 0;

      shared_ptr<FakeScorer> *const fakeScorer = make_shared<FakeScorer>();

      float maxScore = Float::MIN_VALUE;

      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
      // GroupDocs<Object>[] groups = new GroupDocs[groupQueue.size() -
      // groupOffset];
      std::deque<GroupDocs<any>> groups =
          std::deque<std::shared_ptr<GroupDocs>>(groupQueue->size() -
                                                  groupOffset);
      for (int downTo = groupQueue->size() - groupOffset - 1; downTo >= 0;
           downTo--) {
        shared_ptr<OneGroup> *const og = groupQueue->pop();

        // At this point we hold all docs w/ in each group,
        // unsorted; we now sort them:
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: final org.apache.lucene.search.TopDocsCollector<?>
        // collector;
        shared_ptr < TopDocsCollector < ? >> *const collector;
        if (withinGroupSort->equals(Sort::RELEVANCE)) {
          // Sort by score
          if (!needsScores_) {
            throw invalid_argument(
                L"cannot sort by relevance within group: needsScores=false");
          }
          collector = TopScoreDocCollector::create(maxDocsPerGroup);
        } else {
          // Sort by fields
          collector = TopFieldCollector::create(
              withinGroupSort, maxDocsPerGroup, fillSortFields, needsScores_,
              needsScores_, true); // TODO: disable exact counts?
        }

        shared_ptr<LeafCollector> leafCollector =
            collector->getLeafCollector(og->readerContext);
        leafCollector->setScorer(fakeScorer);
        for (int docIDX = 0; docIDX < og->count; docIDX++) {
          constexpr int doc = og->docs[docIDX];
          fakeScorer->doc = doc;
          if (needsScores_) {
            fakeScorer->score_ = og->scores[docIDX];
          }
          leafCollector->collect(doc);
        }
        totalGroupedHitCount += og->count;

        const std::deque<any> groupSortValues;

        if (fillSortFields) {
          // C++ TODO: Java wildcard generics are not converted to C++:
          // ORIGINAL LINE: groupSortValues = new
          // Comparable<?>[comparators.length];
          groupSortValues =
              std::deque < Comparable < ? >> (comparators.size());
          for (int sortFieldIDX = 0; sortFieldIDX < comparators.size();
               sortFieldIDX++) {
            groupSortValues[sortFieldIDX] =
                comparators[sortFieldIDX]->value(og->comparatorSlot);
          }
        } else {
          groupSortValues.clear();
        }

        shared_ptr<TopDocs> *const topDocs =
            collector->topDocs(withinGroupOffset, maxDocsPerGroup);

        // TODO: we could aggregate scores across children
        // by Sum/Avg instead of passing NaN:
        groups[downTo] = make_shared<GroupDocs<any>>(
            NAN, topDocs->getMaxScore(), og->count, topDocs->scoreDocs, nullptr,
            groupSortValues);
        maxScore = max(maxScore, topDocs->getMaxScore());
      }

      /*
      while (groupQueue.size() != 0) {
        final OneGroup og = groupQueue.pop();
        //System.out.println("  leftover: og ord=" + og.groupOrd + " count=" +
      og.count); totalGroupedHitCount += og.count;
      }
      */

      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: return new TopGroups<>(new
      // TopGroups<>(groupSort.getSort(), withinGroupSort.getSort(),
      // totalHitCount, totalGroupedHitCount, groups, maxScore),
      // totalGroupCount);
      return make_shared < TopGroups <
          ? >> (make_shared < TopGroups <
                ? >> (groupSort->getSort(), withinGroupSort->getSort(),
                      totalHitCount, totalGroupedHitCount, groups, maxScore),
                totalGroupCount);
    }

    void BlockGroupingCollector::setScorer(shared_ptr<Scorer> scorer) throw(
        IOException)
    {
      this->scorer = scorer;
      for (auto comparator : leafComparators) {
        comparator->setScorer(scorer);
      }
    }

    void BlockGroupingCollector::collect(int doc) 
    {

      // System.out.println("C " + doc);

      if (doc > groupEndDocID) {
        // Group changed
        if (subDocUpto != 0) {
          processGroup();
        }
        groupEndDocID = lastDocPerGroupBits->advance(doc);
        // System.out.println("  adv " + groupEndDocID + " " +
        // lastDocPerGroupBits);
        subDocUpto = 0;
        groupCompetes = !queueFull;
      }

      totalHitCount++;

      // Always cache doc/score within this group:
      if (subDocUpto == pendingSubDocs.size()) {
        pendingSubDocs = ArrayUtil::grow(pendingSubDocs);
      }
      pendingSubDocs[subDocUpto] = doc;
      if (needsScores_) {
        if (subDocUpto == pendingSubScores.size()) {
          pendingSubScores = ArrayUtil::grow(pendingSubScores);
        }
        pendingSubScores[subDocUpto] = scorer->score();
      }
      subDocUpto++;

      if (groupCompetes) {
        if (subDocUpto == 1) {
          assert(!queueFull);

          // System.out.println("    init copy to bottomSlot=" + bottomSlot);
          for (auto fc : leafComparators) {
            fc->copy(bottomSlot, doc);
            fc->setBottom(bottomSlot);
          }
          topGroupDoc = doc;
        } else {
          // Compare to bottomSlot
          for (int compIDX = 0;; compIDX++) {
            constexpr int c = reversed[compIDX] *
                              leafComparators[compIDX]->compareBottom(doc);
            if (c < 0) {
              // Definitely not competitive -- done
              return;
            } else if (c > 0) {
              // Definitely competitive.
              break;
            } else if (compIDX == compIDXEnd) {
              // Ties with bottom, except we know this docID is
              // > docID in the queue (docs are visited in
              // order), so not competitive:
              return;
            }
          }

          // System.out.println("       best w/in group!");

          for (auto fc : leafComparators) {
            fc->copy(bottomSlot, doc);
            // Necessary because some comparators cache
            // details of bottom slot; this forces them to
            // re-cache:
            fc->setBottom(bottomSlot);
          }
          topGroupDoc = doc;
        }
      } else {
        // We're not sure this group will make it into the
        // queue yet
        for (int compIDX = 0;; compIDX++) {
          constexpr int c =
              reversed[compIDX] * leafComparators[compIDX]->compareBottom(doc);
          if (c < 0) {
            // Definitely not competitive -- done
            // System.out.println("    doc doesn't compete w/ top groups");
            return;
          } else if (c > 0) {
            // Definitely competitive.
            break;
          } else if (compIDX == compIDXEnd) {
            // Ties with bottom, except we know this docID is
            // > docID in the queue (docs are visited in
            // order), so not competitive:
            // System.out.println("    doc doesn't compete w/ top groups");
            return;
          }
        }
        groupCompetes = true;
        for (auto fc : leafComparators) {
          fc->copy(bottomSlot, doc);
          // Necessary because some comparators cache
          // details of bottom slot; this forces them to
          // re-cache:
          fc->setBottom(bottomSlot);
        }
        topGroupDoc = doc;
        // System.out.println("        doc competes w/ top groups");
      }
    }

    void BlockGroupingCollector::doSetNextReader(
        shared_ptr<LeafReaderContext> readerContext) 
    {
      if (subDocUpto != 0) {
        processGroup();
      }
      subDocUpto = 0;
      docBase = readerContext->docBase;
      // System.out.println("setNextReader base=" + docBase + " r=" +
      // readerContext.reader);
      shared_ptr<Scorer> s = lastDocPerGroup->scorer(readerContext);
      if (s == nullptr) {
        lastDocPerGroupBits.reset();
      } else {
        lastDocPerGroupBits = s->begin();
      }
      groupEndDocID = -1;

      currentReaderContext = readerContext;
      for (int i = 0; i < comparators.size(); i++) {
        leafComparators[i] = comparators[i]->getLeafComparator(readerContext);
      }
    }

    bool BlockGroupingCollector::needsScores() { return needsScores_; }
    } // namespace org::apache::lucene::search::grouping