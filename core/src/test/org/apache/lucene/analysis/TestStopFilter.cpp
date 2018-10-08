using namespace std;

#include "TestStopFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using English = org::apache::lucene::util::English;

void TestStopFilter::testExactCase() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Now is The Time");
  shared_ptr<CharArraySet> stopWords =
      make_shared<CharArraySet>(asSet({L"is", L"the", L"Time"}), false);
  shared_ptr<MockTokenizer> *const in_ =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  in_->setReader(reader);
  shared_ptr<TokenStream> stream = make_shared<StopFilter>(in_, stopWords);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Now", L"The"});
}

void TestStopFilter::testStopFilt() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Now is The Time");
  std::deque<wstring> stopWords = {L"is", L"the", L"Time"};
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet(stopWords);
  shared_ptr<MockTokenizer> *const in_ =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  in_->setReader(reader);
  shared_ptr<TokenStream> stream = make_shared<StopFilter>(in_, stopSet);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Now", L"The"});
}

void TestStopFilter::testStopPositons() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  deque<wstring> a = deque<wstring>();
  for (int i = 0; i < 20; i++) {
    wstring w = StringHelper::trim(English::intToEnglish(i));
    sb->append(w)->append(L" ");
    if (i % 3 != 0) {
      a.push_back(w);
    }
  }
  log(sb->toString());
  std::deque<wstring> stopWords = a.toArray(std::deque<wstring>(0));
  for (int i = 0; i < a.size(); i++) {
    log(L"Stop: " + stopWords[i]);
  }
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet(stopWords);
  // with increments
  shared_ptr<StringReader> reader = make_shared<StringReader>(sb->toString());
  shared_ptr<MockTokenizer> *const in_ =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  in_->setReader(reader);
  shared_ptr<StopFilter> stpf = make_shared<StopFilter>(in_, stopSet);
  doTestStopPositons(stpf);
  // with increments, concatenating two stop filters
  deque<wstring> a0 = deque<wstring>();
  deque<wstring> a1 = deque<wstring>();
  for (int i = 0; i < a.size(); i++) {
    if (i % 2 == 0) {
      a0.push_back(a[i]);
    } else {
      a1.push_back(a[i]);
    }
  }
  std::deque<wstring> stopWords0 = a0.toArray(std::deque<wstring>(0));
  for (int i = 0; i < a0.size(); i++) {
    log(L"Stop0: " + stopWords0[i]);
  }
  std::deque<wstring> stopWords1 = a1.toArray(std::deque<wstring>(0));
  for (int i = 0; i < a1.size(); i++) {
    log(L"Stop1: " + stopWords1[i]);
  }
  shared_ptr<CharArraySet> stopSet0 = StopFilter::makeStopSet(stopWords0);
  shared_ptr<CharArraySet> stopSet1 = StopFilter::makeStopSet(stopWords1);
  reader = make_shared<StringReader>(sb->toString());
  shared_ptr<MockTokenizer> *const in1 =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  in1->setReader(reader);
  shared_ptr<StopFilter> stpf0 =
      make_shared<StopFilter>(in1, stopSet0); // first part of the set
  shared_ptr<StopFilter> stpf01 = make_shared<StopFilter>(
      stpf0, stopSet1); // two stop filters concatenated!
  doTestStopPositons(stpf01);
}

void TestStopFilter::testEndStopword() 
{
  shared_ptr<CharArraySet> stopSet = StopFilter::makeStopSet({L"of"});
  shared_ptr<MockTokenizer> *const in_ =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  in_->setReader(make_shared<StringReader>(L"test of"));
  shared_ptr<StopFilter> stpf = make_shared<StopFilter>(in_, stopSet);
  assertTokenStreamContents(stpf, std::deque<wstring>{L"test"},
                            std::deque<int>{0}, std::deque<int>{4}, nullptr,
                            std::deque<int>{1}, nullptr, 7, 1, nullptr, true,
                            nullptr);
}

void TestStopFilter::doTestStopPositons(shared_ptr<StopFilter> stpf) throw(
    IOException)
{
  shared_ptr<CharTermAttribute> termAtt =
      stpf->getAttribute(CharTermAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> posIncrAtt =
      stpf->getAttribute(PositionIncrementAttribute::typeid);
  stpf->reset();
  for (int i = 0; i < 20; i += 3) {
    assertTrue(stpf->incrementToken());
    log(L"Token " + to_wstring(i) + L": " + stpf);
    wstring w = StringHelper::trim(English::intToEnglish(i));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"expecting token " + to_wstring(i) + L" to be " + w, w,
                 termAtt->toString());
    assertEquals(L"all but first token must have position increment of 3",
                 i == 0 ? 1 : 3, posIncrAtt->getPositionIncrement());
  }
  assertFalse(stpf->incrementToken());
  stpf->end();
  delete stpf;
}

void TestStopFilter::log(const wstring &s)
{
  if (VERBOSE) {
    wcout << s << endl;
  }
}
} // namespace org::apache::lucene::analysis