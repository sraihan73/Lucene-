#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

namespace org::apache::lucene::search::grouping
{
template <typename T>
class GroupDocs;
}
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/search/grouping/GroupingSearch.h"

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
using Sort = org::apache::lucene::search::Sort;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class GroupingSearchTest : public LuceneTestCase
{
  GET_CLASS_NAME(GroupingSearchTest)

  // Tests some very basic usages...
public:
  virtual void testBasic() ;

private:
  void addGroupField(std::shared_ptr<Document> doc,
                     const std::wstring &groupField, const std::wstring &value,
                     bool canUseIDV);

  template <typename T1>
  void compareGroupValue(const std::wstring &expected,
                         std::shared_ptr<GroupDocs<T1>> group);

  std::shared_ptr<GroupingSearch>
  createRandomGroupingSearch(const std::wstring &groupField,
                             std::shared_ptr<Sort> groupSort, int docsInGroup,
                             bool canUseIDV);

public:
  virtual void testSetAllGroups() ;

protected:
  std::shared_ptr<GroupingSearchTest> shared_from_this()
  {
    return std::static_pointer_cast<GroupingSearchTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
