#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::search::grouping
{
class IndexContext;
}
namespace org::apache::lucene::search::grouping
{
class GroupedFacetResult;
}
namespace org::apache::lucene::search::grouping
{
class TermGroupFacetCollector;
}
namespace org::apache::lucene::search::grouping
{
class GroupFacetCollector;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::store
{
class Directory;
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

using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using Directory = org::apache::lucene::store::Directory;

class GroupFacetCollectorTest : public AbstractGroupingTestCase
{
  GET_CLASS_NAME(GroupFacetCollectorTest)

public:
  virtual void testSimple() ;

  virtual void testMVGroupedFacetingWithDeletes() ;

private:
  void addField(std::shared_ptr<Document> doc, const std::wstring &field,
                const std::wstring &value, bool canUseIDV);

public:
  virtual void testRandom() ;

private:
  std::shared_ptr<IndexContext>
  createIndexContext(bool multipleFacetValuesPerDocument) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::wstring>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<GroupFacetCollectorTest> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<GroupFacetCollectorTest> outerInstance);

    int compare(const std::wstring &a, const std::wstring &b);
  };

private:
  std::shared_ptr<GroupedFacetResult>
  createExpectedFacetResult(const std::wstring &searchTerm,
                            std::shared_ptr<IndexContext> context, int offset,
                            int limit, int minCount, bool const orderByCount,
                            const std::wstring &facetPrefix);

private:
  class ComparatorAnonymousInnerClass2
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
        public Comparator<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
  private:
    std::shared_ptr<GroupFacetCollectorTest> outerInstance;

    bool orderByCount = false;

  public:
    ComparatorAnonymousInnerClass2(
        std::shared_ptr<GroupFacetCollectorTest> outerInstance,
        bool orderByCount);

    int compare(std::shared_ptr<TermGroupFacetCollector::FacetEntry> a,
                std::shared_ptr<TermGroupFacetCollector::FacetEntry> b);
  };

private:
  std::shared_ptr<GroupFacetCollector> createRandomCollector(
      const std::wstring &groupField, const std::wstring &facetField,
      const std::wstring &facetPrefix, bool multipleFacetsPerDocument);

  std::wstring getFromSet(std::shared_ptr<Set<std::wstring>> set, int index);

private:
  class IndexContext : public std::enable_shared_from_this<IndexContext>
  {
    GET_CLASS_NAME(IndexContext)

  public:
    const int numDocs;
    const std::shared_ptr<DirectoryReader> indexReader;
    const std::unordered_map<
        std::wstring, std::unordered_map<std::wstring, Set<std::wstring>>>
        searchTermToFacetGroups;
    const std::shared_ptr<NavigableSet<std::wstring>> facetValues;
    const std::shared_ptr<Directory> dir;
    const int facetWithMostGroups;
    const int numGroups;
    std::deque<std::wstring> const contentStrings;

    IndexContext(
        std::unordered_map<std::wstring,
                           std::unordered_map<std::wstring, Set<std::wstring>>>
            &searchTermToFacetGroups,
        std::shared_ptr<DirectoryReader> r, int numDocs,
        std::shared_ptr<Directory> dir, int facetWithMostGroups, int numGroups,
        std::deque<std::wstring> &contentStrings,
        std::shared_ptr<NavigableSet<std::wstring>> facetValues);
  };

private:
  class GroupedFacetResult
      : public std::enable_shared_from_this<GroupedFacetResult>
  {
    GET_CLASS_NAME(GroupedFacetResult)

  public:
    const int totalCount;
    const int totalMissingCount;
    const std::deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>
        facetEntries;

  private:
    GroupedFacetResult(
        int totalCount, int totalMissingCount,
        std::deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>
            &facetEntries);

  public:
    virtual int getTotalCount();

    virtual int getTotalMissingCount();

    virtual std::deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>
    getFacetEntries();
  };

protected:
  std::shared_ptr<GroupFacetCollectorTest> shared_from_this()
  {
    return std::static_pointer_cast<GroupFacetCollectorTest>(
        AbstractGroupingTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::grouping
