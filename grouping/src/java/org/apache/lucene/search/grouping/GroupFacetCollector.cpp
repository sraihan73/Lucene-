using namespace std;

#include "GroupFacetCollector.h"

namespace org::apache::lucene::search::grouping
{
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

GroupFacetCollector::GroupFacetCollector(const wstring &groupField,
                                         const wstring &facetField,
                                         shared_ptr<BytesRef> facetPrefix)
    : groupField(groupField), facetField(facetField), facetPrefix(facetPrefix),
      segmentResults(deque<>())
{
}

shared_ptr<GroupedFacetResult>
GroupFacetCollector::mergeSegmentResults(int size, int minCount,
                                         bool orderByCount) 
{
  if (segmentFacetCounts.size() > 0) {
    segmentResults.push_back(createSegmentResult());
    segmentFacetCounts.clear(); // reset
  }

  int totalCount = 0;
  int missingCount = 0;
  shared_ptr<SegmentResultPriorityQueue> segments =
      make_shared<SegmentResultPriorityQueue>(segmentResults.size());
  for (auto segmentResult : segmentResults) {
    missingCount += segmentResult->missing;
    if (segmentResult->mergePos >= segmentResult->maxTermPos) {
      continue;
    }
    totalCount += segmentResult->total;
    segments->push_back(segmentResult);
  }

  shared_ptr<GroupedFacetResult> facetResult = make_shared<GroupedFacetResult>(
      size, minCount, orderByCount, totalCount, missingCount);
  while (segments->size() > 0) {
    shared_ptr<SegmentResult> segmentResult = segments->top();
    shared_ptr<BytesRef> currentFacetValue =
        BytesRef::deepCopyOf(segmentResult->mergeTerm);
    int count = 0;

    do {
      count += segmentResult->counts[segmentResult->mergePos++];
      if (segmentResult->mergePos < segmentResult->maxTermPos) {
        segmentResult->nextTerm();
        segmentResult = segments->updateTop();
      } else {
        segments->pop();
        segmentResult = segments->top();
        if (segmentResult == nullptr) {
          break;
        }
      }
    } while (currentFacetValue->equals(segmentResult->mergeTerm));
    facetResult->addFacetCount(currentFacetValue, count);
  }
  return facetResult;
}

void GroupFacetCollector::setScorer(shared_ptr<Scorer> scorer) throw(
    IOException)
{
}

bool GroupFacetCollector::needsScores() { return false; }

const shared_ptr<java::util::Comparator<std::shared_ptr<FacetEntry>>>
    GroupFacetCollector::GroupedFacetResult::orderByCountAndValue =
        make_shared<ComparatorAnonymousInnerClass>();

GroupFacetCollector::GroupedFacetResult::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int GroupFacetCollector::GroupedFacetResult::ComparatorAnonymousInnerClass::
    compare(shared_ptr<FacetEntry> a, shared_ptr<FacetEntry> b)
{
  int cmp = b->count - a->count; // Highest count first!
  if (cmp != 0) {
    return cmp;
  }
  return a->value->compareTo(b->value);
}

const shared_ptr<java::util::Comparator<std::shared_ptr<FacetEntry>>>
    GroupFacetCollector::GroupedFacetResult::orderByValue =
        make_shared<ComparatorAnonymousInnerClass2>();

GroupFacetCollector::GroupedFacetResult::ComparatorAnonymousInnerClass2::
    ComparatorAnonymousInnerClass2()
{
}

int GroupFacetCollector::GroupedFacetResult::ComparatorAnonymousInnerClass2::
    compare(shared_ptr<FacetEntry> a, shared_ptr<FacetEntry> b)
{
  return a->value->compareTo(b->value);
}

GroupFacetCollector::GroupedFacetResult::GroupedFacetResult(
    int size, int minCount, bool orderByCount, int totalCount,
    int totalMissingCount)
    : maxSize(size),
      facetEntries(set<>(orderByCount ? orderByCountAndValue : orderByValue)),
      totalMissingCount(totalMissingCount), totalCount(totalCount)
{
  currentMin = minCount;
}

void GroupFacetCollector::GroupedFacetResult::addFacetCount(
    shared_ptr<BytesRef> facetValue, int count)
{
  if (count < currentMin) {
    return;
  }

  shared_ptr<FacetEntry> facetEntry =
      make_shared<FacetEntry>(facetValue, count);
  if (facetEntries->size() == maxSize) {
    if (facetEntries->higher(facetEntry) == nullptr) {
      return;
    }
    facetEntries->pollLast();
  }
  facetEntries->add(facetEntry);

  if (facetEntries->size() == maxSize) {
    currentMin = facetEntries->last().count;
  }
}

deque<std::shared_ptr<FacetEntry>>
GroupFacetCollector::GroupedFacetResult::getFacetEntries(int offset, int limit)
{
  deque<std::shared_ptr<FacetEntry>> entries =
      deque<std::shared_ptr<FacetEntry>>();

  int skipped = 0;
  int included = 0;
  for (auto facetEntry : facetEntries) {
    if (skipped < offset) {
      skipped++;
      continue;
    }
    if (included++ >= limit) {
      break;
    }
    entries.push_back(facetEntry);
  }
  return entries;
}

int GroupFacetCollector::GroupedFacetResult::getTotalCount()
{
  return totalCount;
}

int GroupFacetCollector::GroupedFacetResult::getTotalMissingCount()
{
  return totalMissingCount;
}

GroupFacetCollector::FacetEntry::FacetEntry(shared_ptr<BytesRef> value,
                                            int count)
    : value(value), count(count)
{
}

bool GroupFacetCollector::FacetEntry::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<FacetEntry> that = any_cast<std::shared_ptr<FacetEntry>>(o);

  if (count != that->count) {
    return false;
  }
  if (!value->equals(that->value)) {
    return false;
  }

  return true;
}

int GroupFacetCollector::FacetEntry::hashCode()
{
  int result = value->hashCode();
  result = 31 * result + count;
  return result;
}

wstring GroupFacetCollector::FacetEntry::toString()
{
  return wstring(L"FacetEntry{") + L"value=" + value->utf8ToString() +
         L", count=" + to_wstring(count) + L'}';
}

shared_ptr<BytesRef> GroupFacetCollector::FacetEntry::getValue()
{
  return value;
}

int GroupFacetCollector::FacetEntry::getCount() { return count; }

GroupFacetCollector::SegmentResult::SegmentResult(std::deque<int> &counts,
                                                  int total, int missing,
                                                  int maxTermPos)
    : counts(counts), total(total), missing(missing), maxTermPos(maxTermPos)
{
}

GroupFacetCollector::SegmentResultPriorityQueue::SegmentResultPriorityQueue(
    int maxSize)
    : org::apache::lucene::util::PriorityQueue<SegmentResult>(maxSize)
{
}

bool GroupFacetCollector::SegmentResultPriorityQueue::lessThan(
    shared_ptr<SegmentResult> a, shared_ptr<SegmentResult> b)
{
  return a->mergeTerm->compareTo(b->mergeTerm) < 0;
}
} // namespace org::apache::lucene::search::grouping