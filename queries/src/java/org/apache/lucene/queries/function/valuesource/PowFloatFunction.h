#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"

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

/** Function to raise the base "a" to the power "b"
 */
class PowFloatFunction : public DualFloatFunction
{
  GET_CLASS_NAME(PowFloatFunction)
  /**
   * @param   a  the base.
   * @param   b  the exponent.
   */
public:
  PowFloatFunction(std::shared_ptr<ValueSource> a,
                   std::shared_ptr<ValueSource> b);

protected:
  std::wstring name() override;

  float func(int doc, std::shared_ptr<FunctionValues> aVals,
             std::shared_ptr<FunctionValues> bVals)  override;

protected:
  std::shared_ptr<PowFloatFunction> shared_from_this()
  {
    return std::static_pointer_cast<PowFloatFunction>(
        DualFloatFunction::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
