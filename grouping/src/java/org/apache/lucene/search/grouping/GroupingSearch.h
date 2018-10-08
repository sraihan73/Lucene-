#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::grouping
{
template <typename T>
class GroupSelector;
}

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
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

using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using Bits = org::apache::lucene::util::Bits;

/**
 * Convenience class to perform grouping in a non distributed environment.
 *
 * @lucene.experimental
 */
class GroupingSearch : public std::enable_shared_from_this<GroupingSearch>
{
  GET_CLASS_NAME(GroupingSearch)

private:
  const std::shared_ptr<GroupSelector> grouper;
  const std::shared_ptr<Query> groupEndDocs;

  std::shared_ptr<Sort> groupSort = Sort::RELEVANCE;
  std::shared_ptr<Sort> sortWithinGroup = Sort::RELEVANCE;

  int groupDocsOffset = 0;
  int groupDocsLimit = 1;
  bool fillSortFields = false;
  bool includeScores = true;
  bool includeMaxScore = true;

  std::optional<double> maxCacheRAMMB;
  std::optional<int> maxDocsToCache;
  bool cacheScores = false;
  bool allGroups = false;
  bool allGroupHeads = false;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private java.util.std::deque<?> matchingGroups;
  std::shared_ptr < std::deque < ? >> matchingGroups;
  std::shared_ptr<Bits> matchingGroupHeads;

  /**
   * Constructs a <code>GroupingSearch</code> instance that groups documents by
   * index terms using DocValues. The group field can only have one token per
   * document. This means that the field must not be analysed.
   *
   * @param groupField The name of the field to group by.
   */
public:
  GroupingSearch(const std::wstring &groupField);

  /**
   * Constructs a <code>GroupingSearch</code> instance that groups documents by
   * function using a {@link ValueSource} instance.
   *
   * @param groupFunction      The function to group by specified as {@link
   * ValueSource}
   * @param valueSourceContext The context of the specified groupFunction
   */
  template <typename T1, typename T1>
  GroupingSearch(std::shared_ptr<ValueSource> groupFunction,
                 std::unordered_map<T1> valueSourceContext);

  /**
   * Constructor for grouping documents by doc block.
   * This constructor can only be used when documents belonging in a group are
   * indexed in one block.
   *
   * @param groupEndDocs The query that marks the last document in all doc
   * blocks
   */
  GroupingSearch(std::shared_ptr<Query> groupEndDocs);

private:
  GroupingSearch(std::shared_ptr<GroupSelector> grouper,
                 std::shared_ptr<Query> groupEndDocs);

  /**
   * Executes a grouped search. Both the first pass and second pass are executed
   * on the specified searcher.
   *
   * @param searcher    The {@link org.apache.lucene.search.IndexSearcher}
   * instance to execute the grouped search on.
   * @param query       The query to execute with the grouping
   * @param groupOffset The group offset
   * @param groupLimit  The number of groups to return from the specified group
   * offset
   * @return the grouped result as a {@link TopGroups} instance
   * @throws IOException If any I/O related errors occur
   */
public:
  template <typename T>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public <T> TopGroups<T>
  // search(org.apache.lucene.search.IndexSearcher searcher,
  // org.apache.lucene.search.Query query, int groupOffset, int groupLimit)
  // throws java.io.IOException
  std::shared_ptr<TopGroups<T>>
  search(std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> query,
         int groupOffset, int groupLimit) ;

protected:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) protected
  // TopGroups groupByFieldOrFunction(org.apache.lucene.search.IndexSearcher
  // searcher, org.apache.lucene.search.Query query, int groupOffset, int
  // groupLimit) throws java.io.IOException
  virtual std::shared_ptr<TopGroups>
  groupByFieldOrFunction(std::shared_ptr<IndexSearcher> searcher,
                         std::shared_ptr<Query> query, int groupOffset,
                         int groupLimit) ;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: protected TopGroups<?>
  // groupByDocBlock(org.apache.lucene.search.IndexSearcher searcher,
  // org.apache.lucene.search.Query query, int groupOffset, int groupLimit)
  // throws java.io.IOException
  virtual std::shared_ptr < TopGroups <
      ? >> groupByDocBlock(std::shared_ptr<IndexSearcher> searcher,
                           std::shared_ptr<Query> query, int groupOffset,
                           int groupLimit) ;

  /**
   * Enables caching for the second pass search. The cache will not grow over a
   * specified limit in MB. The cache is filled during the first pass searched
   * and then replayed during the second pass searched. If the cache grows
   * beyond the specified limit, then the cache is purged and not used in the
   * second pass search.
   *
   * @param maxCacheRAMMB The maximum amount in MB the cache is allowed to hold
   * @param cacheScores   Whether to cache the scores
   * @return <code>this</code>
   */
public:
  virtual std::shared_ptr<GroupingSearch> setCachingInMB(double maxCacheRAMMB,
                                                         bool cacheScores);

  /**
   * Enables caching for the second pass search. The cache will not contain more
   * than the maximum specified documents. The cache is filled during the first
   * pass searched and then replayed during the second pass searched. If the
   * cache grows beyond the specified limit, then the cache is purged and not
   * used in the second pass search.
   *
   * @param maxDocsToCache The maximum number of documents the cache is allowed
   * to hold
   * @param cacheScores    Whether to cache the scores
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch> setCaching(int maxDocsToCache,
                                                     bool cacheScores);

  /**
   * Disables any enabled cache.
   *
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch> disableCaching();

  /**
   * Specifies how groups are sorted.
   * Defaults to {@link Sort#RELEVANCE}.
   *
   * @param groupSort The sort for the groups.
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch>
  setGroupSort(std::shared_ptr<Sort> groupSort);

  /**
   * Specified how documents inside a group are sorted.
   * Defaults to {@link Sort#RELEVANCE}.
   *
   * @param sortWithinGroup The sort for documents inside a group
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch>
  setSortWithinGroup(std::shared_ptr<Sort> sortWithinGroup);

  /**
   * Specifies the offset for documents inside a group.
   *
   * @param groupDocsOffset The offset for documents inside a
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch>
  setGroupDocsOffset(int groupDocsOffset);

  /**
   * Specifies the number of documents to return inside a group from the
   * specified groupDocsOffset.
   *
   * @param groupDocsLimit The number of documents to return inside a group
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch> setGroupDocsLimit(int groupDocsLimit);

  /**
   * Whether to also fill the sort fields per returned group and groups docs.
   *
   * @param fillSortFields Whether to also fill the sort fields per returned
   * group and groups docs
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch>
  setFillSortFields(bool fillSortFields);

  /**
   * Whether to include the scores per doc inside a group.
   *
   * @param includeScores Whether to include the scores per doc inside a group
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch> setIncludeScores(bool includeScores);

  /**
   * Whether to include the score of the most relevant document per group.
   *
   * @param includeMaxScore Whether to include the score of the most relevant
   * document per group
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch>
  setIncludeMaxScore(bool includeMaxScore);

  /**
   * Whether to also compute all groups matching the query.
   * This can be used to determine the number of groups, which can be used for
   * accurate pagination. <p> When grouping by doc block the number of groups
   * are automatically included in the {@link TopGroups} and this option doesn't
   * have any influence.
   *
   * @param allGroups to also compute all groups matching the query
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch> setAllGroups(bool allGroups);

  /**
   * If {@link #setAllGroups(bool)} was set to <code>true</code> then all
   * matching groups are returned, otherwise an empty collection is returned.
   *
   * @param <T> The group value type. This can be a {@link BytesRef} or a {@link
   * MutableValue} instance. If grouping by doc block this the group value is
   * always <code>null</code>.
   * @return all matching groups are returned, or an empty collection
   */
  template <typename T>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) public <T>
  // java.util.std::deque<T> getAllMatchingGroups()
  std::shared_ptr<std::deque<T>> getAllMatchingGroups();

  /**
   * Whether to compute all group heads (most relevant document per group)
   * matching the query. <p> This feature isn't enabled when grouping by doc
   * block.
   *
   * @param allGroupHeads Whether to compute all group heads (most relevant
   * document per group) matching the query
   * @return <code>this</code>
   */
  virtual std::shared_ptr<GroupingSearch> setAllGroupHeads(bool allGroupHeads);

  /**
   * Returns the matching group heads if {@link #setAllGroupHeads(bool)} was
   * set to true or an empty bit set.
   *
   * @return The matching group heads if {@link #setAllGroupHeads(bool)} was
   * set to true or an empty bit set
   */
  virtual std::shared_ptr<Bits> getAllGroupHeads();
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
