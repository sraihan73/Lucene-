#pragma once
#include "../index/BinaryDocValues.h"
#include "../index/DocIDMerger.h"
#include "../index/EmptyDocValuesProducer.h"
#include "../index/FilteredTermsEnum.h"
#include "../index/NumericDocValues.h"
#include "../index/SortedDocValues.h"
#include "../index/SortedNumericDocValues.h"
#include "../index/SortedSetDocValues.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}

namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
template <typename T>
class DocIDMerger;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class LongBitSet;
}
namespace org::apache::lucene::util
{
class LongValues;
}
namespace org::apache::lucene::index
{
class OrdinalMap;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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
namespace org::apache::lucene::codecs
{

using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using MergeState = org::apache::lucene::index::MergeState;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Abstract API that consumes numeric, binary and
 * sorted docvalues.  Concrete implementations of this
 * actually do "something" with the docvalues (write it into
 * the index in a specific format).
 * <p>
 * The lifecycle is:
 * <ol>
 *   <li>DocValuesConsumer is created by
 *       {@link NormsFormat#normsConsumer(SegmentWriteState)}.
 *   <li>{@link #addNumericField}, {@link #addBinaryField},
 *       {@link #addSortedField}, {@link #addSortedSetField},
 *       or {@link #addSortedNumericField} are called for each Numeric,
 *       Binary, Sorted, SortedSet, or SortedNumeric docvalues field.
 *       The API is a "pull" rather than "push", and the implementation
 *       is free to iterate over the values multiple times
 *       ({@link Iterable#iterator()}).
 *   <li>After all fields are added, the consumer is {@link #close}d.
 * </ol>
 *
 * @lucene.experimental
 */
class DocValuesConsumer : public std::enable_shared_from_this<DocValuesConsumer>
{
  GET_CLASS_NAME(DocValuesConsumer)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  DocValuesConsumer();

  /**
   * Writes numeric docvalues for a field.
   * @param field field information
   * @param valuesProducer Numeric values to write.
   * @throws IOException if an I/O error occurred.
   */
public:
  virtual void
  addNumericField(std::shared_ptr<FieldInfo> field,
                  std::shared_ptr<DocValuesProducer> valuesProducer) = 0;

  /**
   * Writes binary docvalues for a field.
   * @param field field information
   * @param valuesProducer Binary values to write.
   * @throws IOException if an I/O error occurred.
   */
  virtual void
  addBinaryField(std::shared_ptr<FieldInfo> field,
                 std::shared_ptr<DocValuesProducer> valuesProducer) = 0;

  /**
   * Writes pre-sorted binary docvalues for a field.
   * @param field field information
   * @param valuesProducer produces the values and ordinals to write
   * @throws IOException if an I/O error occurred.
   */
  virtual void
  addSortedField(std::shared_ptr<FieldInfo> field,
                 std::shared_ptr<DocValuesProducer> valuesProducer) = 0;

  /**
   * Writes pre-sorted numeric docvalues for a field
   * @param field field information
   * @param valuesProducer produces the values to write
   * @throws IOException if an I/O error occurred.
   */
  virtual void
  addSortedNumericField(std::shared_ptr<FieldInfo> field,
                        std::shared_ptr<DocValuesProducer> valuesProducer) = 0;

  /**
   * Writes pre-sorted set docvalues for a field
   * @param field field information
   * @param valuesProducer produces the values to write
   * @throws IOException if an I/O error occurred.
   */
  virtual void
  addSortedSetField(std::shared_ptr<FieldInfo> field,
                    std::shared_ptr<DocValuesProducer> valuesProducer) = 0;

  /** Merges in the fields from the readers in
   *  <code>mergeState</code>. The default implementation
   *  calls {@link #mergeNumericField}, {@link #mergeBinaryField},
   *  {@link #mergeSortedField}, {@link #mergeSortedSetField},
   *  or {@link #mergeSortedNumericField} for each field,
   *  depending on its type.
   *  Implementations can override this method
   *  for more sophisticated merging (bulk-byte copying, etc). */
  virtual void merge(std::shared_ptr<MergeState> mergeState) ;

  /** Tracks state of one numeric sub-reader that we are merging */
private:
  class NumericDocValuesSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(NumericDocValuesSub)

  public:
    const std::shared_ptr<NumericDocValues> values;

    NumericDocValuesSub(std::shared_ptr<MergeState::DocMap> docMap,
                        std::shared_ptr<NumericDocValues> values);

    int nextDoc()  override;

  protected:
    std::shared_ptr<NumericDocValuesSub> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /**
   * Merges the numeric docvalues from <code>MergeState</code>.
   * <p>
   * The default implementation calls {@link #addNumericField}, passing
   * a DocValuesProducer that merges and filters deleted documents on the fly.
   */
public:
  virtual void
  mergeNumericField(std::shared_ptr<FieldInfo> mergeFieldInfo,
                    std::shared_ptr<MergeState> mergeState) ;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesConsumer> outerInstance;

    std::shared_ptr<FieldInfo> mergeFieldInfo;
    std::shared_ptr<MergeState> mergeState;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<DocValuesConsumer> outerInstance,
        std::shared_ptr<FieldInfo> mergeFieldInfo,
        std::shared_ptr<MergeState> mergeState);

    std::shared_ptr<NumericDocValues> getNumeric(
        std::shared_ptr<FieldInfo> fieldInfo)  override;

  private:
    class NumericDocValuesAnonymousInnerClass : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

      std::shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
          docIDMerger;
      int64_t finalCost = 0;

    public:
      NumericDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance,
          std::shared_ptr<DocIDMerger<std::shared_ptr<NumericDocValuesSub>>>
              docIDMerger,
          int64_t finalCost);

    private:
      int docID = 0;
      std::shared_ptr<NumericDocValuesSub> current;

    public:
      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      bool advanceExact(int target)  override;

      int64_t cost() override;

      int64_t longValue()  override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.NumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

  /** Tracks state of one binary sub-reader that we are merging */
private:
  class BinaryDocValuesSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(BinaryDocValuesSub)

  public:
    const std::shared_ptr<BinaryDocValues> values;

    BinaryDocValuesSub(std::shared_ptr<MergeState::DocMap> docMap,
                       std::shared_ptr<BinaryDocValues> values);

    int nextDoc()  override;

  protected:
    std::shared_ptr<BinaryDocValuesSub> shared_from_this()
    {
      return std::static_pointer_cast<BinaryDocValuesSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /**
   * Merges the binary docvalues from <code>MergeState</code>.
   * <p>
   * The default implementation calls {@link #addBinaryField}, passing
   * a DocValuesProducer that merges and filters deleted documents on the fly.
   */
public:
  virtual void
  mergeBinaryField(std::shared_ptr<FieldInfo> mergeFieldInfo,
                   std::shared_ptr<MergeState> mergeState) ;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesConsumer> outerInstance;

    std::shared_ptr<FieldInfo> mergeFieldInfo;
    std::shared_ptr<MergeState> mergeState;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<DocValuesConsumer> outerInstance,
        std::shared_ptr<FieldInfo> mergeFieldInfo,
        std::shared_ptr<MergeState> mergeState);

    std::shared_ptr<BinaryDocValues>
    getBinary(std::shared_ptr<FieldInfo> fieldInfo)  override;

  private:
    class BinaryDocValuesAnonymousInnerClass : public BinaryDocValues
    {
      GET_CLASS_NAME(BinaryDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

      std::shared_ptr<DocIDMerger<std::shared_ptr<BinaryDocValuesSub>>>
          docIDMerger;
      int64_t finalCost = 0;

    public:
      BinaryDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance,
          std::shared_ptr<DocIDMerger<std::shared_ptr<BinaryDocValuesSub>>>
              docIDMerger,
          int64_t finalCost);

    private:
      std::shared_ptr<BinaryDocValuesSub> current;
      int docID = 0;

    public:
      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      bool advanceExact(int target)  override;

      int64_t cost() override;

      std::shared_ptr<BytesRef> binaryValue()  override;

    protected:
      std::shared_ptr<BinaryDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<BinaryDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.BinaryDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

  /** Tracks state of one sorted numeric sub-reader that we are merging */
private:
  class SortedNumericDocValuesSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(SortedNumericDocValuesSub)

  public:
    const std::shared_ptr<SortedNumericDocValues> values;

    SortedNumericDocValuesSub(std::shared_ptr<MergeState::DocMap> docMap,
                              std::shared_ptr<SortedNumericDocValues> values);

    int nextDoc()  override;

  protected:
    std::shared_ptr<SortedNumericDocValuesSub> shared_from_this()
    {
      return std::static_pointer_cast<SortedNumericDocValuesSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /**
   * Merges the sorted docvalues from <code>toMerge</code>.
   * <p>
   * The default implementation calls {@link #addSortedNumericField}, passing
   * iterables that filter deleted documents.
   */
public:
  virtual void mergeSortedNumericField(
      std::shared_ptr<FieldInfo> mergeFieldInfo,
      std::shared_ptr<MergeState> mergeState) ;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesConsumer> outerInstance;

    std::shared_ptr<FieldInfo> mergeFieldInfo;
    std::shared_ptr<MergeState> mergeState;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<DocValuesConsumer> outerInstance,
        std::shared_ptr<FieldInfo> mergeFieldInfo,
        std::shared_ptr<MergeState> mergeState);

    std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
        std::shared_ptr<FieldInfo> fieldInfo)  override;

  private:
    class SortedNumericDocValuesAnonymousInnerClass
        : public SortedNumericDocValues
    {
      GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

      int64_t finalCost = 0;
      std::shared_ptr<DocIDMerger<std::shared_ptr<SortedNumericDocValuesSub>>>
          docIDMerger;

    public:
      SortedNumericDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance,
          int64_t finalCost,
          std::shared_ptr<
              DocIDMerger<std::shared_ptr<SortedNumericDocValuesSub>>>
              docIDMerger);

    private:
      int docID = 0;
      std::shared_ptr<SortedNumericDocValuesSub> currentSub;

    public:
      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      bool advanceExact(int target)  override;

      int docValueCount() override;

      int64_t cost() override;

      int64_t nextValue()  override;

    protected:
      std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            SortedNumericDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.SortedNumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

  /** Tracks state of one sorted sub-reader that we are merging */
private:
  class SortedDocValuesSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(SortedDocValuesSub)

  public:
    const std::shared_ptr<SortedDocValues> values;
    const std::shared_ptr<LongValues> map_obj;

    SortedDocValuesSub(std::shared_ptr<MergeState::DocMap> docMap,
                       std::shared_ptr<SortedDocValues> values,
                       std::shared_ptr<LongValues> map_obj);

    int nextDoc()  override;

  protected:
    std::shared_ptr<SortedDocValuesSub> shared_from_this()
    {
      return std::static_pointer_cast<SortedDocValuesSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /**
   * Merges the sorted docvalues from <code>toMerge</code>.
   * <p>
   * The default implementation calls {@link #addSortedField}, passing
   * an Iterable that merges ordinals and values and filters deleted documents .
   */
public:
  virtual void
  mergeSortedField(std::shared_ptr<FieldInfo> fieldInfo,
                   std::shared_ptr<MergeState> mergeState) ;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesConsumer> outerInstance;

    std::shared_ptr<FieldInfo> fieldInfo;
    std::shared_ptr<MergeState> mergeState;
    std::deque<std::shared_ptr<SortedDocValues>> dvs;
    std::shared_ptr<OrdinalMap> map_obj;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<DocValuesConsumer> outerInstance,
        std::shared_ptr<FieldInfo> fieldInfo,
        std::shared_ptr<MergeState> mergeState,
        std::deque<std::shared_ptr<SortedDocValues>> &dvs,
        std::shared_ptr<OrdinalMap> map_obj);

    std::shared_ptr<SortedDocValues> getSorted(
        std::shared_ptr<FieldInfo> fieldInfoIn)  override;

  private:
    class SortedDocValuesAnonymousInnerClass : public SortedDocValues
    {
      GET_CLASS_NAME(SortedDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

      int64_t finalCost = 0;
      std::shared_ptr<DocIDMerger<std::shared_ptr<SortedDocValuesSub>>>
          docIDMerger;

    public:
      SortedDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance,
          int64_t finalCost,
          std::shared_ptr<DocIDMerger<std::shared_ptr<SortedDocValuesSub>>>
              docIDMerger);

    private:
      int docID = 0;
      int ord = 0;

    public:
      int docID() override;

      int nextDoc()  override;

      int ordValue() override;

      int advance(int target) override;

      bool advanceExact(int target)  override;

      int64_t cost() override;

      int getValueCount() override;

      std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    protected:
      std::shared_ptr<SortedDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SortedDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.SortedDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

  /** Tracks state of one sorted set sub-reader that we are merging */
private:
  class SortedSetDocValuesSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(SortedSetDocValuesSub)

  public:
    const std::shared_ptr<SortedSetDocValues> values;
    const std::shared_ptr<LongValues> map_obj;

    SortedSetDocValuesSub(std::shared_ptr<MergeState::DocMap> docMap,
                          std::shared_ptr<SortedSetDocValues> values,
                          std::shared_ptr<LongValues> map_obj);

    int nextDoc()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<SortedSetDocValuesSub> shared_from_this()
    {
      return std::static_pointer_cast<SortedSetDocValuesSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /**
   * Merges the sortedset docvalues from <code>toMerge</code>.
   * <p>
   * The default implementation calls {@link #addSortedSetField}, passing
   * an Iterable that merges ordinals and values and filters deleted documents .
   */
public:
  virtual void mergeSortedSetField(
      std::shared_ptr<FieldInfo> mergeFieldInfo,
      std::shared_ptr<MergeState> mergeState) ;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesConsumer> outerInstance;

    std::shared_ptr<FieldInfo> mergeFieldInfo;
    std::shared_ptr<MergeState> mergeState;
    std::deque<std::shared_ptr<SortedSetDocValues>> toMerge;
    std::shared_ptr<OrdinalMap> map_obj;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<DocValuesConsumer> outerInstance,
        std::shared_ptr<FieldInfo> mergeFieldInfo,
        std::shared_ptr<MergeState> mergeState,
        std::deque<std::shared_ptr<SortedSetDocValues>> &toMerge,
        std::shared_ptr<OrdinalMap> map_obj);

    std::shared_ptr<SortedSetDocValues> getSortedSet(
        std::shared_ptr<FieldInfo> fieldInfo)  override;

  private:
    class SortedSetDocValuesAnonymousInnerClass : public SortedSetDocValues
    {
      GET_CLASS_NAME(SortedSetDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

      std::shared_ptr<DocIDMerger<std::shared_ptr<SortedSetDocValuesSub>>>
          docIDMerger;
      int64_t finalCost = 0;

    public:
      SortedSetDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance,
          std::shared_ptr<DocIDMerger<std::shared_ptr<SortedSetDocValuesSub>>>
              docIDMerger,
          int64_t finalCost);

    private:
      int docID = 0;
      std::shared_ptr<SortedSetDocValuesSub> currentSub;

    public:
      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      bool advanceExact(int target)  override;

      int64_t nextOrd()  override;

      int64_t cost() override;

      std::shared_ptr<BytesRef>
      lookupOrd(int64_t ord)  override;

      int64_t getValueCount() override;

    protected:
      std::shared_ptr<SortedSetDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SortedSetDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.SortedSetDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

  // TODO: seek-by-ord to nextSetBit
public:
  class BitsFilteredTermsEnum : public FilteredTermsEnum
  {
    GET_CLASS_NAME(BitsFilteredTermsEnum)
  public:
    const std::shared_ptr<LongBitSet> liveTerms;

    BitsFilteredTermsEnum(
        std::shared_ptr<TermsEnum> in_,
        std::shared_ptr<LongBitSet>
            liveTerms); // <-- not passing false here wasted about 3 hours of my
                        // time!!!!!!!!!!!!!

  protected:
    AcceptStatus
    accept(std::shared_ptr<BytesRef> term)  override;

  protected:
    std::shared_ptr<BitsFilteredTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<BitsFilteredTermsEnum>(
          org.apache.lucene.index.FilteredTermsEnum::shared_from_this());
    }
  };

  /** Helper: returns true if the given docToValue count contains only at most
   * one value */
public:
  static bool
  isSingleValued(std::deque<std::shared_ptr<Number>> &docToValueCount);

  /** Helper: returns single-valued view, using {@code missingValue} when count
   * is zero */
  static std::deque<std::shared_ptr<Number>>
  singletonView(std::deque<std::shared_ptr<Number>> &docToValueCount,
                std::deque<std::shared_ptr<Number>> &values,
                std::shared_ptr<Number> missingValue);

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<Number>> docToValueCount;
    std::deque<std::shared_ptr<Number>> values;
    std::shared_ptr<Number> missingValue;

  public:
    IterableAnonymousInnerClass(
        std::deque<std::shared_ptr<Number>> &docToValueCount,
        std::deque<std::shared_ptr<Number>> &values,
        std::shared_ptr<Number> missingValue);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<IterableAnonymousInnerClass> outerInstance;

      std::deque<std::shared_ptr<Number>>::const_iterator countIterator;
      std::deque<std::shared_ptr<Number>>::const_iterator valuesIterator;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<IterableAnonymousInnerClass> outerInstance,
          std::deque<Number>::const_iterator countIterator,
          std::deque<Number>::const_iterator valuesIterator);

      bool hasNext();

      std::shared_ptr<Number> next();

      void remove();
    };
  };
};

} // namespace org::apache::lucene::codecs
