#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/LongValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/search/LongValues.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/SortField.h"
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
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
namespace org::apache::lucene::queries::function
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using FieldComparatorSource =
    org::apache::lucene::search::FieldComparatorSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleFieldComparator =
    org::apache::lucene::search::SimpleFieldComparator;
using SortField = org::apache::lucene::search::SortField;

/**
 * Instantiates {@link FunctionValues} for a particular reader.
 * <br>
 * Often used when creating a {@link FunctionQuery}.
 *
 *
 */
class ValueSource : public std::enable_shared_from_this<ValueSource>
{
  GET_CLASS_NAME(ValueSource)

  /**
   * Gets the values for this reader and the context that was previously
   * passed to createWeight().  The values must be consumed in a forward
   * docID manner, and you must call this method again to iterate through
   * the values again.
   */
public:
  virtual std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) = 0;

  bool equals(std::any o) = 0;
  override

      int
      hashCode() = 0;
  override

      /**
       * description of field, used in explain()
       */
      virtual std::wstring
      description() = 0;

  virtual std::wstring toString();

  /**
   * Implementations should propagate createWeight to sub-ValueSources which can
   * optionally store weight info in the context. The context object will be
   * passed to getValues() where this info can be retrieved.
   */
  virtual void
  createWeight(std::unordered_map context,
               std::shared_ptr<IndexSearcher> searcher) ;

  /**
   * Returns a new non-threadsafe context map_obj.
   */
  static std::unordered_map newContext(std::shared_ptr<IndexSearcher> searcher);

private:
  class FakeScorer : public Scorer
  {
    GET_CLASS_NAME(FakeScorer)

  public:
    int current = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    float score_ = 0;

    FakeScorer();

    int docID() override;

    float score()  override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

  protected:
    std::shared_ptr<FakeScorer> shared_from_this()
    {
      return std::static_pointer_cast<FakeScorer>(
          org.apache.lucene.search.Scorer::shared_from_this());
    }
  };

  /**
   * Expose this ValueSource as a LongValuesSource
   */
public:
  virtual std::shared_ptr<LongValuesSource> asLongValuesSource();

private:
  class WrappedLongValuesSource : public LongValuesSource
  {
    GET_CLASS_NAME(WrappedLongValuesSource)

  private:
    const std::shared_ptr<ValueSource> in_;

    WrappedLongValuesSource(std::shared_ptr<ValueSource> in_);

  public:
    std::shared_ptr<LongValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class LongValuesAnonymousInnerClass : public LongValues
    {
      GET_CLASS_NAME(LongValuesAnonymousInnerClass)
    private:
      std::shared_ptr<WrappedLongValuesSource> outerInstance;

      std::shared_ptr<DoubleValues> scores;
      std::shared_ptr<
          org::apache::lucene::queries::function::ValueSource::FakeScorer>
          scorer;
      std::shared_ptr<org::apache::lucene::queries::function::FunctionValues>
          fv;

    public:
      LongValuesAnonymousInnerClass(
          std::shared_ptr<WrappedLongValuesSource> outerInstance,
          std::shared_ptr<DoubleValues> scores,
          std::shared_ptr<
              org::apache::lucene::queries::function::ValueSource::FakeScorer>
              scorer,
          std::shared_ptr<
              org::apache::lucene::queries::function::FunctionValues>
              fv);

      int64_t longValue()  override;

      bool advanceExact(int doc)  override;

    protected:
      std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
            org.apache.lucene.search.LongValues::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    bool needsScores() override;

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();

    std::shared_ptr<LongValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

  protected:
    std::shared_ptr<WrappedLongValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<WrappedLongValuesSource>(
          org.apache.lucene.search.LongValuesSource::shared_from_this());
    }
  };

  /**
   * Expose this ValueSource as a DoubleValuesSource
   */
public:
  virtual std::shared_ptr<DoubleValuesSource> asDoubleValuesSource();

private:
  class WrappedDoubleValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(WrappedDoubleValuesSource)

  private:
    const std::shared_ptr<ValueSource> in_;
    std::shared_ptr<IndexSearcher> searcher;

    WrappedDoubleValuesSource(std::shared_ptr<ValueSource> in_);

  public:
    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class DoubleValuesAnonymousInnerClass : public DoubleValues
    {
      GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
    private:
      std::shared_ptr<WrappedDoubleValuesSource> outerInstance;

      std::shared_ptr<DoubleValues> scores;
      std::shared_ptr<
          org::apache::lucene::queries::function::ValueSource::FakeScorer>
          scorer;
      std::shared_ptr<org::apache::lucene::queries::function::FunctionValues>
          fv;

    public:
      DoubleValuesAnonymousInnerClass(
          std::shared_ptr<WrappedDoubleValuesSource> outerInstance,
          std::shared_ptr<DoubleValues> scores,
          std::shared_ptr<
              org::apache::lucene::queries::function::ValueSource::FakeScorer>
              scorer,
          std::shared_ptr<
              org::apache::lucene::queries::function::FunctionValues>
              fv);

      double doubleValue()  override;

      bool advanceExact(int doc)  override;

    protected:
      std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
            org.apache.lucene.search.DoubleValues::shared_from_this());
      }
    };

  public:
    bool needsScores() override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
            std::shared_ptr<Explanation> scoreExplanation) 
        override;

    std::shared_ptr<DoubleValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<WrappedDoubleValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<WrappedDoubleValuesSource>(
          org.apache.lucene.search.DoubleValuesSource::shared_from_this());
    }
  };

public:
  static std::shared_ptr<ValueSource>
  fromDoubleValuesSource(std::shared_ptr<DoubleValuesSource> in_);

private:
  class FromDoubleValuesSource;

  //
  // Sorting by function
  //

  /**
   * EXPERIMENTAL: This method is subject to change.
   * <p>
   * Get the SortField for this ValueSource.  Uses the {@link
   * #getValues(java.util.Map, org.apache.lucene.index.LeafReaderContext)} to
   * populate the SortField.
   *
   * @param reverse true if this is a reverse sort.
   * @return The {@link org.apache.lucene.search.SortField} for the ValueSource
   */
public:
  virtual std::shared_ptr<SortField> getSortField(bool reverse);

public:
  class ValueSourceSortField : public SortField
  {
    GET_CLASS_NAME(ValueSourceSortField)
  private:
    std::shared_ptr<ValueSource> outerInstance;

  public:
    ValueSourceSortField(std::shared_ptr<ValueSource> outerInstance,
                         bool reverse);

    std::shared_ptr<SortField> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

  protected:
    std::shared_ptr<ValueSourceSortField> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceSortField>(
          org.apache.lucene.search.SortField::shared_from_this());
    }
  };

public:
  class ValueSourceComparatorSource : public FieldComparatorSource
  {
    GET_CLASS_NAME(ValueSourceComparatorSource)
  private:
    std::shared_ptr<ValueSource> outerInstance;

    const std::unordered_map context;

  public:
    ValueSourceComparatorSource(std::shared_ptr<ValueSource> outerInstance,
                                std::unordered_map context);

    std::shared_ptr<FieldComparator<double>>
    newComparator(const std::wstring &fieldname, int numHits, int sortPos,
                  bool reversed) override;

  protected:
    std::shared_ptr<ValueSourceComparatorSource> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceComparatorSource>(
          org.apache.lucene.search.FieldComparatorSource::shared_from_this());
    }
  };

  /**
   * Implement a {@link org.apache.lucene.search.FieldComparator} that works
   * off of the {@link FunctionValues} for a ValueSource
   * instead of the normal Lucene FieldComparator that works off of a
   * FieldCache.
   */
public:
  class ValueSourceComparator : public SimpleFieldComparator<double>
  {
    GET_CLASS_NAME(ValueSourceComparator)
  private:
    std::shared_ptr<ValueSource> outerInstance;

    std::deque<double> const values;
    std::shared_ptr<FunctionValues> docVals;
    double bottom = 0;
    const std::unordered_map fcontext;
    double topValue = 0;

  public:
    ValueSourceComparator(std::shared_ptr<ValueSource> outerInstance,
                          std::unordered_map fcontext, int numHits);

    int compare(int slot1, int slot2) override;

    int compareBottom(int doc)  override;

    void copy(int slot, int doc)  override;

    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

    void setBottom(int const bottom) override;

    void setTopValue(std::optional<double> &value) override;

    std::optional<double> value(int slot) override;

    int compareTop(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceComparator> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceComparator>(
          org.apache.lucene.search
              .SimpleFieldComparator<double>::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::queries::function
class ValueSource::FromDoubleValuesSource : public ValueSource
{
  GET_CLASS_NAME(ValueSource::FromDoubleValuesSource)

public:
  const std::shared_ptr<DoubleValuesSource> in_;

private:
  FromDoubleValuesSource(std::shared_ptr<DoubleValuesSource> in_);

public:
  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FunctionValuesAnonymousInnerClass : public FunctionValues
  {
    GET_CLASS_NAME(FunctionValuesAnonymousInnerClass)
  private:
    std::shared_ptr<FromDoubleValuesSource> outerInstance;

    std::shared_ptr<DoubleValues> inner;

  public:
    FunctionValuesAnonymousInnerClass(
        std::shared_ptr<FromDoubleValuesSource> outerInstance,
        std::shared_ptr<DoubleValues> inner);

    std::wstring toString(int doc)  override;

    float floatVal(int doc)  override;

    double doubleVal(int doc)  override;

    bool exists(int doc)  override;

  protected:
    std::shared_ptr<FunctionValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FunctionValuesAnonymousInnerClass>(
          FunctionValues::shared_from_this());
    }
  };

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  std::wstring description() override;

protected:
  std::shared_ptr<FromDoubleValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<FromDoubleValuesSource>(
        ValueSource::shared_from_this());
  }
};
