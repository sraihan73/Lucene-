#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"

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
 * A per-document byte[] with presorted values.
 * <p>
 * Per-Document values in a SortedDocValues are deduplicated, dereferenced,
 * and sorted into a dictionary of unique values. A pointer to the
 * dictionary value (ordinal) can be retrieved for each document. Ordinals
 * are dense and in increasing sorted order.
 *
 * @deprecated Use {@link SortedDocValues} instead.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public abstract class LegacySortedDocValues
// extends LegacyBinaryDocValues
class LegacySortedDocValues : public LegacyBinaryDocValues
{

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  LegacySortedDocValues();

  /**
   * Returns the ordinal for the specified docID.
   * @param  docID document ID to lookup
   * @return ordinal for the document: this is dense, starts at 0, then
   *         increments by 1 for the next value in sorted order. Note that
   *         missing values are indicated by -1.
   */
public:
  virtual int getOrd(int docID) = 0;

  /** Retrieves the value for the specified ordinal. The returned
   * {@link BytesRef} may be re-used across calls to {@link #lookupOrd(int)}
   * so make sure to {@link BytesRef#deepCopyOf(BytesRef) copy it} if you want
   * to keep it around.
   * @param ord ordinal to lookup (must be &gt;= 0 and &lt; {@link
   * #getValueCount()})
   * @see #getOrd(int)
   */
  virtual std::shared_ptr<BytesRef> lookupOrd(int ord) = 0;

  /**
   * Returns the number of unique values.
   * @return number of unique values in this SortedDocValues. This is
   *         also equivalent to one plus the maximum ordinal.
   */
  virtual int getValueCount() = 0;

private:
  const std::shared_ptr<BytesRef> empty = std::make_shared<BytesRef>();

public:
  std::shared_ptr<BytesRef> get(int docID) override;

  /** If {@code key} exists, returns its ordinal, else
   *  returns {@code -insertionPoint-1}, like {@code
   *  Arrays.binarySearch}.
   *
   *  @param key Key to look up
   **/
  virtual int lookupTerm(std::shared_ptr<BytesRef> key);

  /**
   * Returns a {@link TermsEnum} over the values.
   * The enum supports {@link TermsEnum#ord()} and {@link
   * TermsEnum#seekExact(long)}.
   */
  virtual std::shared_ptr<TermsEnum> termsEnum();

protected:
  std::shared_ptr<LegacySortedDocValues> shared_from_this()
  {
    return std::static_pointer_cast<LegacySortedDocValues>(
        LegacyBinaryDocValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
