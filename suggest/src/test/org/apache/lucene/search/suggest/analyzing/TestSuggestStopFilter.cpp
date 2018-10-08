using namespace std;

#include "TestSuggestStopFilter.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestSuggestStopFilter::testEndNotStopWord() 
{
  shared_ptr<CharArraySet> stopWords = StopFilter::makeStopSet({L"to"});
  shared_ptr<Tokenizer> stream = make_shared<MockTokenizer>();
  stream->setReader(make_shared<StringReader>(L"go to"));
  shared_ptr<TokenStream> filter =
      make_shared<SuggestStopFilter>(stream, stopWords);
  assertTokenStreamContents(filter, std::deque<wstring>{L"go", L"to"},
                            std::deque<int>{0, 3}, std::deque<int>{2, 5},
                            nullptr, std::deque<int>{1, 1}, nullptr, 5,
                            std::deque<bool>{false, true}, true);
}

void TestSuggestStopFilter::testEndIsStopWord() 
{

  shared_ptr<CharArraySet> stopWords = StopFilter::makeStopSet({L"to"});
  shared_ptr<Tokenizer> stream = make_shared<MockTokenizer>();
  stream->setReader(make_shared<StringReader>(L"go to "));
  shared_ptr<TokenStream> filter =
      make_shared<SuggestStopFilter>(stream, stopWords);
  filter = make_shared<SuggestStopFilter>(stream, stopWords);
  assertTokenStreamContents(filter, std::deque<wstring>{L"go"},
                            std::deque<int>{0}, std::deque<int>{2}, nullptr,
                            std::deque<int>{1}, nullptr, 6,
                            std::deque<bool>{false}, true);
}

void TestSuggestStopFilter::testMidStopWord() 
{

  shared_ptr<CharArraySet> stopWords = StopFilter::makeStopSet({L"to"});
  shared_ptr<Tokenizer> stream = make_shared<MockTokenizer>();
  stream->setReader(make_shared<StringReader>(L"go to school"));
  shared_ptr<TokenStream> filter =
      make_shared<SuggestStopFilter>(stream, stopWords);

  filter = make_shared<SuggestStopFilter>(stream, stopWords);
  assertTokenStreamContents(filter, std::deque<wstring>{L"go", L"school"},
                            std::deque<int>{0, 6}, std::deque<int>{2, 12},
                            nullptr, std::deque<int>{1, 2}, nullptr, 12,
                            std::deque<bool>{false, false}, true);
}

void TestSuggestStopFilter::testMultipleStopWords() 
{

  shared_ptr<CharArraySet> stopWords =
      StopFilter::makeStopSet({L"to", L"the", L"a"});
  shared_ptr<Tokenizer> stream = make_shared<MockTokenizer>();
  stream->setReader(make_shared<StringReader>(L"go to a the school"));
  shared_ptr<TokenStream> filter =
      make_shared<SuggestStopFilter>(stream, stopWords);

  filter = make_shared<SuggestStopFilter>(stream, stopWords);
  assertTokenStreamContents(filter, std::deque<wstring>{L"go", L"school"},
                            std::deque<int>{0, 12}, std::deque<int>{2, 18},
                            nullptr, std::deque<int>{1, 4}, nullptr, 18,
                            std::deque<bool>{false, false}, true);
}

void TestSuggestStopFilter::testMultipleStopWordsEnd() 
{

  shared_ptr<CharArraySet> stopWords =
      StopFilter::makeStopSet({L"to", L"the", L"a"});
  shared_ptr<Tokenizer> stream = make_shared<MockTokenizer>();
  stream->setReader(make_shared<StringReader>(L"go to a the"));
  shared_ptr<TokenStream> filter =
      make_shared<SuggestStopFilter>(stream, stopWords);

  filter = make_shared<SuggestStopFilter>(stream, stopWords);
  assertTokenStreamContents(filter, std::deque<wstring>{L"go", L"the"},
                            std::deque<int>{0, 8}, std::deque<int>{2, 11},
                            nullptr, std::deque<int>{1, 3}, nullptr, 11,
                            std::deque<bool>{false, true}, true);
}

void TestSuggestStopFilter::testMultipleStopWordsEnd2() 
{

  shared_ptr<CharArraySet> stopWords =
      StopFilter::makeStopSet({L"to", L"the", L"a"});
  shared_ptr<Tokenizer> stream = make_shared<MockTokenizer>();
  stream->setReader(make_shared<StringReader>(L"go to a the "));
  shared_ptr<TokenStream> filter =
      make_shared<SuggestStopFilter>(stream, stopWords);

  filter = make_shared<SuggestStopFilter>(stream, stopWords);
  assertTokenStreamContents(filter, std::deque<wstring>{L"go"},
                            std::deque<int>{0}, std::deque<int>{2}, nullptr,
                            std::deque<int>{1}, nullptr, 12,
                            std::deque<bool>{false}, true);
}
} // namespace org::apache::lucene::search::suggest::analyzing