#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/LegacySortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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

/**
 * This class contains utility methods and constants for DocValues
 */
class DocValues final : public std::enable_shared_from_this<DocValues>
{
  GET_CLASS_NAME(DocValues)

  /* no instantiation */
private:
  DocValues();

  /**
   * An empty {@link BinaryDocValues} which returns no documents
   */
public:
  static std::shared_ptr<BinaryDocValues> emptyBinary();

private:
  class BinaryDocValuesAnonymousInnerClass : public BinaryDocValues
  {
    GET_CLASS_NAME(BinaryDocValuesAnonymousInnerClass)
  public:
    BinaryDocValuesAnonymousInnerClass();

  private:
    int doc = -1;

  public:
    int advance(int target) override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc() override;

    int64_t cost() override;

    std::shared_ptr<BytesRef> binaryValue() override;

  protected:
    std::shared_ptr<BinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BinaryDocValuesAnonymousInnerClass>(
          BinaryDocValues::shared_from_this());
    }
  };

  /**
   * An empty NumericDocValues which returns no documents
   */
public:
  static std::shared_ptr<NumericDocValues> emptyNumeric();

private:
  class NumericDocValuesAnonymousInnerClass : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
  public:
    NumericDocValuesAnonymousInnerClass();

  private:
    int doc = -1;

  public:
    int advance(int target) override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc() override;

    int64_t cost() override;

    int64_t longValue() override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
          NumericDocValues::shared_from_this());
    }
  };

  /**
   * An empty SortedDocValues which returns {@link BytesRef#EMPTY_BYTES} for
   * every document
   */
public:
  static std::shared_ptr<LegacySortedDocValues> emptyLegacySorted();

private:
  class LegacySortedDocValuesAnonymousInnerClass : public LegacySortedDocValues
  {
    GET_CLASS_NAME(LegacySortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRef> empty;

  public:
    LegacySortedDocValuesAnonymousInnerClass(std::shared_ptr<BytesRef> empty);

    int getOrd(int docID) override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int getValueCount() override;

  protected:
    std::shared_ptr<LegacySortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LegacySortedDocValuesAnonymousInnerClass>(
          LegacySortedDocValues::shared_from_this());
    }
  };

  /**
   * An empty SortedDocValues which returns {@link BytesRef#EMPTY_BYTES} for
   * every document
   */
public:
  static std::shared_ptr<SortedDocValues> emptySorted();

private:
  class SortedDocValuesAnonymousInnerClass : public SortedDocValues
  {
    GET_CLASS_NAME(SortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRef> empty;

  public:
    SortedDocValuesAnonymousInnerClass(std::shared_ptr<BytesRef> empty);

  private:
    int doc = -1;

  public:
    int advance(int target) override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc() override;

    int64_t cost() override;

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int getValueCount() override;

  protected:
    std::shared_ptr<SortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SortedDocValuesAnonymousInnerClass>(
          SortedDocValues::shared_from_this());
    }
  };

  /**
   * An empty SortedNumericDocValues which returns zero values for every
   * document
   */
public:
  static std::shared_ptr<SortedNumericDocValues> emptySortedNumeric(int maxDoc);

private:
  class SortedNumericDocValuesAnonymousInnerClass
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass)
  public:
    SortedNumericDocValuesAnonymousInnerClass();

  private:
    int doc = -1;

  public:
    int advance(int target) override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc() override;

    int64_t cost() override;

    int docValueCount() override;

    int64_t nextValue() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass>(
          SortedNumericDocValues::shared_from_this());
    }
  };

  /**
   * An empty SortedDocValues which returns {@link BytesRef#EMPTY_BYTES} for
   * every document
   */
public:
  static std::shared_ptr<SortedSetDocValues> emptySortedSet();

private:
  class SortedSetDocValuesAnonymousInnerClass : public SortedSetDocValues
  {
    GET_CLASS_NAME(SortedSetDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRef> empty;

  public:
    SortedSetDocValuesAnonymousInnerClass(std::shared_ptr<BytesRef> empty);

  private:
    int doc = -1;

  public:
    int advance(int target) override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc() override;

    int64_t cost() override;

    int64_t nextOrd() override;

    std::shared_ptr<BytesRef> lookupOrd(int64_t ord) override;

    int64_t getValueCount() override;

  protected:
    std::shared_ptr<SortedSetDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SortedSetDocValuesAnonymousInnerClass>(
          SortedSetDocValues::shared_from_this());
    }
  };

  /**
   * Returns a multi-valued view over the provided SortedDocValues
   */
public:
  static std::shared_ptr<SortedSetDocValues>
  singleton(std::shared_ptr<SortedDocValues> dv);

  /**
   * Returns a single-valued view of the SortedSetDocValues, if it was
   * previously wrapped with {@link #singleton(SortedDocValues)}, or null.
   */
  static std::shared_ptr<SortedDocValues>
  unwrapSingleton(std::shared_ptr<SortedSetDocValues> dv);

  /**
   * Returns a single-valued view of the SortedNumericDocValues, if it was
   * previously wrapped with {@link #singleton(NumericDocValues)}, or null.
   */
  static std::shared_ptr<NumericDocValues>
  unwrapSingleton(std::shared_ptr<SortedNumericDocValues> dv);

  /**
   * Returns a multi-valued view over the provided NumericDocValues
   */
  static std::shared_ptr<SortedNumericDocValues>
  singleton(std::shared_ptr<NumericDocValues> dv);

  // some helpers, for transition from fieldcache apis.
  // as opposed to the LeafReader apis (which must be strict for consistency),
  // these are lenient

  // helper method: to give a nice error when LeafReader.getXXXDocValues returns
  // null.
private:
  static void checkField(std::shared_ptr<LeafReader> in_,
                         const std::wstring &field,
                         std::deque<DocValuesType> &expected);

  /**
   * Returns NumericDocValues for the field, or {@link #emptyNumeric()} if it
   * has none.
   * @return docvalues instance, or an empty instance if {@code field} does not
   * exist in this reader.
   * @throws IllegalStateException if {@code field} exists, but was not indexed
   * with docvalues.
   * @throws IllegalStateException if {@code field} has docvalues, but the type
   * is not {@link DocValuesType#NUMERIC}.
   * @throws IOException if an I/O error occurs.
   */
public:
  static std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<LeafReader> reader,
             const std::wstring &field) ;

  /**
   * Returns BinaryDocValues for the field, or {@link #emptyBinary} if it has
   * none.
   * @return docvalues instance, or an empty instance if {@code field} does not
   * exist in this reader.
   * @throws IllegalStateException if {@code field} exists, but was not indexed
   * with docvalues.
   * @throws IllegalStateException if {@code field} has docvalues, but the type
   * is not {@link DocValuesType#BINARY} or {@link DocValuesType#SORTED}.
   * @throws IOException if an I/O error occurs.
   */
  static std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<LeafReader> reader,
            const std::wstring &field) ;

  /**
   * Returns SortedDocValues for the field, or {@link #emptySorted} if it has
   * none.
   * @return docvalues instance, or an empty instance if {@code field} does not
   * exist in this reader.
   * @throws IllegalStateException if {@code field} exists, but was not indexed
   * with docvalues.
   * @throws IllegalStateException if {@code field} has docvalues, but the type
   * is not {@link DocValuesType#SORTED}.
   * @throws IOException if an I/O error occurs.
   */
  static std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<LeafReader> reader,
            const std::wstring &field) ;

  /**
   * Returns SortedNumericDocValues for the field, or {@link
   * #emptySortedNumeric} if it has none.
   * @return docvalues instance, or an empty instance if {@code field} does not
   * exist in this reader.
   * @throws IllegalStateException if {@code field} exists, but was not indexed
   * with docvalues.
   * @throws IllegalStateException if {@code field} has docvalues, but the type
   * is not {@link DocValuesType#SORTED_NUMERIC} or {@link
   * DocValuesType#NUMERIC}.
   * @throws IOException if an I/O error occurs.
   */
  static std::shared_ptr<SortedNumericDocValues>
  getSortedNumeric(std::shared_ptr<LeafReader> reader,
                   const std::wstring &field) ;

  /**
   * Returns SortedSetDocValues for the field, or {@link #emptySortedSet} if it
   * has none.
   * @return docvalues instance, or an empty instance if {@code field} does not
   * exist in this reader.
   * @throws IllegalStateException if {@code field} exists, but was not indexed
   * with docvalues.
   * @throws IllegalStateException if {@code field} has docvalues, but the type
   * is not {@link DocValuesType#SORTED_SET} or {@link DocValuesType#SORTED}.
   * @throws IOException if an I/O error occurs.
   */
  static std::shared_ptr<SortedSetDocValues>
  getSortedSet(std::shared_ptr<LeafReader> reader,
               const std::wstring &field) ;

  /**
   * Returns {@code true} if the specified docvalues fields have not been
   * updated
   */
  static bool isCacheable(std::shared_ptr<LeafReaderContext> ctx,
                          std::deque<std::wstring> &fields);
};

} // #include  "core/src/java/org/apache/lucene/index/
