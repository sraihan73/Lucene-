#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <deque>

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

/**
 * Estimates how {@link RamUsageEstimator} estimates physical memory consumption
 * of Java objects.
 */
class StressRamUsageEstimator : public LuceneTestCase
{
  GET_CLASS_NAME(StressRamUsageEstimator)
public:
  class Entry : public std::enable_shared_from_this<Entry>
  {
    GET_CLASS_NAME(Entry)
  public:
    std::any o;
    std::shared_ptr<Entry> next;

    virtual std::shared_ptr<Entry> createNext(std::any o);
  };

public:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile Object guard;
  std::any guard;

  // This shows an easy stack overflow because we're counting recursively.
  virtual void testLargeSetOfByteArrays();

private:
  int64_t shallowSizeOf(std::deque<std::any> &all);

  int64_t shallowSizeOf(std::deque<std::deque<std::any>> &all);

public:
  virtual void testSimpleByteArrays();

protected:
  std::shared_ptr<StressRamUsageEstimator> shared_from_this()
  {
    return std::static_pointer_cast<StressRamUsageEstimator>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
