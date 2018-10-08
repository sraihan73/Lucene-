#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"

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
 * Delegates all methods to a wrapped {@link BinaryDocValues}.
 */
class FilterBinaryDocValues : public BinaryDocValues
{
  GET_CLASS_NAME(FilterBinaryDocValues)

  /** Wrapped values */
protected:
  const std::shared_ptr<BinaryDocValues> in_;

  /** Sole constructor */
  FilterBinaryDocValues(std::shared_ptr<BinaryDocValues> in_);

public:
  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  bool advanceExact(int target)  override;

  int64_t cost() override;

  std::shared_ptr<BytesRef> binaryValue()  override;

protected:
  std::shared_ptr<FilterBinaryDocValues> shared_from_this()
  {
    return std::static_pointer_cast<FilterBinaryDocValues>(
        BinaryDocValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
