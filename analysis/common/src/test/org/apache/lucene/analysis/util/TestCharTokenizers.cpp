using namespace std;

#include "TestCharTokenizers.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/CharTokenizer.h"

namespace org::apache::lucene::analysis::util
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using LetterTokenizer = org::apache::lucene::analysis::core::LetterTokenizer;
using LowerCaseTokenizer =
    org::apache::lucene::analysis::core::LowerCaseTokenizer;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestCharTokenizers::testReadSupplementaryChars() 
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  // create random input
  int num = 1024 + random()->nextInt(1024);
  num *= RANDOM_MULTIPLIER;
  for (int i = 1; i < num; i++) {
    builder->append(L"\ud801\udc1cabc");
    if ((i % 10) == 0) {
      builder->append(L" ");
    }
  }
  // internal buffer size is 1024 make sure we have a surrogate pair right at
  // the border
  builder->insert(1023, L"\ud801\udc1c");
  shared_ptr<Tokenizer> tokenizer =
      make_shared<LowerCaseTokenizer>(newAttributeFactory());
  tokenizer->setReader(make_shared<StringReader>(builder->toString()));
  assertTokenStreamContents(
      tokenizer, builder->toString()->toLowerCase(Locale::ROOT)->split(L" "));
}

void TestCharTokenizers::testExtendCharBuffer() 
{
  for (int i = 0; i < 40; i++) {
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    for (int j = 0; j < 1 + i; j++) {
      builder->append(L"a");
    }
    builder->append(L"\ud801\udc1cabc");
    shared_ptr<Tokenizer> tokenizer =
        make_shared<LowerCaseTokenizer>(newAttributeFactory());
    tokenizer->setReader(make_shared<StringReader>(builder->toString()));
    assertTokenStreamContents(
        tokenizer,
        std::deque<wstring>{builder->toString()->toLowerCase(Locale::ROOT)});
  }
}

void TestCharTokenizers::testMaxWordLength() 
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();

  for (int i = 0; i < 255; i++) {
    builder->append(L"A");
  }
  shared_ptr<Tokenizer> tokenizer =
      make_shared<LowerCaseTokenizer>(newAttributeFactory());
  tokenizer->setReader(
      make_shared<StringReader>(builder->toString() + builder->toString()));
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{builder->toString()->toLowerCase(Locale::ROOT),
                           builder->toString()->toLowerCase(Locale::ROOT)});
}

void TestCharTokenizers::testCustomMaxTokenLength() 
{

  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (int i = 0; i < 100; i++) {
    builder->append(L"A");
  }
  shared_ptr<Tokenizer> tokenizer =
      make_shared<LowerCaseTokenizer>(newAttributeFactory(), 100);
  // Tricky, passing two copies of the string to the reader....
  tokenizer->setReader(
      make_shared<StringReader>(builder->toString() + builder->toString()));
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{builder->toString()->toLowerCase(Locale::ROOT),
                           builder->toString()->toLowerCase(Locale::ROOT)});

  runtime_error e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::LowerCaseTokenizer>(
        newAttributeFactory(), -1);
  });
  TestUtil::assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: -1",
      e.what());

  tokenizer = make_shared<LetterTokenizer>(newAttributeFactory(), 100);
  tokenizer->setReader(
      make_shared<StringReader>(builder->toString() + builder->toString()));
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{builder->toString(), builder->toString()});

  // Let's test that we can get a token longer than 255 through.
  builder->setLength(0);
  for (int i = 0; i < 500; i++) {
    builder->append(L"Z");
  }
  tokenizer = make_shared<LetterTokenizer>(newAttributeFactory(), 500);
  tokenizer->setReader(make_shared<StringReader>(builder->toString()));
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{builder->toString()});

  // Just to be sure what is happening here, token lengths of zero make no
  // sense, Let's try the edge cases, token > I/O buffer (4096)
  builder->setLength(0);
  for (int i = 0; i < 600; i++) {
    builder->append(L"aUrOkIjq"); // 600 * 8 = 4800 chars.
  }

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::LowerCaseTokenizer>(
        newAttributeFactory(), 0);
  });
  TestUtil::assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: 0",
      e.what());

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::LowerCaseTokenizer>(
        newAttributeFactory(), 10'000'000);
  });
  TestUtil::assertEquals(L"maxTokenLen must be greater than 0 and less than "
                         L"1048576 passed: 10000000",
                         e.what());

  tokenizer = make_shared<LowerCaseTokenizer>(newAttributeFactory(), 4800);
  tokenizer->setReader(make_shared<StringReader>(builder->toString()));
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{builder->toString()->toLowerCase(Locale::ROOT)});

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::KeywordTokenizer>(
        newAttributeFactory(), 0);
  });
  TestUtil::assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: 0",
      e.what());

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::KeywordTokenizer>(
        newAttributeFactory(), 10'000'000);
  });
  TestUtil::assertEquals(L"maxTokenLen must be greater than 0 and less than "
                         L"1048576 passed: 10000000",
                         e.what());

  tokenizer = make_shared<KeywordTokenizer>(newAttributeFactory(), 4800);
  tokenizer->setReader(make_shared<StringReader>(builder->toString()));
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{builder->toString()});

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::LetterTokenizer>(
        newAttributeFactory(), 0);
  });
  TestUtil::assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: 0",
      e.what());

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::LetterTokenizer>(
        newAttributeFactory(), 2'000'000);
  });
  TestUtil::assertEquals(L"maxTokenLen must be greater than 0 and less than "
                         L"1048576 passed: 2000000",
                         e.what());

  tokenizer = make_shared<LetterTokenizer>(newAttributeFactory(), 4800);
  tokenizer->setReader(make_shared<StringReader>(builder->toString()));
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{builder->toString()});

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::WhitespaceTokenizer>(
        newAttributeFactory(), 0);
  });
  TestUtil::assertEquals(
      L"maxTokenLen must be greater than 0 and less than 1048576 passed: 0",
      e.what());

  e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<org::apache::lucene::analysis::core::WhitespaceTokenizer>(
        newAttributeFactory(), 3'000'000);
  });
  TestUtil::assertEquals(L"maxTokenLen must be greater than 0 and less than "
                         L"1048576 passed: 3000000",
                         e.what());

  tokenizer = make_shared<WhitespaceTokenizer>(newAttributeFactory(), 4800);
  tokenizer->setReader(make_shared<StringReader>(builder->toString()));
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{builder->toString()});
}

void TestCharTokenizers::testMaxWordLengthWithSupplementary() 
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();

  for (int i = 0; i < 254; i++) {
    builder->append(L"A");
  }
  builder->append(L"\ud801\udc1c");
  shared_ptr<Tokenizer> tokenizer =
      make_shared<LowerCaseTokenizer>(newAttributeFactory());
  tokenizer->setReader(
      make_shared<StringReader>(builder->toString() + builder->toString()));
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{builder->toString()->toLowerCase(Locale::ROOT),
                           builder->toString()->toLowerCase(Locale::ROOT)});
}

void TestCharTokenizers::testCrossPlaneNormalization() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  int num = 1000 * RANDOM_MULTIPLIER;
  for (int i = 0; i < num; i++) {
    wstring s = TestUtil::randomUnicodeString(random());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts
    // = analyzer.tokenStream("foo", s))
    {
      org::apache::lucene::analysis::TokenStream ts =
          analyzer->tokenStream(L"foo", s);
      ts->reset();
      shared_ptr<OffsetAttribute> offsetAtt =
          ts->addAttribute(OffsetAttribute::typeid);
      while (ts->incrementToken()) {
        wstring highlightedText =
            s.substr(offsetAtt->startOffset(),
                     offsetAtt->endOffset() - offsetAtt->startOffset());
        for (int j = 0, cp = 0; j < highlightedText.length();
             j += Character::charCount(cp)) {
          cp = highlightedText.codePointAt(j);
          assertTrue(L"non-letter:" + Integer::toHexString(cp), isalpha(cp));
        }
      }
      ts->end();
    }
  }
  // just for fun
  checkRandomData(random(), analyzer, num);
  delete analyzer;
}

TestCharTokenizers::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestCharTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCharTokenizers::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<LetterTokenizerAnonymousInnerClass>(
          shared_from_this(), BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestCharTokenizers::AnalyzerAnonymousInnerClass::
    LetterTokenizerAnonymousInnerClass::LetterTokenizerAnonymousInnerClass(
        shared_ptr<AnalyzerAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::util::AttributeFactory>
            newAttributeFactory)
    : org::apache::lucene::analysis::core::LetterTokenizer(newAttributeFactory)
{
  this->outerInstance = outerInstance;
}

int TestCharTokenizers::AnalyzerAnonymousInnerClass::
    LetterTokenizerAnonymousInnerClass::normalize(int c)
{
  if (c > 0xffff) {
    return L'δ';
  } else {
    return c;
  }
}

void TestCharTokenizers::testCrossPlaneNormalization2() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  int num = 1000 * RANDOM_MULTIPLIER;
  for (int i = 0; i < num; i++) {
    wstring s = TestUtil::randomUnicodeString(random());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts
    // = analyzer.tokenStream("foo", s))
    {
      org::apache::lucene::analysis::TokenStream ts =
          analyzer->tokenStream(L"foo", s);
      ts->reset();
      shared_ptr<OffsetAttribute> offsetAtt =
          ts->addAttribute(OffsetAttribute::typeid);
      while (ts->incrementToken()) {
        wstring highlightedText =
            s.substr(offsetAtt->startOffset(),
                     offsetAtt->endOffset() - offsetAtt->startOffset());
        for (int j = 0, cp = 0; j < highlightedText.length();
             j += Character::charCount(cp)) {
          cp = highlightedText.codePointAt(j);
          assertTrue(L"non-letter:" + Integer::toHexString(cp), isalpha(cp));
        }
      }
      ts->end();
    }
  }
  // just for fun
  checkRandomData(random(), analyzer, num);
  delete analyzer;
}

TestCharTokenizers::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestCharTokenizers> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCharTokenizers::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<LetterTokenizerAnonymousInnerClass2>(
          shared_from_this(), BaseTokenStreamTestCase::newAttributeFactory());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestCharTokenizers::AnalyzerAnonymousInnerClass2::
    LetterTokenizerAnonymousInnerClass2::LetterTokenizerAnonymousInnerClass2(
        shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance,
        shared_ptr<org::apache::lucene::util::AttributeFactory>
            newAttributeFactory)
    : org::apache::lucene::analysis::core::LetterTokenizer(newAttributeFactory)
{
  this->outerInstance = outerInstance;
}

int TestCharTokenizers::AnalyzerAnonymousInnerClass2::
    LetterTokenizerAnonymousInnerClass2::normalize(int c)
{
  if (c <= 0xffff) {
    return 0x1043C;
  } else {
    return c;
  }
}

void TestCharTokenizers::testDefinitionUsingMethodReference1() throw(
    runtime_error)
{
  shared_ptr<StringReader> *const reader =
      make_shared<StringReader>(L"Tokenizer Test");
  shared_ptr<Tokenizer> *const tokenizer =
      CharTokenizer::fromSeparatorCharPredicate(
          optional<wchar_t>::isWhitespace);
  tokenizer->setReader(reader);
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"Tokenizer", L"Test"});
}

void TestCharTokenizers::testDefinitionUsingMethodReference2() throw(
    runtime_error)
{
  shared_ptr<StringReader> *const reader =
      make_shared<StringReader>(L"Tokenizer(Test)");
  shared_ptr<Tokenizer> *const tokenizer =
      CharTokenizer::fromTokenCharPredicate(optional<wchar_t>::isLetter,
                                            optional<wchar_t>::toUpperCase);
  tokenizer->setReader(reader);
  assertTokenStreamContents(tokenizer,
                            std::deque<wstring>{L"TOKENIZER", L"TEST"});
}

void TestCharTokenizers::testDefinitionUsingLambda() 
{
  shared_ptr<StringReader> *const reader =
      make_shared<StringReader>(L"Tokenizer\u00A0Test Foo");
  shared_ptr<Tokenizer> *const tokenizer =
      CharTokenizer::fromSeparatorCharPredicate(
          [&](any c) { return c == L'\u00A0' || isspace(c); },
          optional<wchar_t>::toLowerCase);
  tokenizer->setReader(reader);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"tokenizer", L"test", L"foo"});
}
} // namespace org::apache::lucene::analysis::util