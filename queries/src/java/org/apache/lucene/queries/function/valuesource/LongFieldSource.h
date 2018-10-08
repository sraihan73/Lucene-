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
namespace org::apache::lucene::util::@ mutable { class MutableValueLong; }
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
using MutableValueLong = org::apache::lucene::util::mutable_::MutableValueLong;

/**
 * Obtains long field values from {@link
 * org.apache.lucene.index.LeafReader#getNumericDocValues} and makes those
 * values available as other numeric types, casting as needed.
 */
class LongFieldSource : public FieldCacheSource
{
  GET_CLASS_NAME(LongFieldSource)

public:
  LongFieldSource(const std::wstring &field);

  std::wstring description() override;

  virtual int64_t externalToLong(const std::wstring &extVal);

  virtual std::any longToObject(int64_t val);

  virtual std::wstring longToString(int64_t val);

  std::shared_ptr<SortField> getSortField(bool reverse) override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class LongDocValuesAnonymousInnerClass : public LongDocValues
  {
    GET_CLASS_NAME(LongDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<LongFieldSource> outerInstance;

    std::shared_ptr<NumericDocValues> arr;

  public:
    LongDocValuesAnonymousInnerClass(
        std::shared_ptr<LongFieldSource> outerInstance,
        std::shared_ptr<NumericDocValues> arr);

    int lastDocID = 0;

  private:
    int64_t getValueForDoc(int doc) ;

  public:
    int64_t longVal(int doc)  override;

    bool exists(int doc)  override;

    std::any objectVal(int doc)  override;

    std::wstring strVal(int doc)  override;

  protected:
    int64_t externalToLong(const std::wstring &extVal) override;

  public:
    std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

  private:
    class ValueFillerAnonymousInnerClass : public ValueFiller
    {
      GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
    private:
      std::shared_ptr<LongDocValuesAnonymousInnerClass> outerInstance;

    public:
      ValueFillerAnonymousInnerClass(
          std::shared_ptr<LongDocValuesAnonymousInnerClass> outerInstance);

    private:
      const std::shared_ptr<MutableValueLong> mval;

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
    std::shared_ptr<LongDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .LongDocValues::shared_from_this());
    }
  };

protected:
  virtual std::shared_ptr<NumericDocValues> getNumericDocValues(
      std::unordered_map context,
      std::shared_ptr<LeafReaderContext> readerContext) ;

  virtual std::shared_ptr<MutableValueLong> newMutableValueLong();

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<LongFieldSource> shared_from_this()
  {
    return std::static_pointer_cast<LongFieldSource>(
        FieldCacheSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
