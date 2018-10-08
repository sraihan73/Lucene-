#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

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
 * Scales values to be between min and max.
 * <p>This implementation currently traverses all of the source values to obtain
 * their min and max.
 * <p>This implementation currently cannot distinguish when documents have been
 * deleted or documents that have no value, and 0.0 values will be used for
 * these cases.  This means that if values are normally all greater than 0.0,
 * one can still end up with 0.0 as the min value to map_obj from.  In these cases,
 * an appropriate map_obj() function could be used as a workaround to change 0.0 to
 * a value in the real range.
 */
class ScaleFloatFunction : public ValueSource
{
  GET_CLASS_NAME(ScaleFloatFunction)
protected:
  const std::shared_ptr<ValueSource> source;
  const float min;
  const float max;

public:
  ScaleFloatFunction(std::shared_ptr<ValueSource> source, float min, float max);

  std::wstring description() override;

private:
  class ScaleInfo : public std::enable_shared_from_this<ScaleInfo>
  {
    GET_CLASS_NAME(ScaleInfo)
  public:
    float minVal = 0;
    float maxVal = 0;
  };

private:
  std::shared_ptr<ScaleInfo> createScaleInfo(
      std::unordered_map context,
      std::shared_ptr<LeafReaderContext> readerContext) ;

public:
  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FloatDocValuesAnonymousInnerClass : public FloatDocValues
  {
    GET_CLASS_NAME(FloatDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<ScaleFloatFunction> outerInstance;

    float scale = 0;
    float minSource = 0;
    float maxSource = 0;
    std::shared_ptr<FunctionValues> vals;

  public:
    FloatDocValuesAnonymousInnerClass(
        std::shared_ptr<ScaleFloatFunction> outerInstance, float scale,
        float minSource, float maxSource, std::shared_ptr<FunctionValues> vals);

    bool exists(int doc)  override;
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
  std::shared_ptr<ScaleFloatFunction> shared_from_this()
  {
    return std::static_pointer_cast<ScaleFloatFunction>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
