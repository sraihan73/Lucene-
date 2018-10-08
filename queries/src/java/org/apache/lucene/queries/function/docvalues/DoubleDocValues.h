#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueSourceScorer.h"
#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueFiller.h"
namespace org::apache::lucene::util::@ mutable { class MutableValueDouble; }
{

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
namespace org::apache::lucene::queries::function::docvalues
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using ValueSourceScorer =
    org::apache::lucene::queries::function::ValueSourceScorer;

/**
 * Abstract {@link FunctionValues} implementation which supports retrieving
 * double values. Implementations can control how the double values are loaded
 * through {@link #doubleVal(int)}}
 */
class DoubleDocValues : public FunctionValues
{
  GET_CLASS_NAME(DoubleDocValues)
protected:
  const std::shared_ptr<ValueSource> vs;

public:
  DoubleDocValues(std::shared_ptr<ValueSource> vs);

  char byteVal(int doc)  override;

  short shortVal(int doc)  override;

  float floatVal(int doc)  override;

  int intVal(int doc)  override;

  int64_t longVal(int doc)  override;

  bool boolVal(int doc)  override;

  double doubleVal(int doc) = 0;
  override override;

  std::wstring strVal(int doc)  override;

  std::any objectVal(int doc)  override;

  std::wstring toString(int doc)  override;

  std::shared_ptr<ValueSourceScorer>
  getRangeScorer(std::shared_ptr<LeafReaderContext> readerContext,
                 const std::wstring &lowerVal, const std::wstring &upperVal,
                 bool includeLower, bool includeUpper) override;

private:
  class ValueSourceScorerAnonymousInnerClass : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass)
  private:
    std::shared_ptr<DoubleDocValues> outerInstance;

    double l = 0;
    double u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass(
        std::shared_ptr<DoubleDocValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, double l, double u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass>(
          org.apache.lucene.queries.function
              .ValueSourceScorer::shared_from_this());
    }
  };

private:
  class ValueSourceScorerAnonymousInnerClass2 : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass2)
  private:
    std::shared_ptr<DoubleDocValues> outerInstance;

    double l = 0;
    double u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass2(
        std::shared_ptr<DoubleDocValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, double l, double u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass2>(
          org.apache.lucene.queries.function
              .ValueSourceScorer::shared_from_this());
    }
  };

private:
  class ValueSourceScorerAnonymousInnerClass3 : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass3)
  private:
    std::shared_ptr<DoubleDocValues> outerInstance;

    double l = 0;
    double u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass3(
        std::shared_ptr<DoubleDocValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, double l, double u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass3>(
          org.apache.lucene.queries.function
              .ValueSourceScorer::shared_from_this());
    }
  };

private:
  class ValueSourceScorerAnonymousInnerClass4 : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass4)
  private:
    std::shared_ptr<DoubleDocValues> outerInstance;

    double l = 0;
    double u = 0;

  public:
    ValueSourceScorerAnonymousInnerClass4(
        std::shared_ptr<DoubleDocValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, double l, double u);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass4>(
          org.apache.lucene.queries.function
              .ValueSourceScorer::shared_from_this());
    }
  };

public:
  std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

private:
  class ValueFillerAnonymousInnerClass : public ValueFiller
  {
    GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
  private:
    std::shared_ptr<DoubleDocValues> outerInstance;

  public:
    ValueFillerAnonymousInnerClass(
        std::shared_ptr<DoubleDocValues> outerInstance);

  private:
    const std::shared_ptr<MutableValueDouble> mval;

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

protected:
  std::shared_ptr<DoubleDocValues> shared_from_this()
  {
    return std::static_pointer_cast<DoubleDocValues>(
        org.apache.lucene.queries.function.FunctionValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/docvalues/
