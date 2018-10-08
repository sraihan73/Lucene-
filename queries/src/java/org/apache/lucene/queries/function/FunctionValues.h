#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}

namespace org::apache::lucene::util::@ mutable { class MutableValue; }
namespace org::apache::lucene::util::@ mutable { class MutableValueFloat; }
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function
{
class ValueSourceScorer;
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
using Explanation = org::apache::lucene::search::Explanation;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;

/**
 * Represents field values as different types.
 * Normally created via a {@link ValueSource} for a particular field and reader.
 *
 *
 */

// FunctionValues is distinct from ValueSource because
// there needs to be an object created at query evaluation time that
// is not referenced by the query itself because:
// - Query objects should be MT safe
// - For caching, Query objects are often used as keys... you don't
//   want the Query carrying around big objects
class FunctionValues : public std::enable_shared_from_this<FunctionValues>
{
  GET_CLASS_NAME(FunctionValues)

public:
  virtual char byteVal(int doc) ;
  virtual short shortVal(int doc) ;

  virtual float floatVal(int doc) ;
  virtual int intVal(int doc) ;
  virtual int64_t longVal(int doc) ;
  virtual double doubleVal(int doc) ;
  // TODO: should we make a termVal, returns BytesRef?
  virtual std::wstring strVal(int doc) ;

  virtual bool boolVal(int doc) ;

  /** returns the bytes representation of the string val - TODO: should this
   * return the indexed raw bytes not? */
  virtual bool
  bytesVal(int doc, std::shared_ptr<BytesRefBuilder> target) ;

  /** Native Java Object representation of the value */
  virtual std::any objectVal(int doc) ;

  /** Returns true if there is a value for this document */
  virtual bool exists(int doc) ;

  /**
   * @param doc The doc to retrieve to sort ordinal for
   * @return the sort ordinal for the specified doc
   * TODO: Maybe we can just use intVal for this...
   */
  virtual int ordVal(int doc) ;

  /**
   * @return the number of unique sort ordinals this instance has
   */
  virtual int numOrd();
  virtual std::wstring toString(int doc) = 0;

  /**
   * Abstraction of the logic required to fill the value of a specified doc into
   * a reusable {@link MutableValue}.  Implementations of {@link FunctionValues}
   * are encouraged to define their own implementations of ValueFiller if their
   * value is not a float.
   *
   * @lucene.experimental
   */
public:
  class ValueFiller : public std::enable_shared_from_this<ValueFiller>
  {
    GET_CLASS_NAME(ValueFiller)
    /** MutableValue will be reused across calls */
  public:
    virtual std::shared_ptr<MutableValue> getValue() = 0;

    /** MutableValue will be reused across calls.  Returns true if the value
     * exists. */
    virtual void fillValue(int doc) = 0;
  };

  /** @lucene.experimental  */
public:
  virtual std::shared_ptr<ValueFiller> getValueFiller();

private:
  class ValueFillerAnonymousInnerClass : public ValueFiller
  {
    GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
  private:
    std::shared_ptr<FunctionValues> outerInstance;

  public:
    ValueFillerAnonymousInnerClass(
        std::shared_ptr<FunctionValues> outerInstance);

  private:
    const std::shared_ptr<MutableValueFloat> mval;

  public:
    std::shared_ptr<MutableValue> getValue() override;

    void fillValue(int doc)  override;

  protected:
    std::shared_ptr<ValueFillerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ValueFillerAnonymousInnerClass>(
          ValueFiller::shared_from_this());
    }
  };

  // For Functions that can work with multiple values from the same document.
  // This does not apply to all functions
public:
  virtual void byteVal(int doc, std::deque<char> &vals) ;
  virtual void shortVal(int doc, std::deque<short> &vals) ;

  virtual void floatVal(int doc, std::deque<float> &vals) ;
  virtual void intVal(int doc, std::deque<int> &vals) ;
  virtual void longVal(int doc,
                       std::deque<int64_t> &vals) ;
  virtual void doubleVal(int doc, std::deque<double> &vals) ;

  // TODO: should we make a termVal, fills BytesRef[]?
  virtual void strVal(int doc,
                      std::deque<std::wstring> &vals) ;

  virtual std::shared_ptr<Explanation> explain(int doc) ;

  /**
   * Yields a {@link Scorer} that matches all documents,
   * and that which produces scores equal to {@link #floatVal(int)}.
   */
  virtual std::shared_ptr<ValueSourceScorer>
  getScorer(std::shared_ptr<LeafReaderContext> readerContext);

private:
  class ValueSourceScorerAnonymousInnerClass : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass)
  private:
    std::shared_ptr<FunctionValues> outerInstance;

  public:
    ValueSourceScorerAnonymousInnerClass(
        std::shared_ptr<FunctionValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext);

    bool matches(int doc) override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass>(
          ValueSourceScorer::shared_from_this());
    }
  };

  /**
   * Yields a {@link Scorer} that matches documents with values between the
   * specified range, and that which produces scores equal to {@link
   * #floatVal(int)}.
   */
  // A RangeValueSource can't easily be a ValueSource that takes another
  // ValueSource because it needs different behavior depending on the type of
  // fields.  There is also a setup cost - parsing and normalizing params, and
  // doing a binary search on the StringIndex.
  // TODO: change "reader" to LeafReaderContext
public:
  virtual std::shared_ptr<ValueSourceScorer>
  getRangeScorer(std::shared_ptr<LeafReaderContext> readerContext,
                 const std::wstring &lowerVal, const std::wstring &upperVal,
                 bool includeLower, bool includeUpper) ;

private:
  class ValueSourceScorerAnonymousInnerClass2 : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass2)
  private:
    std::shared_ptr<FunctionValues> outerInstance;

    float l = 0;
    float u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass2(
        std::shared_ptr<FunctionValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, float l, float u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass2>(
          ValueSourceScorer::shared_from_this());
    }
  };

private:
  class ValueSourceScorerAnonymousInnerClass3 : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass3)
  private:
    std::shared_ptr<FunctionValues> outerInstance;

    float l = 0;
    float u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass3(
        std::shared_ptr<FunctionValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, float l, float u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass3>(
          ValueSourceScorer::shared_from_this());
    }
  };

private:
  class ValueSourceScorerAnonymousInnerClass4 : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass4)
  private:
    std::shared_ptr<FunctionValues> outerInstance;

    float l = 0;
    float u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass4(
        std::shared_ptr<FunctionValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, float l, float u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass4>(
          ValueSourceScorer::shared_from_this());
    }
  };

private:
  class ValueSourceScorerAnonymousInnerClass5 : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass5)
  private:
    std::shared_ptr<FunctionValues> outerInstance;

    float l = 0;
    float u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass5(
        std::shared_ptr<FunctionValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, float l, float u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass5>(
          ValueSourceScorer::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::queries::function
