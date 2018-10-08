#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
namespace org::apache::lucene::util
{
template <typename T>
class PriorityQueue;
}
#include  "core/src/java/org/apache/lucene/util/packed/PagedMutable.h"

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

using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using PagedMutable = org::apache::lucene::util::packed::PagedMutable;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Holds updates of a single DocValues field, for a set of documents within one
 * segment.
 *
 * @lucene.experimental
 */
class DocValuesFieldUpdates
    : public std::enable_shared_from_this<DocValuesFieldUpdates>,
      public Accountable
{
  GET_CLASS_NAME(DocValuesFieldUpdates)

protected:
  static constexpr int PAGE_SIZE = 1024;

private:
  static constexpr int64_t HAS_VALUE_MASK = 1;
  static constexpr int64_t HAS_NO_VALUE_MASK = 0;
  static constexpr int SHIFT = 1; // we use the first bit of each value to mark
                                  // if the doc has a value or not

  /**
   * An iterator over documents and their updated values. Only documents with
   * updates are returned by this iterator, and the documents are returned in
   * increasing order.
   */
public:
  class Iterator : public DocValuesIterator
  {
    GET_CLASS_NAME(Iterator)

  public:
    bool advanceExact(int target) override final;

    int advance(int target) override final;
    int64_t cost() override final;

    int nextDoc() = 0;
    override override; // no IOException

    /**
     * Returns a long value for the current document if this iterator is a long
     * iterator.
     */
    virtual int64_t longValue() = 0;

    /**
     * Returns a binary value for the current document if this iterator is a
     * binary value iterator.
     */
    virtual std::shared_ptr<BytesRef> binaryValue() = 0;

    /** Returns delGen for this packet. */
    virtual int64_t delGen() = 0;

    /**
     * Returns true if this doc has a value
     */
    virtual bool hasValue() = 0;

    /**
     * Wraps the given iterator as a BinaryDocValues instance.
     */
    static std::shared_ptr<BinaryDocValues>
    asBinaryDocValues(std::shared_ptr<Iterator> iterator);

  private:
    class BinaryDocValuesAnonymousInnerClass : public BinaryDocValues
    {
      GET_CLASS_NAME(BinaryDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<
          org::apache::lucene::index::DocValuesFieldUpdates::Iterator>
          iterator;

    public:
      BinaryDocValuesAnonymousInnerClass(
          std::shared_ptr<
              org::apache::lucene::index::DocValuesFieldUpdates::Iterator>
              iterator);

      int docID() override;
      std::shared_ptr<BytesRef> binaryValue() override;
      bool advanceExact(int target) override;
      int nextDoc() override;
      int advance(int target) override;
      int64_t cost() override;

    protected:
      std::shared_ptr<BinaryDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<BinaryDocValuesAnonymousInnerClass>(
            BinaryDocValues::shared_from_this());
      }
    };
    /**
     * Wraps the given iterator as a NumericDocValues instance.
     */
  public:
    static std::shared_ptr<NumericDocValues>
    asNumericDocValues(std::shared_ptr<Iterator> iterator);

  private:
    class NumericDocValuesAnonymousInnerClass : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<
          org::apache::lucene::index::DocValuesFieldUpdates::Iterator>
          iterator;

    public:
      NumericDocValuesAnonymousInnerClass(
          std::shared_ptr<
              org::apache::lucene::index::DocValuesFieldUpdates::Iterator>
              iterator);

      int64_t longValue() override;
      bool advanceExact(int target) override;
      int docID() override;
      int nextDoc() override;
      int advance(int target) override;
      int64_t cost() override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
            NumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<Iterator> shared_from_this()
    {
      return std::static_pointer_cast<Iterator>(
          DocValuesIterator::shared_from_this());
    }
  };

  /** Merge-sorts multiple iterators, one per delGen, favoring the largest
   * delGen that has updates for a given docID. */
public:
  static std::shared_ptr<Iterator>
  mergedIterator(std::deque<std::shared_ptr<Iterator>> &subs);

private:
  class PriorityQueueAnonymousInnerClass
      : public PriorityQueue<std::shared_ptr<Iterator>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  public:
    PriorityQueueAnonymousInnerClass(std::shared_ptr<UnknownType> length);

  protected:
    bool lessThan(std::shared_ptr<Iterator> a,
                  std::shared_ptr<Iterator> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util.PriorityQueue<Iterator>::shared_from_this());
    }
  };

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<PriorityQueue<std::shared_ptr<Iterator>>> queue;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<PriorityQueue<std::shared_ptr<Iterator>>> queue);

  private:
    int doc = -1;

  public:
    int nextDoc();

    int docID();

    int64_t longValue();

    std::shared_ptr<BytesRef> binaryValue();

    int64_t delGen();

    bool hasValue();
  };

public:
  const std::wstring field;
  const DocValuesType type;
  const int64_t delGen;

private:
  const int bitsPerValue;
  bool finished = false;

protected:
  const int maxDoc;
  std::shared_ptr<PagedMutable> docs;
  // C++ NOTE: Fields cannot have the same name as methods:
  int size_ = 0;

  DocValuesFieldUpdates(int maxDoc, int64_t delGen, const std::wstring &field,
                        DocValuesType type);

public:
  bool getFinished();

  virtual void add(int doc, int64_t value) = 0;

  virtual void add(int doc, std::shared_ptr<BytesRef> value) = 0;

  /**
   * Adds the value for the given docID.
   * This method prevents conditional calls to {@link Iterator#longValue()} or
   * {@link Iterator#binaryValue()} since the implementation knows if it's a
   * long value iterator or binary value
   */
  virtual void add(int docId, std::shared_ptr<Iterator> iterator) = 0;

  /**
   * Returns an {@link Iterator} over the updated documents and their
   * values.
   */
  // TODO: also use this for merging, instead of having to write through to disk
  // first
  virtual std::shared_ptr<Iterator> iterator() = 0;

  /** Freezes internal data structures and sorts updates by docID for efficient
   * iteration. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void finish();

private:
  class InPlaceMergeSorterAnonymousInnerClass : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesFieldUpdates> outerInstance;

  public:
    InPlaceMergeSorterAnonymousInnerClass(
        std::shared_ptr<DocValuesFieldUpdates> outerInstance);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

  /** Returns true if this instance contains any updates. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool any();

  // C++ WARNING: The following method was originally marked 'synchronized':
  int size();

  /**
   * Adds an update that resets the documents value.
   * @param doc the doc to update
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void reset(int doc);
  // C++ WARNING: The following method was originally marked 'synchronized':
  int add(int doc);

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int addInternal(int doc, int64_t hasValueMask);

protected:
  virtual void swap(int i, int j);

  virtual void grow(int size);

  virtual void resize(int size);

  void ensureFinished();

public:
  int64_t ramBytesUsed() override;

  // TODO: can't this just be NumericDocValues now?  avoid boxing the long
  // value...
protected:
  class AbstractIterator : public DocValuesFieldUpdates::Iterator
  {
    GET_CLASS_NAME(AbstractIterator)
  private:
    const int size;
    const std::shared_ptr<PagedMutable> docs;
    int64_t idx = 0; // long so we don't overflow if size == Integer.MAX_VALUE
    int doc = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int64_t delGen_;
    // C++ NOTE: Fields cannot have the same name as methods:
    bool hasValue_ = false;

  public:
    AbstractIterator(int size, std::shared_ptr<PagedMutable> docs,
                     int64_t delGen);

    int nextDoc() override final;

    /**
     * Called when the iterator moved to the next document
     * @param idx the internal index to set the value to
     */
  protected:
    virtual void set(int64_t idx) = 0;

  public:
    int docID() override final;

    int64_t delGen() override final;

    bool hasValue() override final;

  protected:
    std::shared_ptr<AbstractIterator> shared_from_this()
    {
      return std::static_pointer_cast<AbstractIterator>(
          DocValuesFieldUpdates.Iterator::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/index/
