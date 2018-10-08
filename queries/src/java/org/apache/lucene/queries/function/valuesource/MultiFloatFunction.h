#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
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
 * Abstract {@link ValueSource} implementation which wraps multiple ValueSources
 * and applies an extendible float function to their values.
 **/
class MultiFloatFunction : public ValueSource
{
  GET_CLASS_NAME(MultiFloatFunction)
protected:
  std::deque<std::shared_ptr<ValueSource>> const sources;

public:
  MultiFloatFunction(std::deque<std::shared_ptr<ValueSource>> &sources);

protected:
  virtual std::wstring name() = 0;
  virtual float func(int doc,
                     std::deque<std::shared_ptr<FunctionValues>> &valsArr) = 0;
  /**
   * Called by {@link FunctionValues#exists} for each document.
   *
   * Default impl returns true if <em>all</em> of the specified
   * <code>values</code>
   * {@link FunctionValues#exists} for the specified doc, else false.
   *
   * @see FunctionValues#exists
   * @see MultiFunction#allExists
   */
  virtual bool exists(
      int doc,
      std::deque<std::shared_ptr<FunctionValues>> &valsArr) ;

public:
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
    std::shared_ptr<MultiFloatFunction> outerInstance;

    std::deque<std::shared_ptr<FunctionValues>> valsArr;

  public:
    FloatDocValuesAnonymousInnerClass(
        std::shared_ptr<MultiFloatFunction> outerInstance,
        std::deque<std::shared_ptr<FunctionValues>> &valsArr);

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

  virtual int hashCode();

  virtual bool equals(std::any o);

protected:
  std::shared_ptr<MultiFloatFunction> shared_from_this()
  {
    return std::static_pointer_cast<MultiFloatFunction>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
