using namespace std;

#include "TestDelegatingAnalyzerWrapper.h"

namespace org::apache::lucene::analysis
{
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDelegatingAnalyzerWrapper::testDelegatesNormalization()
{
  shared_ptr<Analyzer> analyzer1 =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<DelegatingAnalyzerWrapper> w1 =
      make_shared<DelegatingAnalyzerWrapperAnonymousInnerClass>(
          shared_from_this(), Analyzer::GLOBAL_REUSE_STRATEGY, analyzer1);
  assertEquals(make_shared<BytesRef>(L"Ab C"), w1->normalize(L"foo", L"Ab C"));

  shared_ptr<Analyzer> analyzer2 =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true);
  shared_ptr<DelegatingAnalyzerWrapper> w2 =
      make_shared<DelegatingAnalyzerWrapperAnonymousInnerClass2>(
          shared_from_this(), Analyzer::GLOBAL_REUSE_STRATEGY, analyzer2);
  assertEquals(make_shared<BytesRef>(L"ab c"), w2->normalize(L"foo", L"Ab C"));
}

TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass::
    DelegatingAnalyzerWrapperAnonymousInnerClass(
        shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1)
    : DelegatingAnalyzerWrapper(GLOBAL_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
  this->analyzer1 = analyzer1;
}

shared_ptr<Analyzer>
TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass::
    getWrappedAnalyzer(const wstring &fieldName)
{
  return analyzer1;
}

TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass2::
    DelegatingAnalyzerWrapperAnonymousInnerClass2(
        shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer2)
    : DelegatingAnalyzerWrapper(GLOBAL_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
  this->analyzer2 = analyzer2;
}

shared_ptr<Analyzer>
TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass2::
    getWrappedAnalyzer(const wstring &fieldName)
{
  return analyzer2;
}

void TestDelegatingAnalyzerWrapper::testDelegatesAttributeFactory() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer1 = make_shared<MockBytesAnalyzer>();
  shared_ptr<DelegatingAnalyzerWrapper> w1 =
      make_shared<DelegatingAnalyzerWrapperAnonymousInnerClass3>(
          shared_from_this(), Analyzer::GLOBAL_REUSE_STRATEGY, analyzer1);
  assertEquals(make_shared<BytesRef>(
                   (wstring(L"Ab C")).getBytes(StandardCharsets::UTF_16LE)),
               w1->normalize(L"foo", L"Ab C"));
}

TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass3::
    DelegatingAnalyzerWrapperAnonymousInnerClass3(
        shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1)
    : DelegatingAnalyzerWrapper(GLOBAL_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
  this->analyzer1 = analyzer1;
}

shared_ptr<Analyzer>
TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass3::
    getWrappedAnalyzer(const wstring &fieldName)
{
  return analyzer1;
}

void TestDelegatingAnalyzerWrapper::testDelegatesCharFilter() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer1 =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<DelegatingAnalyzerWrapper> w1 =
      make_shared<DelegatingAnalyzerWrapperAnonymousInnerClass4>(
          shared_from_this(), Analyzer::GLOBAL_REUSE_STRATEGY, analyzer1);
  assertEquals(make_shared<BytesRef>(L"az c"), w1->normalize(L"foo", L"ab c"));
}

TestDelegatingAnalyzerWrapper::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Reader> TestDelegatingAnalyzerWrapper::AnalyzerAnonymousInnerClass::
    initReaderForNormalization(const wstring &fieldName,
                               shared_ptr<Reader> reader)
{
  return make_shared<DummyCharFilter>(reader, L'b', L'z');
}

shared_ptr<TokenStreamComponents>
TestDelegatingAnalyzerWrapper::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(attributeFactory(fieldName));
  return make_shared<TokenStreamComponents>(tokenizer);
}

TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass4::
    DelegatingAnalyzerWrapperAnonymousInnerClass4(
        shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1)
    : DelegatingAnalyzerWrapper(GLOBAL_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
  this->analyzer1 = analyzer1;
}

shared_ptr<Analyzer>
TestDelegatingAnalyzerWrapper::DelegatingAnalyzerWrapperAnonymousInnerClass4::
    getWrappedAnalyzer(const wstring &fieldName)
{
  return analyzer1;
}

TestDelegatingAnalyzerWrapper::DummyCharFilter::DummyCharFilter(
    shared_ptr<Reader> input, wchar_t match, wchar_t repl)
    : CharFilter(input), match(match), repl(repl)
{
}

int TestDelegatingAnalyzerWrapper::DummyCharFilter::correct(int currentOff)
{
  return currentOff;
}

int TestDelegatingAnalyzerWrapper::DummyCharFilter::read(
    std::deque<wchar_t> &cbuf, int off, int len) 
{
  constexpr int read = input->read(cbuf, off, len);
  for (int i = 0; i < read; ++i) {
    if (cbuf[off + i] == match) {
      cbuf[off + i] = repl;
    }
  }
  return read;
}
} // namespace org::apache::lucene::analysis