#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::document
{
class LazyDocument;
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
namespace org::apache::lucene::document
{

using namespace org::apache::lucene::analysis;
using namespace org::apache::lucene::index;
using namespace org::apache::lucene::search;
using namespace org::apache::lucene::store;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestLazyDocument : public LuceneTestCase
{
  GET_CLASS_NAME(TestLazyDocument)

public:
  const int NUM_DOCS = atLeast(10);
  std::deque<std::wstring> const FIELDS = std::deque<std::wstring>{
      L"a", L"b", L"c", L"d", L"e", L"f", L"g", L"h", L"i", L"j", L"k"};
  const int NUM_VALUES = atLeast(100);

  std::shared_ptr<Directory> dir = newDirectory();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void removeIndex()
  virtual void removeIndex();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void createIndex() throws Exception
  virtual void createIndex() ;

  virtual void testLazy() ;

private:
  class LazyTestingStoredFieldVisitor : public StoredFieldVisitor
  {
    GET_CLASS_NAME(LazyTestingStoredFieldVisitor)
  public:
    const std::shared_ptr<Document> doc = std::make_shared<Document>();
    const std::shared_ptr<LazyDocument> lazyDoc;
    const std::shared_ptr<Set<std::wstring>> lazyFieldNames;

    LazyTestingStoredFieldVisitor(std::shared_ptr<LazyDocument> l,
                                  std::deque<std::wstring> &fields);

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) override;

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &bytes)  override;

  protected:
    std::shared_ptr<LazyTestingStoredFieldVisitor> shared_from_this()
    {
      return std::static_pointer_cast<LazyTestingStoredFieldVisitor>(
          StoredFieldVisitor::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestLazyDocument> shared_from_this()
  {
    return std::static_pointer_cast<TestLazyDocument>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
