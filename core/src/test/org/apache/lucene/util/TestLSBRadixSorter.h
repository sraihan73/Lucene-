#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class LSBRadixSorter;
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
namespace org::apache::lucene::util
{

class TestLSBRadixSorter : public LuceneTestCase
{
  GET_CLASS_NAME(TestLSBRadixSorter)

public:
  virtual void test(std::shared_ptr<LSBRadixSorter> sorter, int maxLen);

  virtual void test(std::shared_ptr<LSBRadixSorter> sorter,
                    std::deque<int> &arr, int len);

  virtual void testEmpty();

  virtual void testOne();

  virtual void testTwo();

  virtual void testSimple();

  virtual void testRandom();

  virtual void testSorted();

protected:
  std::shared_ptr<TestLSBRadixSorter> shared_from_this()
  {
    return std::static_pointer_cast<TestLSBRadixSorter>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
