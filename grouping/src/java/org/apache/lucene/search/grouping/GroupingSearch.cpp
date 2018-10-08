using namespace std;

#include "GroupingSearch.h"

namespace org::apache::lucene::search::grouping
{
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using CachingCollector = org::apache::lucene::search::CachingCollector;
using Collector = org::apache::lucene::search::Collector;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiCollector = org::apache::lucene::search::MultiCollector;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Weight = org::apache::lucene::search::Weight;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;

GroupingSearch::GroupingSearch(const wstring &groupField)
    : GroupingSearch(new TermGroupSelector(groupField), nullptr)
{
}

template <typename T1, typename T1>
GroupingSearch::GroupingSearch(shared_ptr<ValueSource> groupFunction,
                               unordered_map<T1> valueSourceContext)
    : GroupingSearch(
          new ValueSourceGroupSelector(groupFunction, valueSourceContext),
          nullptr)
{
}

GroupingSearch::GroupingSearch(shared_ptr<Query> groupEndDocs)
    : GroupingSearch(nullptr, groupEndDocs)
{
}

GroupingSearch::GroupingSearch(shared_ptr<GroupSelector> grouper,
                               shared_ptr<Query> groupEndDocs)
    : grouper(grouper), groupEndDocs(groupEndDocs)
{
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public <T> TopGroups<T>
// search(org.apache.lucene.search.IndexSearcher searcher,
// org.apache.lucene.search.Query query, int groupOffset, int groupLimit) throws
// java.io.IOException
shared_ptr<TopGroups<T>>
GroupingSearch::search(shared_ptr<IndexSearcher> searcher,
                       shared_ptr<Query> query, int groupOffset,
                       int groupLimit) 
{
  if (grouper != nullptr) {
    return groupByFieldOrFunction(searcher, query, groupOffset, groupLimit);
  } else if (groupEndDocs != nullptr) {
    return std::static_pointer_cast<TopGroups<T>>(
        groupByDocBlock(searcher, query, groupOffset, groupLimit));
  } else {
    throw make_shared<
        IllegalStateException>(L"Either groupField, groupFunction or "
                               L"groupEndDocs must be set."); // This can't
                                                              // happen...
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) protected
// TopGroups groupByFieldOrFunction(org.apache.lucene.search.IndexSearcher
// searcher, org.apache.lucene.search.Query query, int groupOffset, int
// groupLimit) throws java.io.IOException
shared_ptr<TopGroups>
GroupingSearch::groupByFieldOrFunction(shared_ptr<IndexSearcher> searcher,
                                       shared_ptr<Query> query, int groupOffset,
                                       int groupLimit) 
{
  int topN = groupOffset + groupLimit;

  shared_ptr<FirstPassGroupingCollector> *const firstPassCollector =
      make_shared<FirstPassGroupingCollector>(grouper, groupSort, topN);
  shared_ptr<AllGroupsCollector> *const allGroupsCollector =
      allGroups ? make_shared<AllGroupsCollector>(grouper) : nullptr;
  shared_ptr<AllGroupHeadsCollector> *const allGroupHeadsCollector =
      allGroupHeads
          ? AllGroupHeadsCollector::newCollector(grouper, sortWithinGroup)
          : nullptr;

  shared_ptr<Collector> *const firstRound = MultiCollector::wrap(
      {firstPassCollector, allGroupsCollector, allGroupHeadsCollector});

  shared_ptr<CachingCollector> cachedCollector = nullptr;
  if (maxCacheRAMMB || maxDocsToCache) {
    if (maxCacheRAMMB) {
      cachedCollector =
          CachingCollector::create(firstRound, cacheScores, maxCacheRAMMB);
    } else {
      cachedCollector =
          CachingCollector::create(firstRound, cacheScores, maxDocsToCache);
    }
    searcher->search(query, cachedCollector);
  } else {
    searcher->search(query, firstRound);
  }

  matchingGroups =
      allGroups ? allGroupsCollector->getGroups() : Collections::emptyList();
  matchingGroupHeads = allGroupHeads
                           ? allGroupHeadsCollector->retrieveGroupHeads(
                                 searcher->getIndexReader()->maxDoc())
                           : make_shared<Bits::MatchNoBits>(
                                 searcher->getIndexReader()->maxDoc());

  shared_ptr<deque<std::shared_ptr<SearchGroup>>> topSearchGroups =
      firstPassCollector->getTopGroups(groupOffset, fillSortFields);
  if (topSearchGroups == nullptr) {
    return make_shared<TopGroups>(std::deque<std::shared_ptr<SortField>>(0),
                                  std::deque<std::shared_ptr<SortField>>(0), 0,
                                  0, std::deque<std::shared_ptr<GroupDocs>>(0),
                                  NAN);
  }

  int topNInsideGroup = groupDocsOffset + groupDocsLimit;
  shared_ptr<TopGroupsCollector> secondPassCollector =
      make_shared<TopGroupsCollector>(
          grouper, topSearchGroups, groupSort, sortWithinGroup, topNInsideGroup,
          includeScores, includeMaxScore, fillSortFields);

  if (cachedCollector != nullptr && cachedCollector->isCached()) {
    cachedCollector->replay(secondPassCollector);
  } else {
    searcher->search(query, secondPassCollector);
  }

  if (allGroups) {
    return make_shared<TopGroups>(
        secondPassCollector->getTopGroups(groupDocsOffset),
        matchingGroups->size());
  } else {
    return secondPassCollector->getTopGroups(groupDocsOffset);
  }
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: protected TopGroups<?>
// groupByDocBlock(org.apache.lucene.search.IndexSearcher searcher,
// org.apache.lucene.search.Query query, int groupOffset, int groupLimit) throws
// java.io.IOException
shared_ptr < TopGroups <
    ? >> GroupingSearch::groupByDocBlock(shared_ptr<IndexSearcher> searcher,
                                         shared_ptr<Query> query,
                                         int groupOffset,
                                         int groupLimit) 
{
  int topN = groupOffset + groupLimit;
  shared_ptr<Query> *const endDocsQuery = searcher->rewrite(this->groupEndDocs);
  shared_ptr<Weight> *const groupEndDocs =
      searcher->createWeight(endDocsQuery, false, 1);
  shared_ptr<BlockGroupingCollector> c = make_shared<BlockGroupingCollector>(
      groupSort, topN, includeScores, groupEndDocs);
  searcher->search(query, c);
  int topNInsideGroup = groupDocsOffset + groupDocsLimit;
  return c->getTopGroups(sortWithinGroup, groupOffset, groupDocsOffset,
                         topNInsideGroup, fillSortFields);
}

shared_ptr<GroupingSearch> GroupingSearch::setCachingInMB(double maxCacheRAMMB,
                                                          bool cacheScores)
{
  this->maxCacheRAMMB = maxCacheRAMMB;
  this->maxDocsToCache = nullopt;
  this->cacheScores = cacheScores;
  return shared_from_this();
}

shared_ptr<GroupingSearch> GroupingSearch::setCaching(int maxDocsToCache,
                                                      bool cacheScores)
{
  this->maxDocsToCache = maxDocsToCache;
  this->maxCacheRAMMB = nullopt;
  this->cacheScores = cacheScores;
  return shared_from_this();
}

shared_ptr<GroupingSearch> GroupingSearch::disableCaching()
{
  this->maxCacheRAMMB = nullopt;
  this->maxDocsToCache = nullopt;
  return shared_from_this();
}

shared_ptr<GroupingSearch>
GroupingSearch::setGroupSort(shared_ptr<Sort> groupSort)
{
  this->groupSort = groupSort;
  return shared_from_this();
}

shared_ptr<GroupingSearch>
GroupingSearch::setSortWithinGroup(shared_ptr<Sort> sortWithinGroup)
{
  this->sortWithinGroup = sortWithinGroup;
  return shared_from_this();
}

shared_ptr<GroupingSearch>
GroupingSearch::setGroupDocsOffset(int groupDocsOffset)
{
  this->groupDocsOffset = groupDocsOffset;
  return shared_from_this();
}

shared_ptr<GroupingSearch> GroupingSearch::setGroupDocsLimit(int groupDocsLimit)
{
  this->groupDocsLimit = groupDocsLimit;
  return shared_from_this();
}

shared_ptr<GroupingSearch>
GroupingSearch::setFillSortFields(bool fillSortFields)
{
  this->fillSortFields = fillSortFields;
  return shared_from_this();
}

shared_ptr<GroupingSearch> GroupingSearch::setIncludeScores(bool includeScores)
{
  this->includeScores = includeScores;
  return shared_from_this();
}

shared_ptr<GroupingSearch>
GroupingSearch::setIncludeMaxScore(bool includeMaxScore)
{
  this->includeMaxScore = includeMaxScore;
  return shared_from_this();
}

shared_ptr<GroupingSearch> GroupingSearch::setAllGroups(bool allGroups)
{
  this->allGroups = allGroups;
  return shared_from_this();
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) public <T>
// java.util.deque<T> getAllMatchingGroups()
shared_ptr<deque<T>> GroupingSearch::getAllMatchingGroups()
{
  return std::static_pointer_cast<deque<T>>(matchingGroups);
}

shared_ptr<GroupingSearch> GroupingSearch::setAllGroupHeads(bool allGroupHeads)
{
  this->allGroupHeads = allGroupHeads;
  return shared_from_this();
}

shared_ptr<Bits> GroupingSearch::getAllGroupHeads()
{
  return matchingGroupHeads;
}
} // namespace org::apache::lucene::search::grouping