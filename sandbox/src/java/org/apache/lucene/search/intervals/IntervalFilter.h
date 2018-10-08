#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::intervals
{
class IntervalIterator;
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

namespace org::apache::lucene::search::intervals
{

/**
 * Wraps an {@link IntervalIterator} and passes through those intervals that
 * match the {@link #accept()} function
 */
class IntervalFilter : public IntervalIterator
{
  GET_CLASS_NAME(IntervalFilter)

private:
  const std::shared_ptr<IntervalIterator> in_;

  /**
   * Create a new filter
   */
public:
  IntervalFilter(std::shared_ptr<IntervalIterator> in_);

  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  int64_t cost() override;

  int start() override;

  int end() override;

  float matchCost() override;

  /**
   * @return {@code true} if the wrapped iterator's interval should be passed on
   */
protected:
  virtual bool accept() = 0;

public:
  int nextInterval()  override final;

protected:
  std::shared_ptr<IntervalFilter> shared_from_this()
  {
    return std::static_pointer_cast<IntervalFilter>(
        IntervalIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals