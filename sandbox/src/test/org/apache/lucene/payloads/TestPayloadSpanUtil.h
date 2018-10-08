#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
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
namespace org::apache::lucene::payloads
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPayloadSpanUtil : public LuceneTestCase
{
  GET_CLASS_NAME(TestPayloadSpanUtil)

public:
  static const std::wstring FIELD;

  virtual void testPayloadSpanUtil() ;

public:
  class PayloadAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(PayloadAnalyzer)

  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<PayloadAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<PayloadAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  class PayloadFilter final : public TokenFilter
  {
    GET_CLASS_NAME(PayloadFilter)
  public:
    std::shared_ptr<Set<std::wstring>> entities =
        std::unordered_set<std::wstring>();
    std::shared_ptr<Set<std::wstring>> nopayload =
        std::unordered_set<std::wstring>();
    int pos = 0;
    std::shared_ptr<PayloadAttribute> payloadAtt;
    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<PositionIncrementAttribute> posIncrAtt;

    PayloadFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<PayloadFilter> shared_from_this()
    {
      return std::static_pointer_cast<PayloadFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPayloadSpanUtil> shared_from_this()
  {
    return std::static_pointer_cast<TestPayloadSpanUtil>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/payloads/
