#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/queries/function/docvalues/BoolDocValues.h"
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
using BoolDocValues =
    org::apache::lucene::queries::function::docvalues::BoolDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * {@link BoolFunction} implementation which applies an extendible bool
 * function to the values of a single wrapped {@link ValueSource}.
 *
 * Functions this can be used for include whether a field has a value or not,
 * or inverting the bool value of the wrapped ValueSource.
 */
class SimpleBoolFunction : public BoolFunction
{
  GET_CLASS_NAME(SimpleBoolFunction)
protected:
  const std::shared_ptr<ValueSource> source;

public:
  SimpleBoolFunction(std::shared_ptr<ValueSource> source);

protected:
  virtual std::wstring name() = 0;

  virtual bool func(int doc, std::shared_ptr<FunctionValues> vals) = 0;

public:
  std::shared_ptr<BoolDocValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class BoolDocValuesAnonymousInnerClass : public BoolDocValues
  {
    GET_CLASS_NAME(BoolDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleBoolFunction> outerInstance;

    std::shared_ptr<FunctionValues> vals;

  public:
    BoolDocValuesAnonymousInnerClass(
        std::shared_ptr<SimpleBoolFunction> outerInstance,
        std::shared_ptr<FunctionValues> vals);

    bool boolVal(int doc)  override;
    std::wstring toString(int doc)  override;

  protected:
    std::shared_ptr<BoolDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BoolDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .BoolDocValues::shared_from_this());
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
  std::shared_ptr<SimpleBoolFunction> shared_from_this()
  {
    return std::static_pointer_cast<SimpleBoolFunction>(
        BoolFunction::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
