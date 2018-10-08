#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queries::function
{
class ValueSource;
}

namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::queries::function
{
class ValueFiller;
}
namespace org::apache::lucene::util::@ mutable { class MutableValueBool; }
namespace org::apache::lucene::util::@ mutable { class MutableValue; }

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
namespace org::apache::lucene::queries::function::docvalues
{

using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

/**
 * Abstract {@link FunctionValues} implementation which supports retrieving
 * bool values. Implementations can control how the bool values are loaded
 * through {@link #boolVal(int)}}
 */
class BoolDocValues : public FunctionValues
{
  GET_CLASS_NAME(BoolDocValues)
protected:
  const std::shared_ptr<ValueSource> vs;

public:
  BoolDocValues(std::shared_ptr<ValueSource> vs);

  bool boolVal(int doc) = 0;
  override override;

  char byteVal(int doc)  override;

  short shortVal(int doc)  override;

  float floatVal(int doc)  override;

  int intVal(int doc)  override;

  int64_t longVal(int doc)  override;

  double doubleVal(int doc)  override;

  std::wstring strVal(int doc)  override;

  std::any objectVal(int doc)  override;

  std::wstring toString(int doc)  override;

  std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

private:
  class ValueFillerAnonymousInnerClass : public ValueFiller
  {
    GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
  private:
    std::shared_ptr<BoolDocValues> outerInstance;

  public:
    ValueFillerAnonymousInnerClass(
        std::shared_ptr<BoolDocValues> outerInstance);

  private:
    const std::shared_ptr<MutableValueBool> mval;

  public:
    std::shared_ptr<MutableValue> getValue() override;

    void fillValue(int doc)  override;

  protected:
    std::shared_ptr<ValueFillerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ValueFillerAnonymousInnerClass>(
          ValueFiller::shared_from_this());
    }
  };

protected:
  std::shared_ptr<BoolDocValues> shared_from_this()
  {
    return std::static_pointer_cast<BoolDocValues>(
        org.apache.lucene.queries.function.FunctionValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::docvalues
