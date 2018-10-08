#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriterConfig;
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
class Token;
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

using Token = org::apache::lucene::analysis::Token;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO: we really need to test indexingoffsets, but then getting only docs /
// docs + freqs. not all codecs store prx separate...
// TODO: fix sep codec to index offsets so we can greatly reduce this deque!
class TestPostingsOffsets : public LuceneTestCase
{
  GET_CLASS_NAME(TestPostingsOffsets)
public:
  std::shared_ptr<IndexWriterConfig> iwc;

  void setUp()  override;

  virtual void testBasic() ;

  virtual void testSkipping() ;

  virtual void testPayloads() ;

  virtual void doTestNumbers(bool withPayloads) ;

  virtual void testRandom() ;

  virtual void testWithUnindexedFields() ;

  virtual void testAddFieldTwice() ;

  // NOTE: the next two tests aren't that good as we need an EvilToken...
  virtual void testNegativeOffsets() ;

  virtual void testIllegalOffsets() ;

  virtual void
  testIllegalOffsetsAcrossFieldInstances() ;

  virtual void testBackwardsOffsets() ;

  virtual void testStackedTokens() ;

  virtual void testCrazyOffsetGap() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestPostingsOffsets> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestPostingsOffsets> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  public:
    int getOffsetGap(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testLegalbutVeryLargeOffsets() ;
  // TODO: more tests with other possibilities

private:
  void
  checkTokens(std::deque<std::shared_ptr<Token>> &field1,
              std::deque<std::shared_ptr<Token>> &field2) ;

  void
  checkTokens(std::deque<std::shared_ptr<Token>> &tokens) ;

  std::shared_ptr<Token> makeToken(const std::wstring &text, int posIncr,
                                   int startOffset, int endOffset);

protected:
  std::shared_ptr<TestPostingsOffsets> shared_from_this()
  {
    return std::static_pointer_cast<TestPostingsOffsets>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
