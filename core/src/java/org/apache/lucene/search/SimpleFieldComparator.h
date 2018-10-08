#pragma once
#include "stringhelper.h"
#include <memory>

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

/**
 * Base {@link FieldComparator} implementation that is used for all contexts.
 *
 * @lucene.experimental
 */
template <typename T>
class SimpleFieldComparator : public FieldComparator<T>,
                              public LeafFieldComparator
{
  GET_CLASS_NAME(SimpleFieldComparator)

  /** This method is called before collecting <code>context</code>. */
protected:
  virtual void doSetNextReader(std::shared_ptr<LeafReaderContext> context) = 0;

public:
  std::shared_ptr<LeafFieldComparator>
  getLeafComparator(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override final
  {
    doSetNextReader(context);
    return shared_from_this();
  }

  void setScorer(std::shared_ptr<Scorer> scorer)  override {}

protected:
  std::shared_ptr<SimpleFieldComparator> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFieldComparator>(
        FieldComparator<T>::shared_from_this());
  }
};

  /**
   * Base FieldComparator class for numeric types
   */
  template <typename T1>
  class NumericComparator : public SimpleFieldComparator<T1>
  {
    GET_CLASS_NAME(NumericComparator)
    static_assert(std::is_base_of<Number, T>::value,
                  L"T must inherit from Number");

  protected:
    const T1 missingValue;
    const std::wstring field;
    std::shared_ptr<NumericDocValues> currentReaderValues;

  public:
    NumericComparator(const std::wstring &field, T missingValue)
        : missingValue(missingValue), field(field)
    {
    }

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override
    {
      currentReaderValues = getNumericDocValues(context, field);
    }

    /** Retrieves the NumericDocValues for the field in this segment */
    virtual std::shared_ptr<NumericDocValues>
    getNumericDocValues(std::shared_ptr<LeafReaderContext> context,
                        const std::wstring &field) 
    {
      return DocValues::getNumeric(context->reader(), field);
    }

  protected:
    std::shared_ptr<NumericComparator> shared_from_this()
    {
      return std::static_pointer_cast<NumericComparator>(
          SimpleFieldComparator<T>::shared_from_this());
    }
  };

  /** Parses field's values as double (using {@link
   *  org.apache.lucene.index.LeafReader#getNumericDocValues} and sorts by
   * ascending value */
public:
  class DoubleComparator : public NumericComparator<double>
  {
    GET_CLASS_NAME(DoubleComparator)
  private:
    std::deque<double> const values;
    double bottom = 0;
    double topValue = 0;

    /**
     * Creates a new comparator based on {@link Double#compare} for {@code
     * numHits}. When a document has no value for the field, {@code
     * missingValue} is substituted.
     */
  public:
    DoubleComparator(int numHits, const std::wstring &field,
                     std::optional<double> &missingValue);

  private:
    double getValueForDoc(int doc) ;

  public:
    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    void setBottom(int const bottom) override;

    void setTopValue(std::optional<double> &value) override;

    std::optional<double> value(int slot) override;

    int compareTop(int doc)  override;

  protected:
    std::shared_ptr<DoubleComparator> shared_from_this()
    {
      return std::static_pointer_cast<DoubleComparator>(
          NumericComparator<double>::shared_from_this());
    }
  };

  /** Parses field's values as float (using {@link
   *  org.apache.lucene.index.LeafReader#getNumericDocValues(std::wstring)} and sorts
   * by ascending value */
  class FloatComparator : public NumericComparator<float>
  {
    GET_CLASS_NAME(FloatComparator)
  private:
    std::deque<float> const values;
    float bottom = 0;
    float topValue = 0;

    /**
     * Creates a new comparator based on {@link Float#compare} for {@code
     * numHits}. When a document has no value for the field, {@code
     * missingValue} is substituted.
     */
  public:
    FloatComparator(int numHits, const std::wstring &field,
                    std::optional<float> &missingValue);

  private:
    float getValueForDoc(int doc) ;

  public:
    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    void setBottom(int const bottom) override;

    void setTopValue(std::optional<float> &value) override;

    std::optional<float> value(int slot) override;

    int compareTop(int doc)  override;

  protected:
    std::shared_ptr<FloatComparator> shared_from_this()
    {
      return std::static_pointer_cast<FloatComparator>(
          NumericComparator<float>::shared_from_this());
    }
  };

  /** Parses field's values as int (using {@link
   *  org.apache.lucene.index.LeafReader#getNumericDocValues(std::wstring)} and sorts
   * by ascending value */
  class IntComparator : public NumericComparator<int>
  {
    GET_CLASS_NAME(IntComparator)
  private:
    std::deque<int> const values;
    int bottom = 0; // Value of bottom of queue
    int topValue = 0;

    /**
     * Creates a new comparator based on {@link Integer#compare} for {@code
     * numHits}. When a document has no value for the field, {@code
     * missingValue} is substituted.
     */
  public:
    IntComparator(int numHits, const std::wstring &field,
                  std::optional<int> &missingValue);

  private:
    int getValueForDoc(int doc) ;

  public:
    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    void setBottom(int const bottom) override;

    void setTopValue(std::optional<int> &value) override;

    std::optional<int> value(int slot) override;

    int compareTop(int doc)  override;

  protected:
    std::shared_ptr<IntComparator> shared_from_this()
    {
      return std::static_pointer_cast<IntComparator>(
          NumericComparator<int>::shared_from_this());
    }
  };

  /** Parses field's values as long (using {@link
   *  org.apache.lucene.index.LeafReader#getNumericDocValues(std::wstring)} and sorts
   * by ascending value */
  class LongComparator : public NumericComparator<int64_t>
  {
    GET_CLASS_NAME(LongComparator)
  private:
    std::deque<int64_t> const values;
    int64_t bottom = 0;
    int64_t topValue = 0;

    /**
     * Creates a new comparator based on {@link Long#compare} for {@code
     * numHits}. When a document has no value for the field, {@code
     * missingValue} is substituted.
     */
  public:
    LongComparator(int numHits, const std::wstring &field,
                   std::optional<int64_t> &missingValue);

  private:
    int64_t getValueForDoc(int doc) ;

  public:
    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    void setBottom(int const bottom) override;

    void setTopValue(std::optional<int64_t> &value) override;

    std::optional<int64_t> value(int slot) override;

    int compareTop(int doc)  override;

  protected:
    std::shared_ptr<LongComparator> shared_from_this()
    {
      return std::static_pointer_cast<LongComparator>(
          NumericComparator<long>::shared_from_this());
    }
  };

  /** Sorts by descending relevance.  NOTE: if you are
   *  sorting only by descending relevance and then
   *  secondarily by ascending docID, performance is faster
   *  using {@link TopScoreDocCollector} directly (which {@link
   *  IndexSearcher#search} uses when no {@link Sort} is
   *  specified). */
  class RelevanceComparator final : public FieldComparator<float>,
                                    public LeafFieldComparator
  {
    GET_CLASS_NAME(RelevanceComparator)
  private:
    std::deque<float> const scores;
    float bottom = 0;
    std::shared_ptr<Scorer> scorer;
    float topValue = 0;

    /** Creates a new comparator based on relevance for {@code numHits}. */
  public:
    RelevanceComparator(int numHits);

    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    std::shared_ptr<LeafFieldComparator>
    getLeafComparator(std::shared_ptr<LeafReaderContext> context) override;

    void setBottom(int const bottom) override;

    void setTopValue(std::optional<float> &value) override;

    void setScorer(std::shared_ptr<Scorer> scorer) override;

    std::optional<float> value(int slot) override;

    // Override because we sort reverse of natural Float order:
    int compareValues(std::optional<float> &first,
                      std::optional<float> &second) override;

    int compareTop(int doc)  override;

  protected:
    std::shared_ptr<RelevanceComparator> shared_from_this()
    {
      return std::static_pointer_cast<RelevanceComparator>(
          FieldComparator<float>::shared_from_this());
    }
  };

  /** Sorts by ascending docID */
  class DocComparator final : public FieldComparator<int>,
                              public LeafFieldComparator
  {
    GET_CLASS_NAME(DocComparator)
  private:
    std::deque<int> const docIDs;
    int docBase = 0;
    int bottom = 0;
    int topValue = 0;

    /** Creates a new comparator based on document ids for {@code numHits} */
  public:
    DocComparator(int numHits);

    int compare(int slot1, int slot2) override;

    int compareBottom(int doc) override;

    void copy(int slot, int doc) override;

    std::shared_ptr<LeafFieldComparator>
    getLeafComparator(std::shared_ptr<LeafReaderContext> context) override;

    void setBottom(int const bottom) override;

    void setTopValue(std::optional<int> &value) override;

    std::optional<int> value(int slot) override;

    int compareTop(int doc) override;

    void setScorer(std::shared_ptr<Scorer> scorer) override;

  protected:
    std::shared_ptr<DocComparator> shared_from_this()
    {
      return std::static_pointer_cast<DocComparator>(
          FieldComparator<int>::shared_from_this());
    }
  };
  /** Sorts by field's natural Term sort order, using
   *  ordinals.  This is functionally equivalent to {@link
   *  org.apache.lucene.search.FieldComparator.TermValComparator}, but it first
   * resolves the string to their relative ordinal positions (using the index
   *  returned by {@link
   * org.apache.lucene.index.LeafReader#getSortedDocValues(std::wstring)}), and does
   * most comparisons using the ordinals.  For medium to large results, this
   * comparator will be much faster than {@link
   * org.apache.lucene.search.FieldComparator.TermValComparator}.  For very
   * small result sets it may be slower. */
  class TermOrdValComparator
      : public FieldComparator<std::shared_ptr<BytesRef>>,
        public LeafFieldComparator
  {
    GET_CLASS_NAME(TermOrdValComparator)
    /* Ords for each slot.
       @lucene.internal */
  public:
    std::deque<int> const ords;

    /* Values for each slot.
       @lucene.internal */
    std::deque<std::shared_ptr<BytesRef>> const values;

  private:
    std::deque<std::shared_ptr<BytesRefBuilder>> const tempBRs;

    /* Which reader last copied a value into the slot. When
       we compare two slots, we just compare-by-ord if the
       readerGen is the same; else we must compare the
       values (slower).
       @lucene.internal */
  public:
    std::deque<int> const readerGen;

    /* Gen of current reader we are on.
       @lucene.internal */
    int currentReaderGen = -1;

    /* Current reader's doc ord/values.
       @lucene.internal */
    std::shared_ptr<SortedDocValues> termsIndex;

  private:
    const std::wstring field;

    /* Bottom slot, or -1 if queue isn't full yet
       @lucene.internal */
  public:
    int bottomSlot = -1;

    /* Bottom ord (same as ords[bottomSlot] once bottomSlot
       is set).  Cached for faster compares.
       @lucene.internal */
    int bottomOrd = 0;

    /* True if current bottom slot matches the current
       reader.
       @lucene.internal */
    bool bottomSameReader = false;

    /* Bottom value (same as values[bottomSlot] once
       bottomSlot is set).  Cached for faster compares.
      @lucene.internal */
    std::shared_ptr<BytesRef> bottomValue;

    /** Set by setTopValue. */
    std::shared_ptr<BytesRef> topValue;
    bool topSameReader = false;
    int topOrd = 0;

    /** -1 if missing values are sorted first, 1 if they are
     *  sorted last */
    const int missingSortCmp;

    /** Which ordinal to use for a missing value. */
    const int missingOrd;

    /** Creates this, sorting missing values first. */
    TermOrdValComparator(int numHits, const std::wstring &field);

    /** Creates this, with control over how missing values
     *  are sorted.  Pass sortMissingLast=true to put
     *  missing values at the end. */
    TermOrdValComparator(int numHits, const std::wstring &field,
                         bool sortMissingLast);

  private:
    int getOrdForDoc(int doc) ;

  public:
    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    /** Retrieves the SortedDocValues for the field in this segment */
  protected:
    virtual std::shared_ptr<SortedDocValues>
    getSortedDocValues(std::shared_ptr<LeafReaderContext> context,
                       const std::wstring &field) ;

  public:
    std::shared_ptr<LeafFieldComparator> getLeafComparator(
        std::shared_ptr<LeafReaderContext> context)  override;

    void setBottom(int const bottom)  override;

    void setTopValue(std::shared_ptr<BytesRef> value) override;

    std::shared_ptr<BytesRef> value(int slot) override;

    int compareTop(int doc)  override;

    int compareValues(std::shared_ptr<BytesRef> val1,
                      std::shared_ptr<BytesRef> val2) override;

    void setScorer(std::shared_ptr<Scorer> scorer) override;

  protected:
    std::shared_ptr<TermOrdValComparator> shared_from_this()
    {
      return std::static_pointer_cast<TermOrdValComparator>(
          FieldComparator<org.apache.lucene.util.BytesRef>::shared_from_this());
    }
  };
  /** Sorts by field's natural Term sort order.  All
   *  comparisons are done using BytesRef.compareTo, which is
   *  slow for medium to large result sets but possibly
   *  very fast for very small results sets. */
  class TermValComparator : public FieldComparator<std::shared_ptr<BytesRef>>,
                            public LeafFieldComparator
  {
    GET_CLASS_NAME(TermValComparator)

  private:
    std::deque<std::shared_ptr<BytesRef>> const values;
    std::deque<std::shared_ptr<BytesRefBuilder>> const tempBRs;
    std::shared_ptr<BinaryDocValues> docTerms;
    const std::wstring field;
    std::shared_ptr<BytesRef> bottom;
    std::shared_ptr<BytesRef> topValue;
    const int missingSortCmp;

    /** Sole constructor. */
  public:
    TermValComparator(int numHits, const std::wstring &field,
                      bool sortMissingLast);

  private:
    std::shared_ptr<BytesRef> getValueForDoc(int doc) ;

  public:
    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    /** Retrieves the BinaryDocValues for the field in this segment */
  protected:
    virtual std::shared_ptr<BinaryDocValues>
    getBinaryDocValues(std::shared_ptr<LeafReaderContext> context,
                       const std::wstring &field) ;

  public:
    std::shared_ptr<LeafFieldComparator> getLeafComparator(
        std::shared_ptr<LeafReaderContext> context)  override;

    void setBottom(int const bottom) override;

    void setTopValue(std::shared_ptr<BytesRef> value) override;

    std::shared_ptr<BytesRef> value(int slot) override;

    int compareValues(std::shared_ptr<BytesRef> val1,
                      std::shared_ptr<BytesRef> val2) override;

    int compareTop(int doc)  override;

    void setScorer(std::shared_ptr<Scorer> scorer) override;

  protected:
    std::shared_ptr<TermValComparator> shared_from_this()
    {
      return std::static_pointer_cast<TermValComparator>(
          FieldComparator<org.apache.lucene.util.BytesRef>::shared_from_this());
    }
  };
} // namespace org::apache::lucene::search
