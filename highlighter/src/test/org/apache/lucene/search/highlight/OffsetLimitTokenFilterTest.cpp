using namespace std;

#include "OffsetLimitTokenFilterTest.h"

namespace org::apache::lucene::search::highlight
{
using namespace org::apache::lucene::analysis;

void OffsetLimitTokenFilterTest::testFilter() 
{
  // we disable MockTokenizer checks because we will forcefully limit the
  // tokenstream and call end() before incrementToken() returns false.
  shared_ptr<MockTokenizer> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  stream->setReader(make_shared<StringReader>(
      L"short toolong evenmuchlongertext a ab toolong foo"));
  stream->setEnableChecks(false);
  shared_ptr<OffsetLimitTokenFilter> filter =
      make_shared<OffsetLimitTokenFilter>(stream, 10);
  assertTokenStreamContents(filter, std::deque<wstring>{L"short", L"toolong"});

  stream = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  stream->setReader(make_shared<StringReader>(
      L"short toolong evenmuchlongertext a ab toolong foo"));
  stream->setEnableChecks(false);
  filter = make_shared<OffsetLimitTokenFilter>(stream, 12);
  assertTokenStreamContents(filter, std::deque<wstring>{L"short", L"toolong"});

  stream = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  stream->setReader(make_shared<StringReader>(
      L"short toolong evenmuchlongertext a ab toolong foo"));
  stream->setEnableChecks(false);
  filter = make_shared<OffsetLimitTokenFilter>(stream, 30);
  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"short", L"toolong", L"evenmuchlongertext"});

  checkOneTerm(make_shared<AnalyzerAnonymousInnerClass>(shared_from_this()),
               L"llenges", L"llenges");
}

OffsetLimitTokenFilterTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<OffsetLimitTokenFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
OffsetLimitTokenFilterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setEnableChecks(false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<OffsetLimitTokenFilter>(tokenizer, 10));
}
} // namespace org::apache::lucene::search::highlight