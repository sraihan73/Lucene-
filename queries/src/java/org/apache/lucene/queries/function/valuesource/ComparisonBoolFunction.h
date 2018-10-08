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

namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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
 * Base class for comparison operators useful within an "if"/conditional.
 */
class ComparisonBoolFunction : public BoolFunction
{
  GET_CLASS_NAME(ComparisonBoolFunction)

private:
  const std::shared_ptr<ValueSource> lhs;
  const std::shared_ptr<ValueSource> rhs;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::wstring name_;

public:
  ComparisonBoolFunction(std::shared_ptr<ValueSource> lhs,
                         std::shared_ptr<ValueSource> rhs,
                         const std::wstring &name);

  /** Perform the comparison, returning true or false */
  virtual bool compare(int doc, std::shared_ptr<FunctionValues> lhs,
                       std::shared_ptr<FunctionValues> rhs) = 0;

  /** Uniquely identify the operation (ie "gt", "lt" "gte", etc) */
  virtual std::wstring name();

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class BoolDocValuesAnonymousInnerClass : public BoolDocValues
  {
    GET_CLASS_NAME(BoolDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<ComparisonBoolFunction> outerInstance;

    std::shared_ptr<FunctionValues> lhsVal;
    std::shared_ptr<FunctionValues> rhsVal;
    std::wstring compLabel;

  public:
    BoolDocValuesAnonymousInnerClass(
        std::shared_ptr<ComparisonBoolFunction> outerInstance,
        std::shared_ptr<FunctionValues> lhsVal,
        std::shared_ptr<FunctionValues> rhsVal, const std::wstring &compLabel);

    bool boolVal(int doc)  override;

    std::wstring toString(int doc)  override;

    bool exists(int doc)  override;

  protected:
    std::shared_ptr<BoolDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BoolDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .BoolDocValues::shared_from_this());
    }
  };

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  std::wstring description() override;

  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<ComparisonBoolFunction> shared_from_this()
  {
    return std::static_pointer_cast<ComparisonBoolFunction>(
        BoolFunction::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
