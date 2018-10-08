#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"

#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
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
namespace org::apache::lucene::search
{

using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;

/** Selects a value from the document's set to use as the representative value
 */
class SortedSetSelector : public std::enable_shared_from_this<SortedSetSelector>
{
  GET_CLASS_NAME(SortedSetSelector)

  /**
   * Type of selection to perform.
   * <p>
   * Limitations:
   * <ul>
   *   <li>Fields containing {@link Integer#MAX_VALUE} or more unique values
   *       are unsupported.
   *   <li>Selectors other than ({@link Type#MIN}) require
   *       optional codec support. However several codecs provided by Lucene,
   *       including the current default codec, support this.
   * </ul>
   */
public:
  enum class Type {
    GET_CLASS_NAME(Type)
    /**
     * Selects the minimum value in the set
     */
    MIN,
    /**
     * Selects the maximum value in the set
     */
    MAX,
    /**
     * Selects the middle value in the set.
     * <p>
     * If the set has an even number of values, the lower of the middle two is
     * chosen.
     */
    MIDDLE_MIN,
    /**
     * Selects the middle value in the set.
     * <p>
     * If the set has an even number of values, the higher of the middle two is
     * chosen
     */
    MIDDLE_MAX
  };

  /** Wraps a multi-valued SortedSetDocValues as a single-valued view, using the
   * specified selector */
public:
  static std::shared_ptr<SortedDocValues>
  wrap(std::shared_ptr<SortedSetDocValues> sortedSet, Type selector);

  /** Wraps a SortedSetDocValues and returns the first ordinal (min) */
public:
  class MinValue : public SortedDocValues
  {
    GET_CLASS_NAME(MinValue)
  public:
    const std::shared_ptr<SortedSetDocValues> in_;

  private:
    int ord = 0;

  public:
    MinValue(std::shared_ptr<SortedSetDocValues> in_);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    int lookupTerm(std::shared_ptr<BytesRef> key)  override;

  private:
    void setOrd() ;

  protected:
    std::shared_ptr<MinValue> shared_from_this()
    {
      return std::static_pointer_cast<MinValue>(
          org.apache.lucene.index.SortedDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedSetDocValues and returns the last ordinal (max) */
public:
  class MaxValue : public SortedDocValues
  {
    GET_CLASS_NAME(MaxValue)
  public:
    const std::shared_ptr<SortedSetDocValues> in_;

  private:
    int ord = 0;

  public:
    MaxValue(std::shared_ptr<SortedSetDocValues> in_);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    int lookupTerm(std::shared_ptr<BytesRef> key)  override;

  private:
    void setOrd() ;

  protected:
    std::shared_ptr<MaxValue> shared_from_this()
    {
      return std::static_pointer_cast<MaxValue>(
          org.apache.lucene.index.SortedDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedSetDocValues and returns the middle ordinal (or min of the
   * two) */
public:
  class MiddleMinValue : public SortedDocValues
  {
    GET_CLASS_NAME(MiddleMinValue)
  public:
    const std::shared_ptr<SortedSetDocValues> in_;

  private:
    int ord = 0;
    std::deque<int> ords = std::deque<int>(8);

  public:
    MiddleMinValue(std::shared_ptr<SortedSetDocValues> in_);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    int lookupTerm(std::shared_ptr<BytesRef> key)  override;

  private:
    void setOrd() ;

  protected:
    std::shared_ptr<MiddleMinValue> shared_from_this()
    {
      return std::static_pointer_cast<MiddleMinValue>(
          org.apache.lucene.index.SortedDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedSetDocValues and returns the middle ordinal (or max of the
   * two) */
public:
  class MiddleMaxValue : public SortedDocValues
  {
    GET_CLASS_NAME(MiddleMaxValue)
  public:
    const std::shared_ptr<SortedSetDocValues> in_;

  private:
    int ord = 0;
    std::deque<int> ords = std::deque<int>(8);

  public:
    MiddleMaxValue(std::shared_ptr<SortedSetDocValues> in_);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    int lookupTerm(std::shared_ptr<BytesRef> key)  override;

  private:
    void setOrd() ;

  protected:
    std::shared_ptr<MiddleMaxValue> shared_from_this()
    {
      return std::static_pointer_cast<MiddleMaxValue>(
          org.apache.lucene.index.SortedDocValues::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/
