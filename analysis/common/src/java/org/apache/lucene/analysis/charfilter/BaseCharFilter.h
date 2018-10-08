#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "stringhelper.h"
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
namespace org::apache::lucene::analysis::charfilter
{

using CharFilter = org::apache::lucene::analysis::CharFilter;

/**
 * Base utility class for implementing a {@link CharFilter}.
 * You subclass this, and then record mappings by calling
 * {@link #addOffCorrectMap}, and then invoke the correct
 * method to correct an offset.
 */
class BaseCharFilter : public CharFilter
{
  GET_CLASS_NAME(BaseCharFilter)

private:
  std::deque<int> offsets;
  std::deque<int> diffs;
  int size = 0;

public:
  BaseCharFilter(std::shared_ptr<Reader> in_);

  /** Retrieve the corrected offset. */
protected:
  int correct(int currentOff) override;

  virtual int getLastCumulativeDiff();

  /**
   * <p>
   *   Adds an offset correction mapping at the given output stream offset.
   * </p>
   * <p>
   *   Assumption: the offset given with each successive call to this method
   *   will not be smaller than the offset given at the previous invocation.
   * </p>
   *
   * @param off The output stream offset at which to apply the correction
   * @param cumulativeDiff The input offset is given by adding this
   *                       to the output offset
   */
  virtual void addOffCorrectMap(int off, int cumulativeDiff);

protected:
  std::shared_ptr<BaseCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<BaseCharFilter>(
        org.apache.lucene.analysis.CharFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::charfilter
