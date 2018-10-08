#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
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
 * <code>RangeMapFloatFunction</code> implements a map_obj function over
 * another {@link ValueSource} whose values fall within min and max inclusive to
 * target. <br> Normally Used as an argument to a {@link
 * org.apache.lucene.queries.function.FunctionQuery}
 *
 *
 */
class RangeMapFloatFunction : public ValueSource
{
  GET_CLASS_NAME(RangeMapFloatFunction)
protected:
  const std::shared_ptr<ValueSource> source;
  const float min;
  const float max;
  const std::shared_ptr<ValueSource> target;
  const std::shared_ptr<ValueSource> defaultVal;

public:
  RangeMapFloatFunction(std::shared_ptr<ValueSource> source, float min,
                        float max, float target, std::optional<float> &def);

  RangeMapFloatFunction(std::shared_ptr<ValueSource> source, float min,
                        float max, std::shared_ptr<ValueSource> target,
                        std::shared_ptr<ValueSource> def);

  std::wstring description() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FloatDocValuesAnonymousInnerClass : public FloatDocValues
  {
    GET_CLASS_NAME(FloatDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<RangeMapFloatFunction> outerInstance;

    std::shared_ptr<FunctionValues> vals;
    std::shared_ptr<FunctionValues> targets;
    std::shared_ptr<FunctionValues> defaults;

  public:
    FloatDocValuesAnonymousInnerClass(
        std::shared_ptr<RangeMapFloatFunction> outerInstance,
        std::shared_ptr<FunctionValues> vals,
        std::shared_ptr<FunctionValues> targets,
        std::shared_ptr<FunctionValues> defaults);

    float floatVal(int doc)  override;
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

  virtual int hashCode();

  virtual bool equals(std::any o);

protected:
  std::shared_ptr<RangeMapFloatFunction> shared_from_this()
  {
    return std::static_pointer_cast<RangeMapFloatFunction>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
