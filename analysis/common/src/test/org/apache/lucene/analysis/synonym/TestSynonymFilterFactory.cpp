using namespace std;

#include "TestSynonymFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/apache/lucene/analysis/cjk/CJKAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/pattern/PatternTokenizerFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SolrSynonymParser.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../util/StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::synonym
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PatternTokenizerFactory =
    org::apache::lucene::analysis::pattern::PatternTokenizerFactory;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using StringMockResourceLoader =
    org::apache::lucene::analysis::util::StringMockResourceLoader;
using CJKAnalyzer = org::apache::lucene::analysis::cjk::CJKAnalyzer;
using Version = org::apache::lucene::util::Version;

void TestSynonymFilterFactory::checkSolrSynonyms(
    shared_ptr<TokenFilterFactory> factory) 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"GB");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTrue(std::dynamic_pointer_cast<SynonymFilter>(stream) != nullptr);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"GB", L"gib", L"gigabyte", L"gigabytes"},
      std::deque<int>{1, 0, 0, 0});
}

void TestSynonymFilterFactory::checkWordnetSynonyms(
    shared_ptr<TokenFilterFactory> factory) 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"second");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTrue(std::dynamic_pointer_cast<SynonymFilter>(stream) != nullptr);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"second", L"2nd", L"two"},
                            std::deque<int>{1, 0, 0});
}

void TestSynonymFilterFactory::testSynonyms() 
{
  checkSolrSynonyms(
      tokenFilterFactory(L"Synonym", {L"synonyms", L"synonyms.txt"}));
}

void TestSynonymFilterFactory::testEmptySynonyms() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"GB");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Synonym", Version::LATEST,
                              make_shared<StringMockResourceLoader>(L""),
                              L"synonyms", L"synonyms.txt")
               .create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"GB"});
}

void TestSynonymFilterFactory::testFormat() 
{
  checkSolrSynonyms(tokenFilterFactory(
      L"Synonym", {L"synonyms", L"synonyms.txt", L"format", L"solr"}));
  checkWordnetSynonyms(
      tokenFilterFactory(L"Synonym", {L"synonyms", L"synonyms-wordnet.txt",
                                      L"format", L"wordnet"}));
  // explicit class should work the same as the "solr" alias
  checkSolrSynonyms(
      tokenFilterFactory(L"Synonym", {L"synonyms", L"synonyms.txt", L"format",
                                      SolrSynonymParser::typeid->getName()}));
}

void TestSynonymFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(
        L"Synonym", {L"synonyms", L"synonyms.txt", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestSynonymFilterFactory::testAnalyzer() 
{
  const wstring analyzer = CJKAnalyzer::typeid->getName();
  const wstring tokenizerFactory = PatternTokenizerFactory::typeid->getName();
  shared_ptr<TokenFilterFactory> factory = nullptr;

  factory = tokenFilterFactory(
      L"Synonym", {L"synonyms", L"synonyms2.txt", L"analyzer", analyzer});
  assertNotNull(factory);

  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Synonym",
                       {L"synonyms", L"synonyms.txt", L"analyzer", analyzer,
                        L"tokenizerFactory", tokenizerFactory});
  });
  assertTrue(expected.what()->contains(
      L"Analyzer and TokenizerFactory can't be specified both"));
}

const wstring TestSynonymFilterFactory::TOK_SYN_ARG_VAL = L"argument";
const wstring TestSynonymFilterFactory::TOK_FOO_ARG_VAL = L"foofoofoo";

void TestSynonymFilterFactory::testTokenizerFactoryArguments() throw(
    runtime_error)
{
  const wstring clazz = PatternTokenizerFactory::typeid->getName();
  shared_ptr<TokenFilterFactory> factory = nullptr;

  // simple arg form
  factory = tokenFilterFactory(
      L"Synonym", {L"synonyms", L"synonyms.txt", L"tokenizerFactory", clazz,
                   L"pattern", L"(.*)", L"group", L"0"});
  assertNotNull(factory);
  // prefix
  factory = tokenFilterFactory(
      L"Synonym",
      {L"synonyms", L"synonyms.txt", L"tokenizerFactory", clazz,
       L"tokenizerFactory.pattern", L"(.*)", L"tokenizerFactory.group", L"0"});
  assertNotNull(factory);

  // sanity check that sub-PatternTokenizerFactory fails w/o pattern
  expectThrows(runtime_error::typeid, [&]() {
    tokenFilterFactory(
        L"Synonym", {L"synonyms", L"synonyms.txt", L"tokenizerFactory", clazz});
  });

  // sanity check that sub-PatternTokenizerFactory fails on unexpected
  expectThrows(runtime_error::typeid, [&]() {
    tokenFilterFactory(L"Synonym",
                       {L"synonyms", L"synonyms.txt", L"tokenizerFactory",
                        clazz, L"tokenizerFactory.pattern", L"(.*)",
                        L"tokenizerFactory.bogusbogusbogus", L"bogus",
                        L"tokenizerFactory.group", L"0"});
  });
}
} // namespace org::apache::lucene::analysis::synonym