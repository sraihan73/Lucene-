#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::index
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests {@link Terms#getSumDocFreq()}
 * @lucene.experimental
 */
class TestSumDocFreq : public LuceneTestCase
{
  GET_CLASS_NAME(TestSumDocFreq)

public:
  virtual void testSumDocFreq() ;

private:
  void
  assertSumDocFreq(std::shared_ptr<IndexReader> ir) ;

protected:
  std::shared_ptr<TestSumDocFreq> shared_from_this()
  {
    return std::static_pointer_cast<TestSumDocFreq>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
