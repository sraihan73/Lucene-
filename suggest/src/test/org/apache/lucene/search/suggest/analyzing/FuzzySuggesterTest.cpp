using namespace std;

#include "FuzzySuggesterTest.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Input = org::apache::lucene::search::suggest::Input;
using InputArrayIterator =
    org::apache::lucene::search::suggest::InputArrayIterator;
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using FiniteStringsIterator =
    org::apache::lucene::util::automaton::FiniteStringsIterator;
using Util = org::apache::lucene::util::fst::Util;

void FuzzySuggesterTest::testRandomEdits() 
{
  deque<std::shared_ptr<Input>> keys = deque<std::shared_ptr<Input>>();
  int numTerms = atLeast(100);
  for (int i = 0; i < numTerms; i++) {
    keys.push_back(
        make_shared<Input>(L"boo" + TestUtil::randomSimpleString(random()),
                           1 + random()->nextInt(100)));
  }
  keys.push_back(make_shared<Input>(L"foo bar boo far", 12));
  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", analyzer, analyzer,
      FuzzySuggester::EXACT_FIRST | FuzzySuggester::PRESERVE_SEP, 256, -1, true,
      FuzzySuggester::DEFAULT_MAX_EDITS, FuzzySuggester::DEFAULT_TRANSPOSITIONS,
      0, FuzzySuggester::DEFAULT_MIN_FUZZY_LENGTH,
      FuzzySuggester::DEFAULT_UNICODE_AWARE);
  suggester->build(make_shared<InputArrayIterator>(keys));
  int numIters = atLeast(10);
  for (int i = 0; i < numIters; i++) {
    wstring addRandomEdit = FuzzySuggesterTest::addRandomEdit(
        L"foo bar boo", FuzzySuggester::DEFAULT_NON_FUZZY_PREFIX);
    deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
        TestUtil::stringToCharSequence(addRandomEdit, random()), false, 2);
    assertEquals(addRandomEdit, 1, results.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"foo bar boo far", results[0]->key->toString());
    assertEquals(12, results[0]->value, 0.01F);
  }
  IOUtils::close({analyzer, tempDir});
}

void FuzzySuggesterTest::testNonLatinRandomEdits() 
{
  deque<std::shared_ptr<Input>> keys = deque<std::shared_ptr<Input>>();
  int numTerms = atLeast(100);
  for (int i = 0; i < numTerms; i++) {
    keys.push_back(
        make_shared<Input>(L"буу" + TestUtil::randomSimpleString(random()),
                           1 + random()->nextInt(100)));
  }
  keys.push_back(make_shared<Input>(L"фуу бар буу фар", 12));
  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", analyzer, analyzer,
      FuzzySuggester::EXACT_FIRST | FuzzySuggester::PRESERVE_SEP, 256, -1, true,
      FuzzySuggester::DEFAULT_MAX_EDITS, FuzzySuggester::DEFAULT_TRANSPOSITIONS,
      0, FuzzySuggester::DEFAULT_MIN_FUZZY_LENGTH, true);
  suggester->build(make_shared<InputArrayIterator>(keys));
  int numIters = atLeast(10);
  for (int i = 0; i < numIters; i++) {
    wstring addRandomEdit =
        FuzzySuggesterTest::addRandomEdit(L"фуу бар буу", 0);
    deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
        TestUtil::stringToCharSequence(addRandomEdit, random()), false, 2);
    assertEquals(addRandomEdit, 1, results.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"фуу бар буу фар", results[0]->key->toString());
    assertEquals(12, results[0]->value, 0.01F);
  }
  IOUtils::close({analyzer, tempDir});
}

void FuzzySuggesterTest::testKeyword() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"foo", 50), make_shared<Input>(L"bar", 10),
      make_shared<Input>(L"barbar", 12), make_shared<Input>(L"barbara", 6)};

  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester =
      make_shared<FuzzySuggester>(tempDir, L"fuzzy", analyzer);
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"bariar", random()), false, 2);
  TestUtil::assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);

  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"barbr", random()), false, 2);
  TestUtil::assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);

  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"barbara", random()), false, 2);
  TestUtil::assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbara", results[0]->key->toString());
  assertEquals(6, results[0]->value, 0.01F);

  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"barbar", random()), false, 2);
  TestUtil::assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbara", results[1]->key->toString());
  assertEquals(6, results[1]->value, 0.01F);

  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"barbaa", random()), false, 2);
  TestUtil::assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbara", results[1]->key->toString());
  assertEquals(6, results[1]->value, 0.01F);

  // top N of 2, but only foo is available
  results = suggester->lookup(TestUtil::stringToCharSequence(L"f", random()),
                              false, 2);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo", results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // top N of 1 for 'bar': we return this even though
  // barbar is higher because exactFirst is enabled:
  results = suggester->lookup(TestUtil::stringToCharSequence(L"bar", random()),
                              false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bar", results[0]->key->toString());
  assertEquals(10, results[0]->value, 0.01F);

  // top N Of 2 for 'b'
  results = suggester->lookup(TestUtil::stringToCharSequence(L"b", random()),
                              false, 2);
  TestUtil::assertEquals(2, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bar", results[1]->key->toString());
  assertEquals(10, results[1]->value, 0.01F);

  // top N of 3 for 'ba'
  results = suggester->lookup(TestUtil::stringToCharSequence(L"ba", random()),
                              false, 3);
  TestUtil::assertEquals(3, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbar", results[0]->key->toString());
  assertEquals(12, results[0]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bar", results[1]->key->toString());
  assertEquals(10, results[1]->value, 0.01F);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"barbara", results[2]->key->toString());
  assertEquals(6, results[2]->value, 0.01F);

  IOUtils::close({analyzer, tempDir});
}

void FuzzySuggesterTest::testStandard() 
{
  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"the ghost of christmas past", 50)};

  shared_ptr<Analyzer> standard =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", standard, standard,
      AnalyzingSuggester::EXACT_FIRST | AnalyzingSuggester::PRESERVE_SEP, 256,
      -1, false, FuzzySuggester::DEFAULT_MAX_EDITS,
      FuzzySuggester::DEFAULT_TRANSPOSITIONS,
      FuzzySuggester::DEFAULT_NON_FUZZY_PREFIX,
      FuzzySuggester::DEFAULT_MIN_FUZZY_LENGTH,
      FuzzySuggester::DEFAULT_UNICODE_AWARE);
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"the ghost of chris", random()), false,
      1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"the ghost of christmas past",
                         results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // omit the 'the' since it's a stopword, it's suggested anyway
  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ghost of chris", random()), false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"the ghost of christmas past",
                         results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // omit the 'the' and 'of' since they are stopwords, it's suggested anyway
  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ghost chris", random()), false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"the ghost of christmas past",
                         results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  IOUtils::close({standard, tempDir});
}

void FuzzySuggesterTest::testNoSeps() 
{
  std::deque<std::shared_ptr<Input>> keys = {make_shared<Input>(L"ab cd", 0),
                                              make_shared<Input>(L"abcd", 1)};

  int options = 0;

  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", a, a, options, 256, -1, true, 1, true, 1, 3, false);
  suggester->build(make_shared<InputArrayIterator>(keys));
  // TODO: would be nice if "ab " would allow the test to
  // pass, and more generally if the analyzer can know
  // that the user's current query has ended at a word,
  // but, analyzers don't produce SEP tokens!
  deque<std::shared_ptr<LookupResult>> r = suggester->lookup(
      TestUtil::stringToCharSequence(L"ab c", random()), false, 2);
  TestUtil::assertEquals(2, r.size());

  // With no PRESERVE_SEPS specified, "ab c" should also
  // complete to "abcd", which has higher weight so should
  // appear first:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"abcd", r[0]->key->toString());
  IOUtils::close({a, tempDir});
}

void FuzzySuggesterTest::testGraphDups() 
{

  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"wifi network is slow", 50),
      make_shared<Input>(L"wi fi network is fast", 10)};
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester =
      make_shared<FuzzySuggester>(tempDir, L"fuzzy", analyzer);
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"wifi network", false, 10);
  if (VERBOSE) {
    wcout << L"Results: " << results << endl;
  }
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"wifi network is slow", results[0]->key);
  TestUtil::assertEquals(50, results[0]->value);
  TestUtil::assertEquals(L"wi fi network is fast", results[1]->key);
  TestUtil::assertEquals(10, results[1]->value);
  IOUtils::close({tempDir, analyzer});
}

FuzzySuggesterTest::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<FuzzySuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
FuzzySuggesterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass>(
      shared_from_this(), tokenizer);
}

FuzzySuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::
        TokenStreamComponentsAnonymousInnerClass(
            shared_ptr<AnalyzerAnonymousInnerClass> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
  tokenStreamCounter = 0;
  tokenStreams = std::deque<std::shared_ptr<TokenStream>>{
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"wifi", 1, 1), token(L"hotspot", 0, 2),
          token(L"network", 1, 1), token(L"is", 1, 1), token(L"slow", 1, 1)}),
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"wi", 1, 1), token(L"hotspot", 0, 3), token(L"fi", 1, 1),
          token(L"network", 1, 1), token(L"is", 1, 1), token(L"fast", 1, 1)}),
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"wifi", 1, 1), token(L"hotspot", 0, 2),
          token(L"network", 1, 1)})};
}

shared_ptr<TokenStream> FuzzySuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::getTokenStream()
{
  shared_ptr<TokenStream> result = tokenStreams[tokenStreamCounter];
  tokenStreamCounter++;
  return result;
}

void FuzzySuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::setReader(
        shared_ptr<Reader> reader)
{
}

void FuzzySuggesterTest::testEmpty() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester =
      make_shared<FuzzySuggester>(tempDir, L"fuzzy", analyzer);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>(0)));

  deque<std::shared_ptr<LookupResult>> result =
      suggester->lookup(L"a", false, 20);
  assertTrue(result.empty());
  IOUtils::close({analyzer, tempDir});
}

void FuzzySuggesterTest::testInputPathRequired() 
{

  //  SynonymMap.Builder b = new SynonymMap.Builder(false);
  //  b.add(new CharsRef("ab"), new CharsRef("ba"), true);
  //  final SynonymMap map_obj = b.build();

  //  The Analyzer below mimics the functionality of the SynonymAnalyzer
  //  using the above map_obj, so that the suggest module does not need a dependency
  //  on the synonym module

  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  std::deque<std::shared_ptr<Input>> keys = {make_shared<Input>(L"ab xc", 50),
                                              make_shared<Input>(L"ba xd", 50)};
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester =
      make_shared<FuzzySuggester>(tempDir, L"fuzzy", analyzer);
  suggester->build(make_shared<InputArrayIterator>(keys));
  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"ab x", false, 1);
  assertTrue(results.size() == 1);
  IOUtils::close({analyzer, tempDir});
}

FuzzySuggesterTest::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<FuzzySuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
FuzzySuggesterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass2>(
      shared_from_this(), tokenizer);
}

FuzzySuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::
        TokenStreamComponentsAnonymousInnerClass2(
            shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
  tokenStreamCounter = 0;
  tokenStreams = std::deque<std::shared_ptr<TokenStream>>{
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"ab", 1, 1), token(L"ba", 0, 1), token(L"xc", 1, 1)}),
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"ba", 1, 1), token(L"xd", 1, 1)}),
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"ab", 1, 1), token(L"ba", 0, 1), token(L"x", 1, 1)})};
}

shared_ptr<TokenStream> FuzzySuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::getTokenStream()
{
  shared_ptr<TokenStream> result = tokenStreams[tokenStreamCounter];
  tokenStreamCounter++;
  return result;
}

void FuzzySuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::setReader(
        shared_ptr<Reader> reader)
{
}

shared_ptr<Token> FuzzySuggesterTest::token(const wstring &term, int posInc,
                                            int posLength)
{
  shared_ptr<Token> *const t = make_shared<Token>(term, 0, 0);
  t->setPositionIncrement(posInc);
  t->setPositionLength(posLength);
  return t;
}

shared_ptr<Analyzer> FuzzySuggesterTest::getUnusualAnalyzer()
{
  return make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
}

FuzzySuggesterTest::AnalyzerAnonymousInnerClass3::AnalyzerAnonymousInnerClass3(
    shared_ptr<FuzzySuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
FuzzySuggesterTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass3>(
      shared_from_this(), tokenizer);
}

FuzzySuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::
        TokenStreamComponentsAnonymousInnerClass3(
            shared_ptr<AnalyzerAnonymousInnerClass3> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStream> FuzzySuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::getTokenStream()
{
  // 4th time we are called, return tokens a b,
  // else just a:
  if (count++ != 3) {
    return make_shared<CannedTokenStream>(
        std::deque<std::shared_ptr<Token>>{token(L"a", 1, 1)});
  } else {
    // After that "a b":
    return make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
        token(L"a", 1, 1), token(L"b", 1, 1)});
  }
}

void FuzzySuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::setReader(
        shared_ptr<Reader> reader)
{
}

void FuzzySuggesterTest::testExactFirst() 
{

  shared_ptr<Analyzer> a = getUnusualAnalyzer();
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", a, a,
      AnalyzingSuggester::EXACT_FIRST | AnalyzingSuggester::PRESERVE_SEP, 256,
      -1, true, 1, true, 1, 3, false);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"x y", 1), make_shared<Input>(L"x y z", 3),
          make_shared<Input>(L"x", 2), make_shared<Input>(L"z z z", 20)}));

  // System.out.println("ALL: " + suggester.lookup("x y", false, 6));

  for (int topN = 1; topN < 6; topN++) {
    deque<std::shared_ptr<LookupResult>> results =
        suggester->lookup(L"x y", false, topN);
    // System.out.println("topN=" + topN + " " + results);

    TestUtil::assertEquals(min(topN, 4), results.size());

    TestUtil::assertEquals(L"x y", results[0]->key);
    TestUtil::assertEquals(1, results[0]->value);

    if (topN > 1) {
      TestUtil::assertEquals(L"z z z", results[1]->key);
      TestUtil::assertEquals(20, results[1]->value);

      if (topN > 2) {
        TestUtil::assertEquals(L"x y z", results[2]->key);
        TestUtil::assertEquals(3, results[2]->value);

        if (topN > 3) {
          TestUtil::assertEquals(L"x", results[3]->key);
          TestUtil::assertEquals(2, results[3]->value);
        }
      }
    }
  }
  IOUtils::close({a, tempDir});
}

void FuzzySuggesterTest::testNonExactFirst() 
{

  shared_ptr<Analyzer> a = getUnusualAnalyzer();
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", a, a, AnalyzingSuggester::PRESERVE_SEP, 256, -1, true,
      1, true, 1, 3, false);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"x y", 1), make_shared<Input>(L"x y z", 3),
          make_shared<Input>(L"x", 2), make_shared<Input>(L"z z z", 20)}));

  for (int topN = 1; topN < 6; topN++) {
    deque<std::shared_ptr<LookupResult>> results =
        suggester->lookup(L"p", false, topN);

    TestUtil::assertEquals(min(topN, 4), results.size());

    TestUtil::assertEquals(L"z z z", results[0]->key);
    TestUtil::assertEquals(20, results[0]->value);

    if (topN > 1) {
      TestUtil::assertEquals(L"x y z", results[1]->key);
      TestUtil::assertEquals(3, results[1]->value);

      if (topN > 2) {
        TestUtil::assertEquals(L"x", results[2]->key);
        TestUtil::assertEquals(2, results[2]->value);

        if (topN > 3) {
          TestUtil::assertEquals(L"x y", results[3]->key);
          TestUtil::assertEquals(1, results[3]->value);
        }
      }
    }
  }
  IOUtils::close({a, tempDir});
}

FuzzySuggesterTest::TermFreqPayload2::TermFreqPayload2(
    const wstring &surfaceForm, const wstring &analyzedForm, int64_t weight)
    : surfaceForm(surfaceForm), analyzedForm(analyzedForm), weight(weight)
{
}

int FuzzySuggesterTest::TermFreqPayload2::compareTo(
    shared_ptr<TermFreqPayload2> other)
{
  int cmp = analyzedForm.compare(other->analyzedForm);
  if (cmp != 0) {
    return cmp;
  } else if (weight > other->weight) {
    return -1;
  } else if (weight < other->weight) {
    return 1;
  } else {
    assert(false);
    return 0;
  }
}

bool FuzzySuggesterTest::isStopChar(wchar_t ch, int numStopChars)
{
  // System.out.println("IS? " + ch + ": " + (ch - 'a') + ": " + ((ch - 'a') <
  // numStopChars));
  return (ch - L'a') < numStopChars;
}

FuzzySuggesterTest::TokenEater::TokenEater(bool preserveHoles,
                                           shared_ptr<TokenStream> in_,
                                           int numStopChars)
    : org::apache::lucene::analysis::TokenFilter(in_),
      numStopChars(numStopChars), preserveHoles(preserveHoles)
{
}

void FuzzySuggesterTest::TokenEater::reset() 
{
  TokenFilter::reset();
  first = true;
}

bool FuzzySuggesterTest::TokenEater::incrementToken() 
{
  int skippedPositions = 0;
  while (input->incrementToken()) {
    if (termAtt->length() != 1 ||
        !isStopChar(termAtt->charAt(0), numStopChars)) {
      int posInc = posIncrAtt->getPositionIncrement() + skippedPositions;
      if (first) {
        if (posInc == 0) {
          // first token having posinc=0 is illegal.
          posInc = 1;
        }
        first = false;
      }
      posIncrAtt->setPositionIncrement(posInc);
      // System.out.println("RETURN term=" + termAtt + " numStopChars=" +
      // numStopChars);
      return true;
    }
    if (preserveHoles) {
      skippedPositions += posIncrAtt->getPositionIncrement();
    }
  }

  return false;
}

FuzzySuggesterTest::MockTokenEatingAnalyzer::MockTokenEatingAnalyzer(
    int numStopChars, bool preserveHoles)
{
  this->preserveHoles = preserveHoles;
  this->numStopChars = numStopChars;
}

shared_ptr<Analyzer::TokenStreamComponents>
FuzzySuggesterTest::MockTokenEatingAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false,
                                 MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);
  tokenizer->setEnableChecks(true);
  shared_ptr<TokenStream> next;
  if (numStopChars != 0) {
    next = make_shared<TokenEater>(preserveHoles, tokenizer, numStopChars);
  } else {
    next = tokenizer;
  }
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, next);
}

void FuzzySuggesterTest::testRandom() 
{

  int numQueries = atLeast(100);

  const deque<std::shared_ptr<TermFreqPayload2>> slowCompletor =
      deque<std::shared_ptr<TermFreqPayload2>>();
  const set<wstring> allPrefixes = set<wstring>();
  shared_ptr<Set<wstring>> *const seen = unordered_set<wstring>();

  std::deque<std::shared_ptr<Input>> keys(numQueries);

  bool preserveSep = random()->nextBoolean();
  bool unicodeAware = random()->nextBoolean();

  constexpr int numStopChars = random()->nextInt(10);
  constexpr bool preserveHoles = random()->nextBoolean();

  if (VERBOSE) {
    wcout << L"TEST: " << numQueries << L" words; preserveSep=" << preserveSep
          << L" ; unicodeAware=" << unicodeAware << L" numStopChars="
          << numStopChars << L" preserveHoles=" << preserveHoles << endl;
  }

  for (int i = 0; i < numQueries; i++) {
    int numTokens = TestUtil::nextInt(random(), 1, 4);
    wstring key;
    wstring analyzedKey;
    while (true) {
      key = L"";
      analyzedKey = L"";
      bool lastRemoved = false;
      for (int token = 0; token < numTokens; token++) {
        wstring s;
        while (true) {
          // TODO: would be nice to fix this slowCompletor/comparator to
          // use full range, but we might lose some coverage too...
          s = TestUtil::randomSimpleString(random());
          if (s.length() > 0) {
            if (token > 0) {
              key += L" ";
            }
            if (preserveSep && analyzedKey.length() > 0 &&
                (unicodeAware
                     ? analyzedKey.codePointAt(
                           analyzedKey.codePointCount(0, analyzedKey.length()) -
                           1) != L' '
                     : analyzedKey[analyzedKey.length() - 1] != L' ')) {
              analyzedKey += L" ";
            }
            key += s;
            if (s.length() == 1 && isStopChar(s[0], numStopChars)) {
              if (preserveSep && preserveHoles) {
                analyzedKey += L'\u0000';
              }
              lastRemoved = true;
            } else {
              analyzedKey += s;
              lastRemoved = false;
            }
            break;
          }
        }
      }

      analyzedKey = analyzedKey.replaceAll(L"(^| )\u0000$", L"");

      if (preserveSep && lastRemoved) {
        analyzedKey += L" ";
      }

      // Don't add same surface form more than once:
      if (!seen->contains(key)) {
        seen->add(key);
        break;
      }
    }

    for (int j = 1; j < key.length(); j++) {
      allPrefixes.insert(key.substr(0, j));
    }
    // we can probably do Integer.MAX_VALUE here, but why worry.
    int weight = random()->nextInt(1 << 24);
    keys[i] = make_shared<Input>(key, weight);

    slowCompletor.push_back(
        make_shared<TermFreqPayload2>(key, analyzedKey, weight));
  }

  if (VERBOSE) {
    // Don't just sort original deque, to avoid VERBOSE
    // altering the test:
    deque<std::shared_ptr<TermFreqPayload2>> sorted =
        deque<std::shared_ptr<TermFreqPayload2>>(slowCompletor);
    sort(sorted.begin(), sorted.end());
    for (auto ent : sorted) {
      wcout << L"  surface='" << ent->surfaceForm << L" analyzed='"
            << ent->analyzedForm << L"' weight=" << ent->weight << endl;
    }
  }

  shared_ptr<Analyzer> a =
      make_shared<MockTokenEatingAnalyzer>(numStopChars, preserveHoles);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", a, a,
      preserveSep ? AnalyzingSuggester::PRESERVE_SEP : 0, 256, -1, true, 1,
      false, 1, 3, unicodeAware);
  suggester->build(make_shared<InputArrayIterator>(keys));

  for (auto prefix : allPrefixes) {

    if (VERBOSE) {
      wcout << L"\nTEST: prefix=" << prefix << endl;
    }

    constexpr int topN = TestUtil::nextInt(random(), 1, 10);
    deque<std::shared_ptr<LookupResult>> r = suggester->lookup(
        TestUtil::stringToCharSequence(prefix, random()), false, topN);

    // 2. go thru whole set to find suggestions:
    deque<std::shared_ptr<LookupResult>> matches =
        deque<std::shared_ptr<LookupResult>>();

    // "Analyze" the key:
    std::deque<wstring> tokens = prefix.split(L" ");
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    bool lastRemoved = false;
    for (int i = 0; i < tokens.size(); i++) {
      wstring token = tokens[i];
      if (preserveSep && builder->length() > 0 &&
          !builder->toString()->endsWith(L" ")) {
        builder->append(L' ');
      }

      if (token.length() == 1 && isStopChar(token[0], numStopChars)) {
        if (preserveSep && preserveHoles) {
          builder->append(L"\u0000");
        }
        lastRemoved = true;
      } else {
        builder->append(token);
        lastRemoved = false;
      }
    }

    wstring analyzedKey = builder->toString();

    // Remove trailing sep/holes (TokenStream.end() does
    // not tell us any trailing holes, yet ... there is an
    // issue open for this):
    while (true) {
      wstring s = analyzedKey.replaceAll(L"(^| )\u0000$", L"");
      s = s.replaceAll(L"\\s+$", L"");
      if (s == analyzedKey) {
        break;
      }
      analyzedKey = s;
    }

    if (analyzedKey.length() == 0) {
      // Currently suggester can't suggest from the empty
      // string!  You get no results, not all results...
      continue;
    }

    if (preserveSep && (prefix.endsWith(L" ") || lastRemoved)) {
      analyzedKey += L" ";
    }

    if (VERBOSE) {
      wcout << L"  analyzed: " << analyzedKey << endl;
    }
    shared_ptr<TokenStreamToAutomaton> tokenStreamToAutomaton =
        suggester->getTokenStreamToAutomaton();

    // NOTE: not great that we ask the suggester to give
    // us the "answer key" (ie maybe we have a bug in
    // suggester.toLevA ...) ... but testRandom2() fixes
    // this:
    shared_ptr<Automaton> automaton =
        suggester->convertAutomaton(suggester->toLevenshteinAutomata(
            suggester->toLookupAutomaton(analyzedKey)));
    assertTrue(automaton->isDeterministic());

    // TODO: could be faster... but it's slowCompletor for a reason
    shared_ptr<BytesRefBuilder> spare = make_shared<BytesRefBuilder>();
    for (auto e : slowCompletor) {
      spare->copyChars(e->analyzedForm);
      shared_ptr<FiniteStringsIterator> finiteStrings =
          make_shared<FiniteStringsIterator>(
              suggester->toAutomaton(spare->get(), tokenStreamToAutomaton));
      for (IntsRef string; (string = finiteStrings->next()) != nullptr;) {
        int p = 0;
        shared_ptr<BytesRef> ref = Util::toBytesRef(string, spare);
        bool added = false;
        for (int i = ref->offset; i < ref->length; i++) {
          int q = automaton->step(p, ref->bytes[i] & 0xff);
          if (q == -1) {
            break;
          } else if (automaton->isAccept(q)) {
            matches.push_back(
                make_shared<LookupResult>(e->surfaceForm, e->weight));
            added = true;
            break;
          }
          p = q;
        }
        if (!added && automaton->isAccept(p)) {
          matches.push_back(
              make_shared<LookupResult>(e->surfaceForm, e->weight));
        }
      }
    }

    assertTrue(numStopChars > 0 || matches.size() > 0);

    if (matches.size() > 1) {
      // C++ TODO: The 'Compare' parameter of std::sort produces a bool
      // value, while the Java Comparator parameter produces a tri-state result:
      // ORIGINAL LINE: java.util.Collections.sort(matches, new
      // java.util.Comparator<org.apache.lucene.search.suggest.Lookup.LookupResult>()
      sort(matches.begin(), matches.end(),
           make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));
    }

    if (matches.size() > topN) {
      matches = matches.subList(0, topN);
    }

    if (VERBOSE) {
      wcout << L"  expected:" << endl;
      for (auto lr : matches) {
        wcout << L"    key=" << lr->key << L" weight=" << lr->value << endl;
      }

      wcout << L"  actual:" << endl;
      for (auto lr : r) {
        wcout << L"    key=" << lr->key << L" weight=" << lr->value << endl;
      }
    }

    assertEquals(prefix + L"  " + to_wstring(topN), matches.size(), r.size());
    for (int hit = 0; hit < r.size(); hit++) {
      // System.out.println("  check hit " + hit);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(prefix + L"  " + to_wstring(topN),
                   matches[hit]->key->toString(), r[hit]->key->toString());
      assertEquals(matches[hit]->value, r[hit]->value, 0.0f);
    }
  }
  IOUtils::close({a, tempDir});
}

FuzzySuggesterTest::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(shared_ptr<FuzzySuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

int FuzzySuggesterTest::ComparatorAnonymousInnerClass::compare(
    shared_ptr<LookupResult> left, shared_ptr<LookupResult> right)
{
  int cmp = Float::compare(right->value, left->value);
  if (cmp == 0) {
    return left->compareTo(right);
  } else {
    return cmp;
  }
}

void FuzzySuggesterTest::testMaxSurfaceFormsPerAnalyzedForm() throw(
    runtime_error)
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", a, a, 0, 2, -1, true, 1, true, 1, 3, false);

  deque<std::shared_ptr<Input>> keys =
      Arrays::asList(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"a", 40), make_shared<Input>(L"a ", 50),
          make_shared<Input>(L" a", 60)});

  Collections::shuffle(keys, random());
  suggester->build(make_shared<InputArrayIterator>(keys));

  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"a", false, 5);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L" a", results[0]->key);
  TestUtil::assertEquals(60, results[0]->value);
  TestUtil::assertEquals(L"a ", results[1]->key);
  TestUtil::assertEquals(50, results[1]->value);
  IOUtils::close({a, tempDir});
}

void FuzzySuggesterTest::testEditSeps() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggester = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", a, a, FuzzySuggester::PRESERVE_SEP, 2, -1, true, 2,
      true, 1, 3, false);

  deque<std::shared_ptr<Input>> keys =
      Arrays::asList(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"foo bar", 40),
          make_shared<Input>(L"foo bar baz", 50),
          make_shared<Input>(L"barbaz", 60),
          make_shared<Input>(L"barbazfoo", 10)});

  Collections::shuffle(keys, random());
  suggester->build(make_shared<InputArrayIterator>(keys));

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[foo bar baz/50, foo bar/40]",
                         suggester->lookup(L"foobar", false, 5).toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[foo bar baz/50]",
                         suggester->lookup(L"foobarbaz", false, 5).toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[barbaz/60, barbazfoo/10]",
                         suggester->lookup(L"bar baz", false, 5).toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"[barbazfoo/10]",
      suggester->lookup(L"bar baz foo", false, 5).toString());
  IOUtils::close({a, tempDir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") private static std::wstring
// addRandomEdit(std::wstring string, int prefixLength)
wstring FuzzySuggesterTest::addRandomEdit(const wstring &string,
                                          int prefixLength)
{
  std::deque<wchar_t> input = string.toCharArray();
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (int i = 0; i < input.size(); i++) {
    if (i >= prefixLength && random()->nextBoolean() && i < input.size() - 1) {
      switch (random()->nextInt(4)) {
      case 3:
        if (i < input.size() - 1) {
          // Transpose input[i] and input[1+i]:
          builder->append(input[i + 1]);
          builder->append(input[i]);
          for (int j = i + 2; j < input.size(); j++) {
            builder->append(input[j]);
          }
          return builder->toString();
        }
        // NOTE: fall through to delete:
      case 2:
        // Delete input[i]
        for (int j = i + 1; j < input.size(); j++) {
          builder->append(input[j]);
        }
        return builder->toString();
      case 1:
        // Insert input[i+1] twice
        if (i + 1 < input.size()) {
          builder->append(input[i + 1]);
          builder->append(input[i++]);
          i++;
        }
        for (int j = i; j < input.size(); j++) {
          builder->append(input[j]);
        }
        return builder->toString();
      case 0: {
        // Insert random byte.
        // NOTE: can only use ascii here so that, in
        // UTF8 byte space it's still a single
        // insertion:
        // bytes 0x1e and 0x1f are reserved
        int x = random()->nextBoolean() ? random()->nextInt(30)
                                        : 32 + random()->nextInt(128 - 32);
        builder->append(static_cast<wchar_t>(x));
        for (int j = i; j < input.size(); j++) {
          builder->append(input[j]);
        }
        return builder->toString();
      }
      }
    }

    builder->append(input[i]);
  }

  return builder->toString();
}

wstring FuzzySuggesterTest::randomSimpleString(int maxLen)
{
  constexpr int len = TestUtil::nextInt(random(), 1, maxLen);
  const std::deque<wchar_t> chars = std::deque<wchar_t>(len);
  for (int j = 0; j < len; j++) {
    chars[j] = static_cast<wchar_t>(L'a' + random()->nextInt(4));
  }
  return wstring(chars);
}

void FuzzySuggesterTest::testRandom2() 
{
  constexpr int NUM = atLeast(200);
  const deque<std::shared_ptr<Input>> answers =
      deque<std::shared_ptr<Input>>();
  shared_ptr<Set<wstring>> *const seen = unordered_set<wstring>();
  for (int i = 0; i < NUM; i++) {
    const wstring s = randomSimpleString(8);
    if (!seen->contains(s)) {
      answers.push_back(make_shared<Input>(s, random()->nextInt(1000)));
      seen->add(s);
    }
  }

  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(answers, new
  // java.util.Comparator<org.apache.lucene.search.suggest.Input>()
  sort(answers.begin(), answers.end(),
       make_shared<ComparatorAnonymousInnerClass2>(shared_from_this()));
  if (VERBOSE) {
    wcout << L"\nTEST: targets" << endl;
    for (auto tf : answers) {
      wcout << L"  " << tf->term->utf8ToString() << L" freq=" << tf->v << endl;
    }
  }

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  int maxEdits = random()->nextBoolean() ? 1 : 2;
  int prefixLen = random()->nextInt(4);
  bool transpositions = random()->nextBoolean();
  // TODO: test graph analyzers
  // TODO: test exactFirst / preserveSep permutations
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<FuzzySuggester> suggest = make_shared<FuzzySuggester>(
      tempDir, L"fuzzy", a, a, 0, 256, -1, true, maxEdits, transpositions,
      prefixLen, prefixLen, false);

  if (VERBOSE) {
    wcout << L"TEST: maxEdits=" << maxEdits << L" prefixLen=" << prefixLen
          << L" transpositions=" << transpositions << L" num=" << NUM << endl;
  }

  Collections::shuffle(answers, random());
  suggest->build(make_shared<InputArrayIterator>(
      answers.toArray(std::deque<std::shared_ptr<Input>>(answers.size()))));

  constexpr int ITERS = atLeast(100);
  for (int iter = 0; iter < ITERS; iter++) {
    const wstring frag = randomSimpleString(6);
    if (VERBOSE) {
      wcout << L"\nTEST: iter frag=" << frag << endl;
    }
    const deque<std::shared_ptr<LookupResult>> expected =
        slowFuzzyMatch(prefixLen, maxEdits, transpositions, answers, frag);
    if (VERBOSE) {
      wcout << L"  expected: " << expected.size() << endl;
      for (auto c : expected) {
        wcout << L"    " << c << endl;
      }
    }
    const deque<std::shared_ptr<LookupResult>> actual =
        suggest->lookup(frag, false, NUM);
    if (VERBOSE) {
      wcout << L"  actual: " << actual.size() << endl;
      for (auto c : actual) {
        wcout << L"    " << c << endl;
      }
    }

    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(actual, new CompareByCostThenAlpha());
    sort(actual.begin(), actual.end(), make_shared<CompareByCostThenAlpha>());

    constexpr int limit = min(expected.size(), actual.size());
    for (int ans = 0; ans < limit; ans++) {
      shared_ptr<LookupResult> *const c0 = expected[ans];
      shared_ptr<LookupResult> *const c1 = actual[ans];
      assertEquals(L"expected " + c0->key + L" but got " + c1->key, 0,
                   CHARSEQUENCE_COMPARATOR->compare(c0->key, c1->key));
      TestUtil::assertEquals(c0->value, c1->value);
    }
    TestUtil::assertEquals(expected.size(), actual.size());
  }
  IOUtils::close({a, tempDir});
}

FuzzySuggesterTest::ComparatorAnonymousInnerClass2::
    ComparatorAnonymousInnerClass2(shared_ptr<FuzzySuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

int FuzzySuggesterTest::ComparatorAnonymousInnerClass2::compare(
    shared_ptr<Input> a, shared_ptr<Input> b)
{
  return a->term->compareTo(b->term);
}

deque<std::shared_ptr<LookupResult>> FuzzySuggesterTest::slowFuzzyMatch(
    int prefixLen, int maxEdits, bool allowTransposition,
    deque<std::shared_ptr<Input>> &answers, const wstring &frag)
{
  const deque<std::shared_ptr<LookupResult>> results =
      deque<std::shared_ptr<LookupResult>>();
  constexpr int fragLen = frag.length();
  for (auto tf : answers) {
    // System.out.println("  check s=" + tf.term.utf8ToString());
    bool prefixMatches = true;
    for (int i = 0; i < prefixLen; i++) {
      if (i == fragLen) {
        // Prefix still matches:
        break;
      }
      if (i == tf->term->length ||
          tf->term->bytes[i] != static_cast<char>(frag[i])) {
        prefixMatches = false;
        break;
      }
    }
    // System.out.println("    prefixMatches=" + prefixMatches);

    if (prefixMatches) {
      constexpr int len = tf->term->length;
      if (len >= fragLen - maxEdits) {
        // OK it's possible:
        // System.out.println("    possible");
        int d;
        const wstring s = tf->term->utf8ToString();
        if (fragLen == prefixLen) {
          d = 0;
        } else if (false && len < fragLen) {
          d = getDistance(frag, s, allowTransposition);
        } else {
          // System.out.println("    try loop");
          d = maxEdits + 1;
          // for(int ed=-maxEdits;ed<=maxEdits;ed++) {
          for (int ed = -maxEdits; ed <= maxEdits; ed++) {
            if (s.length() < fragLen - ed) {
              continue;
            }
            wstring check = s.substr(0, fragLen - ed);
            d = getDistance(frag, check, allowTransposition);
            // System.out.println("    sub check s=" + check + " d=" + d);
            if (d <= maxEdits) {
              break;
            }
          }
        }
        if (d <= maxEdits) {
          results.push_back(
              make_shared<LookupResult>(tf->term->utf8ToString(), tf->v));
        }
      }
    }

    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(results, new CompareByCostThenAlpha());
    sort(results.begin(), results.end(), make_shared<CompareByCostThenAlpha>());
  }

  return results;
}

int FuzzySuggesterTest::CharSequenceComparator::compare(
    shared_ptr<std::wstring> o1, shared_ptr<std::wstring> o2)
{
  constexpr int l1 = o1->length();
  constexpr int l2 = o2->length();

  constexpr int aStop = min(l1, l2);
  for (int i = 0; i < aStop; i++) {
    int diff = o1->charAt(i) - o2->charAt(i);
    if (diff != 0) {
      return diff;
    }
  }
  // One is a prefix of the other, or, they are equal:
  return l1 - l2;
}

const shared_ptr<java::util::Comparator<std::shared_ptr<std::wstring>>>
    FuzzySuggesterTest::CHARSEQUENCE_COMPARATOR =
        make_shared<CharSequenceComparator>();

int FuzzySuggesterTest::CompareByCostThenAlpha::compare(
    shared_ptr<LookupResult> a, shared_ptr<LookupResult> b)
{
  if (a->value > b->value) {
    return -1;
  } else if (a->value < b->value) {
    return 1;
  } else {
    constexpr int c = CHARSEQUENCE_COMPARATOR->compare(a->key, b->key);
    assert((c != 0, L"term=" + a->key));
    return c;
  }
}

int FuzzySuggesterTest::getDistance(const wstring &target, const wstring &other,
                                    bool allowTransposition)
{
  shared_ptr<IntsRef> targetPoints;
  shared_ptr<IntsRef> otherPoints;
  int n;
  std::deque<std::deque<int>> d; // cost array

  // NOTE: if we cared, we could 3*m space instead of m*n space, similar to
  // what LevenshteinDistance does, except cycling thru a ring of three
  // horizontal cost arrays... but this comparator is never actually used by
  // DirectSpellChecker, it's only used for merging results from multiple shards
  // in "distributed spellcheck", and it's inefficient in other ways too...

  // cheaper to do this up front once
  targetPoints = toIntsRef(target);
  otherPoints = toIntsRef(other);
  n = targetPoints->length;
  constexpr int m = otherPoints->length;
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: d = new int[n+1][m+1];
  d = RectangularVectors::ReturnRectangularIntVector(n + 1, m + 1);

  if (n == 0 || m == 0) {
    if (n == m) {
      return 0;
    } else {
      return max(n, m);
    }
  }

  // indexes into strings s and t
  int i; // iterates through s
  int j; // iterates through t

  int t_j; // jth character of t

  int cost; // cost

  for (i = 0; i <= n; i++) {
    d[i][0] = i;
  }

  for (j = 0; j <= m; j++) {
    d[0][j] = j;
  }

  for (j = 1; j <= m; j++) {
    t_j = otherPoints->ints[j - 1];

    for (i = 1; i <= n; i++) {
      cost = targetPoints->ints[i - 1] == t_j ? 0 : 1;
      // minimum of cell to the left+1, to the top+1, diagonally left and up
      // +cost
      d[i][j] =
          min(min(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + cost);
      // transposition
      if (allowTransposition && i > 1 && j > 1 &&
          targetPoints->ints[i - 1] == otherPoints->ints[j - 2] &&
          targetPoints->ints[i - 2] == otherPoints->ints[j - 1]) {
        d[i][j] = min(d[i][j], d[i - 2][j - 2] + cost);
      }
    }
  }

  return d[n][m];
}

shared_ptr<IntsRef> FuzzySuggesterTest::toIntsRef(const wstring &s)
{
  shared_ptr<IntsRef> ref = make_shared<IntsRef>(s.length()); // worst case
  int utf16Len = s.length();
  for (int i = 0, cp = 0; i < utf16Len; i += Character::charCount(cp)) {
    cp = ref->ints[ref->length++] = Character::codePointAt(s, i);
  }
  return ref;
}

shared_ptr<Directory> FuzzySuggesterTest::getDirectory()
{
  return newDirectory();
}
} // namespace org::apache::lucene::search::suggest::analyzing