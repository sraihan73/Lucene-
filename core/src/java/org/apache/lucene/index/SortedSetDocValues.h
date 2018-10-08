#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"

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
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/**
 * A multi-valued version of {@link SortedDocValues}.
 * <p>
 * Per-Document values in a SortedSetDocValues are deduplicated, dereferenced,
 * and sorted into a dictionary of unique values. A pointer to the
 * dictionary value (ordinal) can be retrieved for each document. Ordinals
 * are dense and in increasing sorted order.
 */
class SortedSetDocValues : public DocValuesIterator
{
  GET_CLASS_NAME(SortedSetDocValues)

  /** Sole constructor. (For invocation by subclass
   * constructors, typically implicit.) */
protected:
  SortedSetDocValues();

  /** When returned by {@link #nextOrd()} it means there are no more
   * ordinals for the document.
   */
public:
  static constexpr int64_t NO_MORE_ORDS = -1;

  /**
   * Returns the next ordinal for the current document.
   * It is illegal to call this method after {@link #advanceExact(int)}
   * returned {@code false}.
   * @return next ordinal for the document, or {@link #NO_MORE_ORDS}.
   *         ordinals are dense, start at 0, then increment by 1 for
   *         the next value in sorted order.
   */
  virtual int64_t nextOrd() = 0;

  // TODO: should we have a docValueCount, like SortedNumeric?

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
  virtual int64_t
  lookupTerm(std::shared_ptr<BytesRef> key) ;

  /**
   * Returns a {@link TermsEnum} over the values.
   * The enum supports {@link TermsEnum#ord()} and {@link
   * TermsEnum#seekExact(long)}.
   */
  virtual std::shared_ptr<TermsEnum> termsEnum() ;

  /**
   * Returns a {@link TermsEnum} over the values, filtered by a {@link
   * CompiledAutomaton} The enum supports {@link TermsEnum#ord()}.
   */
  virtual std::shared_ptr<TermsEnum>
  intersect(std::shared_ptr<CompiledAutomaton> automaton) ;

protected:
  std::shared_ptr<SortedSetDocValues> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetDocValues>(
        DocValuesIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
