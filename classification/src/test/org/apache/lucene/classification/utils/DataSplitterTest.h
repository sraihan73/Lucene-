#pragma once
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReader;
}

namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::classification::utils
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Testcase for {@link org.apache.lucene.classification.utils.DatasetSplitter}
 */
class DataSplitterTest : public LuceneTestCase
{
  GET_CLASS_NAME(DataSplitterTest)

private:
  std::shared_ptr<LeafReader> originalIndex;
  std::shared_ptr<RandomIndexWriter> indexWriter;
  std::shared_ptr<Directory> dir;

  static const std::wstring textFieldName;
  static const std::wstring classFieldName;
  static const std::wstring idFieldName;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @After public void tearDown() throws Exception
  void tearDown()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSplitOnAllFields() throws Exception
  virtual void testSplitOnAllFields() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSplitOnSomeFields() throws Exception
  virtual void testSplitOnSomeFields() ;

  static void
  assertSplit(std::shared_ptr<LeafReader> originalIndex, double testRatio,
              double crossValidationRatio,
              std::deque<std::wstring> &fieldNames) ;

private:
  static void
  closeQuietly(std::shared_ptr<IndexReader> reader) ;

protected:
  std::shared_ptr<DataSplitterTest> shared_from_this()
  {
    return std::static_pointer_cast<DataSplitterTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification::utils
