#pragma once
#include "stringhelper.h"
#include <memory>

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
 * Simple counter class
 *
 * @lucene.internal
 * @lucene.experimental
 */
class Counter : public std::enable_shared_from_this<Counter>
{
  GET_CLASS_NAME(Counter)

  /**
   * Adds the given delta to the counters current value
   *
   * @param delta
   *          the delta to add
   * @return the counters updated value
   */
public:
  virtual int64_t addAndGet(int64_t delta) = 0;

  /**
   * Returns the counters current value
   *
   * @return the counters current value
   */
  virtual int64_t get() = 0;

  /**
   * Returns a new counter. The returned counter is not thread-safe.
   */
  static std::shared_ptr<Counter> newCounter();

  /**
   * Returns a new counter.
   *
   * @param threadSafe
   *          <code>true</code> if the returned counter can be used by multiple
   *          threads concurrently.
   * @return a new counter.
   */
  static std::shared_ptr<Counter> newCounter(bool threadSafe);

private:
  class SerialCounter;

private:
  class AtomicCounter;
};

} // namespace org::apache::lucene::util
class Counter::SerialCounter final : public Counter
{
  GET_CLASS_NAME(Counter::SerialCounter)
private:
  int64_t count = 0;

public:
  int64_t addAndGet(int64_t delta) override;

  int64_t get() override;

protected:
  std::shared_ptr<SerialCounter> shared_from_this()
  {
    return std::static_pointer_cast<SerialCounter>(Counter::shared_from_this());
  }
};
class Counter::AtomicCounter final : public Counter
{
  GET_CLASS_NAME(Counter::AtomicCounter)
private:
  const std::shared_ptr<AtomicLong> count = std::make_shared<AtomicLong>();

public:
  int64_t addAndGet(int64_t delta) override;

  int64_t get() override;

protected:
  std::shared_ptr<AtomicCounter> shared_from_this()
  {
    return std::static_pointer_cast<AtomicCounter>(Counter::shared_from_this());
  }
};
