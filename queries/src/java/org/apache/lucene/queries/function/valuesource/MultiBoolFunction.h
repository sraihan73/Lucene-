#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queries::function
{
class ValueSource;
}

namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function::docvalues
{
class BoolDocValues;
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
using BoolDocValues =
    org::apache::lucene::queries::function::docvalues::BoolDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Abstract {@link ValueSource} implementation which wraps multiple ValueSources
 * and applies an extendible bool function to their values.
 **/
class MultiBoolFunction : public BoolFunction
{
  GET_CLASS_NAME(MultiBoolFunction)
protected:
  const std::deque<std::shared_ptr<ValueSource>> sources;

public:
  MultiBoolFunction(std::deque<std::shared_ptr<ValueSource>> &sources);

protected:
  virtual std::wstring name() = 0;

  virtual bool func(int doc,
                    std::deque<std::shared_ptr<FunctionValues>> &vals) = 0;

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
    std::shared_ptr<MultiBoolFunction> outerInstance;

    std::deque<std::shared_ptr<FunctionValues>> vals;

  public:
    BoolDocValuesAnonymousInnerClass(
        std::shared_ptr<MultiBoolFunction> outerInstance,
        std::deque<std::shared_ptr<FunctionValues>> &vals);

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
  std::shared_ptr<MultiBoolFunction> shared_from_this()
  {
    return std::static_pointer_cast<MultiBoolFunction>(
        BoolFunction::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
