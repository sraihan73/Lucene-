#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis
{
class Tokenizer;
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
namespace org::apache::lucene::index
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test adding to the info stream when there's an exception thrown during field
 * analysis.
 */
class TestDocInverterPerFieldErrorInfo : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocInverterPerFieldErrorInfo)
private:
  static const std::shared_ptr<FieldType> storedTextType;

private:
  class BadNews : public std::runtime_error
  {
    GET_CLASS_NAME(BadNews)
  private:
    BadNews(const std::wstring &message);

  protected:
    std::shared_ptr<BadNews> shared_from_this()
    {
      return std::static_pointer_cast<BadNews>(
          RuntimeException::shared_from_this());
    }
  };

private:
  class ThrowingAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(ThrowingAnalyzer)
  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenFilterAnonymousInnerClass : public TokenFilter
    {
      GET_CLASS_NAME(TokenFilterAnonymousInnerClass)
    private:
      std::shared_ptr<ThrowingAnalyzer> outerInstance;

    public:
      TokenFilterAnonymousInnerClass(
          std::shared_ptr<ThrowingAnalyzer> outerInstance,
          std::shared_ptr<Tokenizer> tokenizer);

      bool incrementToken()  override;

    protected:
      std::shared_ptr<TokenFilterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TokenFilterAnonymousInnerClass>(
            org.apache.lucene.analysis.TokenFilter::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ThrowingAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<ThrowingAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testInfoStreamGetsFieldName() throws
  // Exception
  virtual void testInfoStreamGetsFieldName() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNoExtraNoise() throws Exception
  virtual void testNoExtraNoise() ;

protected:
  std::shared_ptr<TestDocInverterPerFieldErrorInfo> shared_from_this()
  {
    return std::static_pointer_cast<TestDocInverterPerFieldErrorInfo>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
