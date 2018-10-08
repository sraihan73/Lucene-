#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMultiPassIndexSplitter : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiPassIndexSplitter)
public:
  std::shared_ptr<IndexReader> input;
  int NUM_DOCS = 11;
  std::shared_ptr<Directory> dir;

  void setUp()  override;

  void tearDown()  override;

  /**
   * Test round-robin splitting.
   */
  virtual void testSplitRR() ;

  /**
   * Test sequential splitting.
   */
  virtual void testSplitSeq() ;

protected:
  std::shared_ptr<TestMultiPassIndexSplitter> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiPassIndexSplitter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
