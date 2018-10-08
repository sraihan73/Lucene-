#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueFiller.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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
 * Depending on the bool value of the <code>ifSource</code> function,
 * returns the value of the <code>trueSource</code> or <code>falseSource</code>
 * function.
 */
class IfFunction : public BoolFunction
{
  GET_CLASS_NAME(IfFunction)
private:
  const std::shared_ptr<ValueSource> ifSource;
  const std::shared_ptr<ValueSource> trueSource;
  const std::shared_ptr<ValueSource> falseSource;

public:
  IfFunction(std::shared_ptr<ValueSource> ifSource,
             std::shared_ptr<ValueSource> trueSource,
             std::shared_ptr<ValueSource> falseSource);

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FunctionValuesAnonymousInnerClass : public FunctionValues
  {
    GET_CLASS_NAME(FunctionValuesAnonymousInnerClass)
  private:
    std::shared_ptr<IfFunction> outerInstance;

    std::shared_ptr<FunctionValues> ifVals;
    std::shared_ptr<FunctionValues> trueVals;
    std::shared_ptr<FunctionValues> falseVals;

  public:
    FunctionValuesAnonymousInnerClass(
        std::shared_ptr<IfFunction> outerInstance,
        std::shared_ptr<FunctionValues> ifVals,
        std::shared_ptr<FunctionValues> trueVals,
        std::shared_ptr<FunctionValues> falseVals);

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

    std::wstring toString(int doc)  override;

  protected:
    std::shared_ptr<FunctionValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FunctionValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function
              .FunctionValues::shared_from_this());
    }
  };

public:
  std::wstring description() override;

  virtual int hashCode();

  virtual bool equals(std::any o);

  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<IfFunction> shared_from_this()
  {
    return std::static_pointer_cast<IfFunction>(
        BoolFunction::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
