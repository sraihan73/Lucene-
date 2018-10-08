using namespace std;

#include "TestBugInSomething.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockCharFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/commongrams/CommonGramsFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/WordDelimiterFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ngram/EdgeNGramTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ngram/NGramTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/shingle/ShingleFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/wikipedia/WikipediaTokenizer.h"
#include "TestRandomChains.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using MockCharFilter = org::apache::lucene::analysis::MockCharFilter;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using MappingCharFilter =
    org::apache::lucene::analysis::charfilter::MappingCharFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
using CommonGramsFilter =
    org::apache::lucene::analysis::commongrams::CommonGramsFilter;
using WordDelimiterFilter =
    org::apache::lucene::analysis::miscellaneous::WordDelimiterFilter;
using EdgeNGramTokenizer =
    org::apache::lucene::analysis::ngram::EdgeNGramTokenizer;
using NGramTokenFilter = org::apache::lucene::analysis::ngram::NGramTokenFilter;
using ShingleFilter = org::apache::lucene::analysis::shingle::ShingleFilter;
using WikipediaTokenizer =
    org::apache::lucene::analysis::wikipedia::WikipediaTokenizer;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;

void TestBugInSomething::test() 
{
  shared_ptr<CharArraySet> *const cas = make_shared<CharArraySet>(3, false);
  cas->add(L"jjp");
  cas->add(L"wlmwoknt");
  cas->add(L"tcgyreo");

  shared_ptr<NormalizeCharMap::Builder> *const builder =
      make_shared<NormalizeCharMap::Builder>();
  builder->add(L"mtqlpi", L"");
  builder->add(L"mwoknt", L"jjp");
  builder->add(L"tcgyreo", L"zpfpajyws");
  shared_ptr<NormalizeCharMap> *const map_obj = builder->build();

  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), cas, map_obj);
  checkAnalysisConsistency(random(), a, false, L"wmgddzunizdomqyj");
  delete a;
}

TestBugInSomething::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestBugInSomething> outerInstance, shared_ptr<CharArraySet> cas,
    shared_ptr<NormalizeCharMap> map_obj)
{
  this->outerInstance = outerInstance;
  this->cas = cas;
  this->map_obj = map_obj;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBugInSomething::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t =
      make_shared<MockTokenizer>(MockTokenFilter::ENGLISH_STOPSET, false, -65);
  shared_ptr<TokenFilter> f = make_shared<CommonGramsFilter>(t, cas);
  return make_shared<Analyzer::TokenStreamComponents>(t, f);
}

shared_ptr<Reader> TestBugInSomething::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  reader = make_shared<MockCharFilter>(reader, 0);
  reader = make_shared<MappingCharFilter>(map_obj, reader);
  reader =
      make_shared<TestRandomChains::CheckThatYouDidntReadAnythingReaderWrapper>(
          reader);
  return reader;
}

TestBugInSomething::CharFilterAnonymousInnerClass::
    CharFilterAnonymousInnerClass(shared_ptr<StringReader> java)
    : org::apache::lucene::analysis::CharFilter(StringReader)
{
}

void TestBugInSomething::CharFilterAnonymousInnerClass::mark(int readAheadLimit)
{
  throw make_shared<UnsupportedOperationException>(L"mark(int)");
}

bool TestBugInSomething::CharFilterAnonymousInnerClass::markSupported()
{
  throw make_shared<UnsupportedOperationException>(L"markSupported()");
}

int TestBugInSomething::CharFilterAnonymousInnerClass::read()
{
  throw make_shared<UnsupportedOperationException>(L"read()");
}

int TestBugInSomething::CharFilterAnonymousInnerClass::read(
    std::deque<wchar_t> &cbuf)
{
  throw make_shared<UnsupportedOperationException>(L"read(char[])");
}

int TestBugInSomething::CharFilterAnonymousInnerClass::read(
    shared_ptr<CharBuffer> target)
{
  throw make_shared<UnsupportedOperationException>(L"read(CharBuffer)");
}

bool TestBugInSomething::CharFilterAnonymousInnerClass::ready()
{
  throw make_shared<UnsupportedOperationException>(L"ready()");
}

void TestBugInSomething::CharFilterAnonymousInnerClass::reset()
{
  throw make_shared<UnsupportedOperationException>(L"reset()");
}

int64_t TestBugInSomething::CharFilterAnonymousInnerClass::skip(int64_t n)
{
  throw make_shared<UnsupportedOperationException>(L"skip(long)");
}

int TestBugInSomething::CharFilterAnonymousInnerClass::correct(int currentOff)
{
  throw make_shared<UnsupportedOperationException>(L"correct(int)");
}

TestBugInSomething::CharFilterAnonymousInnerClass::
    ~CharFilterAnonymousInnerClass()
{
  throw make_shared<UnsupportedOperationException>(L"close()");
}

int TestBugInSomething::CharFilterAnonymousInnerClass::read(
    std::deque<wchar_t> &arg0, int arg1, int arg2)
{
  throw make_shared<UnsupportedOperationException>(L"read(char[], int, int)");
}

void TestBugInSomething::testWrapping() 
{
  shared_ptr<CharFilter> cs =
      make_shared<TestRandomChains::CheckThatYouDidntReadAnythingReaderWrapper>(
          wrappedStream);
  runtime_error expected =
      expectThrows(runtime_error::typeid, [&]() { cs->mark(1); });
  assertEquals(L"mark(int)", expected.what());

  expected =
      expectThrows(runtime_error::typeid, [&]() { cs->markSupported(); });
  assertEquals(L"markSupported()", expected.what());

  expected = expectThrows(runtime_error::typeid, [&]() { cs->read(); });
  assertEquals(L"read()", expected.what());

  expected = expectThrows(runtime_error::typeid,
                          [&]() { cs->read(std::deque<wchar_t>(0)); });
  assertEquals(L"read(char[])", expected.what());

  expected = expectThrows(runtime_error::typeid, [&]() {
    cs->read(CharBuffer::wrap(std::deque<wchar_t>(0)));
  });
  assertEquals(L"read(CharBuffer)", expected.what());

  expected = expectThrows(runtime_error::typeid, [&]() { cs->reset(); });
  assertEquals(L"reset()", expected.what());

  expected = expectThrows(runtime_error::typeid, [&]() { cs->skip(1); });
  assertEquals(L"skip(long)", expected.what());

  expected =
      expectThrows(runtime_error::typeid, [&]() { cs->correctOffset(1); });
  assertEquals(L"correct(int)", expected.what());

  expected = expectThrows(runtime_error::typeid, [&]() { cs->close(); });
  assertEquals(L"close()", expected.what());

  expected = expectThrows(runtime_error::typeid,
                          [&]() { cs->read(std::deque<wchar_t>(0), 0, 0); });
  assertEquals(L"read(char[], int, int)", expected.what());
}

TestBugInSomething::SopTokenFilter::SopTokenFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestBugInSomething::SopTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (VERBOSE) {
      wcout << input->getClass().getSimpleName() << L"->"
            << this->reflectAsString(false) << endl;
    }
    return true;
  } else {
    return false;
  }
}

void TestBugInSomething::SopTokenFilter::end() 
{
  TokenFilter::end();
  if (VERBOSE) {
    wcout << input->getClass().getSimpleName() << L".end()" << endl;
  }
}

TestBugInSomething::SopTokenFilter::~SopTokenFilter()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  if (VERBOSE) {
    wcout << input->getClass().getSimpleName() << L".close()" << endl;
  }
}

void TestBugInSomething::SopTokenFilter::reset() 
{
  TokenFilter::reset();
  if (VERBOSE) {
    wcout << input->getClass().getSimpleName() << L".reset()" << endl;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testUnicodeShinglesAndNgrams() throws
// Exception
void TestBugInSomething::testUnicodeShinglesAndNgrams() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), analyzer, 2000);
  delete analyzer;
}

TestBugInSomething::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestBugInSomething> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBugInSomething::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<EdgeNGramTokenizer>(2, 94);
  // TokenStream stream = new SopTokenFilter(tokenizer);
  shared_ptr<TokenStream> stream = make_shared<ShingleFilter>(tokenizer, 5);
  // stream = new SopTokenFilter(stream);
  stream = make_shared<NGramTokenFilter>(stream, 55, 83, false);
  // stream = new SopTokenFilter(stream);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestBugInSomething::testCuriousWikipediaString() 
{
  shared_ptr<CharArraySet> *const protWords = make_shared<CharArraySet>(
      unordered_set<>(Arrays::asList(L"rrdpafa", L"pupmmlu", L"xlq", L"dyy",
                                     L"zqrxrrck", L"o", L"hsrlfvcha")),
      false);
  const std::deque<char> table = std::deque<char>{
      -57,  26,   1,    48,  63,   -23, 55,  -84, 18,   120,  -97, 103,
      58,   13,   84,   89,  57,   -13, -63, 5,   28,   97,   -54, -94,
      102,  -108, -5,   5,   46,   40,  43,  78,  43,   -72,  36,  29,
      124,  -106, -22,  -51, 65,   5,   31,  -42, 6,    -99,  97,  14,
      81,   -128, 74,   100, 54,   -55, -25, 53,  -71,  -98,  44,  33,
      86,   106,  -42,  47,  115,  -89, -18, -26, 22,   -95,  -43, 83,
      -125, 105,  -104, -24, 106,  -16, 126, 115, -105, 97,   65,  -33,
      57,   44,   -1,   123, -68,  100, 13,  -41, -64,  -119, 0,   92,
      94,   -36,  53,   -9,  -102, -18, 90,  94,  -26,  31,   71,  -20};
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), protWords, table);
  checkAnalysisConsistency(random(), a, false,
                           L"B\u28c3\ue0f8[ \ud800\udfc2 </p> jb");
  delete a;
}

TestBugInSomething::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestBugInSomething> outerInstance,
    shared_ptr<CharArraySet> protWords, deque<char> &table)
{
  this->outerInstance = outerInstance;
  this->protWords = protWords;
  this->table = table;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBugInSomething::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<WikipediaTokenizer>();
  shared_ptr<TokenStream> stream = make_shared<SopTokenFilter>(tokenizer);
  stream = make_shared<WordDelimiterFilter>(stream, table, -50, protWords);
  stream = make_shared<SopTokenFilter>(stream);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}
} // namespace org::apache::lucene::analysis::core