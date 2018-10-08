#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::highlight
{
class PositionSpan;
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
namespace org::apache::lucene::search::highlight
{

/**
 * Lightweight class to hold term, weight, and positions used for scoring this
 * term.
 */
class WeightedSpanTerm : public WeightedTerm
{
  GET_CLASS_NAME(WeightedSpanTerm)
public:
  bool positionSensitive = false;

private:
  std::deque<std::shared_ptr<PositionSpan>> positionSpans =
      std::deque<std::shared_ptr<PositionSpan>>();

public:
  WeightedSpanTerm(float weight, const std::wstring &term);

  WeightedSpanTerm(float weight, const std::wstring &term,
                   bool positionSensitive);

  /**
   * Checks to see if this term is valid at <code>position</code>.
   *
   * @param position
   *            to check against valid term positions
   * @return true iff this term is a hit at this position
   */
  virtual bool checkPosition(int position);

  virtual void
  addPositionSpans(std::deque<std::shared_ptr<PositionSpan>> &positionSpans);

  virtual bool isPositionSensitive();

  virtual void setPositionSensitive(bool positionSensitive);

  virtual std::deque<std::shared_ptr<PositionSpan>> getPositionSpans();

protected:
  std::shared_ptr<WeightedSpanTerm> shared_from_this()
  {
    return std::static_pointer_cast<WeightedSpanTerm>(
        WeightedTerm::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::highlight
