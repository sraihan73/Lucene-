#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Bits;
}

namespace org::apache::lucene::index
{
class LegacyBinaryDocValues;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Wraps a {@link LegacyBinaryDocValues} into a {@link BinaryDocValues}.
 *
 * @deprecated Implement {@link BinaryDocValues} directly.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public final class LegacyBinaryDocValuesWrapper
// extends BinaryDocValues
class LegacyBinaryDocValuesWrapper final : public BinaryDocValues
{
private:
  const std::shared_ptr<Bits> docsWithField;
  const std::shared_ptr<LegacyBinaryDocValues> values;
  const int maxDoc;
  // C++ NOTE: Fields cannot have the same name as methods:
  int docID_ = -1;

public:
  LegacyBinaryDocValuesWrapper(std::shared_ptr<Bits> docsWithField,
                               std::shared_ptr<LegacyBinaryDocValues> values);

  int docID() override;

  int nextDoc() override;

  int advance(int target) override;

  bool advanceExact(int target)  override;

  int64_t cost() override;

  std::shared_ptr<BytesRef> binaryValue() override;

protected:
  std::shared_ptr<LegacyBinaryDocValuesWrapper> shared_from_this()
  {
    return std::static_pointer_cast<LegacyBinaryDocValuesWrapper>(
        BinaryDocValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
