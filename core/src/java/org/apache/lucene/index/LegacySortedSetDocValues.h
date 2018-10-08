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
 * A per-document set of presorted byte[] values.
 * <p>
 * Per-Document values in a SortedDocValues are deduplicated, dereferenced,
 * and sorted into a dictionary of unique values. A pointer to the
 * dictionary value (ordinal) can be retrieved for each document. Ordinals
 * are dense and in increasing sorted order.
 *
 * @deprecated Use {@link SortedSetDocValues} instead.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public abstract class LegacySortedSetDocValues
class LegacySortedSetDocValues
    : public std::enable_shared_from_this<LegacySortedSetDocValues>
{

  /** Sole constructor. (For invocation by subclass
   * constructors, typically implicit.) */
protected:
  LegacySortedSetDocValues();

  /** When returned by {@link #nextOrd()} it means there are no more
   *  ordinals for the document.
   */
public:
  static constexpr int64_t NO_MORE_ORDS = -1;

  /**
   * Returns the next ordinal for the current document (previously
   * set by {@link #setDocument(int)}.
   * @return next ordinal for the document, or {@link #NO_MORE_ORDS}.
   *         ordinals are dense, start at 0, then increment by 1 for
   *         the next value in sorted order.
   */
  virtual int64_t nextOrd() = 0;

  /**
   * Sets iteration to the specified docID
   * @param docID document ID
   */
  virtual void setDocument(int docID) = 0;

  /** Retrieves the value for the specified ordinal. The returned
   * {@link BytesRef} may be re-used across calls to lookupOrd so make sure to
   * {@link BytesRef#deepCopyOf(BytesRef) copy it} if you want to keep it
   * around.
   * @param ord ordinal to lookup
   * @see #nextOrd
   */
  virtual std::shared_ptr<BytesRef> lookupOrd(int64_t ord) = 0;

  /**
   * Returns the number of unique values.
   * @return number of unique values in this SortedDocValues. This is
   *         also equivalent to one plus the maximum ordinal.
   */
  virtual int64_t getValueCount() = 0;

  /** If {@code key} exists, returns its ordinal, else
   *  returns {@code -insertionPoint-1}, like {@code
   *  Arrays.binarySearch}.
   *
   *  @param key Key to look up
   **/
  virtual int64_t lookupTerm(std::shared_ptr<BytesRef> key);

  /**
   * Returns a {@link TermsEnum} over the values.
   * The enum supports {@link TermsEnum#ord()} and {@link
   * TermsEnum#seekExact(long)}.
   */
  virtual std::shared_ptr<TermsEnum> termsEnum() ;
};

} // #include  "core/src/java/org/apache/lucene/index/
