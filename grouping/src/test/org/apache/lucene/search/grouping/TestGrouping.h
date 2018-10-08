#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/search/Sort.h"
namespace org::apache::lucene::search::grouping
{
template <typename T>
class FirstPassGroupingCollector;
}
namespace org::apache::lucene::search::grouping
{
template <typename T>
class TopGroupsCollector;
}
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
namespace org::apache::lucene::search::grouping
{
template <typename T>
class SearchGroup;
}
namespace org::apache::lucene::search::grouping
{
template <typename T>
class AllGroupsCollector;
}
namespace org::apache::lucene::search::grouping
{
template <typename T>
class GroupDocs;
}
namespace org::apache::lucene::search::grouping
{
template <typename T>
class TopGroups;
}
#include  "core/src/java/org/apache/lucene/search/SortField.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/grouping/ShardSearcher.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"

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

using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Collector = org::apache::lucene::search::Collector;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO
//   - should test relevance sort too
//   - test null
//   - test ties
//   - test compound sort

class TestGrouping : public LuceneTestCase
{
  GET_CLASS_NAME(TestGrouping)

public:
  virtual void testBasic() ;

private:
  void addGroupField(std::shared_ptr<Document> doc,
                     const std::wstring &groupField, const std::wstring &value);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private FirstPassGroupingCollector<?>
  // createRandomFirstPassCollector(std::wstring groupField,
  // org.apache.lucene.search.Sort groupSort, int topDocs) throws
  // java.io.IOException
  std::shared_ptr < FirstPassGroupingCollector <
      ? >> createRandomFirstPassCollector(const std::wstring &groupField,
                                          std::shared_ptr<Sort> groupSort,
                                          int topDocs) ;

  template <typename T1>
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: private FirstPassGroupingCollector<?>
      // createFirstPassCollector(std::wstring groupField,
      // org.apache.lucene.search.Sort groupSort, int topDocs,
      // FirstPassGroupingCollector<?> firstPassGroupingCollector) throws
      // java.io.IOException
      std::shared_ptr < FirstPassGroupingCollector <
      ? >> createFirstPassCollector(
               const std::wstring &groupField, std::shared_ptr<Sort> groupSort,
               int topDocs,
               std::shared_ptr<FirstPassGroupingCollector<T1>>
                   firstPassGroupingCollector) ;

  template <typename T>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private <T>
  // TopGroupsCollector<T> createSecondPassCollector(FirstPassGroupingCollector
  // firstPassGroupingCollector, org.apache.lucene.search.Sort groupSort,
  // org.apache.lucene.search.Sort sortWithinGroup, int groupOffset, int
  // maxDocsPerGroup, bool getScores, bool getMaxScores, bool
  // fillSortFields) throws java.io.IOException
  std::shared_ptr<TopGroupsCollector<T>> createSecondPassCollector(
      std::shared_ptr<FirstPassGroupingCollector> firstPassGroupingCollector,
      std::shared_ptr<Sort> groupSort, std::shared_ptr<Sort> sortWithinGroup,
      int groupOffset, int maxDocsPerGroup, bool getScores, bool getMaxScores,
      bool fillSortFields) ;

  // Basically converts searchGroups from MutableValue to BytesRef if grouping
  // by ValueSource
  template <typename T1>
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings("unchecked") private
      // TopGroupsCollector<?>
      // createSecondPassCollector(FirstPassGroupingCollector<?>
      // firstPassGroupingCollector, std::wstring groupField,
      // java.util.std::deque<SearchGroup<org.apache.lucene.util.BytesRef>>
      // searchGroups, org.apache.lucene.search.Sort groupSort,
      // org.apache.lucene.search.Sort sortWithinGroup, int maxDocsPerGroup,
      // bool getScores, bool getMaxScores, bool fillSortFields) throws
      // java.io.IOException C++ TODO: Java wildcard generics are not converted
      // to C++:
      std::shared_ptr < TopGroupsCollector <
      ? >> createSecondPassCollector(
               std::shared_ptr<FirstPassGroupingCollector<T1>>
                   firstPassGroupingCollector,
               const std::wstring &groupField,
               std::shared_ptr<
                   std::deque<SearchGroup<std::shared_ptr<BytesRef>>>>
                   searchGroups,
               std::shared_ptr<Sort> groupSort,
               std::shared_ptr<Sort> sortWithinGroup, int maxDocsPerGroup,
               bool getScores, bool getMaxScores,
               bool fillSortFields) ;

  template <typename T1>
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: private AllGroupsCollector<?>
      // createAllGroupsCollector(FirstPassGroupingCollector<?>
      // firstPassGroupingCollector, std::wstring groupField)
      std::shared_ptr < AllGroupsCollector <
      ? >> createAllGroupsCollector(
               std::shared_ptr<FirstPassGroupingCollector<T1>>
                   firstPassGroupingCollector,
               const std::wstring &groupField);

  template <typename T1>
  void compareGroupValue(const std::wstring &expected,
                         std::shared_ptr<GroupDocs<T1>> group);

  template <typename T1>
  std::shared_ptr<std::deque<SearchGroup<std::shared_ptr<BytesRef>>>>
  getSearchGroups(std::shared_ptr<FirstPassGroupingCollector<T1>> c,
                  int groupOffset, bool fillFields) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) private
  // TopGroups<org.apache.lucene.util.BytesRef> getTopGroups(TopGroupsCollector
  // c, int withinGroupOffset)
  std::shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
  getTopGroups(std::shared_ptr<TopGroupsCollector> c, int withinGroupOffset);

private:
  class GroupDoc : public std::enable_shared_from_this<GroupDoc>
  {
    GET_CLASS_NAME(GroupDoc)
  public:
    const int id;
    const std::shared_ptr<BytesRef> group;
    const std::shared_ptr<BytesRef> sort1;
    const std::shared_ptr<BytesRef> sort2;
    // content must be "realN ..."
    const std::wstring content;
    float score = 0;
    float score2 = 0;

    GroupDoc(int id, std::shared_ptr<BytesRef> group,
             std::shared_ptr<BytesRef> sort1, std::shared_ptr<BytesRef> sort2,
             const std::wstring &content);
  };

private:
  std::shared_ptr<Sort> getRandomSort();

  std::shared_ptr<Comparator<std::shared_ptr<GroupDoc>>>
  getComparator(std::shared_ptr<Sort> sort);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<GroupDoc>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestGrouping> outerInstance;

    std::deque<std::shared_ptr<SortField>> sortFields;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestGrouping> outerInstance,
        std::deque<std::shared_ptr<SortField>> &sortFields);

    int compare(std::shared_ptr<GroupDoc> d1, std::shared_ptr<GroupDoc> d2);
  };

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private
  // Comparable<?>[] fillFields(GroupDoc d, org.apache.lucene.search.Sort sort)
  // C++ TODO: Java wildcard generics are not converted to C++:
  std::deque < Comparable <
      ? >> fillFields(std::shared_ptr<GroupDoc> d, std::shared_ptr<Sort> sort);

  std::wstring groupToString(std::shared_ptr<BytesRef> b);

  std::shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
  slowGrouping(std::deque<std::shared_ptr<GroupDoc>> &groupDocs,
               const std::wstring &searchTerm, bool fillFields, bool getScores,
               bool getMaxScores, bool doAllGroups,
               std::shared_ptr<Sort> groupSort, std::shared_ptr<Sort> docSort,
               int topNGroups, int docsPerGroup, int groupOffset,
               int docOffset);

  std::shared_ptr<DirectoryReader> getDocBlockReader(
      std::shared_ptr<Directory> dir,
      std::deque<std::shared_ptr<GroupDoc>> &groupDocs) ;

private:
  class ShardState : public std::enable_shared_from_this<ShardState>
  {
    GET_CLASS_NAME(ShardState)

  public:
    std::deque<std::shared_ptr<ShardSearcher>> const subSearchers;
    std::deque<int> const docStarts;

    ShardState(std::shared_ptr<IndexSearcher> s);
  };

public:
  virtual void testRandom() ;

private:
  void
  verifyShards(std::deque<int> &docStarts,
               std::shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> topGroups);

  std::shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
  searchShards(std::shared_ptr<IndexSearcher> topSearcher,
               std::deque<std::shared_ptr<ShardSearcher>> &subSearchers,
               std::shared_ptr<Query> query, std::shared_ptr<Sort> groupSort,
               std::shared_ptr<Sort> docSort, int groupOffset, int topNGroups,
               int docOffset, int topNDocs, bool getScores, bool getMaxScores,
               bool canUseIDV, bool preFlex) ;

  void
  assertEquals(std::deque<int> &docIDtoID,
               std::shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> expected,
               std::shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> actual,
               bool verifyGroupValues, bool verifyTotalGroupCount,
               bool verifySortValues, bool testScores, bool idvBasedImplsUsed);

private:
  class ShardSearcher : public IndexSearcher
  {
    GET_CLASS_NAME(ShardSearcher)
  private:
    const std::deque<std::shared_ptr<LeafReaderContext>> ctx;

  public:
    ShardSearcher(std::shared_ptr<LeafReaderContext> ctx,
                  std::shared_ptr<IndexReaderContext> parent);

    virtual void
    search(std::shared_ptr<Weight> weight,
           std::shared_ptr<Collector> collector) ;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<ShardSearcher> shared_from_this()
    {
      return std::static_pointer_cast<ShardSearcher>(
          org.apache.lucene.search.IndexSearcher::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestGrouping> shared_from_this()
  {
    return std::static_pointer_cast<TestGrouping>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
