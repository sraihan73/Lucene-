#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/grouping/NullComparator.h"

namespace org::apache::lucene::search::grouping
{
template <typename Ttypename R>
class DistinctValuesCollector;
}
namespace org::apache::lucene::search::grouping
{
template <typename Ttypename R>
class GroupCount;
}
#include  "core/src/java/org/apache/lucene/document/Document.h"
namespace org::apache::lucene::search::grouping
{
template <typename T>
class FirstPassGroupingCollector;
}
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/search/grouping/IndexContext.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

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
using Sort = org::apache::lucene::search::Sort;
using Directory = org::apache::lucene::store::Directory;

class DistinctValuesCollectorTest : public AbstractGroupingTestCase
{
  GET_CLASS_NAME(DistinctValuesCollectorTest)

private:
  static const std::shared_ptr<NullComparator> nullComparator;

  static const std::wstring GROUP_FIELD;
  static const std::wstring COUNT_FIELD;

public:
  virtual void testSimple() ;

  virtual void testRandom() ;

private:
  void printGroups(std::deque<DistinctValuesCollector::GroupCount<
                       Comparable<std::any>, Comparable<std::any>>> &results);

  void assertValues(std::any expected, std::any actual);

  void compare(const std::wstring &expected, std::any groupValue);

  void compareNull(std::any groupValue);

  void addField(std::shared_ptr<Document> doc, const std::wstring &field,
                const std::wstring &value);

  template <typename T, typename R>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private <T
  // extends Comparable<Object>, R extends Comparable<Object>>
  // DistinctValuesCollector<T, R>
  // createDistinctCountCollector(FirstPassGroupingCollector<T>
  // firstPassGroupingCollector, std::wstring countField) throws java.io.IOException
  std::shared_ptr<DistinctValuesCollector<T, R>> createDistinctCountCollector(
      std::shared_ptr<FirstPassGroupingCollector<T>> firstPassGroupingCollector,
      const std::wstring &countField) ;

  template <typename T>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private <T>
  // FirstPassGroupingCollector<T>
  // createRandomFirstPassCollector(org.apache.lucene.search.Sort groupSort,
  // std::wstring groupField, int topNGroups) throws java.io.IOException
  std::shared_ptr<FirstPassGroupingCollector<T>>
  createRandomFirstPassCollector(std::shared_ptr<Sort> groupSort,
                                 const std::wstring &groupField,
                                 int topNGroups) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private
  // java.util.List<DistinctValuesCollector.GroupCount<Comparable<Object>,
  // Comparable<Object>>> createExpectedResult(IndexContext context, std::wstring term,
  // org.apache.lucene.search.Sort groupSort, int topN)
  std::deque<DistinctValuesCollector::GroupCount<Comparable<std::any>,
                                                  Comparable<std::any>>>
  createExpectedResult(std::shared_ptr<IndexContext> context,
                       const std::wstring &term,
                       std::shared_ptr<Sort> groupSort, int topN);

  std::shared_ptr<IndexContext> createIndexContext() ;

private:
  class IndexContext : public std::enable_shared_from_this<IndexContext>
  {
    GET_CLASS_NAME(IndexContext)

  public:
    const std::shared_ptr<Directory> directory;
    const std::shared_ptr<DirectoryReader> indexReader;
    const std::unordered_map<
        std::wstring, std::unordered_map<std::wstring, Set<std::wstring>>>
        searchTermToGroupCounts;
    std::deque<std::wstring> const contentStrings;

    IndexContext(
        std::shared_ptr<Directory> directory,
        std::shared_ptr<DirectoryReader> indexReader,
        std::unordered_map<std::wstring,
                           std::unordered_map<std::wstring, Set<std::wstring>>>
            &searchTermToGroupCounts,
        std::deque<std::wstring> &contentStrings);
  };

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private static class NullComparator implements
  // java.util.Comparator<Comparable<?>>
  class NullComparator : public std::enable_shared_from_this<NullComparator>,
                         public Comparator < Comparable <
                         ? >>
  {

  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Override @SuppressWarnings({"unchecked","rawtypes"})
    // public int compare(Comparable a, Comparable b)
    int compare(Comparable a, Comparable b) override;
  };

protected:
  std::shared_ptr<DistinctValuesCollectorTest> shared_from_this()
  {
    return std::static_pointer_cast<DistinctValuesCollectorTest>(
        AbstractGroupingTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
