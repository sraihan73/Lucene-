#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class SegmentReader;
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
namespace org::apache::lucene::index
{

using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSegmentMerger : public LuceneTestCase
{
  GET_CLASS_NAME(TestSegmentMerger)
  // The variables for the new merged segment
private:
  std::shared_ptr<Directory> mergedDir;
  std::wstring mergedSegment = L"test";
  // First segment to be merged
  std::shared_ptr<Directory> merge1Dir;
  std::shared_ptr<Document> doc1 = std::make_shared<Document>();
  std::shared_ptr<SegmentReader> reader1 = nullptr;
  // Second Segment to be merged
  std::shared_ptr<Directory> merge2Dir;
  std::shared_ptr<Document> doc2 = std::make_shared<Document>();
  std::shared_ptr<SegmentReader> reader2 = nullptr;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void test();

  virtual void testMerge() ;

  virtual void testBuildDocMap();

protected:
  std::shared_ptr<TestSegmentMerger> shared_from_this()
  {
    return std::static_pointer_cast<TestSegmentMerger>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
