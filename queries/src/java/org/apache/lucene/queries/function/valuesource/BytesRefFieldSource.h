#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReaderContext;
}

namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::queries::function
{
class ValueFiller;
}
namespace org::apache::lucene::util::@ mutable { class MutableValueStr; }
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
 * An implementation for retrieving {@link FunctionValues} instances for string
 * based fields.
 */
class BytesRefFieldSource : public FieldCacheSource
{
  GET_CLASS_NAME(BytesRefFieldSource)

public:
  BytesRefFieldSource(const std::wstring &field);

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FunctionValuesAnonymousInnerClass : public FunctionValues
  {
    GET_CLASS_NAME(FunctionValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRefFieldSource> outerInstance;

    std::shared_ptr<BinaryDocValues> binaryValues;

  public:
    FunctionValuesAnonymousInnerClass(
        std::shared_ptr<BytesRefFieldSource> outerInstance,
        std::shared_ptr<BinaryDocValues> binaryValues);

    int lastDocID = 0;

  private:
    std::shared_ptr<BytesRef> getValueForDoc(int doc) ;

  public:
    bool exists(int doc)  override;

    bool bytesVal(int doc, std::shared_ptr<BytesRefBuilder> target) throw(
        IOException) override;

    std::wstring strVal(int doc)  override;

    std::any objectVal(int doc)  override;

    std::wstring toString(int doc)  override;

    std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

  private:
    class ValueFillerAnonymousInnerClass : public ValueFiller
    {
      GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
    private:
      std::shared_ptr<FunctionValuesAnonymousInnerClass> outerInstance;

    public:
      ValueFillerAnonymousInnerClass(
          std::shared_ptr<FunctionValuesAnonymousInnerClass> outerInstance);

    private:
      const std::shared_ptr<MutableValueStr> mval;

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
    std::shared_ptr<FunctionValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FunctionValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function
              .FunctionValues::shared_from_this());
    }
  };

private:
  class DocTermsIndexDocValuesAnonymousInnerClass
      : public DocTermsIndexDocValues
  {
    GET_CLASS_NAME(DocTermsIndexDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<BytesRefFieldSource> outerInstance;

  public:
    DocTermsIndexDocValuesAnonymousInnerClass(
        std::shared_ptr<BytesRefFieldSource> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext,
        const std::wstring &field);

  protected:
    std::wstring toTerm(const std::wstring &readableValue) override;

  public:
    std::any objectVal(int doc)  override;

    std::wstring toString(int doc)  override;

  protected:
    std::shared_ptr<DocTermsIndexDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DocTermsIndexDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .DocTermsIndexDocValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<BytesRefFieldSource> shared_from_this()
  {
    return std::static_pointer_cast<BytesRefFieldSource>(
        FieldCacheSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
