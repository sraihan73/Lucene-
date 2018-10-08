#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LegacySortedSetDocValues.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Wraps a {@link LegacySortedSetDocValues} into a {@link SortedSetDocValues}.
 *
 * @deprecated Implement {@link SortedSetDocValues} directly.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public final class LegacySortedSetDocValuesWrapper
// extends SortedSetDocValues
class LegacySortedSetDocValuesWrapper final : public SortedSetDocValues
{
private:
  const std::shared_ptr<LegacySortedSetDocValues> values;
  const int maxDoc;
  // C++ NOTE: Fields cannot have the same name as methods:
  int docID_ = -1;
  int64_t ord = 0;

public:
  LegacySortedSetDocValuesWrapper(
      std::shared_ptr<LegacySortedSetDocValues> values, int maxDoc);

  int docID() override;

  int nextDoc() override;

  int advance(int target) override;

  bool advanceExact(int target)  override;

  int64_t cost() override;

  int64_t nextOrd() override;

  std::shared_ptr<BytesRef> lookupOrd(int64_t ord) override;

  int64_t getValueCount() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<LegacySortedSetDocValuesWrapper> shared_from_this()
  {
    return std::static_pointer_cast<LegacySortedSetDocValuesWrapper>(
        SortedSetDocValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
