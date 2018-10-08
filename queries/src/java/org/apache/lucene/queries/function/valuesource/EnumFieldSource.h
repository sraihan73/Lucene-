#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
namespace org::apache::lucene::util::@ mutable { class MutableValueInt; }
namespace org::apache::lucene::queries::function
{
class ValueSourceScorer;
}
#include  "core/src/java/org/apache/lucene/queries/function/ValueFiller.h"
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
namespace org::apache::lucene::queries::function::valuesource
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;

/**
 * Obtains int field values from {@link
 * org.apache.lucene.index.LeafReader#getNumericDocValues} and makes those
 * values available as other numeric types, casting as needed. strVal of the
 * value is not the int value, but its string (displayed) value
 */
class EnumFieldSource : public FieldCacheSource
{
  GET_CLASS_NAME(EnumFieldSource)
public:
  static const std::optional<int> DEFAULT_VALUE;

  const std::unordered_map<int, std::wstring> enumIntToStringMap;
  const std::unordered_map<std::wstring, int> enumStringToIntMap;

  EnumFieldSource(const std::wstring &field,
                  std::unordered_map<int, std::wstring> &enumIntToStringMap,
                  std::unordered_map<std::wstring, int> &enumStringToIntMap);

private:
  static std::optional<int> tryParseInt(const std::wstring &valueStr);

  std::wstring intValueToStringValue(std::optional<int> &intVal);

  std::optional<int> stringValueToIntValue(const std::wstring &stringVal);

public:
  std::wstring description() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class IntDocValuesAnonymousInnerClass : public IntDocValues
  {
    GET_CLASS_NAME(IntDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<EnumFieldSource> outerInstance;

    std::shared_ptr<LeafReaderContext> readerContext;
    std::shared_ptr<NumericDocValues> arr;

  public:
    IntDocValuesAnonymousInnerClass(
        std::shared_ptr<EnumFieldSource> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext,
        std::shared_ptr<NumericDocValues> arr);

    const std::shared_ptr<MutableValueInt> val;

    int lastDocID = 0;

  private:
    int getValueForDoc(int doc) ;

  public:
    int intVal(int doc)  override;

    std::wstring strVal(int doc)  override;

    bool exists(int doc)  override;

    std::shared_ptr<ValueSourceScorer>
    getRangeScorer(std::shared_ptr<LeafReaderContext> readerContext,
                   const std::wstring &lowerVal, const std::wstring &upperVal,
                   bool includeLower, bool includeUpper) override;

  private:
    class ValueSourceScorerAnonymousInnerClass : public ValueSourceScorer
    {
      GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass)
    private:
      std::shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance;

      int ll = 0;
      int uu = 0;

    public:
      ValueSourceScorerAnonymousInnerClass(
          std::shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReaderContext> readerContext, int ll, int uu);

      bool matches(int doc)  override;

    protected:
      std::shared_ptr<ValueSourceScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ValueSourceScorerAnonymousInnerClass>(
            org.apache.lucene.queries.function
                .ValueSourceScorer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

  private:
    class ValueFillerAnonymousInnerClass : public ValueFiller
    {
      GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
    private:
      std::shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance;

    public:
      ValueFillerAnonymousInnerClass(
          std::shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance);

    private:
      const std::shared_ptr<MutableValueInt> mval;

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
    std::shared_ptr<IntDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .IntDocValues::shared_from_this());
    }
  };

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<EnumFieldSource> shared_from_this()
  {
    return std::static_pointer_cast<EnumFieldSource>(
        FieldCacheSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
