#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
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
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Pass a the field value through as a std::wstring, no matter the type // Q: doesn't
 *this mean it's a "string"?
 *
 **/
class LiteralValueSource : public ValueSource
{
  GET_CLASS_NAME(LiteralValueSource)
protected:
  const std::wstring string;
  const std::shared_ptr<BytesRef> bytesRef;

public:
  LiteralValueSource(const std::wstring &string);

  /** returns the literal value */
  virtual std::wstring getValue();

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class StrDocValuesAnonymousInnerClass : public StrDocValues
  {
    GET_CLASS_NAME(StrDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<LiteralValueSource> outerInstance;

  public:
    StrDocValuesAnonymousInnerClass(
        std::shared_ptr<LiteralValueSource> outerInstance);

    std::wstring strVal(int doc) override;

    bool bytesVal(int doc, std::shared_ptr<BytesRefBuilder> target) override;

    std::wstring toString(int doc) override;

  protected:
    std::shared_ptr<StrDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StrDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .StrDocValues::shared_from_this());
    }
  };

public:
  std::wstring description() override;

  virtual bool equals(std::any o);

  static const int hash = LiteralValueSource::typeid->hashCode();
  virtual int hashCode();

protected:
  std::shared_ptr<LiteralValueSource> shared_from_this()
  {
    return std::static_pointer_cast<LiteralValueSource>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function::valuesource
