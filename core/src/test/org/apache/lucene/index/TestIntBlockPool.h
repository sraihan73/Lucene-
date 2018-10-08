#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Counter;
}

namespace org::apache::lucene::util
{
class IntBlockPool;
}
namespace org::apache::lucene::util
{
class SliceReader;
}
namespace org::apache::lucene::index
{
class StartEndAndValues;
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

using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * tests basic {@link IntBlockPool} functionality
 */
class TestIntBlockPool : public LuceneTestCase
{
  GET_CLASS_NAME(TestIntBlockPool)

public:
  virtual void testSingleWriterReader();

  virtual void testMultipleWriterReader();

private:
  class ByteTrackingAllocator : public IntBlockPool::Allocator
  {
    GET_CLASS_NAME(ByteTrackingAllocator)
  private:
    const std::shared_ptr<Counter> bytesUsed;

  public:
    ByteTrackingAllocator(std::shared_ptr<Counter> bytesUsed);

    ByteTrackingAllocator(int blockSize, std::shared_ptr<Counter> bytesUsed);

    std::deque<int> getIntBlock() override;

    void recycleIntBlocks(std::deque<std::deque<int>> &blocks, int start,
                          int end) override;

  protected:
    std::shared_ptr<ByteTrackingAllocator> shared_from_this()
    {
      return std::static_pointer_cast<ByteTrackingAllocator>(
          org.apache.lucene.util.IntBlockPool.Allocator::shared_from_this());
    }
  };

private:
  void assertReader(std::shared_ptr<IntBlockPool::SliceReader> reader,
                    std::shared_ptr<StartEndAndValues> values);

private:
  class StartEndAndValues
      : public std::enable_shared_from_this<StartEndAndValues>
  {
    GET_CLASS_NAME(StartEndAndValues)
  public:
    int valueOffset = 0;
    int valueCount = 0;
    int start = 0;
    int end = 0;

    StartEndAndValues(int valueOffset);

    virtual int nextValue();
  };

protected:
  std::shared_ptr<TestIntBlockPool> shared_from_this()
  {
    return std::static_pointer_cast<TestIntBlockPool>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
