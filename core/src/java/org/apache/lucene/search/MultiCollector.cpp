using namespace std;

#include "MultiCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

shared_ptr<Collector> MultiCollector::wrap(deque<Collector> &collectors)
{
  return wrap({Arrays::asList(collectors)});
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static Collector wrap(Iterable<? extends
// Collector> collectors)
shared_ptr<Collector> MultiCollector::wrap(deque<T1> collectors)
{
  // For the user's convenience, we allow null collectors to be passed.
  // However, to improve performance, these null collectors are found
  // and dropped from the array we save for actual collection time.
  int n = 0;
  for (auto c : collectors) {
    if (c != nullptr) {
      n++;
    }
  }

  if (n == 0) {
    throw invalid_argument(L"At least 1 collector must not be null");
  } else if (n == 1) {
    // only 1 Collector - return it.
    shared_ptr<Collector> col = nullptr;
    for (auto c : collectors) {
      if (c != nullptr) {
        col = c;
        break;
      }
    }
    return col;
  } else {
    std::deque<std::shared_ptr<Collector>> colls(n);
    n = 0;
    for (auto c : collectors) {
      if (c != nullptr) {
        colls[n++] = c;
      }
    }
    return make_shared<MultiCollector>(colls);
  }
}

MultiCollector::MultiCollector(deque<Collector> &collectors)
    : cacheScores(numNeedsScores >= 2), collectors(collectors)
{
  int numNeedsScores = 0;
  for (auto collector : collectors) {
    if (collector->needsScores()) {
      numNeedsScores += 1;
    }
  }
}

bool MultiCollector::needsScores()
{
  for (auto collector : collectors) {
    if (collector->needsScores()) {
      return true;
    }
  }
  return false;
}

shared_ptr<LeafCollector> MultiCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  const deque<std::shared_ptr<LeafCollector>> leafCollectors =
      deque<std::shared_ptr<LeafCollector>>();
  for (auto collector : collectors) {
    shared_ptr<LeafCollector> *const leafCollector;
    try {
      leafCollector = collector->getLeafCollector(context);
    } catch (const CollectionTerminatedException &e) {
      // this leaf collector does not need this segment
      continue;
    }
    leafCollectors.push_back(leafCollector);
  }
  switch (leafCollectors.size()) {
  case 0:
    throw make_shared<CollectionTerminatedException>();
  case 1:
    return leafCollectors[0];
  default:
    return make_shared<MultiLeafCollector>(leafCollectors, cacheScores);
  }
}

MultiCollector::MultiLeafCollector::MultiLeafCollector(
    deque<std::shared_ptr<LeafCollector>> &collectors, bool cacheScores)
    : cacheScores(cacheScores),
      collectors(collectors.toArray(
          std::deque<std::shared_ptr<LeafCollector>>(collectors.size())))
{
  this->numCollectors = this->collectors.size();
}

void MultiCollector::MultiLeafCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  if (cacheScores) {
    scorer = make_shared<ScoreCachingWrappingScorer>(scorer);
  }
  for (int i = 0; i < numCollectors; ++i) {
    shared_ptr<LeafCollector> *const c = collectors[i];
    c->setScorer(scorer);
  }
}

void MultiCollector::MultiLeafCollector::removeCollector(int i)
{
  System::arraycopy(collectors, i + 1, collectors, i, numCollectors - i - 1);
  --numCollectors;
  collectors[numCollectors].reset();
}

void MultiCollector::MultiLeafCollector::collect(int doc) 
{
  std::deque<std::shared_ptr<LeafCollector>> collectors = this->collectors;
  int numCollectors = this->numCollectors;
  for (int i = 0; i < numCollectors;) {
    shared_ptr<LeafCollector> *const collector = collectors[i];
    try {
      collector->collect(doc);
      ++i;
    } catch (const CollectionTerminatedException &e) {
      removeCollector(i);
      numCollectors = this->numCollectors;
      if (numCollectors == 0) {
        throw make_shared<CollectionTerminatedException>();
      }
    }
  }
}
} // namespace org::apache::lucene::search