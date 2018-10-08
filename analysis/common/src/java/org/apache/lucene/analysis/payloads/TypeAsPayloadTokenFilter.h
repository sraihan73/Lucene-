#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::payloads
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

/**
 * Makes the {@link TypeAttribute} a payload.
 *
 * Encodes the type using {@link std::wstring#getBytes(std::wstring)} with "UTF-8" as the
 *encoding
 *
 **/
class TypeAsPayloadTokenFilter : public TokenFilter
{
  GET_CLASS_NAME(TypeAsPayloadTokenFilter)
private:
  const std::shared_ptr<PayloadAttribute> payloadAtt =
      addAttribute(PayloadAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);

public:
  TypeAsPayloadTokenFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<TypeAsPayloadTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TypeAsPayloadTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/analysis/payloads/
