#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

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
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTermVectorsReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermVectorsReader)
  // Must be lexicographically sorted, will do in setup, versus trying to
  // maintain here
private:
  std::deque<std::wstring> testFields = {L"f1", L"f2", L"f3", L"f4"};
  std::deque<bool> testFieldsStorePos = {true, false, true, false};
  std::deque<bool> testFieldsStoreOff = {true, false, false, true};
  std::deque<std::wstring> testTerms = {L"this", L"is", L"a", L"test"};
  std::deque<std::deque<int>> positions =
      std::deque<std::deque<int>>(testTerms.size());
  std::shared_ptr<Directory> dir;
  std::shared_ptr<SegmentCommitInfo> seg;
  std::shared_ptr<FieldInfos> fieldInfos =
      std::make_shared<FieldInfos>(std::deque<std::shared_ptr<FieldInfo>>(0));
  static int TERM_FREQ;

private:
  class TestToken : public std::enable_shared_from_this<TestToken>,
                    public Comparable<std::shared_ptr<TestToken>>
  {
    GET_CLASS_NAME(TestToken)
  public:
    std::wstring text;
    int pos = 0;
    int startOffset = 0;
    int endOffset = 0;
    int compareTo(std::shared_ptr<TestToken> other) override;
  };

public:
  std::deque<std::shared_ptr<TestToken>> tokens =
      std::deque<std::shared_ptr<TestToken>>(testTerms.size() * TERM_FREQ);

  void setUp()  override;

  void tearDown()  override;

private:
  class MyTokenizer : public Tokenizer
  {
    GET_CLASS_NAME(MyTokenizer)
  private:
    std::shared_ptr<TestTermVectorsReader> outerInstance;

    int tokenUpto = 0;

    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;

  public:
    MyTokenizer(std::shared_ptr<TestTermVectorsReader> outerInstance);

    bool incrementToken() override;

    void reset()  override;

  protected:
    std::shared_ptr<MyTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<MyTokenizer>(
          Tokenizer::shared_from_this());
    }
  };

private:
  class MyAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(MyAnalyzer)
  private:
    std::shared_ptr<TestTermVectorsReader> outerInstance;

  public:
    MyAnalyzer(std::shared_ptr<TestTermVectorsReader> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<MyAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<MyAnalyzer>(Analyzer::shared_from_this());
    }
  };

public:
  virtual void test() ;

  virtual void testReader() ;

  virtual void testDocsEnum() ;

  virtual void testPositionReader() ;

  virtual void testOffsetReader() ;

  virtual void testIllegalPayloadsWithoutPositions() ;

  virtual void testIllegalOffsetsWithoutVectors() ;

  virtual void testIllegalPositionsWithoutVectors() ;

  virtual void
  testIllegalVectorPayloadsWithoutVectors() ;

  virtual void testIllegalVectorsWithoutIndexed() ;

  virtual void
  testIllegalVectorPositionsWithoutIndexed() ;

  virtual void
  testIllegalVectorOffsetsWithoutIndexed() ;

  virtual void
  testIllegalVectorPayloadsWithoutIndexed() ;

protected:
  std::shared_ptr<TestTermVectorsReader> shared_from_this()
  {
    return std::static_pointer_cast<TestTermVectorsReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
