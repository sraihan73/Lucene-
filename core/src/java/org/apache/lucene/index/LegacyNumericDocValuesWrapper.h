#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Bits;
}

namespace org::apache::lucene::index
{
class LegacyNumericDocValues;
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

using Bits = org::apache::lucene::util::Bits;

/**
 * Wraps a {@link LegacyNumericDocValues} into a {@link NumericDocValues}.
 *
 * @deprecated Implement {@link NumericDocValues} directly.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public final class LegacyNumericDocValuesWrapper
// extends NumericDocValues
class LegacyNumericDocValuesWrapper final : public NumericDocValues
{
private:
  const std::shared_ptr<Bits> docsWithField;
  const std::shared_ptr<LegacyNumericDocValues> values;
  const int maxDoc;
  // C++ NOTE: Fields cannot have the same name as methods:
  int docID_ = -1;
  int64_t value = 0;

public:
  LegacyNumericDocValuesWrapper(std::shared_ptr<Bits> docsWithField,
                                std::shared_ptr<LegacyNumericDocValues> values);

  int docID() override;

  int nextDoc() override;

  int advance(int target) override;

  bool advanceExact(int target)  override;

  int64_t cost() override;

  int64_t longValue() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<LegacyNumericDocValuesWrapper> shared_from_this()
  {
    return std::static_pointer_cast<LegacyNumericDocValuesWrapper>(
        NumericDocValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
