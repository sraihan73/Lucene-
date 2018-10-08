#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
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
namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * Wraps a whitespace tokenizer with a filter that sets
 * the first token, and odd tokens to posinc=1, and all others
 * to 0, encoding the position as pos: XXX in the payload.
 **/
class MockPayloadAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(MockPayloadAnalyzer)

public:
  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

protected:
  std::shared_ptr<MockPayloadAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<MockPayloadAnalyzer>(
        Analyzer::shared_from_this());
  }
};

/**
 *
 *
 **/
class MockPayloadFilter final : public TokenFilter
{
  GET_CLASS_NAME(MockPayloadFilter)
public:
  std::wstring fieldName;

  int pos = 0;

  int i = 0;

  const std::shared_ptr<PositionIncrementAttribute> posIncrAttr;
  const std::shared_ptr<PayloadAttribute> payloadAttr;
  const std::shared_ptr<CharTermAttribute> termAttr;

  MockPayloadFilter(std::shared_ptr<TokenStream> input,
                    const std::wstring &fieldName);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<MockPayloadFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockPayloadFilter>(
        TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
