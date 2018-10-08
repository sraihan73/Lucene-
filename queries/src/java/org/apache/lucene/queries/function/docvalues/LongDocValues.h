#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queries::function
{
class ValueSource;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function
{
class ValueSourceScorer;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::queries::function
{
class ValueFiller;
}
namespace org::apache::lucene::util::@ mutable { class MutableValueLong; }
namespace org::apache::lucene::util::@ mutable { class MutableValue; }

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
 * Abstract {@link FunctionValues} implementation which supports retrieving long
 * values. Implementations can control how the long values are loaded through
 * {@link #longVal(int)}}
 */
class LongDocValues : public FunctionValues
{
  GET_CLASS_NAME(LongDocValues)
protected:
  const std::shared_ptr<ValueSource> vs;

public:
  LongDocValues(std::shared_ptr<ValueSource> vs);

  char byteVal(int doc)  override;

  short shortVal(int doc)  override;

  float floatVal(int doc)  override;

  int intVal(int doc)  override;

  int64_t longVal(int doc) = 0;
  override override;

  double doubleVal(int doc)  override;

  bool boolVal(int doc)  override;

  std::wstring strVal(int doc)  override;

  std::any objectVal(int doc)  override;

  std::wstring toString(int doc)  override;

protected:
  virtual int64_t externalToLong(const std::wstring &extVal);

public:
  std::shared_ptr<ValueSourceScorer>
  getRangeScorer(std::shared_ptr<LeafReaderContext> readerContext,
                 const std::wstring &lowerVal, const std::wstring &upperVal,
                 bool includeLower, bool includeUpper) override;

private:
  class ValueSourceScorerAnonymousInnerClass : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass)
  private:
    std::shared_ptr<LongDocValues> outerInstance;

    int64_t ll = 0;
    int64_t uu = 0;

  public:
    ValueSourceScorerAnonymousInnerClass(
        std::shared_ptr<LongDocValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, int64_t ll,
        int64_t uu);

    bool matches(int doc)  override;

  protected:
    std::shared_ptr<ValueSourceScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass>(
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
    std::shared_ptr<LongDocValues> outerInstance;

  public:
    ValueFillerAnonymousInnerClass(
        std::shared_ptr<LongDocValues> outerInstance);

  private:
    const std::shared_ptr<MutableValueLong> mval;

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
  std::shared_ptr<LongDocValues> shared_from_this()
  {
    return std::static_pointer_cast<LongDocValues>(
        org.apache.lucene.queries.function.FunctionValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::docvalues
