#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/SortField.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueFiller.h"
namespace org::apache::lucene::util::@ mutable { class MutableValueDouble; }
{

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
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using SortField = org::apache::lucene::search::SortField;

/**
 * Obtains double field values from {@link
 * org.apache.lucene.index.LeafReader#getNumericDocValues} and makes those
 * values available as other numeric types, casting as needed.
 */
class DoubleFieldSource : public FieldCacheSource
{
  GET_CLASS_NAME(DoubleFieldSource)

public:
  DoubleFieldSource(const std::wstring &field);

  std::wstring description() override;

  std::shared_ptr<SortField> getSortField(bool reverse) override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class DoubleDocValuesAnonymousInnerClass : public DoubleDocValues
  {
    GET_CLASS_NAME(DoubleDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DoubleFieldSource> outerInstance;

    std::shared_ptr<NumericDocValues> values;

  public:
    DoubleDocValuesAnonymousInnerClass(
        std::shared_ptr<DoubleFieldSource> outerInstance,
        std::shared_ptr<NumericDocValues> values);

    int lastDocID = 0;

  private:
    double getValueForDoc(int doc) ;

  public:
    double doubleVal(int doc)  override;

    bool exists(int doc)  override;

    std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

  private:
    class ValueFillerAnonymousInnerClass : public ValueFiller
    {
      GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
    private:
      std::shared_ptr<DoubleDocValuesAnonymousInnerClass> outerInstance;

    public:
      ValueFillerAnonymousInnerClass(
          std::shared_ptr<DoubleDocValuesAnonymousInnerClass> outerInstance);

    private:
      const std::shared_ptr<MutableValueDouble> mval;

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
    std::shared_ptr<DoubleDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .DoubleDocValues::shared_from_this());
    }
  };

protected:
  virtual std::shared_ptr<NumericDocValues> getNumericDocValues(
      std::unordered_map context,
      std::shared_ptr<LeafReaderContext> readerContext) ;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<DoubleFieldSource> shared_from_this()
  {
    return std::static_pointer_cast<DoubleFieldSource>(
        FieldCacheSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
