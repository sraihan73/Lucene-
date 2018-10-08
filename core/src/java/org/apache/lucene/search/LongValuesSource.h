#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/search/LongValues.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/SortField.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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
 * Base class for producing {@link LongValues}
 *
 * To obtain a {@link LongValues} object for a leaf reader, clients should
 * call {@link #rewrite(IndexSearcher)} against the top-level searcher, and
 * then {@link #getValues(LeafReaderContext, DoubleValues)}.
 *
 * LongValuesSource objects for long and int-valued NumericDocValues fields can
 * be obtained by calling {@link #fromLongField(std::wstring)} and {@link
 * #fromIntField(std::wstring)}.
 *
 * To obtain a LongValuesSource from a float or double-valued NumericDocValues
 * field, use {@link DoubleValuesSource#fromFloatField(std::wstring)} or {@link
 * DoubleValuesSource#fromDoubleField(std::wstring)} and then call {@link
 * DoubleValuesSource#toLongValuesSource()}.
 */
class LongValuesSource : public std::enable_shared_from_this<LongValuesSource>,
                         public SegmentCacheable
{
  GET_CLASS_NAME(LongValuesSource)

  /**
   * Returns a {@link LongValues} instance for the passed-in LeafReaderContext
   * and scores
   *
   * If scores are not needed to calculate the values (ie {@link #needsScores()
   * returns false}, callers may safely pass {@code null} for the {@code scores}
   * parameter.
   */
public:
  virtual std::shared_ptr<LongValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores) = 0;

  /**
   * Return true if document scores are needed to calculate values
   */
  virtual bool needsScores() = 0;

  int hashCode() = 0;
  override

      bool
      equals(std::any obj) = 0;
  override

      std::wstring
      toString() = 0;
  override

      /**
       * Return a LongValuesSource specialised for the given IndexSearcher
       *
       * Implementations should assume that this will only be called once.
       * IndexSearcher-independent implementations can just return {@code this}
       */
      virtual std::shared_ptr<LongValuesSource>
      rewrite(std::shared_ptr<IndexSearcher> searcher) = 0;

  /**
   * Create a sort field based on the value of this producer
   * @param reverse true if the sort should be decreasing
   */
  virtual std::shared_ptr<SortField> getSortField(bool reverse);

  /**
   * Convert to a DoubleValuesSource by casting long values to doubles
   */
  virtual std::shared_ptr<DoubleValuesSource> toDoubleValuesSource();

private:
  class DoubleLongValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(DoubleLongValuesSource)

  private:
    const std::shared_ptr<LongValuesSource> inner;

    DoubleLongValuesSource(std::shared_ptr<LongValuesSource> inner);

  public:
    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class DoubleValuesAnonymousInnerClass : public DoubleValues
    {
      GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
    private:
      std::shared_ptr<DoubleLongValuesSource> outerInstance;

      std::shared_ptr<org::apache::lucene::search::LongValues> v;

    public:
      DoubleValuesAnonymousInnerClass(
          std::shared_ptr<DoubleLongValuesSource> outerInstance,
          std::shared_ptr<org::apache::lucene::search::LongValues> v);

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
    std::shared_ptr<DoubleValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    virtual std::wstring toString();

    bool needsScores() override;

    virtual bool equals(std::any o);

    virtual int hashCode();

  protected:
    std::shared_ptr<DoubleLongValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<DoubleLongValuesSource>(
          DoubleValuesSource::shared_from_this());
    }
  };

  /**
   * Creates a LongValuesSource that wraps a long-valued field
   */
public:
  static std::shared_ptr<LongValuesSource>
  fromLongField(const std::wstring &field);

  /**
   * Creates a LongValuesSource that wraps an int-valued field
   */
  static std::shared_ptr<LongValuesSource>
  fromIntField(const std::wstring &field);

  /**
   * Creates a LongValuesSource that always returns a constant value
   */
  static std::shared_ptr<LongValuesSource> constant(int64_t value);

private:
  class ConstantLongValuesSource;

private:
  class FieldValuesSource;

private:
  class LongValuesSortField : public SortField
  {
    GET_CLASS_NAME(LongValuesSortField)

  public:
    const std::shared_ptr<LongValuesSource> producer;

    LongValuesSortField(std::shared_ptr<LongValuesSource> producer,
                        bool reverse);

    bool needsScores() override;

    virtual std::wstring toString();

    std::shared_ptr<SortField> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

  protected:
    std::shared_ptr<LongValuesSortField> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesSortField>(
          SortField::shared_from_this());
    }
  };

private:
  class LongValuesHolder : public std::enable_shared_from_this<LongValuesHolder>
  {
    GET_CLASS_NAME(LongValuesHolder)
  public:
    std::shared_ptr<LongValues> values;
  };

private:
  class LongValuesComparatorSource : public FieldComparatorSource
  {
    GET_CLASS_NAME(LongValuesComparatorSource)
  private:
    const std::shared_ptr<LongValuesSource> producer;

  public:
    LongValuesComparatorSource(std::shared_ptr<LongValuesSource> producer);

    std::shared_ptr<FieldComparator<int64_t>>
    newComparator(const std::wstring &fieldname, int numHits, int sortPos,
                  bool reversed) override;

  private:
    class LongComparatorAnonymousInnerClass
        : public FieldComparator::LongComparator
    {
      GET_CLASS_NAME(LongComparatorAnonymousInnerClass)
    private:
      std::shared_ptr<LongValuesComparatorSource> outerInstance;

    public:
      LongComparatorAnonymousInnerClass(
          std::shared_ptr<LongValuesComparatorSource> outerInstance,
          int numHits, const std::wstring &fieldname);

      std::shared_ptr<LeafReaderContext> ctx;
      std::shared_ptr<LongValuesHolder> holder;

    protected:
      std::shared_ptr<NumericDocValues> getNumericDocValues(
          std::shared_ptr<LeafReaderContext> context,
          const std::wstring &field)  override;

    public:
      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

    protected:
      std::shared_ptr<LongComparatorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<LongComparatorAnonymousInnerClass>(
            FieldComparator.LongComparator::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<LongValuesComparatorSource> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesComparatorSource>(
          FieldComparatorSource::shared_from_this());
    }
  };

private:
  static std::shared_ptr<LongValues>
  toLongValues(std::shared_ptr<NumericDocValues> in_);

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  private:
    std::shared_ptr<NumericDocValues> in_;

  public:
    LongValuesAnonymousInnerClass(std::shared_ptr<NumericDocValues> in_);

    int64_t longValue()  override;

    bool advanceExact(int target)  override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
          LongValues::shared_from_this());
    }
  };

private:
  static std::shared_ptr<NumericDocValues>
  asNumericDocValues(std::shared_ptr<LongValuesHolder> in_);

private:
  class NumericDocValuesAnonymousInnerClass : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<
        org::apache::lucene::search::LongValuesSource::LongValuesHolder>
        in_;

  public:
    NumericDocValuesAnonymousInnerClass(
        std::shared_ptr<
            org::apache::lucene::search::LongValuesSource::LongValuesHolder>
            in_);

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
};

} // namespace org::apache::lucene::search
class LongValuesSource::ConstantLongValuesSource : public LongValuesSource
{
  GET_CLASS_NAME(LongValuesSource::ConstantLongValuesSource)

private:
  const int64_t value;

  ConstantLongValuesSource(int64_t value);

public:
  std::shared_ptr<LongValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  private:
    std::shared_ptr<ConstantLongValuesSource> outerInstance;

  public:
    LongValuesAnonymousInnerClass(
        std::shared_ptr<ConstantLongValuesSource> outerInstance);

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

  virtual int hashCode();

  virtual bool equals(std::any o);

  virtual std::wstring toString();

  std::shared_ptr<LongValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<ConstantLongValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<ConstantLongValuesSource>(
        LongValuesSource::shared_from_this());
  }
};
class LongValuesSource::FieldValuesSource : public LongValuesSource
{
  GET_CLASS_NAME(LongValuesSource::FieldValuesSource)

public:
  const std::wstring field;

private:
  FieldValuesSource(const std::wstring &field);

public:
  virtual bool equals(std::any o);

  virtual std::wstring toString();

  virtual int hashCode();

  std::shared_ptr<LongValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  bool needsScores() override;

  std::shared_ptr<LongValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<FieldValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<FieldValuesSource>(
        LongValuesSource::shared_from_this());
  }
};
