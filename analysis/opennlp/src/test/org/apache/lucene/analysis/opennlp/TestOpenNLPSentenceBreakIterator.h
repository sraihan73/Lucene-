#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/CharArrayIterator.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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

namespace org::apache::lucene::analysis::opennlp
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestOpenNLPSentenceBreakIterator : public LuceneTestCase
{
  GET_CLASS_NAME(TestOpenNLPSentenceBreakIterator)

private:
  static const std::wstring TEXT;
  static std::deque<std::wstring> const SENTENCES;
  static const std::wstring PADDING;
  static const std::wstring sentenceModelFile;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void populateCache() throws
  // java.io.IOException
  static void populateCache() ;

  virtual void testThreeSentences() ;

private:
  std::shared_ptr<CharacterIterator>
  getCharArrayIterator(const std::wstring &text);

  std::shared_ptr<CharacterIterator>
  getCharArrayIterator(const std::wstring &text, int start, int length);

private:
  class CharArrayIteratorAnonymousInnerClass : public CharArrayIterator
  {
    GET_CLASS_NAME(CharArrayIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<TestOpenNLPSentenceBreakIterator> outerInstance;

  public:
    CharArrayIteratorAnonymousInnerClass(
        std::shared_ptr<TestOpenNLPSentenceBreakIterator> outerInstance);

    // Lie about all surrogates to the sentence tokenizer,
    // instead we treat them all as SContinue so we won't break around them.
  protected:
    wchar_t jreBugWorkaround(wchar_t ch) override;

  protected:
    std::shared_ptr<CharArrayIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CharArrayIteratorAnonymousInnerClass>(
          org.apache.lucene.analysis.util
              .CharArrayIterator::shared_from_this());
    }
  };

private:
  void do3SentenceTest(std::shared_ptr<BreakIterator> bi);

public:
  virtual void testSingleSentence() ;

private:
  void test1Sentence(std::shared_ptr<BreakIterator> bi,
                     const std::wstring &text);

public:
  virtual void testSliceEnd() ;

  virtual void testSliceStart() ;

  virtual void testSliceMiddle() ;

  /** the current position must be ignored, initial position is always first()
   */
  virtual void testFirstPosition() ;

  virtual void testWhitespaceOnly() ;

  virtual void testEmptyString() ;

private:
  void test0Sentences(std::shared_ptr<BreakIterator> bi);

protected:
  std::shared_ptr<TestOpenNLPSentenceBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<TestOpenNLPSentenceBreakIterator>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::opennlp
