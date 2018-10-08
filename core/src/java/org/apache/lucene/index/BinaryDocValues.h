#pragma once
#include "stringhelper.h"
#include <memory>
#include "core/src/java/org/apache/lucene/index/DocValuesIterator.h"

// C++ NOTE: Forward class declarations:
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

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A per-document numeric value.
 */
class BinaryDocValues : public DocValuesIterator
{
  GET_CLASS_NAME(BinaryDocValues)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  BinaryDocValues();

  /**
   * Returns the binary value for the current document ID.
   * It is illegal to call this method after {@link #advanceExact(int)}
   * returned {@code false}.
   * @return binary value
   */
public:
  virtual std::shared_ptr<BytesRef> binaryValue() = 0;

protected:
  std::shared_ptr<BinaryDocValues> shared_from_this()
  {
    return std::static_pointer_cast<BinaryDocValues>(
        DocValuesIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::index