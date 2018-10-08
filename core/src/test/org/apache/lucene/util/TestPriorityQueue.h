#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <deque>
#if 0
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

class TestPriorityQueue : public LuceneTestCase
{
  GET_CLASS_NAME(TestPriorityQueue)

private:
  class IntegerQueue : public PriorityQueue<int>
  {
    GET_CLASS_NAME(IntegerQueue)
  public:
    IntegerQueue(int count);

  protected:
    bool lessThan(std::optional<int> &a, std::optional<int> &b) override;

    void checkValidity();

  protected:
    std::shared_ptr<IntegerQueue> shared_from_this()
    {
      return std::static_pointer_cast<IntegerQueue>(
          PriorityQueue<int>::shared_from_this());
    }
  };

public:
  virtual void testPQ() ;

  static void testPQ(int count, std::shared_ptr<Random> gen);

  virtual void testClear();

  virtual void testFixedSize();

  virtual void testInsertWithOverflow();

  virtual void testRemovalsAndInsertions();

  virtual void testIteratorEmpty();

  virtual void testIteratorOne();

  virtual void testIteratorTwo();

  virtual void testIteratorRandom();

protected:
  std::shared_ptr<TestPriorityQueue> shared_from_this()
  {
    return std::static_pointer_cast<TestPriorityQueue>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
#endif
