#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/TermState.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/IntersectVisitor.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"

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

using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using VirtualMethod = org::apache::lucene::util::VirtualMethod;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/**
 * A {@link FilterLeafReader} that can be used to apply
 * additional checks for tests.
 */
class AssertingLeafReader : public FilterLeafReader
{
  GET_CLASS_NAME(AssertingLeafReader)

private:
  static void assertThread(const std::wstring &object,
                           std::shared_ptr<Thread> creationThread);

public:
  AssertingLeafReader(std::shared_ptr<LeafReader> in_);

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  std::shared_ptr<Fields> getTermVectors(int docID)  override;

  /**
   * Wraps a Fields but with additional asserts
   */
public:
  class AssertingFields : public FilterFields
  {
    GET_CLASS_NAME(AssertingFields)
  public:
    AssertingFields(std::shared_ptr<Fields> in_);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

  protected:
    std::shared_ptr<AssertingFields> shared_from_this()
    {
      return std::static_pointer_cast<AssertingFields>(
          FilterFields::shared_from_this());
    }
  };

  /**
   * Wraps a Terms but with additional asserts
   */
public:
  class AssertingTerms : public FilterTerms
  {
    GET_CLASS_NAME(AssertingTerms)
  public:
    AssertingTerms(std::shared_ptr<Terms> in_);

    std::shared_ptr<TermsEnum>
    intersect(std::shared_ptr<CompiledAutomaton> automaton,
              std::shared_ptr<BytesRef> bytes)  override;

    std::shared_ptr<BytesRef> getMin()  override;

    std::shared_ptr<BytesRef> getMax()  override;

    std::shared_ptr<TermsEnum> iterator()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingTerms> shared_from_this()
    {
      return std::static_pointer_cast<AssertingTerms>(
          FilterTerms::shared_from_this());
    }
  };

public:
  static const std::shared_ptr<VirtualMethod<std::shared_ptr<TermsEnum>>>
      SEEK_EXACT;

public:
  class AssertingTermsEnum : public FilterTermsEnum
  {
    GET_CLASS_NAME(AssertingTermsEnum)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();

  private:
    enum class State {
      GET_CLASS_NAME(State) INITIAL,
      POSITIONED,
      UNPOSITIONED
    };

  private:
    State state = State::INITIAL;
    const bool delegateOverridesSeekExact;

  public:
    AssertingTermsEnum(std::shared_ptr<TermsEnum> in_);

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

    // TODO: we should separately track if we are 'at the end' ?
    // someone should not call next() after it returns null!!!!
    std::shared_ptr<BytesRef> next()  override;

    int64_t ord()  override;

    int docFreq()  override;

    int64_t totalTermFreq()  override;

    std::shared_ptr<BytesRef> term()  override;

    void seekExact(int64_t ord)  override;

    SeekStatus
    seekCeil(std::shared_ptr<BytesRef> term)  override;

    bool seekExact(std::shared_ptr<BytesRef> text)  override;

    std::shared_ptr<TermState> termState()  override;

    void
    seekExact(std::shared_ptr<BytesRef> term,
              std::shared_ptr<TermState> state)  override;

    virtual std::wstring toString();

    virtual void reset();

  protected:
    std::shared_ptr<AssertingTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<AssertingTermsEnum>(
          FilterTermsEnum::shared_from_this());
    }
  };

public:
  enum class DocsEnumState {
    GET_CLASS_NAME(DocsEnumState) START,
    ITERATING,
    FINISHED
  };

  /** Wraps a docsenum with additional checks */
public:
  class AssertingPostingsEnum : public FilterPostingsEnum
  {
    GET_CLASS_NAME(AssertingPostingsEnum)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();
    DocsEnumState state = DocsEnumState::START;

  public:
    int positionCount = 0;
    int positionMax = 0;

  private:
    int doc = 0;

  public:
    AssertingPostingsEnum(std::shared_ptr<PostingsEnum> in_);

    int nextDoc()  override;

    int advance(int target)  override;

    int docID() override;

    int freq()  override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    virtual void reset();

  protected:
    std::shared_ptr<AssertingPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<AssertingPostingsEnum>(
          FilterPostingsEnum::shared_from_this());
    }
  };

  /** Wraps a NumericDocValues but with additional asserts */
public:
  class AssertingNumericDocValues : public NumericDocValues
  {
    GET_CLASS_NAME(AssertingNumericDocValues)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();
    const std::shared_ptr<NumericDocValues> in_;
    const int maxDoc;
    int lastDocID = -1;
    bool exists = false;

  public:
    AssertingNumericDocValues(std::shared_ptr<NumericDocValues> in_,
                              int maxDoc);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t longValue()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<AssertingNumericDocValues>(
          NumericDocValues::shared_from_this());
    }
  };

  /** Wraps a BinaryDocValues but with additional asserts */
public:
  class AssertingBinaryDocValues : public BinaryDocValues
  {
    GET_CLASS_NAME(AssertingBinaryDocValues)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();
    const std::shared_ptr<BinaryDocValues> in_;
    const int maxDoc;
    int lastDocID = -1;
    bool exists = false;

  public:
    AssertingBinaryDocValues(std::shared_ptr<BinaryDocValues> in_, int maxDoc);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    std::shared_ptr<BytesRef> binaryValue()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<AssertingBinaryDocValues>(
          BinaryDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedDocValues but with additional asserts */
public:
  class AssertingSortedDocValues : public SortedDocValues
  {
    GET_CLASS_NAME(AssertingSortedDocValues)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();
    const std::shared_ptr<SortedDocValues> in_;
    const int maxDoc;
    const int valueCount;
    int lastDocID = -1;
    bool exists = false;

  public:
    AssertingSortedDocValues(std::shared_ptr<SortedDocValues> in_, int maxDoc);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int ordValue()  override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    std::shared_ptr<BytesRef> binaryValue()  override;

    int lookupTerm(std::shared_ptr<BytesRef> key)  override;

  protected:
    std::shared_ptr<AssertingSortedDocValues> shared_from_this()
    {
      return std::static_pointer_cast<AssertingSortedDocValues>(
          SortedDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedNumericDocValues but with additional asserts */
public:
  class AssertingSortedNumericDocValues : public SortedNumericDocValues
  {
    GET_CLASS_NAME(AssertingSortedNumericDocValues)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();
    const std::shared_ptr<SortedNumericDocValues> in_;
    const int maxDoc;
    int lastDocID = -1;
    int valueUpto = 0;
    bool exists = false;

  public:
    AssertingSortedNumericDocValues(std::shared_ptr<SortedNumericDocValues> in_,
                                    int maxDoc);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t nextValue()  override;

    int docValueCount() override;

  protected:
    std::shared_ptr<AssertingSortedNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<AssertingSortedNumericDocValues>(
          SortedNumericDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedSetDocValues but with additional asserts */
public:
  class AssertingSortedSetDocValues : public SortedSetDocValues
  {
    GET_CLASS_NAME(AssertingSortedSetDocValues)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();
    const std::shared_ptr<SortedSetDocValues> in_;
    const int maxDoc;
    const int64_t valueCount;
    int lastDocID = -1;
    int64_t lastOrd = NO_MORE_ORDS;
    bool exists = false;

  public:
    AssertingSortedSetDocValues(std::shared_ptr<SortedSetDocValues> in_,
                                int maxDoc);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t nextOrd()  override;

    std::shared_ptr<BytesRef>
    lookupOrd(int64_t ord)  override;

    int64_t getValueCount() override;

    int64_t
    lookupTerm(std::shared_ptr<BytesRef> key)  override;

  protected:
    std::shared_ptr<AssertingSortedSetDocValues> shared_from_this()
    {
      return std::static_pointer_cast<AssertingSortedSetDocValues>(
          SortedSetDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedSetDocValues but with additional asserts */
public:
  class AssertingPointValues : public PointValues
  {
    GET_CLASS_NAME(AssertingPointValues)

  private:
    const std::shared_ptr<PointValues> in_;

    /** Sole constructor. */
  public:
    AssertingPointValues(std::shared_ptr<PointValues> in_, int maxDoc);

    virtual std::shared_ptr<PointValues> getWrapped();

  private:
    void assertStats(int maxDoc);

  public:
    void intersect(std::shared_ptr<IntersectVisitor> visitor) throw(
        IOException) override;

    int64_t
    estimatePointCount(std::shared_ptr<IntersectVisitor> visitor) override;

    std::deque<char> getMinPackedValue()  override;

    std::deque<char> getMaxPackedValue()  override;

    int getNumDimensions()  override;

    int getBytesPerDimension()  override;

    int64_t size() override;

    int getDocCount() override;

  protected:
    std::shared_ptr<AssertingPointValues> shared_from_this()
    {
      return std::static_pointer_cast<AssertingPointValues>(
          PointValues::shared_from_this());
    }
  };

  /** Validates in the 1D case that all points are visited in order, and point
   * values are in bounds of the last cell checked */
public:
  class AssertingIntersectVisitor
      : public std::enable_shared_from_this<AssertingIntersectVisitor>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(AssertingIntersectVisitor)
  public:
    const std::shared_ptr<IntersectVisitor> in_;
    const int numDims;
    const int bytesPerDim;
    std::deque<char> const lastDocValue;
    std::deque<char> const lastMinPackedValue;
    std::deque<char> const lastMaxPackedValue;

  private:
    Relation lastCompareResult = static_cast<Relation>(0);
    int lastDocID = -1;
    int docBudget = 0;

  public:
    AssertingIntersectVisitor(int numDims, int bytesPerDim,
                              std::shared_ptr<IntersectVisitor> in_);

    void visit(int docID)  override;

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    void grow(int count) override;

    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue) override;
  };

public:
  std::shared_ptr<NumericDocValues>
  getNumericDocValues(const std::wstring &field)  override;

  std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedDocValues>
  getSortedDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
      const std::wstring &field)  override;

  std::shared_ptr<SortedSetDocValues>
  getSortedSetDocValues(const std::wstring &field)  override;

  std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field)  override;

  std::shared_ptr<PointValues>
  getPointValues(const std::wstring &field)  override;

  /** Wraps a Bits but with additional asserts */
public:
  class AssertingBits : public std::enable_shared_from_this<AssertingBits>,
                        public Bits
  {
    GET_CLASS_NAME(AssertingBits)
  private:
    const std::shared_ptr<Thread> creationThread = Thread::currentThread();

  public:
    const std::shared_ptr<Bits> in_;

    AssertingBits(std::shared_ptr<Bits> in_);

    bool get(int index) override;

    int length() override;
  };

public:
  std::shared_ptr<Bits> getLiveDocs() override;

  // we don't change behavior of the reader: just validate the API.

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<AssertingLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<AssertingLeafReader>(
        FilterLeafReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
