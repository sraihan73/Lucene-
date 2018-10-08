#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"

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
namespace org::apache::lucene::codecs::idversion
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using BytesRef = org::apache::lucene::util::BytesRef;

// TODO: can we take a BytesRef token instead?

/** Produces a single std::wstring token from the provided value, with the provided
 * payload. */
class StringAndPayloadField : public Field
{
  GET_CLASS_NAME(StringAndPayloadField)

public:
  static const std::shared_ptr<FieldType> TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static StringAndPayloadField::StaticConstructor staticConstructor;

  const std::shared_ptr<BytesRef> payload;

public:
  StringAndPayloadField(const std::wstring &name, const std::wstring &value,
                        std::shared_ptr<BytesRef> payload);

  std::shared_ptr<TokenStream>
  tokenStream(std::shared_ptr<Analyzer> analyzer,
              std::shared_ptr<TokenStream> reuse) override;

public:
  class SingleTokenWithPayloadTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(SingleTokenWithPayloadTokenStream)

  private:
    const std::shared_ptr<CharTermAttribute> termAttribute =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<PayloadAttribute> payloadAttribute =
        addAttribute(PayloadAttribute::typeid);
    bool used = false;
    std::wstring value = L"";
    std::shared_ptr<BytesRef> payload;

    /** Sets the string value. */
  public:
    void setValue(const std::wstring &value, std::shared_ptr<BytesRef> payload);

    bool incrementToken() override;

    void reset() override;

    virtual ~SingleTokenWithPayloadTokenStream();

  protected:
    std::shared_ptr<SingleTokenWithPayloadTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<SingleTokenWithPayloadTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

protected:
  std::shared_ptr<StringAndPayloadField> shared_from_this()
  {
    return std::static_pointer_cast<StringAndPayloadField>(
        org.apache.lucene.document.Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/idversion/
