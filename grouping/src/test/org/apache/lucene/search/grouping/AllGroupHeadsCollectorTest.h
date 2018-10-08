#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Bits.h"

#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/search/grouping/GroupDoc.h"
#include  "core/src/java/org/apache/lucene/search/SortField.h"
namespace org::apache::lucene::search::grouping
{
template <typename T>
class AllGroupHeadsCollector;
}
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
using DocValuesType = org::apache::lucene::index::DocValuesType;
using Sort = org::apache::lucene::search::Sort;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class AllGroupHeadsCollectorTest : public LuceneTestCase
{
  GET_CLASS_NAME(AllGroupHeadsCollectorTest)

public:
  virtual void testBasic() ;

  virtual void testRandom() ;

private:
  bool arrayContains(std::deque<int> &expected, std::deque<int> &actual);

  bool openBitSetContains(std::deque<int> &expectedDocs,
                          std::shared_ptr<Bits> actual,
                          int maxDoc) ;

  std::deque<int>
  createExpectedGroupHeads(const std::wstring &searchTerm,
                           std::deque<std::shared_ptr<GroupDoc>> &groupDocs,
                           std::shared_ptr<Sort> docSort, bool sortByScoreOnly,
                           std::deque<int> &fieldIdToDocID);

  std::shared_ptr<Sort> getRandomSort(bool scoreOnly);

  std::shared_ptr<Comparator<std::shared_ptr<GroupDoc>>>
  getComparator(std::shared_ptr<Sort> sort, bool const sortByScoreOnly,
                std::deque<int> &fieldIdToDocID);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<GroupDoc>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<AllGroupHeadsCollectorTest> outerInstance;

    bool sortByScoreOnly = false;
    std::deque<int> fieldIdToDocID;
    std::deque<std::shared_ptr<SortField>> sortFields;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<AllGroupHeadsCollectorTest> outerInstance,
        bool sortByScoreOnly, std::deque<int> &fieldIdToDocID,
        std::deque<std::shared_ptr<SortField>> &sortFields);

    int compare(std::shared_ptr<GroupDoc> d1, std::shared_ptr<GroupDoc> d2);
  };

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private
  // AllGroupHeadsCollector<?> createRandomCollector(std::wstring groupField,
  // org.apache.lucene.search.Sort sortWithinGroup) C++ TODO: Java wildcard
  // generics are not converted to C++:
  std::shared_ptr < AllGroupHeadsCollector <
      ? >> createRandomCollector(const std::wstring &groupField,
                                 std::shared_ptr<Sort> sortWithinGroup);

  void addGroupField(std::shared_ptr<Document> doc,
                     const std::wstring &groupField, const std::wstring &value,
                     DocValuesType valueType);

private:
  class GroupDoc : public std::enable_shared_from_this<GroupDoc>
  {
    GET_CLASS_NAME(GroupDoc)
  public:
    const int id;
    const std::shared_ptr<BytesRef> group;
    const std::shared_ptr<BytesRef> sort1;
    const std::shared_ptr<BytesRef> sort2;
    const std::shared_ptr<BytesRef> sort3;
    // content must be "realN ..."
    const std::wstring content;
    float score = 0;

    GroupDoc(int id, std::shared_ptr<BytesRef> group,
             std::shared_ptr<BytesRef> sort1, std::shared_ptr<BytesRef> sort2,
             std::shared_ptr<BytesRef> sort3, const std::wstring &content);
  };

protected:
  std::shared_ptr<AllGroupHeadsCollectorTest> shared_from_this()
  {
    return std::static_pointer_cast<AllGroupHeadsCollectorTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
