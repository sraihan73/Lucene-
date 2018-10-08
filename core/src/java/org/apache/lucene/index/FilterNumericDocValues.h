#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class NumericDocValues;
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

/**
 * Delegates all methods to a wrapped {@link NumericDocValues}.
 */
class FilterNumericDocValues : public NumericDocValues
{
  GET_CLASS_NAME(FilterNumericDocValues)

  /** Wrapped values */
protected:
  const std::shared_ptr<NumericDocValues> in_;

  /** Sole constructor */
  FilterNumericDocValues(std::shared_ptr<NumericDocValues> in_);

public:
  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  bool advanceExact(int target)  override;

  int64_t cost() override;

  int64_t longValue()  override;

protected:
  std::shared_ptr<FilterNumericDocValues> shared_from_this()
  {
    return std::static_pointer_cast<FilterNumericDocValues>(
        NumericDocValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
