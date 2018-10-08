using namespace std;

#include "TestMultiAnalyzerQPHelper.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
int TestMultiAnalyzerQPHelper::multiToken = 0;

void TestMultiAnalyzerQPHelper::testMultiAnalyzer() 
{

  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(make_shared<MultiAnalyzer>());

  // trivial, no multiple tokens:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo", qp->parse(L"foo", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo", qp->parse(L"\"foo\"", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo foobar", qp->parse(L"foo foobar", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo foobar\"",
               qp->parse(L"\"foo foobar\"", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo foobar blah\"",
               qp->parse(L"\"foo foobar blah\"", L"")->toString());

  // two tokens at the same position:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(multi multi2) foo", qp->parse(L"multi foo", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo (multi multi2)", qp->parse(L"foo multi", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(multi multi2) (multi multi2)",
               qp->parse(L"multi multi", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(foo (multi multi2)) +(bar (multi multi2))",
               qp->parse(L"+(foo multi) +(bar multi)", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(foo (multi multi2)) field:\"bar (multi multi2)\"",
               qp->parse(L"+(foo multi) field:\"bar multi\"", L"")->toString());

  // phrases:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"",
               qp->parse(L"\"multi foo\"", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo (multi multi2)\"",
               qp->parse(L"\"foo multi\"", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo (multi multi2) foobar (multi multi2)\"",
               qp->parse(L"\"foo multi foobar multi\"", L"")->toString());

  // fields:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(field:multi field:multi2) field:foo",
               qp->parse(L"field:multi field:foo", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:\"(multi multi2) foo\"",
               qp->parse(L"field:\"multi foo\"", L"")->toString());

  // three tokens at one position:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"triplemulti multi3 multi2",
               qp->parse(L"triplemulti", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo (triplemulti multi3 multi2) foobar",
               qp->parse(L"foo triplemulti foobar", L"")->toString());

  // phrase with non-default slop:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"~10",
               qp->parse(L"\"multi foo\"~10", L"")->toString());

  // phrase with non-default boost:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(\"(multi multi2) foo\")^2.0",
               qp->parse(L"\"multi foo\"^2", L"")->toString());

  // phrase after changing default slop
  qp->setPhraseSlop(99);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"~99 bar",
               qp->parse(L"\"multi foo\" bar", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"~99 \"foo bar\"~2",
               qp->parse(L"\"multi foo\" \"foo bar\"~2", L"")->toString());
  qp->setPhraseSlop(0);

  // non-default operator:
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(multi multi2) +foo",
               qp->parse(L"multi foo", L"")->toString());
}

void TestMultiAnalyzerQPHelper::testPosIncrementAnalyzer() throw(
    QueryNodeException)
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(make_shared<PosIncrementAnalyzer>());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"quick brown", qp->parse(L"the quick brown", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"? quick brown\"",
               qp->parse(L"\"the quick brown\"", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"quick brown fox",
               qp->parse(L"the quick brown fox", L"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"? quick brown fox\"",
               qp->parse(L"\"the quick brown fox\"", L"")->toString());
}

shared_ptr<TokenStreamComponents>
TestMultiAnalyzerQPHelper::MultiAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(result,
                                            make_shared<TestFilter>(result));
}

TestMultiAnalyzerQPHelper::TestFilter::TestFilter(shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
}

bool TestMultiAnalyzerQPHelper::TestFilter::incrementToken() 
{
  if (multiToken > 0) {
    termAtt->setEmpty()->append(L"multi" + to_wstring(multiToken + 1));
    offsetAtt->setOffset(prevStartOffset, prevEndOffset);
    typeAtt->setType(prevType);
    posIncrAtt->setPositionIncrement(0);
    multiToken--;
    return true;
  } else {
    bool next = input->incrementToken();
    if (!next) {
      return false;
    }
    prevType = typeAtt->type();
    prevStartOffset = offsetAtt->startOffset();
    prevEndOffset = offsetAtt->endOffset();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring text = termAtt->toString();
    if (text == L"triplemulti") {
      multiToken = 2;
      return true;
    } else if (text == L"multi") {
      multiToken = 1;
      return true;
    } else {
      return true;
    }
  }
}

void TestMultiAnalyzerQPHelper::TestFilter::reset() 
{
  TokenFilter::reset();
  this->prevType = L"";
  this->prevStartOffset = 0;
  this->prevEndOffset = 0;
}

shared_ptr<TokenStreamComponents>
TestMultiAnalyzerQPHelper::PosIncrementAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(
      result, make_shared<TestPosIncrementFilter>(result));
}

TestMultiAnalyzerQPHelper::TestPosIncrementFilter::TestPosIncrementFilter(
    shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
}

bool TestMultiAnalyzerQPHelper::TestPosIncrementFilter::incrementToken() throw(
    IOException)
{
  while (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (termAtt->toString()->equals(L"the")) {
      // stopword, do nothing
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    else if (termAtt->toString()->equals(L"quick")) {
      posIncrAtt->setPositionIncrement(2);
      return true;
    } else {
      posIncrAtt->setPositionIncrement(1);
      return true;
    }
  }
  return false;
}
} // namespace org::apache::lucene::queryparser::flexible::standard