using namespace std;

#include "TestMultiAnalyzer.h"

namespace org::apache::lucene::queryparser::classic
{
using namespace org::apache::lucene::analysis;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using Query = org::apache::lucene::search::Query;
int TestMultiAnalyzer::multiToken = 0;

void TestMultiAnalyzer::testMultiAnalyzer() 
{

  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(L"", make_shared<MultiAnalyzer>());

  // trivial, no multiple tokens:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo", qp->parse(L"foo")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo", qp->parse(L"\"foo\"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo foobar", qp->parse(L"foo foobar")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo foobar\"", qp->parse(L"\"foo foobar\"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo foobar blah\"",
               qp->parse(L"\"foo foobar blah\"")->toString());

  // two tokens at the same position:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym(multi multi2) foo",
               qp->parse(L"multi foo")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo Synonym(multi multi2)",
               qp->parse(L"foo multi")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym(multi multi2) Synonym(multi multi2)",
               qp->parse(L"multi multi")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(foo Synonym(multi multi2)) +(bar Synonym(multi multi2))",
               qp->parse(L"+(foo multi) +(bar multi)")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+(foo Synonym(multi multi2)) field:\"bar (multi multi2)\"",
               qp->parse(L"+(foo multi) field:\"bar multi\"")->toString());

  // phrases:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"",
               qp->parse(L"\"multi foo\"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo (multi multi2)\"",
               qp->parse(L"\"foo multi\"")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo (multi multi2) foobar (multi multi2)\"",
               qp->parse(L"\"foo multi foobar multi\"")->toString());

  // fields:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym(field:multi field:multi2) field:foo",
               qp->parse(L"field:multi field:foo")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:\"(multi multi2) foo\"",
               qp->parse(L"field:\"multi foo\"")->toString());

  // three tokens at one position:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym(multi2 multi3 triplemulti)",
               qp->parse(L"triplemulti")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo Synonym(multi2 multi3 triplemulti) foobar",
               qp->parse(L"foo triplemulti foobar")->toString());

  // phrase with non-default slop:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"~10",
               qp->parse(L"\"multi foo\"~10")->toString());

  // phrase with non-default boost:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(\"(multi multi2) foo\")^2.0",
               qp->parse(L"\"multi foo\"^2")->toString());

  // phrase after changing default slop
  qp->setPhraseSlop(99);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"~99 bar",
               qp->parse(L"\"multi foo\" bar")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"~99 \"foo bar\"~2",
               qp->parse(L"\"multi foo\" \"foo bar\"~2")->toString());
  qp->setPhraseSlop(0);

  // non-default operator:
  qp->setDefaultOperator(QueryParserBase::AND_OPERATOR);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"+Synonym(multi multi2) +foo",
               qp->parse(L"multi foo")->toString());
}

void TestMultiAnalyzer::testMultiAnalyzerWithSubclassOfQueryParser() throw(
    ParseException)
{

  shared_ptr<DumbQueryParser> qp =
      make_shared<DumbQueryParser>(L"", make_shared<MultiAnalyzer>());
  qp->setPhraseSlop(99); // modified default slop

  // direct call to (super's) getFieldQuery to demonstrate differnce
  // between phrase and multiphrase with modified default slop
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"foo bar\"~99",
               qp->getSuperFieldQuery(L"", L"foo bar", true)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) bar\"~99",
               qp->getSuperFieldQuery(L"", L"multi bar", true)->toString());

  // ask sublcass to parse phrase with modified default slop
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"\"(multi multi2) foo\"~99 bar",
               qp->parse(L"\"multi foo\" bar")->toString());
}

void TestMultiAnalyzer::testPosIncrementAnalyzer() 
{
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(L"", make_shared<PosIncrementAnalyzer>());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"quick brown", qp->parse(L"the quick brown")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"quick brown fox",
               qp->parse(L"the quick brown fox")->toString());
}

shared_ptr<TokenStreamComponents>
TestMultiAnalyzer::MultiAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(result,
                                            make_shared<TestFilter>(result));
}

TestMultiAnalyzer::TestFilter::TestFilter(shared_ptr<TokenStream> in_)
    : TokenFilter(in_), termAtt(addAttribute(CharTermAttribute::typeid)),
      posIncrAtt(addAttribute(PositionIncrementAttribute::typeid)),
      offsetAtt(addAttribute(OffsetAttribute::typeid)),
      typeAtt(addAttribute(TypeAttribute::typeid))
{
}

bool TestMultiAnalyzer::TestFilter::incrementToken() 
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

void TestMultiAnalyzer::TestFilter::reset() 
{
  TokenFilter::reset();
  this->prevType = L"";
  this->prevStartOffset = 0;
  this->prevEndOffset = 0;
}

shared_ptr<TokenStreamComponents>
TestMultiAnalyzer::PosIncrementAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(
      result, make_shared<TestPosIncrementFilter>(result));
}

TestMultiAnalyzer::TestPosIncrementFilter::TestPosIncrementFilter(
    shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
}

bool TestMultiAnalyzer::TestPosIncrementFilter::incrementToken() throw(
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

TestMultiAnalyzer::DumbQueryParser::DumbQueryParser(const wstring &f,
                                                    shared_ptr<Analyzer> a)
    : QueryParser(f, a)
{
}

shared_ptr<Query> TestMultiAnalyzer::DumbQueryParser::getSuperFieldQuery(
    const wstring &f, const wstring &t, bool quoted) 
{
  return QueryParser::getFieldQuery(f, t, quoted);
}

shared_ptr<Query> TestMultiAnalyzer::DumbQueryParser::getFieldQuery(
    const wstring &f, const wstring &t, bool quoted) 
{
  return make_shared<DumbQueryWrapper>(getSuperFieldQuery(f, t, quoted));
}

TestMultiAnalyzer::DumbQueryWrapper::DumbQueryWrapper(shared_ptr<Query> q)
{
  this->q = Objects::requireNonNull(q);
}

wstring TestMultiAnalyzer::DumbQueryWrapper::toString(const wstring &f)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return q->toString(f);
}

bool TestMultiAnalyzer::DumbQueryWrapper::equals(any other)
{
  return sameClassAs(other) &&
         Objects::equals(
             q, (any_cast<std::shared_ptr<DumbQueryWrapper>>(other)).q);
}

int TestMultiAnalyzer::DumbQueryWrapper::hashCode()
{
  return classHash() & q->hashCode();
}
} // namespace org::apache::lucene::queryparser::classic