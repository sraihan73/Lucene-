#pragma once
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::classification::utils
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Testcase for {@link
 * org.apache.lucene.classification.utils.DocToDoubleVectorUtils}
 */
class DocToDoubleVectorUtilsTest : public LuceneTestCase
{
  GET_CLASS_NAME(DocToDoubleVectorUtilsTest)

private:
  std::shared_ptr<IndexReader> index;
  std::shared_ptr<Directory> dir;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @After public void tearDown() throws Exception
  void tearDown()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDenseFreqDoubleArrayConversion()
  // throws Exception
  virtual void testDenseFreqDoubleArrayConversion() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSparseFreqDoubleArrayConversion()
  // throws Exception
  virtual void testSparseFreqDoubleArrayConversion() ;

protected:
  std::shared_ptr<DocToDoubleVectorUtilsTest> shared_from_this()
  {
    return std::static_pointer_cast<DocToDoubleVectorUtilsTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification::utils
