#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queries::function
{
class ValueSource;
}

namespace org::apache::lucene::queries::function
{
class FunctionValues;
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
namespace org::apache::lucene::queries::function::valuesource
{

using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

/**
 * <code>MaxFloatFunction</code> returns the max of its components.
 */
class MaxFloatFunction : public MultiFloatFunction
{
  GET_CLASS_NAME(MaxFloatFunction)
public:
  MaxFloatFunction(std::deque<std::shared_ptr<ValueSource>> &sources);

protected:
  std::wstring name() override;

  float func(int doc, std::deque<std::shared_ptr<FunctionValues>>
                          &valsArr)  override;

  /**
   * True if <em>any</em> of the specified <code>values</code>
   * {@link FunctionValues#exists} for the specified doc, else false.
   *
   * @see MultiFunction#anyExists
   */
  bool exists(int doc, std::deque<std::shared_ptr<FunctionValues>>
                           &valsArr)  override;

protected:
  std::shared_ptr<MaxFloatFunction> shared_from_this()
  {
    return std::static_pointer_cast<MaxFloatFunction>(
        MultiFloatFunction::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
