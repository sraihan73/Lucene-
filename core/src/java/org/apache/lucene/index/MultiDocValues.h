#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::index
{
class OrdinalMap;
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
 * A wrapper for CompositeIndexReader providing access to DocValues.
 *
 * <p><b>NOTE</b>: for multi readers, you'll get better
 * performance by gathering the sub readers using
 * {@link IndexReader#getContext()} to get the
 * atomic leaves and then operate per-LeafReader,
 * instead of using this class.
 *
 * <p><b>NOTE</b>: This is very costly.
 *
 * @lucene.experimental
 * @lucene.internal
 */
class MultiDocValues : public std::enable_shared_from_this<MultiDocValues>
{
  GET_CLASS_NAME(MultiDocValues)

  /** No instantiation */
private:
  MultiDocValues();

  /** Returns a NumericDocValues for a reader's norms (potentially merging
   * on-the-fly). <p> This is a slow way to access normalization values.
   * Instead, access them per-segment with {@link
   * LeafReader#getNormValues(std::wstring)}
   * </p>
   */
public:
  static std::shared_ptr<NumericDocValues>
  getNormValues(std::shared_ptr<IndexReader> r,
                const std::wstring &field) ;

private:
  class NumericDocValuesAnonymousInnerClass : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
  private:
    std::wstring field;
    std::deque<std::shared_ptr<LeafReaderContext>> leaves;

  public:
    NumericDocValuesAnonymousInnerClass(
        const std::wstring &field,
        std::deque<std::shared_ptr<LeafReaderContext>> &leaves);

  private:
    int nextLeaf = 0;
    std::shared_ptr<NumericDocValues> currentValues;
    std::shared_ptr<LeafReaderContext> currentLeaf;
    int docID = -1;

  public:
    int nextDoc()  override;

    int docID() override;

    int advance(int targetDocID)  override;

    bool advanceExact(int targetDocID)  override;

    int64_t longValue()  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
          NumericDocValues::shared_from_this());
    }
  };

  /** Returns a NumericDocValues for a reader's docvalues (potentially merging
   * on-the-fly) */
public:
  static std::shared_ptr<NumericDocValues>
  getNumericValues(std::shared_ptr<IndexReader> r,
                   const std::wstring &field) ;

private:
  class NumericDocValuesAnonymousInnerClass2 : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass2)
  private:
    std::wstring field;
    std::deque<std::shared_ptr<LeafReaderContext>> leaves;

  public:
    NumericDocValuesAnonymousInnerClass2(
        const std::wstring &field,
        std::deque<std::shared_ptr<LeafReaderContext>> &leaves);

  private:
    int nextLeaf = 0;
    std::shared_ptr<NumericDocValues> currentValues;
    std::shared_ptr<LeafReaderContext> currentLeaf;
    int docID = -1;

  public:
    int docID() override;

    int nextDoc()  override;

    int advance(int targetDocID)  override;

    bool advanceExact(int targetDocID)  override;
    int64_t longValue()  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass2>(
          NumericDocValues::shared_from_this());
    }
  };

  /** Returns a BinaryDocValues for a reader's docvalues (potentially merging
   * on-the-fly) */
public:
  static std::shared_ptr<BinaryDocValues>
  getBinaryValues(std::shared_ptr<IndexReader> r,
                  const std::wstring &field) ;

private:
  class BinaryDocValuesAnonymousInnerClass : public BinaryDocValues
  {
    GET_CLASS_NAME(BinaryDocValuesAnonymousInnerClass)
  private:
    std::wstring field;
    std::deque<std::shared_ptr<LeafReaderContext>> leaves;

  public:
    BinaryDocValuesAnonymousInnerClass(
        const std::wstring &field,
        std::deque<std::shared_ptr<LeafReaderContext>> &leaves);

  private:
    int nextLeaf = 0;
    std::shared_ptr<BinaryDocValues> currentValues;
    std::shared_ptr<LeafReaderContext> currentLeaf;
    int docID = -1;

  public:
    int nextDoc()  override;

    int docID() override;

    int advance(int targetDocID)  override;

    bool advanceExact(int targetDocID)  override;

    std::shared_ptr<BytesRef> binaryValue()  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<BinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BinaryDocValuesAnonymousInnerClass>(
          BinaryDocValues::shared_from_this());
    }
  };

  /** Returns a SortedNumericDocValues for a reader's docvalues (potentially
   * merging on-the-fly) <p> This is a slow way to access sorted numeric values.
   * Instead, access them per-segment with {@link
   * LeafReader#getSortedNumericDocValues(std::wstring)}
   * </p>
   * */
public:
  static std::shared_ptr<SortedNumericDocValues>
  getSortedNumericValues(std::shared_ptr<IndexReader> r,
                         const std::wstring &field) ;

private:
  class SortedNumericDocValuesAnonymousInnerClass
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<LeafReaderContext>> leaves;
    std::deque<
        std::shared_ptr<org::apache::lucene::index::SortedNumericDocValues>>
        values;
    int64_t finalTotalCost = 0;

  public:
    SortedNumericDocValuesAnonymousInnerClass(
        std::deque<std::shared_ptr<LeafReaderContext>> &leaves,
        std::deque<
            std::shared_ptr<org::apache::lucene::index::SortedNumericDocValues>>
            &values,
        int64_t finalTotalCost);

  private:
    int nextLeaf = 0;
    std::shared_ptr<SortedNumericDocValues> currentValues;
    std::shared_ptr<LeafReaderContext> currentLeaf;
    int docID = -1;

  public:
    int nextDoc()  override;

    int docID() override;

    int advance(int targetDocID)  override;

    bool advanceExact(int targetDocID)  override;

    int64_t cost() override;

    int docValueCount() override;

    int64_t nextValue()  override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass>(
          SortedNumericDocValues::shared_from_this());
    }
  };

  /** Returns a SortedDocValues for a reader's docvalues (potentially doing
   * extremely slow things). <p> This is an extremely slow way to access sorted
   * values. Instead, access them per-segment with {@link
   * LeafReader#getSortedDocValues(std::wstring)}
   * </p>
   */
public:
  static std::shared_ptr<SortedDocValues>
  getSortedValues(std::shared_ptr<IndexReader> r,
                  const std::wstring &field) ;

  /** Returns a SortedSetDocValues for a reader's docvalues (potentially doing
   * extremely slow things). <p> This is an extremely slow way to access sorted
   * values. Instead, access them per-segment with {@link
   * LeafReader#getSortedSetDocValues(std::wstring)}
   * </p>
   */
  static std::shared_ptr<SortedSetDocValues>
  getSortedSetValues(std::shared_ptr<IndexReader> r,
                     const std::wstring &field) ;

  /**
   * Implements SortedDocValues over n subs, using an OrdinalMap
   * @lucene.internal
   */
public:
  class MultiSortedDocValues : public SortedDocValues
  {
    GET_CLASS_NAME(MultiSortedDocValues)
    /** docbase for each leaf: parallel with {@link #values} */
  public:
    std::deque<int> const docStarts;
    /** leaf values */
    std::deque<std::shared_ptr<SortedDocValues>> const values;
    /** ordinal map_obj mapping ords from <code>values</code> to global ord space */
    const std::shared_ptr<OrdinalMap> mapping;

  private:
    const int64_t totalCost;

    int nextLeaf = 0;
    std::shared_ptr<SortedDocValues> currentValues;
    int currentDocStart = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

    /** Creates a new MultiSortedDocValues over <code>values</code> */
  public:
    MultiSortedDocValues(std::deque<std::shared_ptr<SortedDocValues>> &values,
                         std::deque<int> &docStarts,
                         std::shared_ptr<OrdinalMap> mapping,
                         int64_t totalCost) ;

    int docID() override;

    int nextDoc()  override;

    int advance(int targetDocID)  override;

    bool advanceExact(int targetDocID)  override;

    int ordValue()  override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<MultiSortedDocValues> shared_from_this()
    {
      return std::static_pointer_cast<MultiSortedDocValues>(
          SortedDocValues::shared_from_this());
    }
  };

  /**
   * Implements MultiSortedSetDocValues over n subs, using an OrdinalMap
   * @lucene.internal
   */
public:
  class MultiSortedSetDocValues : public SortedSetDocValues
  {
    GET_CLASS_NAME(MultiSortedSetDocValues)
    /** docbase for each leaf: parallel with {@link #values} */
  public:
    std::deque<int> const docStarts;
    /** leaf values */
    std::deque<std::shared_ptr<SortedSetDocValues>> const values;
    /** ordinal map_obj mapping ords from <code>values</code> to global ord space */
    const std::shared_ptr<OrdinalMap> mapping;

  private:
    const int64_t totalCost;

    int nextLeaf = 0;
    std::shared_ptr<SortedSetDocValues> currentValues;
    int currentDocStart = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

    /** Creates a new MultiSortedSetDocValues over <code>values</code> */
  public:
    MultiSortedSetDocValues(
        std::deque<std::shared_ptr<SortedSetDocValues>> &values,
        std::deque<int> &docStarts, std::shared_ptr<OrdinalMap> mapping,
        int64_t totalCost) ;

    int docID() override;

    int nextDoc()  override;

    int advance(int targetDocID)  override;

    bool advanceExact(int targetDocID)  override;

    int64_t nextOrd()  override;

    std::shared_ptr<BytesRef>
    lookupOrd(int64_t ord)  override;

    int64_t getValueCount() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<MultiSortedSetDocValues> shared_from_this()
    {
      return std::static_pointer_cast<MultiSortedSetDocValues>(
          SortedSetDocValues::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index
