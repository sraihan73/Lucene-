using namespace std;

#include "TestTypeTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/English.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/TypeTokenFilter.h"

namespace org::apache::lucene::analysis::core
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using English = org::apache::lucene::util::English;

void TestTypeTokenFilter::testTypeFilter() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"121 is palindrome, while 123 is not");
  shared_ptr<Set<wstring>> stopTypes = asSet({L"<NUM>"});
  shared_ptr<StandardTokenizer> *const input =
      make_shared<StandardTokenizer>(newAttributeFactory());
  input->setReader(reader);
  shared_ptr<TokenStream> stream =
      make_shared<TypeTokenFilter>(input, stopTypes);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"is", L"palindrome", L"while", L"is", L"not"});
}

void TestTypeTokenFilter::testStopPositons() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 10; i < 20; i++) {
    if (i % 3 != 0) {
      sb->append(i)->append(L" ");
    } else {
      wstring w = StringHelper::trim(English::intToEnglish(i));
      sb->append(w)->append(L" ");
    }
  }
  log(sb->toString());
  std::deque<wstring> stopTypes = {L"<NUM>"};
  shared_ptr<Set<wstring>> stopSet = asSet(stopTypes);

  // with increments
  shared_ptr<StringReader> reader = make_shared<StringReader>(sb->toString());
  shared_ptr<StandardTokenizer> *const input = make_shared<StandardTokenizer>();
  input->setReader(reader);
  shared_ptr<TypeTokenFilter> typeTokenFilter =
      make_shared<TypeTokenFilter>(input, stopSet);
  testPositons(typeTokenFilter);
}

void TestTypeTokenFilter::testPositons(shared_ptr<TypeTokenFilter> stpf) throw(
    IOException)
{
  shared_ptr<TypeAttribute> typeAtt = stpf->getAttribute(TypeAttribute::typeid);
  shared_ptr<CharTermAttribute> termAttribute =
      stpf->getAttribute(CharTermAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> posIncrAtt =
      stpf->getAttribute(PositionIncrementAttribute::typeid);
  stpf->reset();
  while (stpf->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    log(L"Token: " + termAttribute->toString() + L": " + typeAtt->type() +
        L" - " + to_wstring(posIncrAtt->getPositionIncrement()));
    assertEquals(L"if position increment is enabled the "
                 L"positionIncrementAttribute value should be 3, otherwise 1",
                 posIncrAtt->getPositionIncrement(), 3);
  }
  stpf->end();
  delete stpf;
}

void TestTypeTokenFilter::testTypeFilterWhitelist() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"121 is palindrome, while 123 is not");
  shared_ptr<Set<wstring>> stopTypes = Collections::singleton(L"<NUM>");
  shared_ptr<StandardTokenizer> *const input =
      make_shared<StandardTokenizer>(newAttributeFactory());
  input->setReader(reader);
  shared_ptr<TokenStream> stream =
      make_shared<TypeTokenFilter>(input, stopTypes, true);
  assertTokenStreamContents(stream, std::deque<wstring>{L"121", L"123"});
}

void TestTypeTokenFilter::log(const wstring &s)
{
  if (VERBOSE) {
    wcout << s << endl;
  }
}
} // namespace org::apache::lucene::analysis::core