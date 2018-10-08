#pragma once
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/State.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

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

using namespace org::apache::lucene::analysis;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDocumentWriter : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocumentWriter)
private:
  std::shared_ptr<Directory> dir;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void test();

  virtual void testAddDocument() ;

  virtual void testPositionIncrementGap() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocumentWriter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestDocumentWriter> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    int getPositionIncrementGap(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testTokenReuse() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDocumentWriter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestDocumentWriter> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenFilterAnonymousInnerClass : public TokenFilter
    {
      GET_CLASS_NAME(TokenFilterAnonymousInnerClass)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance;

    public:
      TokenFilterAnonymousInnerClass(
          std::shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance,
          std::shared_ptr<org::apache::lucene::analysis::Tokenizer> tokenizer);

      bool first = false;
      std::shared_ptr<AttributeSource::State> state;

      bool incrementToken()  override;

      void reset()  override;

      const std::shared_ptr<CharTermAttribute> termAtt;
      const std::shared_ptr<PayloadAttribute> payloadAtt;
      const std::shared_ptr<PositionIncrementAttribute> posIncrAtt;

    protected:
      std::shared_ptr<TokenFilterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TokenFilterAnonymousInnerClass>(
            TokenFilter::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPreAnalyzedField() ;

private:
  class TokenStreamAnonymousInnerClass : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocumentWriter> outerInstance;

  public:
    TokenStreamAnonymousInnerClass(
        std::shared_ptr<TestDocumentWriter> outerInstance);

  private:
    std::deque<std::wstring> tokens;
    int index = 0;

    std::shared_ptr<CharTermAttribute> termAtt;

  public:
    bool incrementToken() override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass>(
          TokenStream::shared_from_this());
    }
  };

  /**
   * Test adding two fields with the same name, one indexed
   * the other stored only. The omitNorms and omitTermFreqAndPositions setting
   * of the stored field should not affect the indexed one (LUCENE-1590)
   */
public:
  virtual void testLUCENE_1590() ;

protected:
  std::shared_ptr<TestDocumentWriter> shared_from_this()
  {
    return std::static_pointer_cast<TestDocumentWriter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
