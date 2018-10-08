using namespace std;

#include "EarlyTerminatingSortingCollector.h"

namespace org::apache::lucene::search
{
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionTerminatedException =
    org::apache::lucene::search::CollectionTerminatedException;
using Collector = org::apache::lucene::search::Collector;
using FilterCollector = org::apache::lucene::search::FilterCollector;
using FilterLeafCollector = org::apache::lucene::search::FilterLeafCollector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Sort = org::apache::lucene::search::Sort;
using TopDocsCollector = org::apache::lucene::search::TopDocsCollector;
using TotalHitCountCollector =
    org::apache::lucene::search::TotalHitCountCollector;

bool EarlyTerminatingSortingCollector::canEarlyTerminate(
    shared_ptr<Sort> searchSort, shared_ptr<Sort> mergePolicySort)
{
  std::deque<std::shared_ptr<SortField>> fields1 = searchSort->getSort();
  std::deque<std::shared_ptr<SortField>> fields2 = mergePolicySort->getSort();
  // early termination is possible if fields1 is a prefix of fields2
  if (fields1.size() > fields2.size()) {
    return false;
  }
  return Arrays::asList(fields1).equals(
      Arrays::asList(fields2).subList(0, fields1.size()));
}

EarlyTerminatingSortingCollector::EarlyTerminatingSortingCollector(
    shared_ptr<Collector> in_, shared_ptr<Sort> sort, int numDocsToCollect)
    : org::apache::lucene::search::FilterCollector(in_), sort(sort),
      numDocsToCollect(numDocsToCollect)
{
  if (numDocsToCollect <= 0) {
    throw invalid_argument(L"numDocsToCollect must always be > 0, got " +
                           to_wstring(numDocsToCollect));
  }
}

shared_ptr<LeafCollector> EarlyTerminatingSortingCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Sort> segmentSort = context->reader()->getMetaData()->getSort();
  if (segmentSort != nullptr && canEarlyTerminate(sort, segmentSort) == false) {
    throw make_shared<IllegalStateException>(
        L"Cannot early terminate with sort order " + sort +
        L" if segments are sorted with " + segmentSort);
  }

  if (segmentSort != nullptr) {
    // segment is sorted, can early-terminate
    return make_shared<FilterLeafCollectorAnonymousInnerClass>(
        shared_from_this(), FilterCollector::getLeafCollector(context));
  } else {
    return FilterCollector::getLeafCollector(context);
  }
}

EarlyTerminatingSortingCollector::FilterLeafCollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass(
        shared_ptr<EarlyTerminatingSortingCollector> outerInstance,
        shared_ptr<LeafCollector> getLeafCollector)
    : org::apache::lucene::search::FilterLeafCollector(getLeafCollector)
{
  this->outerInstance = outerInstance;
}

void EarlyTerminatingSortingCollector::FilterLeafCollectorAnonymousInnerClass::
    collect(int doc) 
{
  outerInstance->super->collect(doc);
  if (++numCollected >= outerInstance->numDocsToCollect) {
    outerInstance->terminatedEarly_->set(true);
    throw make_shared<CollectionTerminatedException>();
  }
}

bool EarlyTerminatingSortingCollector::terminatedEarly()
{
  return terminatedEarly_->get();
}
} // namespace org::apache::lucene::search