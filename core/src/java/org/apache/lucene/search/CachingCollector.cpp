using namespace std;

#include "CachingCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

CachingCollector::CachedScorer::CachedScorer() : Scorer(nullptr) {}

shared_ptr<DocIdSetIterator> CachingCollector::CachedScorer::iterator()
{
  throw make_shared<UnsupportedOperationException>();
}

float CachingCollector::CachedScorer::score() { return score_; }

int CachingCollector::CachedScorer::docID() { return doc; }

CachingCollector::NoScoreCachingCollector::NoScoreCachingCollector(
    shared_ptr<Collector> in_, int maxDocsToCache)
    : CachingCollector(in_)
{
  this->maxDocsToCache = maxDocsToCache;
  contexts = deque<>();
  docs = deque<>();
}

shared_ptr<NoScoreCachingLeafCollector>
CachingCollector::NoScoreCachingCollector::wrap(shared_ptr<LeafCollector> in_,
                                                int maxDocsToCache)
{
  return make_shared<NoScoreCachingLeafCollector>(shared_from_this(), in_,
                                                  maxDocsToCache);
}

shared_ptr<LeafCollector>
CachingCollector::NoScoreCachingCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  postCollection();
  shared_ptr<LeafCollector> *const in_ = this->in_->getLeafCollector(context);
  if (contexts.size() > 0) {
    contexts.push_back(context);
  }
  if (maxDocsToCache >= 0) {
    return lastCollector = wrap(in_, maxDocsToCache);
  } else {
    return in_;
  }
}

void CachingCollector::NoScoreCachingCollector::invalidate()
{
  maxDocsToCache = -1;
  contexts.clear();
  this->docs.clear();
}

void CachingCollector::NoScoreCachingCollector::postCollect(
    shared_ptr<NoScoreCachingLeafCollector> collector)
{
  const std::deque<int> docs = collector->cachedDocs();
  maxDocsToCache -= docs.size();
  this->docs.push_back(docs);
}

void CachingCollector::NoScoreCachingCollector::postCollection()
{
  if (lastCollector != nullptr) {
    if (!lastCollector->hasCache()) {
      invalidate();
    } else {
      postCollect(lastCollector);
    }
    lastCollector.reset();
  }
}

void CachingCollector::NoScoreCachingCollector::collect(
    shared_ptr<LeafCollector> collector, int i) 
{
  const std::deque<int> docs = this->docs[i];
  for (auto doc : docs) {
    collector->collect(doc);
  }
}

void CachingCollector::NoScoreCachingCollector::replay(
    shared_ptr<Collector> other) 
{
  postCollection();
  if (!isCached()) {
    throw make_shared<IllegalStateException>(
        L"cannot replay: cache was cleared because too much RAM was required");
  }
  assert(docs.size() == contexts.size());
  for (int i = 0; i < contexts.size(); ++i) {
    shared_ptr<LeafReaderContext> *const context = contexts[i];
    shared_ptr<LeafCollector> *const collector =
        other->getLeafCollector(context);
    collect(collector, i);
  }
}

CachingCollector::ScoreCachingCollector::ScoreCachingCollector(
    shared_ptr<Collector> in_, int maxDocsToCache)
    : NoScoreCachingCollector(in_, maxDocsToCache)
{
  scores = deque<>();
}

shared_ptr<NoScoreCachingLeafCollector>
CachingCollector::ScoreCachingCollector::wrap(shared_ptr<LeafCollector> in_,
                                              int maxDocsToCache)
{
  return make_shared<ScoreCachingLeafCollector>(shared_from_this(), in_,
                                                maxDocsToCache);
}

void CachingCollector::ScoreCachingCollector::postCollect(
    shared_ptr<NoScoreCachingLeafCollector> collector)
{
  shared_ptr<ScoreCachingLeafCollector> *const coll =
      std::static_pointer_cast<ScoreCachingLeafCollector>(collector);
  NoScoreCachingCollector::postCollect(coll);
  scores.push_back(coll->cachedScores());
}

bool CachingCollector::ScoreCachingCollector::needsScores() { return true; }

void CachingCollector::ScoreCachingCollector::collect(
    shared_ptr<LeafCollector> collector, int i) 
{
  const std::deque<int> docs = this->docs[i];
  const std::deque<float> scores = this->scores[i];
  assert(docs.size() == scores.size());
  shared_ptr<CachedScorer> *const scorer = make_shared<CachedScorer>();
  collector->setScorer(scorer);
  for (int j = 0; j < docs.size(); ++j) {
    scorer->doc = docs[j];
    scorer->score_ = scores[j];
    collector->collect(scorer->doc);
  }
}

CachingCollector::NoScoreCachingLeafCollector::NoScoreCachingLeafCollector(
    shared_ptr<CachingCollector> outerInstance, shared_ptr<LeafCollector> in_,
    int maxDocsToCache)
    : FilterLeafCollector(in_), maxDocsToCache(maxDocsToCache),
      outerInstance(outerInstance)
{
  docs = std::deque<int>(min(maxDocsToCache, INITIAL_ARRAY_SIZE));
  docCount = 0;
}

void CachingCollector::NoScoreCachingLeafCollector::grow(int newLen)
{
  docs = Arrays::copyOf(docs, newLen);
}

void CachingCollector::NoScoreCachingLeafCollector::invalidate()
{
  docs.clear();
  docCount = -1;
  outerInstance->cached = false;
}

void CachingCollector::NoScoreCachingLeafCollector::buffer(int doc) throw(
    IOException)
{
  docs[docCount] = doc;
}

void CachingCollector::NoScoreCachingLeafCollector::collect(int doc) throw(
    IOException)
{
  if (docs.size() > 0) {
    if (docCount >= docs.size()) {
      if (docCount >= maxDocsToCache) {
        invalidate();
      } else {
        constexpr int newLen = min(
            ArrayUtil::oversize(docCount + 1, Integer::BYTES), maxDocsToCache);
        grow(newLen);
      }
    }
    if (docs.size() > 0) {
      buffer(doc);
      ++docCount;
    }
  }
  FilterLeafCollector::collect(doc);
}

bool CachingCollector::NoScoreCachingLeafCollector::hasCache()
{
  return docs.size() > 0;
}

std::deque<int> CachingCollector::NoScoreCachingLeafCollector::cachedDocs()
{
  return docs.empty() ? nullptr : Arrays::copyOf(docs, docCount);
}

CachingCollector::ScoreCachingLeafCollector::ScoreCachingLeafCollector(
    shared_ptr<CachingCollector> outerInstance, shared_ptr<LeafCollector> in_,
    int maxDocsToCache)
    : NoScoreCachingLeafCollector(outerInstance, in_, maxDocsToCache),
      outerInstance(outerInstance)
{
  scores = std::deque<float>(docs.size());
}

void CachingCollector::ScoreCachingLeafCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
  NoScoreCachingLeafCollector::setScorer(scorer);
}

void CachingCollector::ScoreCachingLeafCollector::grow(int newLen)
{
  NoScoreCachingLeafCollector::grow(newLen);
  scores = Arrays::copyOf(scores, newLen);
}

void CachingCollector::ScoreCachingLeafCollector::invalidate()
{
  NoScoreCachingLeafCollector::invalidate();
  scores.clear();
}

void CachingCollector::ScoreCachingLeafCollector::buffer(int doc) throw(
    IOException)
{
  NoScoreCachingLeafCollector::buffer(doc);
  scores[docCount] = scorer->score();
}

std::deque<float> CachingCollector::ScoreCachingLeafCollector::cachedScores()
{
  return docs.empty() ? nullptr : Arrays::copyOf(scores, docCount);
}

shared_ptr<CachingCollector> CachingCollector::create(bool cacheScores,
                                                      double maxRAMMB)
{
  shared_ptr<Collector> other =
      make_shared<SimpleCollectorAnonymousInnerClass>();
  return create(other, cacheScores, maxRAMMB);
}

CachingCollector::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass()
{
}

void CachingCollector::SimpleCollectorAnonymousInnerClass::collect(int doc) {}

bool CachingCollector::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

shared_ptr<CachingCollector>
CachingCollector::create(shared_ptr<Collector> other, bool cacheScores,
                         double maxRAMMB)
{
  int bytesPerDoc = Integer::BYTES;
  if (cacheScores) {
    bytesPerDoc += Float::BYTES;
  }
  constexpr int maxDocsToCache =
      static_cast<int>((maxRAMMB * 1024 * 1024) / bytesPerDoc);
  return create(other, cacheScores, maxDocsToCache);
}

shared_ptr<CachingCollector>
CachingCollector::create(shared_ptr<Collector> other, bool cacheScores,
                         int maxDocsToCache)
{
  return cacheScores
             ? make_shared<ScoreCachingCollector>(other, maxDocsToCache)
             : make_shared<NoScoreCachingCollector>(other, maxDocsToCache);
}

CachingCollector::CachingCollector(shared_ptr<Collector> in_)
    : FilterCollector(in_)
{
  cached = true;
}

bool CachingCollector::isCached() { return cached; }
} // namespace org::apache::lucene::search