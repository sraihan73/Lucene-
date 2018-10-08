#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::search::grouping
{
template <typename T>
class AllGroupsCollector;
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
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class AllGroupsCollectorTest : public LuceneTestCase
{
  GET_CLASS_NAME(AllGroupsCollectorTest)

public:
  virtual void testTotalGroupCount() ;

private:
  void addGroupField(std::shared_ptr<Document> doc,
                     const std::wstring &groupField, const std::wstring &value);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private AllGroupsCollector<?> createRandomCollector(std::wstring
  // groupField)
  std::shared_ptr < AllGroupsCollector <
      ? >> createRandomCollector(const std::wstring &groupField);

protected:
  std::shared_ptr<AllGroupsCollectorTest> shared_from_this()
  {
    return std::static_pointer_cast<AllGroupsCollectorTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::grouping
