#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueFiller.h"

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

/**
 * {@link ValueSource} implementation which only returns the values from the
 * provided ValueSources which are available for a particular docId.
 * Consequently, when combined with a {@link ConstValueSource}, this function
 * serves as a way to return a default value when the values for a field are
 * unavailable.
 */
class DefFunction : public MultiFunction
{
  GET_CLASS_NAME(DefFunction)
public:
  DefFunction(std::deque<std::shared_ptr<ValueSource>> &sources);

protected:
  std::wstring name() override;

public:
  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map fcontext,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class ValuesAnonymousInnerClass : public Values
  {
    GET_CLASS_NAME(ValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DefFunction> outerInstance;

  public:
    ValuesAnonymousInnerClass(
        std::shared_ptr<DefFunction> outerInstance,
        std::deque<std::shared_ptr<FunctionValues>> &valsArr);

    const int upto;

  private:
    std::shared_ptr<FunctionValues> get(int doc) ;

  public:
    char byteVal(int doc)  override;

    short shortVal(int doc)  override;

    float floatVal(int doc)  override;

    int intVal(int doc)  override;

    int64_t longVal(int doc)  override;

    double doubleVal(int doc)  override;

    std::wstring strVal(int doc)  override;

    bool boolVal(int doc)  override;

    bool bytesVal(int doc, std::shared_ptr<BytesRefBuilder> target) throw(
        IOException) override;

    std::any objectVal(int doc)  override;

    bool exists(int doc)  override;

    std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

  protected:
    std::shared_ptr<ValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ValuesAnonymousInnerClass>(
          Values::shared_from_this());
    }
  };

protected:
  std::shared_ptr<DefFunction> shared_from_this()
  {
    return std::static_pointer_cast<DefFunction>(
        MultiFunction::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
