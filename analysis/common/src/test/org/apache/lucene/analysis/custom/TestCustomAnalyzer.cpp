using namespace std;

#include "TestCustomAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/SetOnce.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/HTMLStripCharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseTokenizerFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/StopFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizerFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/custom/CustomAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ASCIIFoldingFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/reverse/ReverseStringFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/standard/ClassicTokenizerFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"

namespace org::apache::lucene::analysis::custom
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using HTMLStripCharFilterFactory =
    org::apache::lucene::analysis::charfilter::HTMLStripCharFilterFactory;
using MappingCharFilterFactory =
    org::apache::lucene::analysis::charfilter::MappingCharFilterFactory;
using LowerCaseFilterFactory =
    org::apache::lucene::analysis::core::LowerCaseFilterFactory;
using LowerCaseTokenizer =
    org::apache::lucene::analysis::core::LowerCaseTokenizer;
using LowerCaseTokenizerFactory =
    org::apache::lucene::analysis::core::LowerCaseTokenizerFactory;
using StopFilterFactory =
    org::apache::lucene::analysis::core::StopFilterFactory;
using WhitespaceTokenizerFactory =
    org::apache::lucene::analysis::core::WhitespaceTokenizerFactory;
using ASCIIFoldingFilterFactory =
    org::apache::lucene::analysis::miscellaneous::ASCIIFoldingFilterFactory;
using ReverseStringFilterFactory =
    org::apache::lucene::analysis::reverse::ReverseStringFilterFactory;
using ClassicTokenizerFactory =
    org::apache::lucene::analysis::standard::ClassicTokenizerFactory;
using StandardTokenizerFactory =
    org::apache::lucene::analysis::standard::StandardTokenizerFactory;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using BytesRef = org::apache::lucene::util::BytesRef;
using AlreadySetException =
    org::apache::lucene::util::SetOnce::AlreadySetException;
using Version = org::apache::lucene::util::Version;

void TestCustomAnalyzer::testWhitespaceFactoryWithFolding() 
{
  shared_ptr<CustomAnalyzer> a =
      CustomAnalyzer::builder()
          ->withTokenizer(WhitespaceTokenizerFactory::typeid)
          .addTokenFilter(ASCIIFoldingFilterFactory::typeid,
                          L"preserveOriginal", L"true")
          .addTokenFilter(LowerCaseFilterFactory::typeid)
          .build();

  assertSame(WhitespaceTokenizerFactory::typeid,
             a->getTokenizerFactory()->getClass());
  assertEquals(Collections::emptyList(), a->getCharFilterFactories());
  deque<std::shared_ptr<TokenFilterFactory>> tokenFilters =
      a->getTokenFilterFactories();
  assertEquals(2, tokenFilters.size());
  assertSame(ASCIIFoldingFilterFactory::typeid, tokenFilters[0]->getClass());
  assertSame(LowerCaseFilterFactory::typeid, tokenFilters[1]->getClass());
  assertEquals(0, a->getPositionIncrementGap(L"dummy"));
  assertEquals(1, a->getOffsetGap(L"dummy"));
  assertSame(Version::LATEST, a->getVersion());

  assertAnalyzesTo(a, L"foo bar FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
                   std::deque<int>{1, 1, 1, 1});
  assertAnalyzesTo(a, L"föó bär FÖÖ BAR",
                   std::deque<wstring>{L"foo", L"föó", L"bar", L"bär", L"foo",
                                        L"föö", L"bar"},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete a;
}

void TestCustomAnalyzer::testWhitespaceWithFolding() 
{
  shared_ptr<CustomAnalyzer> a =
      CustomAnalyzer::builder()
          ->withTokenizer(L"whitespace")
          .addTokenFilter(L"asciifolding", L"preserveOriginal", L"true")
          .addTokenFilter(L"lowercase")
          .build();

  assertSame(WhitespaceTokenizerFactory::typeid,
             a->getTokenizerFactory()->getClass());
  assertEquals(Collections::emptyList(), a->getCharFilterFactories());
  deque<std::shared_ptr<TokenFilterFactory>> tokenFilters =
      a->getTokenFilterFactories();
  assertEquals(2, tokenFilters.size());
  assertSame(ASCIIFoldingFilterFactory::typeid, tokenFilters[0]->getClass());
  assertSame(LowerCaseFilterFactory::typeid, tokenFilters[1]->getClass());
  assertEquals(0, a->getPositionIncrementGap(L"dummy"));
  assertEquals(1, a->getOffsetGap(L"dummy"));
  assertSame(Version::LATEST, a->getVersion());

  assertAnalyzesTo(a, L"foo bar FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
                   std::deque<int>{1, 1, 1, 1});
  assertAnalyzesTo(a, L"föó bär FÖÖ BAR",
                   std::deque<wstring>{L"foo", L"föó", L"bar", L"bär", L"foo",
                                        L"föö", L"bar"},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete a;
}

void TestCustomAnalyzer::testFactoryHtmlStripClassicFolding() throw(
    runtime_error)
{
  shared_ptr<CustomAnalyzer> a =
      CustomAnalyzer::builder()
          ->withDefaultMatchVersion(Version::LUCENE_6_0_0)
          ->addCharFilter(HTMLStripCharFilterFactory::typeid)
          .withTokenizer(ClassicTokenizerFactory::typeid)
          .addTokenFilter(ASCIIFoldingFilterFactory::typeid,
                          L"preserveOriginal", L"true")
          .addTokenFilter(LowerCaseFilterFactory::typeid)
          .withPositionIncrementGap(100)
          .withOffsetGap(1000)
          .build();

  assertSame(ClassicTokenizerFactory::typeid,
             a->getTokenizerFactory()->getClass());
  deque<std::shared_ptr<CharFilterFactory>> charFilters =
      a->getCharFilterFactories();
  assertEquals(1, charFilters.size());
  assertEquals(HTMLStripCharFilterFactory::typeid, charFilters[0]->getClass());
  deque<std::shared_ptr<TokenFilterFactory>> tokenFilters =
      a->getTokenFilterFactories();
  assertEquals(2, tokenFilters.size());
  assertSame(ASCIIFoldingFilterFactory::typeid, tokenFilters[0]->getClass());
  assertSame(LowerCaseFilterFactory::typeid, tokenFilters[1]->getClass());
  assertEquals(100, a->getPositionIncrementGap(L"dummy"));
  assertEquals(1000, a->getOffsetGap(L"dummy"));
  assertSame(Version::LUCENE_6_0_0, a->getVersion());

  assertAnalyzesTo(a, L"<p>foo bar</p> FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
                   std::deque<int>{1, 1, 1, 1});
  assertAnalyzesTo(a, L"<p><b>föó</b> bär     FÖÖ BAR</p>",
                   std::deque<wstring>{L"foo", L"föó", L"bar", L"bär", L"foo",
                                        L"föö", L"bar"},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete a;
}

void TestCustomAnalyzer::testHtmlStripClassicFolding() 
{
  shared_ptr<CustomAnalyzer> a =
      CustomAnalyzer::builder()
          ->withDefaultMatchVersion(Version::LUCENE_6_0_0)
          ->addCharFilter(L"htmlstrip")
          .withTokenizer(L"classic")
          .addTokenFilter(L"asciifolding", L"preserveOriginal", L"true")
          .addTokenFilter(L"lowercase")
          .withPositionIncrementGap(100)
          .withOffsetGap(1000)
          .build();

  assertSame(ClassicTokenizerFactory::typeid,
             a->getTokenizerFactory()->getClass());
  deque<std::shared_ptr<CharFilterFactory>> charFilters =
      a->getCharFilterFactories();
  assertEquals(1, charFilters.size());
  assertEquals(HTMLStripCharFilterFactory::typeid, charFilters[0]->getClass());
  deque<std::shared_ptr<TokenFilterFactory>> tokenFilters =
      a->getTokenFilterFactories();
  assertEquals(2, tokenFilters.size());
  assertSame(ASCIIFoldingFilterFactory::typeid, tokenFilters[0]->getClass());
  assertSame(LowerCaseFilterFactory::typeid, tokenFilters[1]->getClass());
  assertEquals(100, a->getPositionIncrementGap(L"dummy"));
  assertEquals(1000, a->getOffsetGap(L"dummy"));
  assertSame(Version::LUCENE_6_0_0, a->getVersion());

  assertAnalyzesTo(a, L"<p>foo bar</p> FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
                   std::deque<int>{1, 1, 1, 1});
  assertAnalyzesTo(a, L"<p><b>föó</b> bär     FÖÖ BAR</p>",
                   std::deque<wstring>{L"foo", L"föó", L"bar", L"bär", L"foo",
                                        L"föö", L"bar"},
                   std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  delete a;
}

void TestCustomAnalyzer::testStopWordsFromClasspath() 
{
  shared_ptr<CustomAnalyzer> a =
      CustomAnalyzer::builder()
          ->withTokenizer(WhitespaceTokenizerFactory::typeid)
          .addTokenFilter(L"stop", L"ignoreCase", L"true", L"words",
                          L"org/apache/lucene/analysis/custom/teststop.txt",
                          L"format", L"wordset")
          .build();

  assertSame(WhitespaceTokenizerFactory::typeid,
             a->getTokenizerFactory()->getClass());
  assertEquals(Collections::emptyList(), a->getCharFilterFactories());
  deque<std::shared_ptr<TokenFilterFactory>> tokenFilters =
      a->getTokenFilterFactories();
  assertEquals(1, tokenFilters.size());
  assertSame(StopFilterFactory::typeid, tokenFilters[0]->getClass());
  assertEquals(0, a->getPositionIncrementGap(L"dummy"));
  assertEquals(1, a->getOffsetGap(L"dummy"));
  assertSame(Version::LATEST, a->getVersion());

  assertAnalyzesTo(a, L"foo Foo Bar", std::deque<wstring>(0));
  delete a;
}

void TestCustomAnalyzer::testStopWordsFromClasspathWithMap() throw(
    runtime_error)
{
  unordered_map<wstring, wstring> stopConfig1 =
      unordered_map<wstring, wstring>();
  stopConfig1.emplace(L"ignoreCase", L"true");
  stopConfig1.emplace(L"words",
                      L"org/apache/lucene/analysis/custom/teststop.txt");
  stopConfig1.emplace(L"format", L"wordset");

  unordered_map<wstring, wstring> stopConfig2 =
      unordered_map<wstring, wstring>(stopConfig1);
  unordered_map<wstring, wstring> stopConfigImmutable =
      stopConfig1;

  shared_ptr<CustomAnalyzer> a = CustomAnalyzer::builder()
                                     ->withTokenizer(L"whitespace")
                                     .addTokenFilter(L"stop", stopConfig1)
                                     .build();
  assertTrue(stopConfig1.empty());
  assertAnalyzesTo(a, L"foo Foo Bar", std::deque<wstring>(0));

  a = CustomAnalyzer::builder()
          ->withTokenizer(WhitespaceTokenizerFactory::typeid)
          .addTokenFilter(StopFilterFactory::typeid, stopConfig2)
          .build();
  assertTrue(stopConfig2.empty());
  assertAnalyzesTo(a, L"foo Foo Bar", std::deque<wstring>(0));

  // try with unmodifiableMap, should fail
  expectThrows(UnsupportedOperationException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withTokenizer(L"whitespace")
        .addTokenFilter(L"stop", stopConfigImmutable)
        .build();
  });
  delete a;
}

void TestCustomAnalyzer::testStopWordsFromFile() 
{
  shared_ptr<CustomAnalyzer> a =
      CustomAnalyzer::builder(this->getDataPath(L""))
          ->withTokenizer(L"whitespace")
          .addTokenFilter(L"stop", L"ignoreCase", L"true", L"words",
                          L"teststop.txt", L"format", L"wordset")
          .build();
  assertAnalyzesTo(a, L"foo Foo Bar", std::deque<wstring>(0));
  delete a;
}

void TestCustomAnalyzer::testStopWordsFromFileAbsolute() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<CustomAnalyzer> a =
      CustomAnalyzer::builder(Paths->get(L"."))
          ->withTokenizer(L"whitespace")
          .addTokenFilter(L"stop", L"ignoreCase", L"true", L"words",
                          this->getDataPath(L"teststop.txt")->toString(),
                          L"format", L"wordset")
          .build();
  assertAnalyzesTo(a, L"foo Foo Bar", std::deque<wstring>(0));
  delete a;
}

void TestCustomAnalyzer::testIncorrectOrder() 
{
  expectThrows(IllegalStateException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->addCharFilter(L"htmlstrip")
        .withDefaultMatchVersion(Version::LATEST)
        .withTokenizer(L"whitespace")
        .build();
  });
}

void TestCustomAnalyzer::testMissingSPI() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    CustomAnalyzer::builder()->withTokenizer(L"foobar_nonexistent").build();
  });
  assertTrue(expected.what()->contains(L"SPI"));
  assertTrue(expected.what()->contains(L"does not exist"));
}

void TestCustomAnalyzer::testSetTokenizerTwice() 
{
  expectThrows(AlreadySetException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withTokenizer(L"whitespace")
        .withTokenizer(StandardTokenizerFactory::typeid)
        .build();
  });
}

void TestCustomAnalyzer::testSetMatchVersionTwice() 
{
  expectThrows(AlreadySetException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withDefaultMatchVersion(Version::LATEST)
        ->withDefaultMatchVersion(Version::LATEST)
        ->withTokenizer(L"standard")
        .build();
  });
}

void TestCustomAnalyzer::testSetPosIncTwice() 
{
  expectThrows(AlreadySetException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withPositionIncrementGap(2)
        ->withPositionIncrementGap(3)
        ->withTokenizer(L"standard")
        .build();
  });
}

void TestCustomAnalyzer::testSetOfsGapTwice() 
{
  expectThrows(AlreadySetException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withOffsetGap(2)
        ->withOffsetGap(3)
        ->withTokenizer(L"standard")
        .build();
  });
}

void TestCustomAnalyzer::testNoTokenizer() 
{
  expectThrows(IllegalStateException::typeid,
               [&]() { CustomAnalyzer::builder()->build(); });
}

void TestCustomAnalyzer::testNullTokenizer() 
{
  expectThrows(NullPointerException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withTokenizer(static_cast<wstring>(nullptr))
        .build();
  });
}

void TestCustomAnalyzer::testNullTokenizerFactory() 
{
  expectThrows(NullPointerException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withTokenizer(
            static_cast<type_info<std::shared_ptr<TokenizerFactory>>>(nullptr))
        .build();
  });
}

void TestCustomAnalyzer::testNullParamKey() 
{
  expectThrows(NullPointerException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withTokenizer(L"whitespace", {L"", L"foo"})
        ->build();
  });
}

void TestCustomAnalyzer::testNullMatchVersion() 
{
  expectThrows(NullPointerException::typeid, [&]() {
    CustomAnalyzer::builder()
        ->withDefaultMatchVersion(nullptr)
        ->withTokenizer(L"whitespace")
        .build();
  });
}

TestCustomAnalyzer::DummyCharFilter::DummyCharFilter(shared_ptr<Reader> input,
                                                     wchar_t match,
                                                     wchar_t repl)
    : org::apache::lucene::analysis::CharFilter(input), match(match), repl(repl)
{
}

int TestCustomAnalyzer::DummyCharFilter::correct(int currentOff)
{
  return currentOff;
}

int TestCustomAnalyzer::DummyCharFilter::read(std::deque<wchar_t> &cbuf,
                                              int off,
                                              int len) 
{
  constexpr int read = input->read(cbuf, off, len);
  for (int i = 0; i < read; ++i) {
    if (cbuf[off + i] == match) {
      cbuf[off + i] = repl;
    }
  }
  return read;
}

TestCustomAnalyzer::DummyCharFilterFactory::DummyCharFilterFactory(
    unordered_map<wstring, wstring> &args)
    : DummyCharFilterFactory(args, L'0', L'1')
{
}

TestCustomAnalyzer::DummyCharFilterFactory::DummyCharFilterFactory(
    unordered_map<wstring, wstring> &args, wchar_t match, wchar_t repl)
    : org::apache::lucene::analysis::util::CharFilterFactory(args),
      match(match), repl(repl)
{
}

shared_ptr<Reader>
TestCustomAnalyzer::DummyCharFilterFactory::create(shared_ptr<Reader> input)
{
  return make_shared<DummyCharFilter>(input, match, repl);
}

TestCustomAnalyzer::DummyMultiTermAwareCharFilterFactory::
    DummyMultiTermAwareCharFilterFactory(unordered_map<wstring, wstring> &args)
    : DummyCharFilterFactory(args)
{
}

shared_ptr<AbstractAnalysisFactory> TestCustomAnalyzer::
    DummyMultiTermAwareCharFilterFactory::getMultiTermComponent()
{
  return make_shared<DummyCharFilterFactory>(Collections::emptyMap(), L'0',
                                             L'2');
}

TestCustomAnalyzer::DummyTokenizerFactory::DummyTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args)
{
}

shared_ptr<Tokenizer> TestCustomAnalyzer::DummyTokenizerFactory::create(
    shared_ptr<AttributeFactory> factory)
{
  return make_shared<LowerCaseTokenizer>(factory);
}

TestCustomAnalyzer::DummyMultiTermAwareTokenizerFactory::
    DummyMultiTermAwareTokenizerFactory(unordered_map<wstring, wstring> &args)
    : DummyTokenizerFactory(args)
{
}

shared_ptr<AbstractAnalysisFactory>
TestCustomAnalyzer::DummyMultiTermAwareTokenizerFactory::getMultiTermComponent()
{
  return make_shared<DummyTokenFilterFactory>(Collections::emptyMap());
}

TestCustomAnalyzer::DummyTokenFilterFactory::DummyTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
}

shared_ptr<TokenStream> TestCustomAnalyzer::DummyTokenFilterFactory::create(
    shared_ptr<TokenStream> input)
{
  return input;
}

TestCustomAnalyzer::DummyMultiTermAwareTokenFilterFactory::
    DummyMultiTermAwareTokenFilterFactory(unordered_map<wstring, wstring> &args)
    : DummyTokenFilterFactory(args)
{
}

shared_ptr<AbstractAnalysisFactory> TestCustomAnalyzer::
    DummyMultiTermAwareTokenFilterFactory::getMultiTermComponent()
{
  return make_shared<ASCIIFoldingFilterFactory>(Collections::emptyMap());
}

void TestCustomAnalyzer::testNormalization() 
{
  shared_ptr<CustomAnalyzer> analyzer1 =
      CustomAnalyzer::builder()
          ->withTokenizer(DummyTokenizerFactory::typeid,
                          Collections::emptyMap())
          ->addCharFilter(DummyCharFilterFactory::typeid,
                          Collections::emptyMap())
          ->addTokenFilter(DummyTokenFilterFactory::typeid,
                           Collections::emptyMap())
          ->build();
  assertEquals(make_shared<BytesRef>(L"0À"),
               analyzer1->normalize(L"dummy", L"0À"));

  shared_ptr<CustomAnalyzer> analyzer2 =
      CustomAnalyzer::builder()
          ->withTokenizer(DummyMultiTermAwareTokenizerFactory::typeid,
                          Collections::emptyMap())
          ->addCharFilter(DummyMultiTermAwareCharFilterFactory::typeid,
                          Collections::emptyMap())
          ->addTokenFilter(DummyMultiTermAwareTokenFilterFactory::typeid,
                           Collections::emptyMap())
          ->build();
  assertEquals(make_shared<BytesRef>(L"2A"),
               analyzer2->normalize(L"dummy", L"0À"));
}

void TestCustomAnalyzer::testNormalizationWithMultipleTokenFilters() throw(
    IOException)
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder()
          ->withTokenizer(WhitespaceTokenizerFactory::typeid,
                          Collections::emptyMap())
          ->addTokenFilter(LowerCaseFilterFactory::typeid,
                           Collections::emptyMap())
          ->addTokenFilter(ASCIIFoldingFilterFactory::typeid,
                           Collections::emptyMap())
          ->build();
  assertEquals(make_shared<BytesRef>(L"a b e"),
               analyzer->normalize(L"dummy", L"À B é"));
}

void TestCustomAnalyzer::testNormalizationWithMultiplCharFilters() throw(
    IOException)
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder()
          ->withTokenizer(WhitespaceTokenizerFactory::typeid,
                          Collections::emptyMap())
          ->addCharFilter(
              MappingCharFilterFactory::typeid,
              unordered_map<>(Collections::singletonMap(
                  L"mapping",
                  L"org/apache/lucene/analysis/custom/mapping1.txt")))
          ->addCharFilter(
              MappingCharFilterFactory::typeid,
              unordered_map<>(Collections::singletonMap(
                  L"mapping",
                  L"org/apache/lucene/analysis/custom/mapping2.txt")))
          ->build();
  assertEquals(make_shared<BytesRef>(L"e f c"),
               analyzer->normalize(L"dummy", L"a b c"));
}

void TestCustomAnalyzer::testNormalizationWithLowerCaseTokenizer() throw(
    IOException)
{
  shared_ptr<CustomAnalyzer> analyzer1 =
      CustomAnalyzer::builder()
          ->withTokenizer(LowerCaseTokenizerFactory::typeid,
                          Collections::emptyMap())
          ->build();
  assertEquals(make_shared<BytesRef>(L"abc"),
               analyzer1->normalize(L"dummy", L"ABC"));
}

void TestCustomAnalyzer::testConditions() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder()
          ->withTokenizer(L"whitespace")
          .addTokenFilter(L"lowercase")
          .whenTerm([&](any t) { t->toString()->contains(L"o"); })
          .addTokenFilter(L"uppercase")
          .addTokenFilter(ReverseStringFilterFactory::typeid)
          .endwhen()
          .addTokenFilter(L"asciifolding")
          .build();

  assertAnalyzesTo(
      analyzer, L"Héllo world whaT's hãppening",
      std::deque<wstring>{L"OLLEH", L"DLROW", L"what's", L"happening"});
}

void TestCustomAnalyzer::testConditionsWithResourceLoader() 
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder()
          ->withTokenizer(L"whitespace")
          .addTokenFilter(L"lowercase")
          .when(L"protectedterm", L"protected",
                L"org/apache/lucene/analysis/custom/teststop.txt")
          .addTokenFilter(L"reversestring")
          .endwhen()
          .build();

  assertAnalyzesTo(analyzer, L"FOO BAR BAZ",
                   std::deque<wstring>{L"foo", L"bar", L"zab"});
}

void TestCustomAnalyzer::testConditionsWithWrappedResourceLoader() throw(
    IOException)
{
  shared_ptr<CustomAnalyzer> analyzer =
      CustomAnalyzer::builder()
          ->withTokenizer(L"whitespace")
          .addTokenFilter(L"lowercase")
          .whenTerm(
              [&](any t) { return t->toString()->contains(L"o") == false; })
          .addTokenFilter(L"stop", L"ignoreCase", L"true", L"words",
                          L"org/apache/lucene/analysis/custom/teststop.txt",
                          L"format", L"wordset")
          .endwhen()
          .build();

  assertAnalyzesTo(analyzer, L"foo bar baz",
                   std::deque<wstring>{L"foo", L"baz"});
}
} // namespace org::apache::lucene::analysis::custom