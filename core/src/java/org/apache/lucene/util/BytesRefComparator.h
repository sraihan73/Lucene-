#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::util
{

/** Specialized {@link BytesRef} comparator that
 * {@link FixedLengthBytesRefArray#iterator(Comparator)} has optimizations
 * for.
 * @lucene.internal */
class BytesRefComparator
    : public std::enable_shared_from_this<BytesRefComparator>,
      public Comparator<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(BytesRefComparator)

public:
  const int comparedBytesCount;

  /** Sole constructor.
   * @param comparedBytesCount the maximum number of bytes to compare. */
protected:
  BytesRefComparator(int comparedBytesCount);

  /** Return the unsigned byte to use for comparison at index {@code i}, or
   * {@code -1} if all bytes that are useful for comparisons are exhausted.
   * This may only be called with a value of {@code i} between {@code 0}
   * included and {@code comparedBytesCount} excluded. */
  virtual int byteAt(std::shared_ptr<BytesRef> ref, int i) = 0;

public:
  int compare(std::shared_ptr<BytesRef> o1,
              std::shared_ptr<BytesRef> o2) override;
};

} // #include  "core/src/java/org/apache/lucene/util/
