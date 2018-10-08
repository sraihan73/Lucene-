#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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

/** A simple float function with a single argument
 */
class SimpleFloatFunction : public SingleFunction
{
  GET_CLASS_NAME(SimpleFloatFunction)
public:
  SimpleFloatFunction(std::shared_ptr<ValueSource> source);

protected:
  virtual float func(int doc, std::shared_ptr<FunctionValues> vals) = 0;

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
    std::shared_ptr<SimpleFloatFunction> outerInstance;

    std::shared_ptr<FunctionValues> vals;

  public:
    FloatDocValuesAnonymousInnerClass(
        std::shared_ptr<SimpleFloatFunction> outerInstance,
        std::shared_ptr<FunctionValues> vals);

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

protected:
  std::shared_ptr<SimpleFloatFunction> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFloatFunction>(
        SingleFunction::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
