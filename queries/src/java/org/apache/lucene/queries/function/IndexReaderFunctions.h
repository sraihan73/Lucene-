#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::search
{
class DoubleValuesSource;
}
namespace org::apache::lucene::search
{
class LongValuesSource;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class DoubleValues;
}
namespace org::apache::lucene::search
{
class LongValues;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class IndexReader;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;

/**
 * Class exposing static helper methods for generating DoubleValuesSource
 * instances over some IndexReader statistics
 */
class IndexReaderFunctions final
    : public std::enable_shared_from_this<IndexReaderFunctions>
{
  GET_CLASS_NAME(IndexReaderFunctions)

  // non-instantiable class
private:
  IndexReaderFunctions();

  /**
   * Creates a constant value source returning the docFreq of a given term
   *
   * @see IndexReader#docFreq(Term)
   */
public:
  static std::shared_ptr<DoubleValuesSource>
  docFreq(std::shared_ptr<Term> term);

  /**
   * Creates a constant value source returning the index's maxDoc
   *
   * @see IndexReader#maxDoc()
   */
  static std::shared_ptr<DoubleValuesSource> maxDoc();

  /**
   * Creates a constant value source returning the index's numDocs
   *
   * @see IndexReader#numDocs()
   */
  static std::shared_ptr<DoubleValuesSource> numDocs();

  /**
   * Creates a constant value source returning the number of deleted docs in the
   * index
   *
   * @see IndexReader#numDeletedDocs()
   */
  static std::shared_ptr<DoubleValuesSource> numDeletedDocs();

  /**
   * Creates a constant value source returning the sumTotalTermFreq for a field
   *
   * @see IndexReader#getSumTotalTermFreq(std::wstring)
   */
  static std::shared_ptr<LongValuesSource>
  sumTotalTermFreq(const std::wstring &field);

private:
  class SumTotalTermFreqValuesSource : public LongValuesSource
  {
    GET_CLASS_NAME(SumTotalTermFreqValuesSource)

  private:
    const std::wstring field;

    SumTotalTermFreqValuesSource(const std::wstring &field);

  public:
    std::shared_ptr<LongValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

    bool needsScores() override;

    virtual bool equals(std::any o);

    virtual int hashCode();

    std::shared_ptr<LongValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

    virtual std::wstring toString();

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<SumTotalTermFreqValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<SumTotalTermFreqValuesSource>(
          org.apache.lucene.search.LongValuesSource::shared_from_this());
    }
  };

private:
  class NoCacheConstantLongValuesSource : public LongValuesSource
  {
    GET_CLASS_NAME(NoCacheConstantLongValuesSource)

  public:
    const int64_t value;
    const std::shared_ptr<LongValuesSource> parent;

  private:
    NoCacheConstantLongValuesSource(int64_t value,
                                    std::shared_ptr<LongValuesSource> parent);

  public:
    std::shared_ptr<LongValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class LongValuesAnonymousInnerClass : public LongValues
    {
      GET_CLASS_NAME(LongValuesAnonymousInnerClass)
    private:
      std::shared_ptr<NoCacheConstantLongValuesSource> outerInstance;

    public:
      LongValuesAnonymousInnerClass(
          std::shared_ptr<NoCacheConstantLongValuesSource> outerInstance);

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
    bool needsScores() override;

    std::shared_ptr<LongValuesSource>
    rewrite(std::shared_ptr<IndexSearcher> reader)  override;

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<NoCacheConstantLongValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<NoCacheConstantLongValuesSource>(
          org.apache.lucene.search.LongValuesSource::shared_from_this());
    }
  };

  /**
   * Creates a value source that returns the term freq of a given term for each
   * document
   *
   * @see PostingsEnum#freq()
   */
public:
  static std::shared_ptr<DoubleValuesSource>
  termFreq(std::shared_ptr<Term> term);

private:
  class TermFreqDoubleValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(TermFreqDoubleValuesSource)

  private:
    const std::shared_ptr<Term> term;

    TermFreqDoubleValuesSource(std::shared_ptr<Term> term);

  public:
    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class DoubleValuesAnonymousInnerClass : public DoubleValues
    {
      GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
    private:
      std::shared_ptr<TermFreqDoubleValuesSource> outerInstance;

      std::shared_ptr<PostingsEnum> pe;

    public:
      DoubleValuesAnonymousInnerClass(
          std::shared_ptr<TermFreqDoubleValuesSource> outerInstance,
          std::shared_ptr<PostingsEnum> pe);

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

    std::shared_ptr<DoubleValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

    virtual std::wstring toString();

    virtual bool equals(std::any o);

    virtual int hashCode();

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<TermFreqDoubleValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<TermFreqDoubleValuesSource>(
          org.apache.lucene.search.DoubleValuesSource::shared_from_this());
    }
  };

  /**
   * Creates a constant value source returning the totalTermFreq for a given
   * term
   *
   * @see IndexReader#totalTermFreq(Term)
   */
public:
  static std::shared_ptr<DoubleValuesSource>
  totalTermFreq(std::shared_ptr<Term> term);

  /**
   * Creates a constant value source returning the sumDocFreq for a given field
   *
   * @see IndexReader#getSumDocFreq(std::wstring)
   */
  static std::shared_ptr<DoubleValuesSource>
  sumDocFreq(const std::wstring &field);

  /**
   * Creates a constant value source returning the docCount for a given field
   *
   * @see IndexReader#getDocCount(std::wstring)
   */
  static std::shared_ptr<DoubleValuesSource>
  docCount(const std::wstring &field);

  using ReaderFunction = std::function<double(IndexReader reader)>;

private:
  class IndexReaderDoubleValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(IndexReaderDoubleValuesSource)

  private:
    const ReaderFunction func;
    const std::wstring description;

    IndexReaderDoubleValuesSource(ReaderFunction func,
                                  const std::wstring &description);

  public:
    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

    bool needsScores() override;

    std::shared_ptr<DoubleValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

    virtual std::wstring toString();

    virtual bool equals(std::any o);

    virtual int hashCode();

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<IndexReaderDoubleValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<IndexReaderDoubleValuesSource>(
          org.apache.lucene.search.DoubleValuesSource::shared_from_this());
    }
  };

private:
  class NoCacheConstantDoubleValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(NoCacheConstantDoubleValuesSource)

  public:
    const double value;
    const std::shared_ptr<DoubleValuesSource> parent;

  private:
    NoCacheConstantDoubleValuesSource(
        double value, std::shared_ptr<DoubleValuesSource> parent);

  public:
    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class DoubleValuesAnonymousInnerClass : public DoubleValues
    {
      GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
    private:
      std::shared_ptr<NoCacheConstantDoubleValuesSource> outerInstance;

    public:
      DoubleValuesAnonymousInnerClass(
          std::shared_ptr<NoCacheConstantDoubleValuesSource> outerInstance);

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

    std::shared_ptr<DoubleValuesSource>
    rewrite(std::shared_ptr<IndexSearcher> reader)  override;

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<NoCacheConstantDoubleValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<NoCacheConstantDoubleValuesSource>(
          org.apache.lucene.search.DoubleValuesSource::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::queries::function
