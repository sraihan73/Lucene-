using namespace std;

#include "AnalyzingSuggesterTest.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedBinaryTokenStream =
    org::apache::lucene::analysis::CannedBinaryTokenStream;
using BinaryToken =
    org::apache::lucene::analysis::CannedBinaryTokenStream::BinaryToken;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using MockUTF16TermAttributeImpl =
    org::apache::lucene::analysis::MockUTF16TermAttributeImpl;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Input = org::apache::lucene::search::suggest::Input;
using InputArrayIterator =
    org::apache::lucene::search::suggest::InputArrayIterator;
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void AnalyzingSuggesterTest::testKeyword() 
{
  deque<std::shared_ptr<Input>> keys = shuffle(
      make_shared<Input>(L"foo", 50), make_shared<Input>(L"bar", 10),
      make_shared<Input>(L"barbar", 10), make_shared<Input>(L"barbar", 12),
      make_shared<Input>(L"barbara", 6), make_shared<Input>(L"bar", 5),
      make_shared<Input>(L"barbara", 1));

  shared_ptr<Directory> tempDir = getDirectory();

  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<AnalyzingSuggester> suggester =
      make_shared<AnalyzingSuggester>(tempDir, L"suggest", analyzer);
  suggester->build(make_shared<InputArrayIterator>(keys));

  // top N of 2, but only foo is available
  deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
      TestUtil::stringToCharSequence(L"f", random()), false, 2);
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

void AnalyzingSuggesterTest::testKeywordWithPayloads() 
{
  deque<std::shared_ptr<Input>> keys = shuffle(
      {make_shared<Input>(L"foo", 50, make_shared<BytesRef>(L"hello")),
       make_shared<Input>(L"bar", 10, make_shared<BytesRef>(L"goodbye")),
       make_shared<Input>(L"barbar", 12, make_shared<BytesRef>(L"thank you")),
       make_shared<Input>(L"bar", 9,
                          make_shared<BytesRef>(L"should be deduplicated")),
       make_shared<Input>(
           L"bar", 8, make_shared<BytesRef>(L"should also be deduplicated")),
       make_shared<Input>(L"barbara", 6,
                          make_shared<BytesRef>(L"for all the fish"))});

  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester =
      make_shared<AnalyzingSuggester>(tempDir, L"suggest", analyzer);
  suggester->build(make_shared<InputArrayIterator>(keys));
  for (int i = 0; i < 2; i++) {
    // top N of 2, but only foo is available
    deque<std::shared_ptr<LookupResult>> results = suggester->lookup(
        TestUtil::stringToCharSequence(L"f", random()), false, 2);
    TestUtil::assertEquals(1, results.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"foo", results[0]->key->toString());
    assertEquals(50, results[0]->value, 0.01F);
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello"),
                           results[0]->payload);

    // top N of 1 for 'bar': we return this even though
    // barbar is higher because exactFirst is enabled:
    results = suggester->lookup(
        TestUtil::stringToCharSequence(L"bar", random()), false, 1);
    TestUtil::assertEquals(1, results.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"bar", results[0]->key->toString());
    assertEquals(10, results[0]->value, 0.01F);
    TestUtil::assertEquals(make_shared<BytesRef>(L"goodbye"),
                           results[0]->payload);

    // top N Of 2 for 'b'
    results = suggester->lookup(TestUtil::stringToCharSequence(L"b", random()),
                                false, 2);
    TestUtil::assertEquals(2, results.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"barbar", results[0]->key->toString());
    assertEquals(12, results[0]->value, 0.01F);
    TestUtil::assertEquals(make_shared<BytesRef>(L"thank you"),
                           results[0]->payload);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"bar", results[1]->key->toString());
    assertEquals(10, results[1]->value, 0.01F);
    TestUtil::assertEquals(make_shared<BytesRef>(L"goodbye"),
                           results[1]->payload);

    // top N of 3 for 'ba'
    results = suggester->lookup(TestUtil::stringToCharSequence(L"ba", random()),
                                false, 3);
    TestUtil::assertEquals(3, results.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"barbar", results[0]->key->toString());
    assertEquals(12, results[0]->value, 0.01F);
    TestUtil::assertEquals(make_shared<BytesRef>(L"thank you"),
                           results[0]->payload);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"bar", results[1]->key->toString());
    assertEquals(10, results[1]->value, 0.01F);
    TestUtil::assertEquals(make_shared<BytesRef>(L"goodbye"),
                           results[1]->payload);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"barbara", results[2]->key->toString());
    assertEquals(6, results[2]->value, 0.01F);
    TestUtil::assertEquals(make_shared<BytesRef>(L"for all the fish"),
                           results[2]->payload);
  }
  IOUtils::close({analyzer, tempDir});
}

void AnalyzingSuggesterTest::testRandomRealisticKeys() 
{
  shared_ptr<LineFileDocs> lineFile = make_shared<LineFileDocs>(random());
  unordered_map<wstring, int64_t> mapping =
      unordered_map<wstring, int64_t>();
  deque<std::shared_ptr<Input>> keys = deque<std::shared_ptr<Input>>();

  int howMany = atLeast(100); // this might bring up duplicates
  for (int i = 0; i < howMany; i++) {
    shared_ptr<Document> nextDoc = lineFile->nextDoc();
    wstring title = nextDoc->getField(L"title")->stringValue();
    int randomWeight = random()->nextInt(100);
    int maxLen = min(title.length(), 500);
    wstring prefix = title.substr(0, maxLen);
    keys.push_back(make_shared<Input>(prefix, randomWeight));
    if (mapping.find(prefix) == mapping.end() ||
        mapping[prefix] < randomWeight) {
      mapping.emplace(prefix, static_cast<int64_t>(randomWeight));
    }
  }
  shared_ptr<Analyzer> indexAnalyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Analyzer> queryAnalyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();

  shared_ptr<AnalyzingSuggester> analyzingSuggester =
      make_shared<AnalyzingSuggester>(
          tempDir, L"suggest", indexAnalyzer, queryAnalyzer,
          AnalyzingSuggester::EXACT_FIRST | AnalyzingSuggester::PRESERVE_SEP,
          256, -1, random()->nextBoolean());
  bool doPayloads = random()->nextBoolean();
  if (doPayloads) {
    deque<std::shared_ptr<Input>> keysAndPayloads =
        deque<std::shared_ptr<Input>>();
    for (auto termFreq : keys) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      keysAndPayloads.push_back(make_shared<Input>(
          termFreq->term, termFreq->v,
          make_shared<BytesRef>(Long::toString(termFreq->v))));
    }
    analyzingSuggester->build(make_shared<InputArrayIterator>(keysAndPayloads));
  } else {
    analyzingSuggester->build(make_shared<InputArrayIterator>(keys));
  }

  for (auto termFreq : keys) {
    deque<std::shared_ptr<LookupResult>> lookup = analyzingSuggester->lookup(
        termFreq->term->utf8ToString(), false, keys.size());
    for (auto lookupResult : lookup) {
      TestUtil::assertEquals(mapping[lookupResult->key],
                             static_cast<int64_t>(lookupResult->value));
      if (doPayloads) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        TestUtil::assertEquals(lookupResult->payload->utf8ToString(),
                               Long::toString(lookupResult->value));
      } else {
        assertNull(lookupResult->payload);
      }
    }
  }

  IOUtils::close({lineFile, indexAnalyzer, queryAnalyzer, tempDir});
}

void AnalyzingSuggesterTest::testStandard() 
{
  const wstring input =
      L"the ghost of christmas past the"; // trailing stopword there just to
                                          // perturb possible bugs
  std::deque<std::shared_ptr<Input>> keys = {make_shared<Input>(input, 50)};

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<Analyzer> standard =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", standard, standard,
      AnalyzingSuggester::EXACT_FIRST | AnalyzingSuggester::PRESERVE_SEP, 256,
      -1, false);

  suggester->build(make_shared<InputArrayIterator>(keys));
  deque<std::shared_ptr<LookupResult>> results;

  // round-trip
  results = suggester->lookup(TestUtil::stringToCharSequence(input, random()),
                              false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(input, results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // prefix of input stopping part way through christmas
  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"the ghost of chris", random()), false,
      1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(input, results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // omit the 'the' since it's a stopword, it's suggested anyway
  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ghost of chris", random()), false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(input, results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // omit the 'the' and 'of' since they are stopwords, it's suggested anyway
  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ghost chris", random()), false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(input, results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  // trailing stopword "the"
  results = suggester->lookup(
      TestUtil::stringToCharSequence(L"ghost christmas past the", random()),
      false, 1);
  TestUtil::assertEquals(1, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(input, results[0]->key->toString());
  assertEquals(50, results[0]->value, 0.01F);

  IOUtils::close({standard, tempDir});
}

void AnalyzingSuggesterTest::testEmpty() 
{
  shared_ptr<Analyzer> standard =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester =
      make_shared<AnalyzingSuggester>(tempDir, L"suggest", standard);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>(0)));

  deque<std::shared_ptr<LookupResult>> result =
      suggester->lookup(L"a", false, 20);
  assertTrue(result.empty());
  IOUtils::close({standard, tempDir});
}

void AnalyzingSuggesterTest::testNoSeps() 
{
  std::deque<std::shared_ptr<Input>> keys = {make_shared<Input>(L"ab cd", 0),
                                              make_shared<Input>(L"abcd", 1)};

  int options = 0;

  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, options, 256, -1, true);
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

void AnalyzingSuggesterTest::testGraphDups() 
{

  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  std::deque<std::shared_ptr<Input>> keys = {
      make_shared<Input>(L"wifi network is slow", 50),
      make_shared<Input>(L"wi fi network is fast", 10)};
  // AnalyzingSuggester suggester = new AnalyzingSuggester(analyzer,
  // AnalyzingSuggester.EXACT_FIRST, 256, -1);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester =
      make_shared<AnalyzingSuggester>(tempDir, L"suggest", analyzer);
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
  IOUtils::close({analyzer, tempDir});
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
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

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::getTokenStream()
{
  shared_ptr<TokenStream> result = tokenStreams[tokenStreamCounter];
  tokenStreamCounter++;
  return result;
}

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::setReader(
        shared_ptr<Reader> reader)
{
}

void AnalyzingSuggesterTest::testInputPathRequired() 
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
  shared_ptr<AnalyzingSuggester> suggester =
      make_shared<AnalyzingSuggester>(tempDir, L"suggest", analyzer);
  suggester->build(make_shared<InputArrayIterator>(keys));
  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"ab x", false, 1);
  assertTrue(results.size() == 1);
  IOUtils::close({analyzer, tempDir});
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass2>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
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

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::getTokenStream()
{
  shared_ptr<TokenStream> result = tokenStreams[tokenStreamCounter];
  tokenStreamCounter++;
  return result;
}

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::setReader(
        shared_ptr<Reader> reader)
{
}

shared_ptr<Token> AnalyzingSuggesterTest::token(const wstring &term, int posInc,
                                                int posLength)
{
  shared_ptr<Token> *const t = make_shared<Token>(term, 0, 0);
  t->setPositionIncrement(posInc);
  t->setPositionLength(posLength);
  return t;
}

shared_ptr<CannedBinaryTokenStream::BinaryToken>
AnalyzingSuggesterTest::token(shared_ptr<BytesRef> term)
{
  return make_shared<CannedBinaryTokenStream::BinaryToken>(term);
}

shared_ptr<Analyzer> AnalyzingSuggesterTest::getUnusualAnalyzer()
{
  return make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass3>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::
        TokenStreamComponentsAnonymousInnerClass3(
            shared_ptr<AnalyzerAnonymousInnerClass3> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
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

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::setReader(
        shared_ptr<Reader> reader)
{
}

void AnalyzingSuggesterTest::testExactFirst() 
{

  shared_ptr<Analyzer> a = getUnusualAnalyzer();
  int options =
      AnalyzingSuggester::EXACT_FIRST | AnalyzingSuggester::PRESERVE_SEP;
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, options, 256, -1, true);
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

void AnalyzingSuggesterTest::testNonExactFirst() 
{

  shared_ptr<Analyzer> a = getUnusualAnalyzer();
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, AnalyzingSuggester::PRESERVE_SEP, 256, -1,
      true);

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

AnalyzingSuggesterTest::TermFreq2::TermFreq2(const wstring &surfaceForm,
                                             const wstring &analyzedForm,
                                             int64_t weight,
                                             shared_ptr<BytesRef> payload)
    : surfaceForm(surfaceForm), analyzedForm(analyzedForm), weight(weight),
      payload(payload)
{
}

int AnalyzingSuggesterTest::TermFreq2::compareTo(shared_ptr<TermFreq2> other)
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

wstring AnalyzingSuggesterTest::TermFreq2::toString()
{
  return surfaceForm + L"/" + to_wstring(weight);
}

bool AnalyzingSuggesterTest::isStopChar(wchar_t ch, int numStopChars)
{
  // System.out.println("IS? " + ch + ": " + (ch - 'a') + ": " + ((ch - 'a') <
  // numStopChars));
  return (ch - L'a') < numStopChars;
}

AnalyzingSuggesterTest::TokenEater::TokenEater(bool preserveHoles,
                                               shared_ptr<TokenStream> in_,
                                               int numStopChars)
    : org::apache::lucene::analysis::TokenFilter(in_),
      numStopChars(numStopChars), preserveHoles(preserveHoles)
{
}

void AnalyzingSuggesterTest::TokenEater::reset() 
{
  TokenFilter::reset();
  first = true;
}

bool AnalyzingSuggesterTest::TokenEater::incrementToken() 
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

AnalyzingSuggesterTest::MockTokenEatingAnalyzer::MockTokenEatingAnalyzer(
    int numStopChars, bool preserveHoles)
{
  this->preserveHoles = preserveHoles;
  this->numStopChars = numStopChars;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::MockTokenEatingAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer = make_shared<MockTokenizer>(
      MockUTF16TermAttributeImpl::UTF16_TERM_ATTRIBUTE_FACTORY,
      MockTokenizer::WHITESPACE, false,
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

wchar_t AnalyzingSuggesterTest::SEP = L'\u001F';

void AnalyzingSuggesterTest::testRandom() 
{

  int numQueries = atLeast(1000);

  const deque<std::shared_ptr<TermFreq2>> slowCompletor =
      deque<std::shared_ptr<TermFreq2>>();
  const set<wstring> allPrefixes = set<wstring>();
  shared_ptr<Set<wstring>> *const seen = unordered_set<wstring>();

  bool doPayloads = random()->nextBoolean();

  std::deque<std::shared_ptr<Input>> keys;
  std::deque<std::shared_ptr<Input>> payloadKeys;
  if (doPayloads) {
    payloadKeys = std::deque<std::shared_ptr<Input>>(numQueries);
  } else {
    keys = std::deque<std::shared_ptr<Input>>(numQueries);
  }

  bool preserveSep = random()->nextBoolean();

  constexpr int numStopChars = random()->nextInt(10);
  constexpr bool preserveHoles = random()->nextBoolean();

  if (VERBOSE) {
    wcout << L"TEST: " << numQueries << L" words; preserveSep=" << preserveSep
          << L" numStopChars=" << numStopChars << L" preserveHoles="
          << preserveHoles << endl;
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
                analyzedKey[analyzedKey.length() - 1] != SEP) {
              analyzedKey += SEP;
            }
            key += s;
            if (s.length() == 1 && isStopChar(s[0], numStopChars)) {
              lastRemoved = true;
              if (preserveSep && preserveHoles) {
                analyzedKey += SEP;
              }
            } else {
              lastRemoved = false;
              analyzedKey += s;
            }
            break;
          }
        }
      }

      analyzedKey =
          analyzedKey.replaceAll(L"(^|" + StringHelper::toString(SEP) + L")" +
                                     StringHelper::toString(SEP) + L"$",
                                 L"");

      if (preserveSep && lastRemoved) {
        analyzedKey += SEP;
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
    shared_ptr<BytesRef> payload;
    if (doPayloads) {
      std::deque<char> bytes(random()->nextInt(10));
      random()->nextBytes(bytes);
      payload = make_shared<BytesRef>(bytes);
      payloadKeys[i] = make_shared<Input>(key, weight, payload);
    } else {
      keys[i] = make_shared<Input>(key, weight);
      payload.reset();
    }

    slowCompletor.push_back(
        make_shared<TermFreq2>(key, analyzedKey, weight, payload));
  }

  if (VERBOSE) {
    // Don't just sort original deque, to avoid VERBOSE
    // altering the test:
    deque<std::shared_ptr<TermFreq2>> sorted =
        deque<std::shared_ptr<TermFreq2>>(slowCompletor);
    sort(sorted.begin(), sorted.end());
    for (auto ent : sorted) {
      wcout << L"  surface='" << ent->surfaceForm << L"' analyzed='"
            << ent->analyzedForm << L"' weight=" << ent->weight << endl;
    }
  }

  shared_ptr<Analyzer> a =
      make_shared<MockTokenEatingAnalyzer>(numStopChars, preserveHoles);
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a,
      preserveSep ? AnalyzingSuggester::PRESERVE_SEP : 0, 256, -1, true);
  if (doPayloads) {
    suggester->build(make_shared<InputArrayIterator>(shuffle(payloadKeys)));
  } else {
    suggester->build(make_shared<InputArrayIterator>(shuffle(keys)));
  }

  for (auto prefix : allPrefixes) {

    if (VERBOSE) {
      wcout << L"\nTEST: prefix=" << prefix << endl;
    }

    constexpr int topN = TestUtil::nextInt(random(), 1, 10);
    deque<std::shared_ptr<LookupResult>> r = suggester->lookup(
        TestUtil::stringToCharSequence(prefix, random()), false, topN);

    // 2. go thru whole set to find suggestions:
    deque<std::shared_ptr<TermFreq2>> matches =
        deque<std::shared_ptr<TermFreq2>>();

    // "Analyze" the key:
    std::deque<wstring> tokens = prefix.split(L" ");
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    bool lastRemoved = false;
    for (int i = 0; i < tokens.size(); i++) {
      wstring token = tokens[i];
      if (preserveSep && builder->length() > 0 &&
          !builder->toString()->endsWith(L"" + StringHelper::toString(SEP))) {
        builder->append(SEP);
      }

      if (token.length() == 1 && isStopChar(token[0], numStopChars)) {
        if (preserveSep && preserveHoles) {
          builder->append(SEP);
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
      wstring s =
          analyzedKey.replaceAll(StringHelper::toString(SEP) + L"$", L"");
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
      analyzedKey += SEP;
    }

    if (VERBOSE) {
      wcout << L"  analyzed: " << analyzedKey << endl;
    }

    // TODO: could be faster... but it's slowCompletor for a reason
    for (auto e : slowCompletor) {
      if (StringHelper::startsWith(e->analyzedForm, analyzedKey)) {
        matches.push_back(e);
      }
    }

    assertTrue(numStopChars > 0 || matches.size() > 0);

    if (matches.size() > 1) {
      // C++ TODO: The 'Compare' parameter of std::sort produces a bool
      // value, while the Java Comparator parameter produces a tri-state result:
      // ORIGINAL LINE: java.util.Collections.sort(matches, new
      // java.util.Comparator<TermFreq2>()
      sort(matches.begin(), matches.end(),
           make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));
    }

    if (matches.size() > topN) {
      matches = matches.subList(0, topN);
    }

    if (VERBOSE) {
      wcout << L"  expected:" << endl;
      for (auto lr : matches) {
        wcout << L"    key=" << lr->surfaceForm << L" weight=" << lr->weight
              << endl;
      }

      wcout << L"  actual:" << endl;
      for (auto lr : r) {
        wcout << L"    key=" << lr->key << L" weight=" << lr->value << endl;
      }
    }

    TestUtil::assertEquals(matches.size(), r.size());

    for (int hit = 0; hit < r.size(); hit++) {
      // System.out.println("  check hit " + hit);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(matches[hit]->surfaceForm->toString(),
                             r[hit]->key->toString());
      assertEquals(matches[hit]->weight, r[hit]->value, 0.0f);
      if (doPayloads) {
        TestUtil::assertEquals(matches[hit]->payload, r[hit]->payload);
      }
    }
  }
  IOUtils::close({a, tempDir});
}

AnalyzingSuggesterTest::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

int AnalyzingSuggesterTest::ComparatorAnonymousInnerClass::compare(
    shared_ptr<TermFreq2> left, shared_ptr<TermFreq2> right)
{
  int cmp = Float::compare(right->weight, left->weight);
  if (cmp == 0) {
    return left->analyzedForm.compare(right->analyzedForm);
  } else {
    return cmp;
  }
}

void AnalyzingSuggesterTest::testMaxSurfaceFormsPerAnalyzedForm() throw(
    runtime_error)
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, 0, 2, -1, true);
  suggester->build(make_shared<InputArrayIterator>(
      shuffle({make_shared<Input>(L"a", 40), make_shared<Input>(L"a ", 50),
               make_shared<Input>(L" a", 60)})));

  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"a", false, 5);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L" a", results[0]->key);
  TestUtil::assertEquals(60, results[0]->value);
  TestUtil::assertEquals(L"a ", results[1]->key);
  TestUtil::assertEquals(50, results[1]->value);
  IOUtils::close({a, tempDir});
}

void AnalyzingSuggesterTest::testQueueExhaustion() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, AnalyzingSuggester::EXACT_FIRST, 256, -1,
      true);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"a", 2), make_shared<Input>(L"a b c", 3),
          make_shared<Input>(L"a c a", 1), make_shared<Input>(L"a c b", 1)}));

  suggester->lookup(L"a", false, 4);
  IOUtils::close({a, tempDir});
}

void AnalyzingSuggesterTest::testExactFirstMissingResult() 
{

  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, AnalyzingSuggester::EXACT_FIRST, 256, -1,
      true);

  suggester->build(make_shared<InputArrayIterator>(
      std::deque<std::shared_ptr<Input>>{make_shared<Input>(L"a", 5),
                                          make_shared<Input>(L"a b", 3),
                                          make_shared<Input>(L"a c", 4)}));

  TestUtil::assertEquals(3, suggester->getCount());
  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"a", false, 3);
  TestUtil::assertEquals(3, results.size());
  TestUtil::assertEquals(L"a", results[0]->key);
  TestUtil::assertEquals(5, results[0]->value);
  TestUtil::assertEquals(L"a c", results[1]->key);
  TestUtil::assertEquals(4, results[1]->value);
  TestUtil::assertEquals(L"a b", results[2]->key);
  TestUtil::assertEquals(3, results[2]->value);

  // Try again after save/load:
  shared_ptr<Path> tmpDir = createTempDir(L"AnalyzingSuggesterTest");

  shared_ptr<Path> path = tmpDir->resolve(L"suggester");

  shared_ptr<OutputStream> os = Files::newOutputStream(path);
  suggester->store(os);
  os->close();

  shared_ptr<InputStream> is = Files::newInputStream(path);
  suggester->load(is);
  is->close();

  TestUtil::assertEquals(3, suggester->getCount());
  results = suggester->lookup(L"a", false, 3);
  TestUtil::assertEquals(3, results.size());
  TestUtil::assertEquals(L"a", results[0]->key);
  TestUtil::assertEquals(5, results[0]->value);
  TestUtil::assertEquals(L"a c", results[1]->key);
  TestUtil::assertEquals(4, results[1]->value);
  TestUtil::assertEquals(L"a b", results[2]->key);
  TestUtil::assertEquals(3, results[2]->value);
  IOUtils::close({a, tempDir});
}

void AnalyzingSuggesterTest::testDupSurfaceFormsMissingResults() throw(
    runtime_error)
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, 0, 256, -1, true);

  suggester->build(make_shared<InputArrayIterator>(shuffle(
      {make_shared<Input>(L"hambone", 6), make_shared<Input>(L"nellie", 5)})));

  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"nellie", false, 2);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"hambone", results[0]->key);
  TestUtil::assertEquals(6, results[0]->value);
  TestUtil::assertEquals(L"nellie", results[1]->key);
  TestUtil::assertEquals(5, results[1]->value);

  // Try again after save/load:
  shared_ptr<Path> tmpDir = createTempDir(L"AnalyzingSuggesterTest");

  shared_ptr<Path> path = tmpDir->resolve(L"suggester");

  shared_ptr<OutputStream> os = Files::newOutputStream(path);
  suggester->store(os);
  os->close();

  shared_ptr<InputStream> is = Files::newInputStream(path);
  suggester->load(is);
  is->close();

  results = suggester->lookup(L"nellie", false, 2);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"hambone", results[0]->key);
  TestUtil::assertEquals(6, results[0]->value);
  TestUtil::assertEquals(L"nellie", results[1]->key);
  TestUtil::assertEquals(5, results[1]->value);
  IOUtils::close({a, tempDir});
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::
        TokenStreamComponentsAnonymousInnerClass(
            shared_ptr<AnalyzerAnonymousInnerClass> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::getTokenStream()
{
  return make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
      token(L"hairy", 1, 1), token(L"smelly", 0, 1), token(L"dog", 1, 1)});
}

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass::setReader(
        shared_ptr<Reader> reader)
{
}

void AnalyzingSuggesterTest::testDupSurfaceFormsMissingResults2() throw(
    runtime_error)
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, 0, 256, -1, true);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"a", 6), make_shared<Input>(L"b", 5)}));

  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"a", false, 2);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"a", results[0]->key);
  TestUtil::assertEquals(6, results[0]->value);
  TestUtil::assertEquals(L"b", results[1]->key);
  TestUtil::assertEquals(5, results[1]->value);

  // Try again after save/load:
  shared_ptr<Path> tmpDir = createTempDir(L"AnalyzingSuggesterTest");

  shared_ptr<Path> path = tmpDir->resolve(L"suggester");

  shared_ptr<OutputStream> os = Files::newOutputStream(path);
  suggester->store(os);
  os->close();

  shared_ptr<InputStream> is = Files::newInputStream(path);
  suggester->load(is);
  is->close();

  results = suggester->lookup(L"a", false, 2);
  TestUtil::assertEquals(2, results.size());
  TestUtil::assertEquals(L"a", results[0]->key);
  TestUtil::assertEquals(6, results[0]->value);
  TestUtil::assertEquals(L"b", results[1]->key);
  TestUtil::assertEquals(5, results[1]->value);
  IOUtils::close({a, tempDir});
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass2>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::
        TokenStreamComponentsAnonymousInnerClass2(
            shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::getTokenStream()
{
  if (count == 0) {
    count++;
    return make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
        token(L"p", 1, 1), token(L"q", 1, 1), token(L"r", 0, 1),
        token(L"s", 0, 1)});
  } else {
    return make_shared<CannedTokenStream>(
        std::deque<std::shared_ptr<Token>>{token(L"p", 1, 1)});
  }
}

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass2::
    TokenStreamComponentsAnonymousInnerClass2::setReader(
        shared_ptr<Reader> reader)
{
}

void AnalyzingSuggesterTest::testTieBreakOnSurfaceForm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, 0, 256, -1, true);

  // make 50 inputs all with the same cost of 1, random strings
  std::deque<std::shared_ptr<Input>> inputs(100);
  for (int i = 0; i < inputs.size(); i++) {
    inputs[i] = make_shared<Input>(TestUtil::randomSimpleString(random()), 1);
  }

  suggester->build(make_shared<InputArrayIterator>(inputs));

  // Try to save/load:
  shared_ptr<Path> tmpDir = createTempDir(L"AnalyzingSuggesterTest");
  shared_ptr<Path> path = tmpDir->resolve(L"suggester");

  shared_ptr<OutputStream> os = Files::newOutputStream(path);
  suggester->store(os);
  os->close();

  shared_ptr<InputStream> is = Files::newInputStream(path);
  suggester->load(is);
  is->close();

  // now suggest everything, and check that stuff comes back in order
  deque<std::shared_ptr<LookupResult>> results =
      suggester->lookup(L"", false, 50);
  TestUtil::assertEquals(50, results.size());
  for (int i = 1; i < 50; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring previous = results[i - 1]->toString();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring current = results[i]->toString();
    assertTrue(L"surface forms out of order: previous=" + previous +
                   L",current=" + current,
               current.compare(previous) >= 0);
  }

  IOUtils::close({a, tempDir});
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass3>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::
        TokenStreamComponentsAnonymousInnerClass3(
            shared_ptr<AnalyzerAnonymousInnerClass3> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::getTokenStream()
{
  return make_shared<CannedTokenStream>(
      std::deque<std::shared_ptr<Token>>{token(L"dog", 1, 1)});
}

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass3::
    TokenStreamComponentsAnonymousInnerClass3::setReader(
        shared_ptr<Reader> reader)
{
}

void AnalyzingSuggesterTest::test0ByteKeys() 
{
  shared_ptr<Analyzer> *const a =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, 0, 256, -1, true);

  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"a a", 50), make_shared<Input>(L"a b", 50)}));

  IOUtils::close({a, tempDir});
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass4>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass4::
    TokenStreamComponentsAnonymousInnerClass4::
        TokenStreamComponentsAnonymousInnerClass4(
            shared_ptr<AnalyzerAnonymousInnerClass4> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
  tokenStreamCounter = 0;
  tokenStreams = std::deque<std::shared_ptr<TokenStream>>{
      make_shared<CannedBinaryTokenStream>(
          std::deque<std::shared_ptr<CannedBinaryTokenStream::BinaryToken>>{
              token(make_shared<BytesRef>(std::deque<char>{0x0, 0x0, 0x0}))}),
      make_shared<CannedBinaryTokenStream>(
          std::deque<std::shared_ptr<CannedBinaryTokenStream::BinaryToken>>{
              token(make_shared<BytesRef>(std::deque<char>{0x0, 0x0}))}),
      make_shared<CannedBinaryTokenStream>(
          std::deque<std::shared_ptr<CannedBinaryTokenStream::BinaryToken>>{
              token(make_shared<BytesRef>(std::deque<char>{0x0, 0x0, 0x0}))}),
      make_shared<CannedBinaryTokenStream>(
          std::deque<std::shared_ptr<CannedBinaryTokenStream::BinaryToken>>{
              token(make_shared<BytesRef>(std::deque<char>{0x0, 0x0}))})};
}

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass4::
    TokenStreamComponentsAnonymousInnerClass4::getTokenStream()
{
  shared_ptr<TokenStream> result = tokenStreams[tokenStreamCounter];
  tokenStreamCounter++;
  return result;
}

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass4::
    TokenStreamComponentsAnonymousInnerClass4::setReader(
        shared_ptr<Reader> reader)
{
}

void AnalyzingSuggesterTest::testDupSurfaceFormsMissingResults3() throw(
    runtime_error)
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, AnalyzingSuggester::PRESERVE_SEP, 256, -1,
      true);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"a a", 7), make_shared<Input>(L"a a", 7),
          make_shared<Input>(L"a c", 6), make_shared<Input>(L"a c", 3),
          make_shared<Input>(L"a b", 5)}));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[a a/7, a c/6, a b/5]",
                         suggester->lookup(L"a", false, 3).toString());
  IOUtils::close({tempDir, a});
}

void AnalyzingSuggesterTest::testEndingSpace() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, AnalyzingSuggester::PRESERVE_SEP, 256, -1,
      true);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"i love lucy", 7),
          make_shared<Input>(L"isla de muerta", 8)}));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[isla de muerta/8, i love lucy/7]",
                         suggester->lookup(L"i", false, 3).toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[i love lucy/7]",
                         suggester->lookup(L"i ", false, 3).toString());
  IOUtils::close({a, tempDir});
}

void AnalyzingSuggesterTest::testTooManyExpansions() 
{

  shared_ptr<Analyzer> *const a =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, 0, 256, 1, true);
  suggester->build(make_shared<InputArrayIterator>(
      std::deque<std::shared_ptr<Input>>{make_shared<Input>(L"a", 1)}));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[a/1]",
                         suggester->lookup(L"a", false, 1).toString());
  IOUtils::close({a, tempDir});
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<AnalyzingSuggesterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);

  return make_shared<TokenStreamComponentsAnonymousInnerClass5>(
      shared_from_this(), tokenizer);
}

AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass5::
    TokenStreamComponentsAnonymousInnerClass5::
        TokenStreamComponentsAnonymousInnerClass5(
            shared_ptr<AnalyzerAnonymousInnerClass5> outerInstance,
            shared_ptr<Tokenizer> tokenizer)
    : TokenStreamComponents(tokenizer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStream> AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass5::
    TokenStreamComponentsAnonymousInnerClass5::getTokenStream()
{
  shared_ptr<Token> a = make_shared<Token>(L"a", 0, 1);
  a->setPositionIncrement(1);
  shared_ptr<Token> b = make_shared<Token>(L"b", 0, 1);
  b->setPositionIncrement(0);
  return make_shared<CannedTokenStream>(
      std::deque<std::shared_ptr<Token>>{a, b});
}

void AnalyzingSuggesterTest::AnalyzerAnonymousInnerClass5::
    TokenStreamComponentsAnonymousInnerClass5::setReader(
        shared_ptr<Reader> reader)
{
}

void AnalyzingSuggesterTest::testIllegalLookupArgument() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester = make_shared<AnalyzingSuggester>(
      tempDir, L"suggest", a, a, 0, 256, -1, true);
  suggester->build(
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>{
          make_shared<Input>(L"а где Люси?", 7)}));
  expectThrows(invalid_argument::typeid,
               [&]() { suggester->lookup(L"а\u001E", false, 3); });
  expectThrows(invalid_argument::typeid,
               [&]() { suggester->lookup(L"а\u001F", false, 3); });

  IOUtils::close({a, tempDir});
}

deque<std::shared_ptr<Input>>
AnalyzingSuggesterTest::shuffle(deque<Input> &values)
{
  const deque<std::shared_ptr<Input>> asList =
      deque<std::shared_ptr<Input>>(values->length);
  for (shared_ptr<Input> value : values) {
    asList.push_back(value);
  }
  Collections::shuffle(asList, random());
  return asList;
}

void AnalyzingSuggesterTest::testTooLongSuggestion() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<AnalyzingSuggester> suggester =
      make_shared<AnalyzingSuggester>(tempDir, L"suggest", a);
  wstring bigString = TestUtil::randomSimpleString(random(), 30000, 30000);
  try {
    suggester->build(make_shared<InputArrayIterator>(
        std::deque<std::shared_ptr<Input>>{make_shared<Input>(bigString, 7)}));
    fail(L"did not hit expected exception");
  } catch (const invalid_argument &iae) {
    // expected
    assertTrue(iae.what()->contains(L"input automaton is too large"));
  }
  IOUtils::close({a, tempDir});
}

shared_ptr<Directory> AnalyzingSuggesterTest::getDirectory()
{
  return newDirectory();
}
} // namespace org::apache::lucene::search::suggest::analyzing