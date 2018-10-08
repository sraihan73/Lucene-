#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/SortField.h"
#include  "core/src/java/org/apache/lucene/search/LongValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/LongValues.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
using NumericDocValues = org::apache::lucene::index::NumericDocValues;

/**
 * Base class for producing {@link DoubleValues}
 *
 * To obtain a {@link DoubleValues} object for a leaf reader, clients should
 * call
 * {@link #rewrite(IndexSearcher)} against the top-level searcher, and then
 * call {@link #getValues(LeafReaderContext, DoubleValues)} on the resulting
 * DoubleValuesSource.
 *
 * DoubleValuesSource objects for NumericDocValues fields can be obtained by
 * calling
 * {@link #fromDoubleField(std::wstring)}, {@link #fromFloatField(std::wstring)}, {@link
 * #fromIntField(std::wstring)} or {@link #fromLongField(std::wstring)}, or from {@link
 * #fromField(std::wstring, LongToDoubleFunction)} if special long-to-double encoding
 * is required.
 *
 * Scores may be used as a source for value calculations by wrapping a {@link
 * Scorer} using
 * {@link #fromScorer(Scorer)} and passing the resulting DoubleValues to {@link
 * #getValues(LeafReaderContext, DoubleValues)}. The scores can then be accessed
 * using the {@link #SCORES} DoubleValuesSource.
 */
class DoubleValuesSource
    : public std::enable_shared_from_this<DoubleValuesSource>,
      public SegmentCacheable
{
  GET_CLASS_NAME(DoubleValuesSource)

  /**
   * Returns a {@link DoubleValues} instance for the passed-in LeafReaderContext
   * and scores
   *
   * If scores are not needed to calculate the values (ie {@link #needsScores()
   * returns false}, callers may safely pass {@code null} for the {@code scores}
   * parameter.
   */
public:
  virtual std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores) = 0;

  /**
   * Return true if document scores are needed to calculate values
   */
  virtual bool needsScores() = 0;

  /**
   * An explanation of the value for the named document.
   *
   * @param ctx the readers context to create the {@link Explanation} for.
   * @param docId the document's id relative to the given context's reader
   * @return an Explanation for the value
   * @throws IOException if an {@link IOException} occurs
   */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) ;

  /**
   * Return a DoubleValuesSource specialised for the given IndexSearcher
   *
   * Implementations should assume that this will only be called once.
   * IndexReader-independent implementations can just return {@code this}
   *
   * Queries that use DoubleValuesSource objects should call rewrite() during
   * {@link Query#createWeight(IndexSearcher, bool, float)} rather than
   * during
   * {@link Query#rewrite(IndexReader)} to avoid IndexReader reference leakage
   */
  virtual std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> reader) = 0;

  /**
   * Create a sort field based on the value of this producer
   * @param reverse true if the sort should be decreasing
   */
  virtual std::shared_ptr<SortField> getSortField(bool reverse);

  int hashCode() = 0;
  override

      bool
      equals(std::any obj) = 0;
  override

      std::wstring
      toString() = 0;
  override

      /**
       * Convert to a LongValuesSource by casting the double values to longs
       */
      std::shared_ptr<LongValuesSource>
      toLongValuesSource();

private:
  class LongDoubleValuesSource : public LongValuesSource
  {
    GET_CLASS_NAME(LongDoubleValuesSource)

  private:
    const std::shared_ptr<DoubleValuesSource> inner;

    LongDoubleValuesSource(std::shared_ptr<DoubleValuesSource> inner);

  public:
    std::shared_ptr<LongValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class LongValuesAnonymousInnerClass : public LongValues
    {
      GET_CLASS_NAME(LongValuesAnonymousInnerClass)
    private:
      std::shared_ptr<LongDoubleValuesSource> outerInstance;

      std::shared_ptr<org::apache::lucene::search::DoubleValues> in_;

    public:
      LongValuesAnonymousInnerClass(
          std::shared_ptr<LongDoubleValuesSource> outerInstance,
          std::shared_ptr<org::apache::lucene::search::DoubleValues> in_);

      int64_t longValue()  override;

      bool advanceExact(int doc)  override;

    protected:
      std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
            LongValues::shared_from_this());
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
    std::shared_ptr<LongDoubleValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<LongDoubleValuesSource>(
          LongValuesSource::shared_from_this());
    }
  };

  /**
   * Creates a DoubleValuesSource that wraps a generic NumericDocValues field
   *
   * @param field the field to wrap, must have NumericDocValues
   * @param decoder a function to convert the long-valued doc values to doubles
   */
public:
  static std::shared_ptr<DoubleValuesSource>
  fromField(const std::wstring &field,
            std::function<double(int64_t)> &decoder);

  /**
   * Creates a DoubleValuesSource that wraps a double-valued field
   */
  static std::shared_ptr<DoubleValuesSource>
  fromDoubleField(const std::wstring &field);

  /**
   * Creates a DoubleValuesSource that wraps a float-valued field
   */
  static std::shared_ptr<DoubleValuesSource>
  fromFloatField(const std::wstring &field);

  /**
   * Creates a DoubleValuesSource that wraps a long-valued field
   */
  static std::shared_ptr<DoubleValuesSource>
  fromLongField(const std::wstring &field);

  /**
   * Creates a DoubleValuesSource that wraps an int-valued field
   */
  static std::shared_ptr<DoubleValuesSource>
  fromIntField(const std::wstring &field);

  /**
   * A DoubleValuesSource that exposes a document's score
   *
   * If this source is used as part of a values calculation, then callers must
   * not pass {@code null} as the {@link DoubleValues} parameter on {@link
   * #getValues(LeafReaderContext, DoubleValues)}
   */
  static const std::shared_ptr<DoubleValuesSource> SCORES;

private:
  class DoubleValuesSourceAnonymousInnerClass;

  /**
   * Creates a DoubleValuesSource that always returns a constant value
   */
public:
  static std::shared_ptr<DoubleValuesSource> constant(double value);

private:
  class ConstantValuesSource;

  /**
   * Returns a DoubleValues instance that wraps scores returned by a Scorer
   */
public:
  static std::shared_ptr<DoubleValues>
  fromScorer(std::shared_ptr<Scorer> scorer);

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::search::Scorer> scorer;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::Scorer> scorer);

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          DoubleValues::shared_from_this());
    }
  };

private:
  class FieldValuesSource;

private:
  class DoubleValuesSortField : public SortField
  {
    GET_CLASS_NAME(DoubleValuesSortField)

  public:
    const std::shared_ptr<DoubleValuesSource> producer;

    DoubleValuesSortField(std::shared_ptr<DoubleValuesSource> producer,
                          bool reverse);

    bool needsScores() override;

    virtual std::wstring toString();

    std::shared_ptr<SortField> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

  protected:
    std::shared_ptr<DoubleValuesSortField> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesSortField>(
          SortField::shared_from_this());
    }
  };

private:
  class DoubleValuesHolder
      : public std::enable_shared_from_this<DoubleValuesHolder>
  {
    GET_CLASS_NAME(DoubleValuesHolder)
  public:
    std::shared_ptr<DoubleValues> values;
  };

private:
  class DoubleValuesComparatorSource : public FieldComparatorSource
  {
    GET_CLASS_NAME(DoubleValuesComparatorSource)
  private:
    const std::shared_ptr<DoubleValuesSource> producer;

  public:
    DoubleValuesComparatorSource(std::shared_ptr<DoubleValuesSource> producer);

    std::shared_ptr<FieldComparator<double>>
    newComparator(const std::wstring &fieldname, int numHits, int sortPos,
                  bool reversed) override;

  private:
    class DoubleComparatorAnonymousInnerClass
        : public FieldComparator::DoubleComparator
    {
      GET_CLASS_NAME(DoubleComparatorAnonymousInnerClass)
    private:
      std::shared_ptr<DoubleValuesComparatorSource> outerInstance;

    public:
      DoubleComparatorAnonymousInnerClass(
          std::shared_ptr<DoubleValuesComparatorSource> outerInstance,
          int numHits, const std::wstring &fieldname);

      std::shared_ptr<LeafReaderContext> ctx;
      std::shared_ptr<DoubleValuesHolder> holder;

    protected:
      std::shared_ptr<NumericDocValues> getNumericDocValues(
          std::shared_ptr<LeafReaderContext> context,
          const std::wstring &field)  override;

    public:
      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

    protected:
      std::shared_ptr<DoubleComparatorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DoubleComparatorAnonymousInnerClass>(
            FieldComparator.DoubleComparator::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DoubleValuesComparatorSource> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesComparatorSource>(
          FieldComparatorSource::shared_from_this());
    }
  };

private:
  static std::shared_ptr<NumericDocValues>
  asNumericDocValues(std::shared_ptr<DoubleValuesHolder> in_,
                     std::function<int64_t(double)> &converter);

private:
  class NumericDocValuesAnonymousInnerClass : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<
        org::apache::lucene::search::DoubleValuesSource::DoubleValuesHolder>
        in_;
    std::function<int64_t(double)> converter;

  public:
    NumericDocValuesAnonymousInnerClass(
        std::shared_ptr<
            org::apache::lucene::search::DoubleValuesSource::DoubleValuesHolder>
            in_,
        std::function<int64_t(double)> &converter);

    int64_t longValue()  override;

    bool advanceExact(int target)  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

  /**
   * Create a DoubleValuesSource that returns the score of a particular query
   */
public:
  static std::shared_ptr<DoubleValuesSource>
  fromQuery(std::shared_ptr<Query> query);

private:
  class QueryDoubleValuesSource;

private:
  class WeightDoubleValuesSource;
};

} // namespace org::apache::lucene::search
class DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass
    : public DoubleValuesSource
{
  GET_CLASS_NAME(DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass)
public:
  DoubleValuesSourceAnonymousInnerClass();

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) override;

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher) override;

protected:
  std::shared_ptr<DoubleValuesSourceAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<DoubleValuesSourceAnonymousInnerClass>(
        DoubleValuesSource::shared_from_this());
  }
};
class DoubleValuesSource::ConstantValuesSource : public DoubleValuesSource
{
  GET_CLASS_NAME(DoubleValuesSource::ConstantValuesSource)

private:
  const double value;

  ConstantValuesSource(double value);

public:
  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher) override;

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<ConstantValuesSource> outerInstance;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<ConstantValuesSource> outerInstance);

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          DoubleValues::shared_from_this());
    }
  };

public:
  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) override;

  virtual int hashCode();

  virtual bool equals(std::any o);

  virtual std::wstring toString();

protected:
  std::shared_ptr<ConstantValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<ConstantValuesSource>(
        DoubleValuesSource::shared_from_this());
  }
};
class DoubleValuesSource::FieldValuesSource : public DoubleValuesSource
{
  GET_CLASS_NAME(DoubleValuesSource::FieldValuesSource)

public:
  const std::wstring field;
  const std::function<double(int64_t)> decoder;

private:
  FieldValuesSource(const std::wstring &field,
                    std::function<double(int64_t)> &decoder);

public:
  virtual bool equals(std::any o);

  virtual std::wstring toString();

  virtual int hashCode();

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass2 : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<FieldValuesSource> outerInstance;

    std::shared_ptr<NumericDocValues> values;

  public:
    DoubleValuesAnonymousInnerClass2(
        std::shared_ptr<FieldValuesSource> outerInstance,
        std::shared_ptr<NumericDocValues> values);

    double doubleValue()  override;

    bool advanceExact(int target)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass2>(
          DoubleValues::shared_from_this());
    }
  };

public:
  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) 
      override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<FieldValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<FieldValuesSource>(
        DoubleValuesSource::shared_from_this());
  }
};
class DoubleValuesSource::QueryDoubleValuesSource : public DoubleValuesSource
{
  GET_CLASS_NAME(DoubleValuesSource::QueryDoubleValuesSource)

private:
  const std::shared_ptr<Query> query;

  QueryDoubleValuesSource(std::shared_ptr<Query> query);

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

  bool needsScores() override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

  virtual std::wstring toString();

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

protected:
  std::shared_ptr<QueryDoubleValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<QueryDoubleValuesSource>(
        DoubleValuesSource::shared_from_this());
  }
};
class DoubleValuesSource::WeightDoubleValuesSource : public DoubleValuesSource
{
  GET_CLASS_NAME(DoubleValuesSource::WeightDoubleValuesSource)

private:
  const std::shared_ptr<Weight> weight;

  WeightDoubleValuesSource(std::shared_ptr<Weight> weight);

public:
  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<WeightDoubleValuesSource> outerInstance;

    std::shared_ptr<org::apache::lucene::search::Scorer> scorer;
    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> it;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<WeightDoubleValuesSource> outerInstance,
        std::shared_ptr<org::apache::lucene::search::Scorer> scorer,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> it);

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          DoubleValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) 
      override;

  bool needsScores() override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

protected:
  std::shared_ptr<WeightDoubleValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<WeightDoubleValuesSource>(
        DoubleValuesSource::shared_from_this());
  }
};
