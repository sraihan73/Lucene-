#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include "core/src/java/org/apache/lucene/index/BinaryDocValues.h"

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
 * A per-document byte[] with presorted values.  This is fundamentally an
 * iterator over the int ord values per document, with random access APIs
 * to resolve an int ord to BytesRef.
 * <p>
 * Per-Document values in a SortedDocValues are deduplicated, dereferenced,
 * and sorted into a dictionary of unique values. A pointer to the
 * dictionary value (ordinal) can be retrieved for each document. Ordinals
 * are dense and in increasing sorted order.
 */

class SortedDocValues : public BinaryDocValues
{
  GET_CLASS_NAME(SortedDocValues)

  /** Sole constructor. (For invocation by subclass
   * constructors, typically implicit.) */
protected:
  SortedDocValues();

  /**
   * Returns the ordinal for the current docID.
   * It is illegal to call this method after {@link #advanceExact(int)}
   * returned {@code false}.
   * @return ordinal for the document: this is dense, starts at 0, then
   *         increments by 1 for the next value in sorted order.
   */
public:
  virtual int ordValue() = 0;

  /** Retrieves the value for the specified ordinal. The returned
   * {@link BytesRef} may be re-used across calls to {@link #lookupOrd(int)}
   * so make sure to {@link BytesRef#deepCopyOf(BytesRef) copy it} if you want
   * to keep it around.
   * @param ord ordinal to lookup (must be &gt;= 0 and &lt; {@link
   * #getValueCount()})
   * @see #ordValue()
   */
  virtual std::shared_ptr<BytesRef> lookupOrd(int ord) = 0;

private:
  const std::shared_ptr<BytesRef> empty = std::make_shared<BytesRef>();

public:
  std::shared_ptr<BytesRef> binaryValue()  override;

  /**
   * Returns the number of unique values.
   * @return number of unique values in this SortedDocValues. This is
   *         also equivalent to one plus the maximum ordinal.
   */
  virtual int getValueCount() = 0;

  /** If {@code key} exists, returns its ordinal, else
   *  returns {@code -insertionPoint-1}, like {@code
   *  Arrays.binarySearch}.
   *
   *  @param key Key to look up
   **/
  virtual int lookupTerm(std::shared_ptr<BytesRef> key) ;

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
  std::shared_ptr<SortedDocValues> shared_from_this()
  {
    return std::static_pointer_cast<SortedDocValues>(
        BinaryDocValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
