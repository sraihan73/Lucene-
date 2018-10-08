#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class Term;
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
namespace org::apache::lucene::search
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Term position unit test.
 *
 *
 */
class TestPositionIncrement : public LuceneTestCase
{
  GET_CLASS_NAME(TestPositionIncrement)

public:
  static constexpr bool VERBOSE = false;

  virtual void testSetPosition() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestPositionIncrement> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestPositionIncrement> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenizerAnonymousInnerClass : public Tokenizer
    {
      GET_CLASS_NAME(TokenizerAnonymousInnerClass)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance;

    public:
      TokenizerAnonymousInnerClass(
          std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance);

      // TODO: use CannedTokenStream
    private:
      std::deque<std::wstring> const TOKENS;
      std::deque<int> const INCREMENTS;
      int i = 0;

    public:
      std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
      std::shared_ptr<CharTermAttribute> termAtt;
      std::shared_ptr<OffsetAttribute> offsetAtt;

      bool incrementToken() override;

      void reset()  override;

    protected:
      std::shared_ptr<TokenizerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TokenizerAnonymousInnerClass>(
            org.apache.lucene.analysis.Tokenizer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  class PayloadSpanCollector
      : public std::enable_shared_from_this<PayloadSpanCollector>,
        public SpanCollector
  {
    GET_CLASS_NAME(PayloadSpanCollector)

  public:
    std::deque<std::shared_ptr<BytesRef>> payloads =
        std::deque<std::shared_ptr<BytesRef>>();

    void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                     std::shared_ptr<Term> term)  override;

    void reset() override;
  };

public:
  virtual void testPayloadsPos0() ;

protected:
  std::shared_ptr<TestPositionIncrement> shared_from_this()
  {
    return std::static_pointer_cast<TestPositionIncrement>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
