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
 * Abstract parent class for {@link ValueSource} implementations that wrap
 * multiple ValueSources and apply their own logic.
 */
class MultiFunction : public ValueSource
{
  GET_CLASS_NAME(MultiFunction)
protected:
  const std::deque<std::shared_ptr<ValueSource>> sources;

public:
  MultiFunction(std::deque<std::shared_ptr<ValueSource>> &sources);

protected:
  virtual std::wstring name() = 0;

public:
  std::wstring description() override;

  /**
   * Helper utility for {@link FunctionValues}
   *
   * @return true if <em>all</em> of the specified <code>values</code>
   *         {@link FunctionValues#exists} for the specified doc, else false.
   */
  static bool allExists(
      int doc,
      std::deque<std::shared_ptr<FunctionValues>> &values) ;

  /**
   * Helper utility for {@link FunctionValues}
   *
   * @return true if <em>any</em> of the specified <code>values</code>
   *         {@link FunctionValues#exists} for the specified doc, else false.
   */
  static bool anyExists(
      int doc,
      std::deque<std::shared_ptr<FunctionValues>> &values) ;

  /**
   * Equivilent the the {@code FunctionValues[]} method with the same name, but
   * optimized for dealing with exactly 2 arguments.
   *
   * @return true if <em>both</em> of the specified <code>values</code>
   *         {@link FunctionValues#exists} for the specified doc, else false.
   * @see #anyExists(int,FunctionValues[])
   */
  static bool
  allExists(int doc, std::shared_ptr<FunctionValues> values1,
            std::shared_ptr<FunctionValues> values2) ;

  /**
   * Equivilent the the {@code FunctionValues[]} method with the same name, but
   * optimized for dealing with exactly 2 arguments.
   *
   * @return true if <em>either</em> of the specified <code>values</code>
   *         {@link FunctionValues#exists} for the specified doc, else false.
   * @see #anyExists(int,FunctionValues[])
   */
  static bool
  anyExists(int doc, std::shared_ptr<FunctionValues> values1,
            std::shared_ptr<FunctionValues> values2) ;

  static std::wstring
  description(const std::wstring &name,
              std::deque<std::shared_ptr<ValueSource>> &sources);

  static std::deque<std::shared_ptr<FunctionValues>>
  valsArr(std::deque<std::shared_ptr<ValueSource>> &sources,
          std::unordered_map fcontext,
          std::shared_ptr<LeafReaderContext> readerContext) ;

public:
  class Values : public FunctionValues
  {
    GET_CLASS_NAME(Values)
  private:
    std::shared_ptr<MultiFunction> outerInstance;

  public:
    std::deque<std::shared_ptr<FunctionValues>> const valsArr;

    Values(std::shared_ptr<MultiFunction> outerInstance,
           std::deque<std::shared_ptr<FunctionValues>> &valsArr);

    std::wstring toString(int doc)  override;

    std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

  protected:
    std::shared_ptr<Values> shared_from_this()
    {
      return std::static_pointer_cast<Values>(
          org.apache.lucene.queries.function
              .FunctionValues::shared_from_this());
    }
  };

public:
  static std::wstring
  toString(const std::wstring &name,
           std::deque<std::shared_ptr<FunctionValues>> &valsArr,
           int doc) ;

  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

  virtual int hashCode();

  virtual bool equals(std::any o);

protected:
  std::shared_ptr<MultiFunction> shared_from_this()
  {
    return std::static_pointer_cast<MultiFunction>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
