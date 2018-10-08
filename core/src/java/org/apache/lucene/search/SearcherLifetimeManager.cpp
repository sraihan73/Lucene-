using namespace std;

#include "SearcherLifetimeManager.h"

namespace org::apache::lucene::search
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using IOUtils = org::apache::lucene::util::IOUtils;

SearcherLifetimeManager::SearcherTracker::SearcherTracker(
    shared_ptr<IndexSearcher> searcher)
    : searcher(searcher), recordTimeSec(System::nanoTime() / NANOS_PER_SEC),
      version((std::static_pointer_cast<DirectoryReader>(
                   searcher->getIndexReader()))
                  ->getVersion())
{
  searcher->getIndexReader()->incRef();
  // Use nanoTime not currentTimeMillis since it [in
  // theory] reduces risk from clock shift
}

int SearcherLifetimeManager::SearcherTracker::compareTo(
    shared_ptr<SearcherTracker> other)
{
  return Double::compare(other->recordTimeSec, recordTimeSec);
}

// C++ WARNING: The following method was originally marked 'synchronized':
SearcherLifetimeManager::SearcherTracker::~SearcherTracker()
{
  searcher->getIndexReader()->decRef();
}

void SearcherLifetimeManager::ensureOpen()
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(
        L"this SearcherLifetimeManager instance is closed");
  }
}

int64_t SearcherLifetimeManager::record(
    shared_ptr<IndexSearcher> searcher) 
{
  ensureOpen();
  // TODO: we don't have to use IR.getVersion to track;
  // could be risky (if it's buggy); we could get better
  // bug isolation if we assign our own private ID:
  constexpr int64_t version =
      (std::static_pointer_cast<DirectoryReader>(searcher->getIndexReader()))
          ->getVersion();
  shared_ptr<SearcherTracker> tracker = searchers->get(version);
  if (tracker == nullptr) {
    // System.out.println("RECORD version=" + version + " ms=" +
    // System.currentTimeMillis());
    tracker = make_shared<SearcherTracker>(searcher);
    if (searchers->putIfAbsent(version, tracker) != nullptr) {
      // Another thread beat us -- must decRef to undo
      // incRef done by SearcherTracker ctor:
      delete tracker;
    }
  } else if (tracker->searcher != searcher) {
    throw invalid_argument(
        L"the provided searcher has the same underlying reader version yet the "
        L"searcher instance differs from before (new=" +
        searcher + L" vs old=" + tracker->searcher);
  }

  return version;
}

shared_ptr<IndexSearcher> SearcherLifetimeManager::acquire(int64_t version)
{
  ensureOpen();
  shared_ptr<SearcherTracker> *const tracker = searchers->get(version);
  if (tracker != nullptr && tracker->searcher->getIndexReader()->tryIncRef()) {
    return tracker->searcher;
  }

  return nullptr;
}

void SearcherLifetimeManager::release(shared_ptr<IndexSearcher> s) throw(
    IOException)
{
  s->getIndexReader()->decRef();
}

SearcherLifetimeManager::PruneByAge::PruneByAge(double maxAgeSec)
    : maxAgeSec(maxAgeSec)
{
  if (maxAgeSec < 0) {
    throw invalid_argument(L"maxAgeSec must be > 0 (got " +
                           to_wstring(maxAgeSec) + L")");
  }
}

bool SearcherLifetimeManager::PruneByAge::doPrune(
    double ageSec, shared_ptr<IndexSearcher> searcher)
{
  return ageSec > maxAgeSec;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SearcherLifetimeManager::prune(shared_ptr<Pruner> pruner) throw(
    IOException)
{
  // Cannot just pass searchers.values() to ArrayList ctor
  // (not thread-safe since the values can change while
  // ArrayList is init'ing itself); must instead iterate
  // ourselves:
  const deque<std::shared_ptr<SearcherTracker>> trackers =
      deque<std::shared_ptr<SearcherTracker>>();
  for (auto tracker : searchers) {
    trackers.push_back(tracker->second);
  }
  sort(trackers.begin(), trackers.end());
  double lastRecordTimeSec = 0.0;
  constexpr double now = System::nanoTime() / NANOS_PER_SEC;
  for (auto tracker : trackers) {
    constexpr double ageSec;
    if (lastRecordTimeSec == 0.0) {
      ageSec = 0.0;
    } else {
      ageSec = now - lastRecordTimeSec;
    }
    // First tracker is always age 0.0 sec, since it's
    // still "live"; second tracker's age (= seconds since
    // it was "live") is now minus first tracker's
    // recordTime, etc:
    if (pruner->doPrune(ageSec, tracker->searcher)) {
      // System.out.println("PRUNE version=" + tracker.version + " age=" +
      // ageSec + " ms=" + System.currentTimeMillis());
      searchers->remove(tracker->version);
      tracker->close();
    }
    lastRecordTimeSec = tracker->recordTimeSec;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
SearcherLifetimeManager::~SearcherLifetimeManager()
{
  closed = true;
  const deque<std::shared_ptr<SearcherTracker>> toClose =
      deque<std::shared_ptr<SearcherTracker>>(searchers->values());

  // Remove up front in case exc below, so we don't
  // over-decRef on double-close:
  for (auto tracker : toClose) {
    searchers->remove(tracker->version);
  }

  IOUtils::close(toClose);

  // Make some effort to catch mis-use:
  if (searchers->size() != 0) {
    throw make_shared<IllegalStateException>(
        L"another thread called record while this SearcherLifetimeManager "
        L"instance was being closed; not all searchers were closed");
  }
}
} // namespace org::apache::lucene::search