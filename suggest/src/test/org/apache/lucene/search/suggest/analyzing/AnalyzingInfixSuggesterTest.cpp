using namespace std;

#include "AnalyzingInfixSuggesterTest.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using Input = org::apache::lucene::search::suggest::Input;
using InputArrayIterator =
    org::apache::lucene::search::suggest::InputArrayIterator;
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

void AnalyzingInfixSuggesterTest::testBasic() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"lend me your ear", 8,
                         make_shared<BytesRef>(L"foobar")),
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"))};

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ear", random()), 10, true, true);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(L"foobaz", results[0]->payload.utf8ToString());

  TestUtil::assertEquals(L"lend me your ear", results[1]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[1]->highlightKey);
  TestUtil::assertEquals(8, results[1]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), results[1]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"ear ", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"lend me your ear", results[0]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[0]->highlightKey);
  TestUtil::assertEquals(8, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), results[0]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"pen", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>pen</b>ny saved is a <b>pen</b>ny earned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"p", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>p</b>enny saved is a <b>p</b>enny earned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"money penny", random()), 10, false,
      true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>penny</b> saved is a <b>penny</b> earned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"penny ea", random()), 10, false, true);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(
      L"a <b>penny</b> saved is a <b>penny</b> <b>ea</b>rned",
      results[0]->highlightKey);
  TestUtil::assertEquals(L"lend me your ear", results[1]->key);
  TestUtil::assertEquals(L"lend me your <b>ea</b>r", results[1]->highlightKey);

  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"money penny", random()), 10, false,
      false);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  assertNull(results[0]->highlightKey);

  testConstructorDefaults(suggester, keys, a, true, true);
  testConstructorDefaults(suggester, keys, a, true, false);
  testConstructorDefaults(suggester, keys, a, false, false);
  testConstructorDefaults(suggester, keys, a, false, true);

  delete suggester;
  delete a;
}

void AnalyzingInfixSuggesterTest::testConstructorDefaults(
    shared_ptr<AnalyzingInfixSuggester> suggester,
    std::deque<std::shared_ptr<Input>> &keys, shared_ptr<Analyzer> a,
    bool allTermsRequired, bool highlight) 
{
  shared_ptr<AnalyzingInfixSuggester> suggester2 =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false,
                                           allTermsRequired, highlight);
  suggester2->build(make_shared<InputArrayIterator>(keys));

  shared_ptr<std::wstring> key =
      TestUtil::stringToCharSequence(L"penny ea", random());

  deque<std::shared_ptr<LookupResult>> results1 =
      suggester->lookup(key, 10, allTermsRequired, highlight);
  deque<std::shared_ptr<LookupResult>> results2 =
      suggester2->lookup(key, false, 10);
  TestUtil::assertEquals(results1.size(), results2.size());
  TestUtil::assertEquals(results1[0]->key, results2[0]->key);
  TestUtil::assertEquals(results1[0]->highlightKey, results2[0]->highlightKey);

  delete suggester2;
}

void AnalyzingInfixSuggesterTest::testAfterLoad() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"lend me your ear", 8,
                         make_shared<BytesRef>(L"foobar")),
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"))};

  shared_ptr<Path> tempDir = createTempDir(L"AnalyzingInfixSuggesterTest");

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a, a, 3,
                                           false);
  suggester->build(make_shared<InputArrayIterator>(keys));
  TestUtil::assertEquals(2, suggester->getCount());
  delete suggester;

  suggester = make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a,
                                                   a, 3, false);
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ear", random()), 10, true, true);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);
  TestUtil::assertEquals(2, suggester->getCount());
  delete suggester;
  delete a;
}

AnalyzingInfixSuggesterTest::LookupHighlightFragment::LookupHighlightFragment(
    const wstring &text, bool isHit)
    : text(text), isHit(isHit)
{
}

wstring AnalyzingInfixSuggesterTest::LookupHighlightFragment::toString()
{
  return L"LookupHighlightFragment(text=" + text + L" isHit=" +
         StringHelper::toString(isHit) + L")";
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public void
// testHighlightAsObject() throws Exception
void AnalyzingInfixSuggesterTest::testHighlightAsObject() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"))};

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggesterAnonymousInnerClass>(
          shared_from_this(), newDirectory(), a, a);
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ear", random()), 10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(
      L"a penny saved is a penny <b>ear</b>ned",
      toString(any_cast<deque<std::shared_ptr<LookupHighlightFragment>>>(
          results[0]->highlightKey)));
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);
  delete suggester;
  delete a;
}

AnalyzingInfixSuggesterTest::AnalyzingInfixSuggesterAnonymousInnerClass::
    AnalyzingInfixSuggesterAnonymousInnerClass(
        shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        shared_ptr<org::apache::lucene::store::BaseDirectoryWrapper>
            newDirectory,
        shared_ptr<Analyzer> a, shared_ptr<Analyzer> a)
    : AnalyzingInfixSuggester(newDirectory, a, a, 3, false)
{
  this->outerInstance = outerInstance;
}

any AnalyzingInfixSuggesterTest::AnalyzingInfixSuggesterAnonymousInnerClass::
    highlight(const wstring &text, shared_ptr<Set<wstring>> matchedTokens,
              const wstring &prefixToken) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // queryAnalyzer.tokenStream("text", new java.io.StringReader(text)))
  {
    org::apache::lucene::analysis::TokenStream ts = queryAnalyzer::tokenStream(
        L"text", make_shared<java::io::StringReader>(text));
    shared_ptr<CharTermAttribute> termAtt =
        ts->addAttribute(CharTermAttribute::typeid);
    shared_ptr<OffsetAttribute> offsetAtt =
        ts->addAttribute(OffsetAttribute::typeid);
    ts->reset();
    deque<std::shared_ptr<LookupHighlightFragment>> fragments =
        deque<std::shared_ptr<LookupHighlightFragment>>();
    int upto = 0;
    while (ts->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring token = termAtt->toString();
      int startOffset = offsetAtt->startOffset();
      int endOffset = offsetAtt->endOffset();
      if (upto < startOffset) {
        fragments.push_back(make_shared<LookupHighlightFragment>(
            text.substr(upto, startOffset - upto), false));
        upto = startOffset;
      } else if (upto > startOffset) {
        continue;
      }

      if (matchedTokens->contains(token)) {
        // Token matches.
        fragments.push_back(make_shared<LookupHighlightFragment>(
            text.substr(startOffset, endOffset - startOffset), true));
        upto = endOffset;
      } else if (prefixToken != L"" &&
                 StringHelper::startsWith(token, prefixToken)) {
        fragments.push_back(make_shared<LookupHighlightFragment>(
            text.substr(startOffset, prefixToken.length()), true));
        if (prefixToken.length() < token.length()) {
          fragments.push_back(make_shared<LookupHighlightFragment>(
              text.substr(startOffset + prefixToken.length(),
                          (startOffset + token.length()) -
                              (startOffset + prefixToken.length())),
              false));
        }
        upto = endOffset;
      }
    }
    ts->end();
    int endOffset = offsetAtt->endOffset();
    if (upto < endOffset) {
      fragments.push_back(
          make_shared<LookupHighlightFragment>(text.substr(upto), false));
    }

    return fragments;
  }
}

wstring AnalyzingInfixSuggesterTest::toString(
    deque<std::shared_ptr<LookupHighlightFragment>> &fragments)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (auto fragment : fragments) {
    if (fragment->isHit) {
      sb->append(L"<b>");
    }
    sb->append(fragment->text);
    if (fragment->isHit) {
      sb->append(L"</b>");
    }
  }

  return sb->toString();
}

void AnalyzingInfixSuggesterTest::testRandomMinPrefixLength() throw(
    runtime_error)
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"lend me your ear", 8,
                         make_shared<BytesRef>(L"foobar")),
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"))};
  shared_ptr<Path> tempDir = createTempDir(L"AnalyzingInfixSuggesterTest");

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  int minPrefixLength = random()->nextInt(10);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a, a,
                                           minPrefixLength, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      bool doHighlight = j == 0;

      deque<std::shared_ptr<LookupResult>> results =
          suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                            10, true, doHighlight);
      TestUtil::assertEquals(2, results.size());
      TestUtil::assertEquals(L"a penny saved is a penny earned",
                             results[0]->key);
      if (doHighlight) {
        TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                               results[0]->highlightKey);
      }
      TestUtil::assertEquals(10, results[0]->value);
      TestUtil::assertEquals(L"lend me your ear", results[1]->key);
      if (doHighlight) {
        TestUtil::assertEquals(L"lend me your <b>ear</b>",
                               results[1]->highlightKey);
      }
      TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"),
                             results[0]->payload);
      TestUtil::assertEquals(8, results[1]->value);
      TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"),
                             results[1]->payload);

      results =
          suggester->lookup(TestUtil::stringToCharSequence(L"ear ", random()),
                            10, true, doHighlight);
      TestUtil::assertEquals(1, results.size());
      TestUtil::assertEquals(L"lend me your ear", results[0]->key);
      if (doHighlight) {
        TestUtil::assertEquals(L"lend me your <b>ear</b>",
                               results[0]->highlightKey);
      }
      TestUtil::assertEquals(8, results[0]->value);
      TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"),
                             results[0]->payload);

      results =
          suggester->lookup(TestUtil::stringToCharSequence(L"pen", random()),
                            10, true, doHighlight);
      TestUtil::assertEquals(1, results.size());
      TestUtil::assertEquals(L"a penny saved is a penny earned",
                             results[0]->key);
      if (doHighlight) {
        TestUtil::assertEquals(L"a <b>pen</b>ny saved is a <b>pen</b>ny earned",
                               results[0]->highlightKey);
      }
      TestUtil::assertEquals(10, results[0]->value);
      TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"),
                             results[0]->payload);

      results =
          suggester->lookup(TestUtil::stringToCharSequence(L"p", random()), 10,
                            true, doHighlight);
      TestUtil::assertEquals(1, results.size());
      TestUtil::assertEquals(L"a penny saved is a penny earned",
                             results[0]->key);
      if (doHighlight) {
        TestUtil::assertEquals(L"a <b>p</b>enny saved is a <b>p</b>enny earned",
                               results[0]->highlightKey);
      }
      TestUtil::assertEquals(10, results[0]->value);
      TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"),
                             results[0]->payload);
    }

    // Make sure things still work after close and reopen:
    delete suggester;
    suggester = make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a,
                                                     a, minPrefixLength, false);
  }
  delete suggester;
  delete a;
}

void AnalyzingInfixSuggesterTest::testHighlight() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"))};

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  suggester->build(make_shared<InputArrayIterator>(keys));
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"penn", random()), 10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>penn</b>y saved is a <b>penn</b>y earned",
                         results[0]->highlightKey);
  delete suggester;
  delete a;
}

void AnalyzingInfixSuggesterTest::testHighlightCaseChange() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"a Penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"))};

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  suggester->build(make_shared<InputArrayIterator>(keys));
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"penn", random()), 10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a Penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>Penn</b>y saved is a <b>penn</b>y earned",
                         results[0]->highlightKey);
  delete suggester;

  // Try again, but overriding addPrefixMatch to highlight
  // the entire hit:
  suggester = make_shared<AnalyzingInfixSuggesterAnonymousInnerClass2>(
      shared_from_this(), newDirectory(), a, a);
  suggester->build(make_shared<InputArrayIterator>(keys));
  results = suggester->lookup(TestUtil::stringToCharSequence(L"penn", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a Penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>Penny</b> saved is a <b>penny</b> earned",
                         results[0]->highlightKey);
  delete suggester;
  delete a;
}

AnalyzingInfixSuggesterTest::AnalyzingInfixSuggesterAnonymousInnerClass2::
    AnalyzingInfixSuggesterAnonymousInnerClass2(
        shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        shared_ptr<org::apache::lucene::store::BaseDirectoryWrapper>
            newDirectory,
        shared_ptr<Analyzer> a, shared_ptr<Analyzer> a)
    : AnalyzingInfixSuggester(newDirectory, a, a, 3, false)
{
  this->outerInstance = outerInstance;
}

void AnalyzingInfixSuggesterTest::AnalyzingInfixSuggesterAnonymousInnerClass2::
    addPrefixMatch(shared_ptr<StringBuilder> sb, const wstring &surface,
                   const wstring &analyzed, const wstring &prefixToken)
{
  sb->append(L"<b>");
  sb->append(surface);
  sb->append(L"</b>");
}

void AnalyzingInfixSuggesterTest::testDoubleClose() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"))};

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  suggester->build(make_shared<InputArrayIterator>(keys));
  delete suggester;
  delete suggester;
  delete a;
}

void AnalyzingInfixSuggesterTest::testSuggestStopFilter() 
{
  shared_ptr<CharArraySet> *const stopWords = StopFilter::makeStopSet({L"a"});
  shared_ptr<Analyzer> indexAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), stopWords);

  shared_ptr<Analyzer> queryAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), stopWords);

  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), indexAnalyzer,
                                           queryAnalyzer, 3, false);

  std::deque<std::shared_ptr<Input>> keys = {make_shared<Input>(
      L"a bob for apples", 10, make_shared<BytesRef>(L"foobaz"))};

  suggester->build(make_shared<InputArrayIterator>(keys));
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"a", random()), 10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a bob for apples", results[0]->key);
  TestUtil::assertEquals(L"a bob for <b>a</b>pples", results[0]->highlightKey);
  delete suggester;
  IOUtils::close({suggester, indexAnalyzer, queryAnalyzer});
}

AnalyzingInfixSuggesterTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        shared_ptr<CharArraySet> stopWords)
{
  this->outerInstance = outerInstance;
  this->stopWords = stopWords;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingInfixSuggesterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokens = make_shared<MockTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokens, make_shared<StopFilter>(tokens, stopWords));
}

AnalyzingInfixSuggesterTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        shared_ptr<CharArraySet> stopWords)
{
  this->outerInstance = outerInstance;
  this->stopWords = stopWords;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingInfixSuggesterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokens = make_shared<MockTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokens, make_shared<SuggestStopFilter>(tokens, stopWords));
}

void AnalyzingInfixSuggesterTest::testEmptyAtStart() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>(0)));
  suggester->add(make_shared<BytesRef>(L"a penny saved is a penny earned"),
                 nullptr, 10, make_shared<BytesRef>(L"foobaz"));
  suggester->add(make_shared<BytesRef>(L"lend me your ear"), nullptr, 8,
                 make_shared<BytesRef>(L"foobar"));
  suggester->refresh();
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ear", random()), 10, true, true);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  TestUtil::assertEquals(L"lend me your ear", results[1]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[1]->highlightKey);
  TestUtil::assertEquals(8, results[1]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), results[1]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"ear ", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"lend me your ear", results[0]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[0]->highlightKey);
  TestUtil::assertEquals(8, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), results[0]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"pen", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>pen</b>ny saved is a <b>pen</b>ny earned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"p", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>p</b>enny saved is a <b>p</b>enny earned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  delete suggester;
  delete a;
}

void AnalyzingInfixSuggesterTest::testBothExactAndPrefix() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>(0)));
  suggester->add(make_shared<BytesRef>(L"the pen is pretty"), nullptr, 10,
                 make_shared<BytesRef>(L"foobaz"));
  suggester->refresh();

  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"pen p", random()), 10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"the pen is pretty", results[0]->key);
  TestUtil::assertEquals(L"the <b>pen</b> is <b>p</b>retty",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);
  delete suggester;
  delete a;
}

wstring AnalyzingInfixSuggesterTest::randomText()
{
  int numWords = TestUtil::nextInt(random(), 1, 4);

  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (int i = 0; i < numWords; i++) {
    if (i > 0) {
      b->append(L' ');
    }
    b->append(TestUtil::randomSimpleString(random(), 1, 10));
  }

  return b->toString();
}

AnalyzingInfixSuggesterTest::LookupThread::LookupThread(
    shared_ptr<AnalyzingInfixSuggester> suggester)
    : suggester(suggester)
{
}

void AnalyzingInfixSuggesterTest::LookupThread::finish() throw(
    InterruptedException)
{
  stop = true;
  this->join();
}

void AnalyzingInfixSuggesterTest::LookupThread::run()
{
  while (stop == false) {
    wstring query = randomText();
    int topN = TestUtil::nextInt(LuceneTestCase::random(), 1, 100);
    bool allTermsRequired = LuceneTestCase::random()->nextBoolean();
    bool doHilite = LuceneTestCase::random()->nextBoolean();
    // We don't verify the results; just doing
    // simultaneous lookups while adding/updating to
    // see if there are any thread hazards:
    try {
      suggester->lookup(
          TestUtil::stringToCharSequence(query, LuceneTestCase::random()), topN,
          allTermsRequired, doHilite);
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  }
}

void AnalyzingInfixSuggesterTest::testRandomNRT() 
{
  shared_ptr<Path> *const tempDir =
      createTempDir(L"AnalyzingInfixSuggesterTest");
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  int minPrefixChars = random()->nextInt(7);
  if (VERBOSE) {
    wcout << L"  minPrefixChars=" << minPrefixChars << endl;
  }

  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a, a,
                                           minPrefixChars, false);

  // Initial suggester built with nothing:
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>(0)));

  shared_ptr<LookupThread> lookupThread = make_shared<LookupThread>(suggester);
  lookupThread->start();

  int iters = atLeast(1000);
  int visibleUpto = 0;

  shared_ptr<Set<int64_t>> usedWeights = unordered_set<int64_t>();
  shared_ptr<Set<wstring>> usedKeys = unordered_set<wstring>();

  deque<std::shared_ptr<Input>> inputs = deque<std::shared_ptr<Input>>();
  deque<std::shared_ptr<Update>> pendingUpdates =
      deque<std::shared_ptr<Update>>();

  for (int iter = 0; iter < iters; iter++) {
    wstring text;
    while (true) {
      text = randomText();
      if (usedKeys->contains(text) == false) {
        usedKeys->add(text);
        break;
      }
    }

    // Carefully pick a weight we never used, to sidestep
    // tie-break problems:
    int64_t weight;
    while (true) {
      weight = random()->nextInt(10 * iters);
      if (usedWeights->contains(weight) == false) {
        usedWeights->add(weight);
        break;
      }
    }

    if (inputs.size() > 0 && random()->nextInt(4) == 1) {
      // Update an existing suggestion
      shared_ptr<Update> update = make_shared<Update>();
      update->index = random()->nextInt(inputs.size());
      update->weight = weight;
      shared_ptr<Input> input = inputs[update->index];
      pendingUpdates.push_back(update);
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L" update input="
              << input->term->utf8ToString() << L"/" << weight << endl;
      }
      suggester->update(input->term, nullptr, weight, input->term);

    } else {
      // Add a new suggestion
      inputs.push_back(
          make_shared<Input>(text, weight, make_shared<BytesRef>(text)));
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L" add input=" << text << L"/"
              << weight << endl;
      }
      shared_ptr<BytesRef> br = make_shared<BytesRef>(text);
      suggester->add(br, nullptr, weight, br);
    }

    if (random()->nextInt(15) == 7) {
      if (VERBOSE) {
        wcout << L"TEST: now refresh suggester" << endl;
      }
      suggester->refresh();
      visibleUpto = inputs.size();
      for (auto update : pendingUpdates) {
        shared_ptr<Input> oldInput = inputs[update->index];
        shared_ptr<Input> newInput = make_shared<Input>(
            oldInput->term, update->weight, oldInput->payload);
        inputs[update->index] = newInput;
      }
      pendingUpdates.clear();
    }

    if (random()->nextInt(50) == 7) {
      if (VERBOSE) {
        wcout << L"TEST: now close/reopen suggester" << endl;
      }
      lookupThread->finish();
      delete suggester;
      suggester = make_shared<AnalyzingInfixSuggester>(
          newFSDirectory(tempDir), a, a, minPrefixChars, false);
      lookupThread = make_shared<LookupThread>(suggester);
      lookupThread->start();

      visibleUpto = inputs.size();
      for (auto update : pendingUpdates) {
        shared_ptr<Input> oldInput = inputs[update->index];
        shared_ptr<Input> newInput = make_shared<Input>(
            oldInput->term, update->weight, oldInput->payload);
        inputs[update->index] = newInput;
      }
      pendingUpdates.clear();
    }

    if (visibleUpto > 0) {
      wstring query = randomText();
      bool lastPrefix = random()->nextInt(5) != 1;
      if (lastPrefix == false) {
        query += L" ";
      }

      std::deque<wstring> queryTerms = query.split(L"\\s");
      bool allTermsRequired = random()->nextInt(10) == 7;
      bool doHilite = random()->nextBoolean();

      if (VERBOSE) {
        wcout << L"TEST: lookup \"" << query << L"\" allTermsRequired="
              << allTermsRequired << L" doHilite=" << doHilite << endl;
      }

      // Stupid slow but hopefully correct matching:
      deque<std::shared_ptr<Input>> expected =
          deque<std::shared_ptr<Input>>();
      for (int i = 0; i < visibleUpto; i++) {
        shared_ptr<Input> input = inputs[i];
        std::deque<wstring> inputTerms =
            input->term->utf8ToString().split(L"\\s");
        bool match = false;
        for (int j = 0; j < queryTerms.size(); j++) {
          if (j < queryTerms.size() - 1 || lastPrefix == false) {
            // Exact match
            for (int k = 0; k < inputTerms.size(); k++) {
              if (inputTerms[k] == queryTerms[j]) {
                match = true;
                break;
              }
            }
          } else {
            // Prefix match
            for (int k = 0; k < inputTerms.size(); k++) {
              if (StringHelper::startsWith(inputTerms[k], queryTerms[j])) {
                match = true;
                break;
              }
            }
          }
          if (match) {
            if (allTermsRequired == false) {
              // At least one query term does match:
              break;
            }
            match = false;
          } else if (allTermsRequired) {
            // At least one query term does not match:
            break;
          }
        }

        if (match) {
          if (doHilite) {
            expected.push_back(
                make_shared<Input>(hilite(lastPrefix, inputTerms, queryTerms),
                                   input->v, input->term));
          } else {
            expected.push_back(input);
          }
        }
      }

      // C++ TODO: The 'Compare' parameter of std::sort produces a bool
      // value, while the Java Comparator parameter produces a tri-state result:
      // ORIGINAL LINE: java.util.Collections.sort(expected, (a1, b) ->
      sort(expected.begin(), expected.end(), [&](a1, b) {
        if (a1::v > b::v) {
          return -1;
        } else if (a1::v < b::v) {
          return 1;
        } else {
          return 0;
        }
      });

      if (expected.empty() == false) {

        int topN = TestUtil::nextInt(random(), 1, expected.size());

        deque<std::shared_ptr<LookupResult>> actual =
            suggester->lookup(TestUtil::stringToCharSequence(query, random()),
                              topN, allTermsRequired, doHilite);

        int expectedCount = min(topN, expected.size());

        if (VERBOSE) {
          wcout << L"  expected:" << endl;
          for (int i = 0; i < expectedCount; i++) {
            shared_ptr<Input> x = expected[i];
            wcout << L"    " << x->term->utf8ToString() << L"/" << x->v << endl;
          }
          wcout << L"  actual:" << endl;
          for (auto result : actual) {
            wcout << L"    " << result << endl;
          }
        }

        TestUtil::assertEquals(expectedCount, actual.size());
        for (int i = 0; i < expectedCount; i++) {
          if (doHilite) {
            TestUtil::assertEquals(expected[i]->term.utf8ToString(),
                                   actual[i]->highlightKey);
          } else {
            TestUtil::assertEquals(expected[i]->term.utf8ToString(),
                                   actual[i]->key);
          }
          TestUtil::assertEquals(expected[i]->v, actual[i]->value);
          TestUtil::assertEquals(expected[i]->payload, actual[i]->payload);
        }
      } else {
        if (VERBOSE) {
          wcout << L"  no expected matches" << endl;
        }
      }
    }
  }

  lookupThread->finish();
  delete suggester;
  delete a;
}

wstring AnalyzingInfixSuggesterTest::hilite(bool lastPrefix,
                                            std::deque<wstring> &inputTerms,
                                            std::deque<wstring> &queryTerms)
{
  // Stupid slow but hopefully correct highlighter:
  // System.out.println("hilite: lastPrefix=" + lastPrefix + " inputTerms=" +
  // Arrays.toString(inputTerms) + " queryTerms=" + Arrays.toString(queryTerms));
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (int i = 0; i < inputTerms.size(); i++) {
    if (i > 0) {
      b->append(L' ');
    }
    wstring inputTerm = inputTerms[i];
    // System.out.println("  inputTerm=" + inputTerm);
    bool matched = false;
    for (int j = 0; j < queryTerms.size(); j++) {
      wstring queryTerm = queryTerms[j];
      // System.out.println("    queryTerm=" + queryTerm);
      if (j < queryTerms.size() - 1 || lastPrefix == false) {
        // System.out.println("      check exact");
        if (inputTerm == queryTerm) {
          b->append(L"<b>");
          b->append(inputTerm);
          b->append(L"</b>");
          matched = true;
          break;
        }
      } else if (StringHelper::startsWith(inputTerm, queryTerm)) {
        b->append(L"<b>");
        b->append(queryTerm);
        b->append(L"</b>");
        b->append(inputTerm.substr(queryTerm.length(),
                                   inputTerm.length() - queryTerm.length()));
        matched = true;
        break;
      }
    }

    if (matched == false) {
      b->append(inputTerm);
    }
  }

  return b->toString();
}

void AnalyzingInfixSuggesterTest::testBasicNRT() 
{
  std::deque<std::shared_ptr<Input>> keys = {make_shared<Input>(
      L"lend me your ear", 8, make_shared<BytesRef>(L"foobar"))};

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ear", random()), 10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"lend me your ear", results[0]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[0]->highlightKey);
  TestUtil::assertEquals(8, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), results[0]->payload);

  // Add a new suggestion:
  suggester->add(make_shared<BytesRef>(L"a penny saved is a penny earned"),
                 nullptr, 10, make_shared<BytesRef>(L"foobaz"));

  // Must refresh to see any newly added suggestions:
  suggester->refresh();

  results = suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                              10, true, true);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  TestUtil::assertEquals(L"lend me your ear", results[1]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[1]->highlightKey);
  TestUtil::assertEquals(8, results[1]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), results[1]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"ear ", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"lend me your ear", results[0]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[0]->highlightKey);
  TestUtil::assertEquals(8, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), results[0]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"pen", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>pen</b>ny saved is a <b>pen</b>ny earned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  results = suggester->lookup(TestUtil::stringToCharSequence(L"p", random()),
                              10, true, true);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[0]->key);
  TestUtil::assertEquals(L"a <b>p</b>enny saved is a <b>p</b>enny earned",
                         results[0]->highlightKey);
  TestUtil::assertEquals(10, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[0]->payload);

  // Change the weight:
  suggester->update(make_shared<BytesRef>(L"lend me your ear"), nullptr, 12,
                    make_shared<BytesRef>(L"foobox"));

  // Must refresh to see any newly added suggestions:
  suggester->refresh();

  results = suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                              10, true, true);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"lend me your ear", results[0]->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", results[0]->highlightKey);
  TestUtil::assertEquals(12, results[0]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobox"), results[0]->payload);
  TestUtil::assertEquals(L"a penny saved is a penny earned", results[1]->key);
  TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                         results[1]->highlightKey);
  TestUtil::assertEquals(10, results[1]->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), results[1]->payload);
  delete suggester;
  delete a;
}

void AnalyzingInfixSuggesterTest::testNRTWithParallelAdds() throw(
    IOException, InterruptedException)
{
  std::deque<wstring> keys = {L"python",  L"java",   L"c",  L"scala", L"ruby",
                               L"clojure", L"erlang", L"go", L"swift", L"lisp"};
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<Path> tempDir = createTempDir(L"AIS_NRT_PERSIST_TEST");
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a, a, 3,
                                           false);
  std::deque<std::shared_ptr<Thread>> multiAddThreads(10);
  // Cannot call refresh on an suggester when no docs are added to the index
  expectThrows(IllegalStateException::typeid, [&]() { suggester->refresh(); });

  for (int i = 0; i < 10; i++) {
    multiAddThreads[i] =
        make_shared<Thread>(make_shared<IndexDocument>(suggester, keys[i]));
  }
  for (int i = 0; i < 10; i++) {
    multiAddThreads[i]->start();
  }
  // Make sure all threads have completed indexing
  for (int i = 0; i < 10; i++) {
    multiAddThreads[i]->join();
  }

  suggester->refresh();
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"python", random()), 10, true, false);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"python", results[0]->key);

  // Test if the index is getting persisted correctly and can be reopened.
  suggester->commit();
  delete suggester;

  shared_ptr<AnalyzingInfixSuggester> suggester2 =
      make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a, a, 3,
                                           false);
  results = suggester2->lookup(
      TestUtil::stringToCharSequence(L"python", random()), 10, true, false);
  TestUtil::assertEquals(1, results.size());
  TestUtil::assertEquals(L"python", results[0]->key);

  delete suggester2;
  delete a;
}

AnalyzingInfixSuggesterTest::IndexDocument::IndexDocument(
    shared_ptr<AnalyzingInfixSuggester> suggester, const wstring &key)
{
  this->suggester = suggester;
  this->key = key;
}

void AnalyzingInfixSuggesterTest::IndexDocument::run()
{
  try {
    suggester->add(make_shared<BytesRef>(key), nullptr, 10, nullptr);
  } catch (const IOException &e) {
    fail(L"Could not build suggest dictionary correctly");
  }
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
AnalyzingInfixSuggesterTest::asSet(deque<wstring> &values)
{
  unordered_set<std::shared_ptr<BytesRef>> result =
      unordered_set<std::shared_ptr<BytesRef>>();
  for (wstring value : values) {
    result.insert(make_shared<BytesRef>(value));
  }

  return result;
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
AnalyzingInfixSuggesterTest::asSet(deque<char> &values)
{
  unordered_set<std::shared_ptr<BytesRef>> result =
      unordered_set<std::shared_ptr<BytesRef>>();
  for (char[] value : values) {
    result.insert(make_shared<BytesRef>(value));
  }

  return result;
}

void AnalyzingInfixSuggesterTest::testBasicContext() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"lend me your ear", 8,
                         make_shared<BytesRef>(L"foobar"),
                         asSet({L"foo", L"bar"})),
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"),
                         asSet({L"foo", L"baz"}))};

  shared_ptr<Path> tempDir = createTempDir(L"analyzingInfixContext");

  for (int iter = 0; iter < 2; iter++) {
    shared_ptr<AnalyzingInfixSuggester> suggester;
    shared_ptr<Analyzer> a =
        make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
    if (iter == 0) {
      suggester = make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir),
                                                       a, a, 3, false);
      suggester->build(make_shared<InputArrayIterator>(keys));
    } else {
      // Test again, after close/reopen:
      suggester = make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir),
                                                       a, a, 3, false);
    }

    // No context provided, all results returned
    deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
        TestUtil::stringToCharSequence(L"ear", random()), 10, true, true);
    TestUtil::assertEquals(2, results.size());
    shared_ptr<LookupResult> result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

    result = results[1];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

    // Both have "foo" context:
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          asSet({L"foo"}), 10, true, true);
    TestUtil::assertEquals(2, results.size());

    result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

    result = results[1];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

    // Only one has "bar" context:
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          asSet({L"bar"}), 10, true, true);
    TestUtil::assertEquals(1, results.size());

    result = results[0];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

    // None do not have "foo" context:
    unordered_map<std::shared_ptr<BytesRef>, BooleanClause::Occur> contextInfo =
        unordered_map<std::shared_ptr<BytesRef>, BooleanClause::Occur>();
    contextInfo.emplace(make_shared<BytesRef>(L"foo"),
                        BooleanClause::Occur::MUST_NOT);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          contextInfo, 10, true, true);
    TestUtil::assertEquals(0, results.size());

    // Only one does not have "bar" context:
    contextInfo.clear();
    contextInfo.emplace(make_shared<BytesRef>(L"bar"),
                        BooleanClause::Occur::MUST_NOT);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          contextInfo, 10, true, true);
    TestUtil::assertEquals(1, results.size());

    result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

    // Both have "foo" or "bar" context:
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          asSet({L"foo", L"bar"}), 10, true, true);
    TestUtil::assertEquals(2, results.size());

    result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

    result = results[1];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

    // Both have "bar" or "baz" context:
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          asSet({L"bar", L"baz"}), 10, true, true);
    TestUtil::assertEquals(2, results.size());

    result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

    result = results[1];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

    // Only one has "foo" and "bar" context:
    contextInfo.clear();
    contextInfo.emplace(make_shared<BytesRef>(L"foo"),
                        BooleanClause::Occur::MUST);
    contextInfo.emplace(make_shared<BytesRef>(L"bar"),
                        BooleanClause::Occur::MUST);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          contextInfo, 10, true, true);
    TestUtil::assertEquals(1, results.size());

    result = results[0];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

    // None have "bar" and "baz" context:
    contextInfo.clear();
    contextInfo.emplace(make_shared<BytesRef>(L"bar"),
                        BooleanClause::Occur::MUST);
    contextInfo.emplace(make_shared<BytesRef>(L"baz"),
                        BooleanClause::Occur::MUST);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          contextInfo, 10, true, true);
    TestUtil::assertEquals(0, results.size());

    // None do not have "foo" and do not have "bar" context:
    contextInfo.clear();
    contextInfo.emplace(make_shared<BytesRef>(L"foo"),
                        BooleanClause::Occur::MUST_NOT);
    contextInfo.emplace(make_shared<BytesRef>(L"bar"),
                        BooleanClause::Occur::MUST_NOT);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          contextInfo, 10, true, true);
    TestUtil::assertEquals(0, results.size());

    // Both do not have "bar" and do not have "baz" context:
    contextInfo.clear();
    contextInfo.emplace(make_shared<BytesRef>(L"bar"),
                        BooleanClause::Occur::MUST_NOT);
    contextInfo.emplace(make_shared<BytesRef>(L"baz"),
                        BooleanClause::Occur::MUST_NOT);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          asSet({L"bar", L"baz"}), 10, true, true);
    TestUtil::assertEquals(2, results.size());

    result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

    result = results[1];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

    // Only one has "foo" and does not have "bar" context:
    contextInfo.clear();
    contextInfo.emplace(make_shared<BytesRef>(L"foo"),
                        BooleanClause::Occur::MUST);
    contextInfo.emplace(make_shared<BytesRef>(L"bar"),
                        BooleanClause::Occur::MUST_NOT);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          contextInfo, 10, true, true);
    TestUtil::assertEquals(1, results.size());

    result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

    // LUCENE-6464 Using the advanced context filtering by query.
    // Note that this is just a sanity test as all the above tests run through
    // the filter by query method
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    suggester->addContextToQuery(query, make_shared<BytesRef>(L"foo"),
                                 BooleanClause::Occur::MUST);
    suggester->addContextToQuery(query, make_shared<BytesRef>(L"bar"),
                                 BooleanClause::Occur::MUST_NOT);
    results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          query->build(), 10, true, true);
    TestUtil::assertEquals(1, results.size());

    delete suggester;
    delete a;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAddPrefixMatch() throws
// java.io.IOException
void AnalyzingInfixSuggesterTest::testAddPrefixMatch() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(dir, a);

  TestUtil::assertEquals(L"<b>Sol</b>r",
                         pfmToString(suggester, L"Solr", L"Sol"));
  TestUtil::assertEquals(L"<b>Solr</b>",
                         pfmToString(suggester, L"Solr", L"Solr"));

  // Test SOLR-6085 - the analyzed tokens match due to ss->ß normalization
  TestUtil::assertEquals(L"<b>daß</b>",
                         pfmToString(suggester, L"daß", L"dass"));

  delete dir;
  delete suggester;
  delete a;
}

wstring AnalyzingInfixSuggesterTest::pfmToString(
    shared_ptr<AnalyzingInfixSuggester> suggester, const wstring &surface,
    const wstring &prefix) 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  suggester->addPrefixMatch(sb, surface, L"", prefix);
  return sb->toString();
}

void AnalyzingInfixSuggesterTest::testBinaryContext() 
{
  std::deque<char> context1(4);
  std::deque<char> context2(5);
  std::deque<char> context3(1);
  context3[0] = static_cast<char>(0xff);

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"lend me your ear", 8,
                         make_shared<BytesRef>(L"foobar"),
                         asSet({context1, context2})),
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"),
                         asSet({context1, context3}))};

  shared_ptr<Path> tempDir = createTempDir(L"analyzingInfixContext");

  for (int iter = 0; iter < 2; iter++) {
    shared_ptr<AnalyzingInfixSuggester> suggester;
    shared_ptr<Analyzer> a =
        make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
    if (iter == 0) {
      suggester = make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir),
                                                       a, a, 3, false);
      suggester->build(make_shared<InputArrayIterator>(keys));
    } else {
      // Test again, after close/reopen:
      suggester = make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir),
                                                       a, a, 3, false);
    }

    // Both have context1:
    deque<std::shared_ptr<LookupResult>> results =
        suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                          asSet(context1), 10, true, true);
    TestUtil::assertEquals(2, results.size());

    shared_ptr<LookupResult> result = results[0];
    TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
    TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                           result->highlightKey);
    TestUtil::assertEquals(10, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(context1)));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(context3)));

    result = results[1];
    TestUtil::assertEquals(L"lend me your ear", result->key);
    TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
    TestUtil::assertEquals(8, result->value);
    TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
    assertNotNull(result->contexts);
    TestUtil::assertEquals(2, result->contexts->size());
    assertTrue(result->contexts->contains(make_shared<BytesRef>(context1)));
    assertTrue(result->contexts->contains(make_shared<BytesRef>(context2)));

    delete suggester;
    delete a;
  }
}

void AnalyzingInfixSuggesterTest::testContextNotAllTermsRequired() throw(
    runtime_error)
{

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"lend me your ear", 8,
                         make_shared<BytesRef>(L"foobar"),
                         asSet({L"foo", L"bar"})),
      make_shared<Input>(L"a penny saved is a penny earned", 10,
                         make_shared<BytesRef>(L"foobaz"),
                         asSet({L"foo", L"baz"}))};
  shared_ptr<Path> tempDir = createTempDir(L"analyzingInfixContext");

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newFSDirectory(tempDir), a, a, 3,
                                           false);
  suggester->build(make_shared<InputArrayIterator>(keys));

  // No context provided, all results returned
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ear", random()), 10, false, true);
  TestUtil::assertEquals(2, results.size());
  shared_ptr<LookupResult> result = results[0];
  TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
  TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                         result->highlightKey);
  TestUtil::assertEquals(10, result->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
  assertNotNull(result->contexts);
  TestUtil::assertEquals(2, result->contexts->size());
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

  result = results[1];
  TestUtil::assertEquals(L"lend me your ear", result->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
  TestUtil::assertEquals(8, result->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
  assertNotNull(result->contexts);
  TestUtil::assertEquals(2, result->contexts->size());
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

  // Both have "foo" context:
  results = suggester->lookup(TestUtil::stringToCharSequence(L"ear", random()),
                              asSet({L"foo"}), 10, false, true);
  TestUtil::assertEquals(2, results.size());

  result = results[0];
  TestUtil::assertEquals(L"a penny saved is a penny earned", result->key);
  TestUtil::assertEquals(L"a penny saved is a penny <b>ear</b>ned",
                         result->highlightKey);
  TestUtil::assertEquals(10, result->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobaz"), result->payload);
  assertNotNull(result->contexts);
  TestUtil::assertEquals(2, result->contexts->size());
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"baz")));

  result = results[1];
  TestUtil::assertEquals(L"lend me your ear", result->key);
  TestUtil::assertEquals(L"lend me your <b>ear</b>", result->highlightKey);
  TestUtil::assertEquals(8, result->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
  assertNotNull(result->contexts);
  TestUtil::assertEquals(2, result->contexts->size());
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

  // Only one has "foo" context and len
  results = suggester->lookup(TestUtil::stringToCharSequence(L"len", random()),
                              asSet({L"foo"}), 10, false, true);
  TestUtil::assertEquals(1, results.size());

  result = results[0];
  TestUtil::assertEquals(L"lend me your ear", result->key);
  TestUtil::assertEquals(L"<b>len</b>d me your ear", result->highlightKey);
  TestUtil::assertEquals(8, result->value);
  TestUtil::assertEquals(make_shared<BytesRef>(L"foobar"), result->payload);
  assertNotNull(result->contexts);
  TestUtil::assertEquals(2, result->contexts->size());
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"foo")));
  assertTrue(result->contexts->contains(make_shared<BytesRef>(L"bar")));

  delete suggester;
}

void AnalyzingInfixSuggesterTest::testCloseIndexWriterOnBuild() throw(
    runtime_error)
{
  // C++ TODO: Local classes are not converted by Java to C++ Converter:
  //      class MyAnalyzingInfixSuggester extends AnalyzingInfixSuggester
  //    {
  //      public MyAnalyzingInfixSuggester(Directory dir, Analyzer
  //      indexAnalyzer, Analyzer queryAnalyzer, int minPrefixChars, bool
  //      commitOnBuild, bool allTermsRequired, bool highlight, bool
  //      closeIndexWriterOnBuild) throws IOException
  //      {
  //        super(dir, indexAnalyzer, queryAnalyzer, minPrefixChars,
  //        commitOnBuild, allTermsRequired, highlight,
  //        closeIndexWriterOnBuild);
  //      }
  //      public IndexWriter getIndexWriter()
  //      {
  //        return writer;
  //      }
  //      public SearcherManager getSearcherManager()
  //      {
  //        return searcherMgr;
  //      }
  //    }

  // After build(), when closeIndexWriterOnBuild = true:
  // * The IndexWriter should be null
  // * The SearcherManager should be non-null
  // * SearcherManager's IndexWriter reference should be closed
  //   (as evidenced by maybeRefreshBlocking() throwing AlreadyClosedException)
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<Path> tempDir = createTempDir(L"analyzingInfixContext");
  shared_ptr<MyAnalyzingInfixSuggester> *const suggester =
      make_shared<MyAnalyzingInfixSuggester>(
          newFSDirectory(tempDir), a, a, 3, false,
          AnalyzingInfixSuggester::DEFAULT_ALL_TERMS_REQUIRED,
          AnalyzingInfixSuggester::DEFAULT_HIGHLIGHT, true);
  suggester->build(make_shared<InputArrayIterator>(sharedInputs));
  assertNull(suggester->getIndexWriter());
  assertNotNull(suggester->getSearcherManager());
  expectThrows(AlreadyClosedException::typeid, [&]() {
    suggester->getSearcherManager().maybeRefreshBlocking();
  });

  suggester->close();

  // After instantiating from an already-built suggester dir:
  // * The IndexWriter should be null
  // * The SearcherManager should be non-null
  shared_ptr<MyAnalyzingInfixSuggester> *const suggester2 =
      make_shared<MyAnalyzingInfixSuggester>(
          newFSDirectory(tempDir), a, a, 3, false,
          AnalyzingInfixSuggester::DEFAULT_ALL_TERMS_REQUIRED,
          AnalyzingInfixSuggester::DEFAULT_HIGHLIGHT, true);
  assertNull(suggester2->getIndexWriter());
  assertNotNull(suggester2->getSearcherManager());

  suggester2->close();
  delete a;
}

void AnalyzingInfixSuggesterTest::testCommitAfterBuild() 
{
  performOperationWithAllOptionCombinations([&](any suggester) {
    suggester::build(make_shared<InputArrayIterator>(sharedInputs));
    suggester::commit();
  });
}

void AnalyzingInfixSuggesterTest::testRefreshAfterBuild() 
{
  performOperationWithAllOptionCombinations([&](any suggester) {
    suggester::build(make_shared<InputArrayIterator>(sharedInputs));
    suggester::refresh();
  });
}

void AnalyzingInfixSuggesterTest::testDisallowCommitBeforeBuild() throw(
    runtime_error)
{
  performOperationWithAllOptionCombinations([&](any suggester) {
    expectThrows(IllegalStateException::typeid, suggester::commit);
  });
}

void AnalyzingInfixSuggesterTest::testDisallowRefreshBeforeBuild() throw(
    runtime_error)
{
  performOperationWithAllOptionCombinations([&](any suggester) {
    expectThrows(IllegalStateException::typeid, suggester::refresh);
  });
}

void AnalyzingInfixSuggesterTest::performOperationWithAllOptionCombinations(
    shared_ptr<SuggesterOperation> operation) 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);

  shared_ptr<AnalyzingInfixSuggester> suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a);
  operation->operate(suggester);
  delete suggester;

  suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, false);
  operation->operate(suggester);
  delete suggester;

  suggester =
      make_shared<AnalyzingInfixSuggester>(newDirectory(), a, a, 3, true);
  operation->operate(suggester);
  delete suggester;

  suggester = make_shared<AnalyzingInfixSuggester>(
      newDirectory(), a, a, 3, true,
      AnalyzingInfixSuggester::DEFAULT_ALL_TERMS_REQUIRED,
      AnalyzingInfixSuggester::DEFAULT_HIGHLIGHT, true);
  operation->operate(suggester);
  delete suggester;

  suggester = make_shared<AnalyzingInfixSuggester>(
      newDirectory(), a, a, 3, true,
      AnalyzingInfixSuggester::DEFAULT_ALL_TERMS_REQUIRED,
      AnalyzingInfixSuggester::DEFAULT_HIGHLIGHT, false);
  operation->operate(suggester);
  delete suggester;

  suggester = make_shared<AnalyzingInfixSuggester>(
      newDirectory(), a, a, 3, false,
      AnalyzingInfixSuggester::DEFAULT_ALL_TERMS_REQUIRED,
      AnalyzingInfixSuggester::DEFAULT_HIGHLIGHT, true);
  operation->operate(suggester);
  delete suggester;

  suggester = make_shared<AnalyzingInfixSuggester>(
      newDirectory(), a, a, 3, false,
      AnalyzingInfixSuggester::DEFAULT_ALL_TERMS_REQUIRED,
      AnalyzingInfixSuggester::DEFAULT_HIGHLIGHT, false);
  operation->operate(suggester);
  delete suggester;

  delete a;
}
} // namespace org::apache::lucene::search::suggest::analyzing