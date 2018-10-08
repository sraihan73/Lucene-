#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

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
class FunctionValues;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
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
namespace org::apache::lucene::queries::function::valuesource
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * <code>ReciprocalFloatFunction</code> implements a reciprocal function f(x) =
 * a/(mx+b), based on the float value of a field or function as exported by
 * {@link org.apache.lucene.queries.function.ValueSource}. <br>
 *
 * When a and b are equal, and x&gt;=0, this function has a maximum value of 1
 * that drops as x increases. Increasing the value of a and b together results
 * in a movement of the entire function to a flatter part of the curve. <p>These
 * properties make this an idea function for boosting more recent documents.
 * <p>Example:<code>  recip(ms(NOW,mydatefield),3.16e-11,1,1)</code>
 * <p>A multiplier of 3.16e-11 changes the units from milliseconds to years
 * (since there are about 3.16e10 milliseconds per year).  Thus, a very recent
 * date will yield a value close to 1/(0+1) or 1, a date a year in the past will
 * get a multiplier of about 1/(1+1) or 1/2, and date two years old will yield
 * 1/(2+1) or 1/3.
 *
 * @see org.apache.lucene.queries.function.FunctionQuery
 *
 *
 */
class ReciprocalFloatFunction : public ValueSource
{
  GET_CLASS_NAME(ReciprocalFloatFunction)
protected:
  const std::shared_ptr<ValueSource> source;
  const float m;
  const float a;
  const float b;

  /**
   *  f(source) = a/(m*float(source)+b)
   */
public:
  ReciprocalFloatFunction(std::shared_ptr<ValueSource> source, float m, float a,
                          float b);

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FloatDocValuesAnonymousInnerClass : public FloatDocValues
  {
    GET_CLASS_NAME(FloatDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<ReciprocalFloatFunction> outerInstance;

    std::shared_ptr<FunctionValues> vals;

  public:
    FloatDocValuesAnonymousInnerClass(
        std::shared_ptr<ReciprocalFloatFunction> outerInstance,
        std::shared_ptr<FunctionValues> vals);

    float floatVal(int doc)  override;
    bool exists(int doc)  override;
    std::wstring toString(int doc)  override;

  protected:
    std::shared_ptr<FloatDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FloatDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .FloatDocValues::shared_from_this());
    }
  };

public:
  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

  std::wstring description() override;

  virtual int hashCode();

  virtual bool equals(std::any o);

protected:
  std::shared_ptr<ReciprocalFloatFunction> shared_from_this()
  {
    return std::static_pointer_cast<ReciprocalFloatFunction>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
