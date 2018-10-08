#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SortedDocValues;
}

namespace org::apache::lucene::queries::function
{
class ValueSource;
}
namespace org::apache::lucene::util::@ mutable { class MutableValueStr; }
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::queries::function
{
class ValueSourceScorer;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::queries::function
{
class ValueFiller;
}
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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using ValueSourceScorer =
    org::apache::lucene::queries::function::ValueSourceScorer;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using MutableValueStr = org::apache::lucene::util::mutable_::MutableValueStr;

/**
 * Serves as base class for FunctionValues based on DocTermsIndex.
 * @lucene.internal
 */
class DocTermsIndexDocValues : public FunctionValues
{
  GET_CLASS_NAME(DocTermsIndexDocValues)
protected:
  const std::shared_ptr<SortedDocValues> termsIndex;
  const std::shared_ptr<ValueSource> vs;
  const std::shared_ptr<MutableValueStr> val =
      std::make_shared<MutableValueStr>();
  const std::shared_ptr<CharsRefBuilder> spareChars =
      std::make_shared<CharsRefBuilder>();

private:
  const std::wstring field;
  int lastDocID = 0;

public:
  DocTermsIndexDocValues(std::shared_ptr<ValueSource> vs,
                         std::shared_ptr<LeafReaderContext> context,
                         const std::wstring &field) ;

protected:
  DocTermsIndexDocValues(const std::wstring &field,
                         std::shared_ptr<ValueSource> vs,
                         std::shared_ptr<SortedDocValues> termsIndex);

  virtual int getOrdForDoc(int doc) ;

  virtual std::wstring toTerm(const std::wstring &readableValue) = 0;

public:
  bool exists(int doc)  override;

  int ordVal(int doc)  override;

  int numOrd() override;

  bool
  bytesVal(int doc,
           std::shared_ptr<BytesRefBuilder> target)  override;

  std::wstring strVal(int doc)  override;

  bool boolVal(int doc)  override;

  std::any objectVal(int doc) = 0;
  override override; // force subclasses to override

  std::shared_ptr<ValueSourceScorer>
  getRangeScorer(std::shared_ptr<LeafReaderContext> readerContext,
                 const std::wstring &lowerVal, const std::wstring &upperVal,
                 bool includeLower,
                 bool includeUpper)  override;

private:
  class ValueSourceScorerAnonymousInnerClass : public ValueSourceScorer
  {
    GET_CLASS_NAME(ValueSourceScorerAnonymousInnerClass)
  private:
    std::shared_ptr<DocTermsIndexDocValues> outerInstance;

    std::shared_ptr<LeafReaderContext> readerContext;
    int ll = 0;
    int uu = 0;

  public:
    ValueSourceScorerAnonymousInnerClass(
        std::shared_ptr<DocTermsIndexDocValues> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext, int ll, int uu);

    const std::shared_ptr<SortedDocValues> values;

  private:
    int outerInstance->lastDocID = 0;

  public:
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
  std::wstring toString(int doc)  override;

  std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

private:
  class ValueFillerAnonymousInnerClass : public ValueFiller
  {
    GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
  private:
    std::shared_ptr<DocTermsIndexDocValues> outerInstance;

  public:
    ValueFillerAnonymousInnerClass(
        std::shared_ptr<DocTermsIndexDocValues> outerInstance);

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

  // TODO: why?
public:
  static std::shared_ptr<SortedDocValues>
  open(std::shared_ptr<LeafReaderContext> context,
       const std::wstring &field) ;

  /**
   * Custom Exception to be thrown when the DocTermsIndex for a field cannot be
   * generated
   */
public:
  class DocTermsIndexException final : public std::runtime_error
  {
    GET_CLASS_NAME(DocTermsIndexException)

  public:
    DocTermsIndexException(const std::wstring &fieldName,
                           std::runtime_error const cause);

  protected:
    std::shared_ptr<DocTermsIndexException> shared_from_this()
    {
      return std::static_pointer_cast<DocTermsIndexException>(
          RuntimeException::shared_from_this());
    }
  };

protected:
  std::shared_ptr<DocTermsIndexDocValues> shared_from_this()
  {
    return std::static_pointer_cast<DocTermsIndexDocValues>(
        org.apache.lucene.queries.function.FunctionValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::docvalues
