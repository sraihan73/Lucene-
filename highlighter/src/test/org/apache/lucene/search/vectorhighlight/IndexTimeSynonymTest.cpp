using namespace std;

#include "IndexTimeSynonymTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;

void IndexTimeSynonymTest::testFieldTermStackIndex1wSearch1term() throw(
    runtime_error)
{
  makeIndex1w();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"Mac"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(1, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Mac(11,20,3)", stack->pop()->toString());
}

void IndexTimeSynonymTest::testFieldTermStackIndex1wSearch2terms() throw(
    runtime_error)
{
  makeIndex1w();

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(tq(L"Mac"), Occur::SHOULD);
  bq->add(tq(L"MacBook"), Occur::SHOULD);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(bq->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(1, stack->termList.size());
  shared_ptr<TermInfo> ti = stack->pop();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Mac(11,20,3)", ti->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"MacBook(11,20,3)", ti->getNext()->toString());
  assertSame(ti, ti->getNext()->getNext());
}

void IndexTimeSynonymTest::testFieldTermStackIndex1w2wSearch1term() throw(
    runtime_error)
{
  makeIndex1w2w();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"pc"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(1, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"pc(3,5,1)", stack->pop()->toString());
}

void IndexTimeSynonymTest::testFieldTermStackIndex1w2wSearch1phrase() throw(
    runtime_error)
{
  makeIndex1w2w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"personal", L"computer"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(2, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"personal(3,5,1)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(3,5,2)", stack->pop()->toString());
}

void IndexTimeSynonymTest::testFieldTermStackIndex1w2wSearch1partial() throw(
    runtime_error)
{
  makeIndex1w2w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(tq(L"computer"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(1, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(3,5,2)", stack->pop()->toString());
}

void IndexTimeSynonymTest::
    testFieldTermStackIndex1w2wSearch1term1phrase() 
{
  makeIndex1w2w();

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(tq(L"pc"), Occur::SHOULD);
  bq->add(pqF({L"personal", L"computer"}), Occur::SHOULD);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(bq->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(2, stack->termList.size());
  shared_ptr<TermInfo> ti = stack->pop();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"pc(3,5,1)", ti->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"personal(3,5,1)", ti->getNext()->toString());
  assertSame(ti, ti->getNext()->getNext());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(3,5,2)", stack->pop()->toString());
}

void IndexTimeSynonymTest::testFieldTermStackIndex2w1wSearch1term() throw(
    runtime_error)
{
  makeIndex2w1w();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"pc"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(1, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"pc(3,20,1)", stack->pop()->toString());
}

void IndexTimeSynonymTest::testFieldTermStackIndex2w1wSearch1phrase() throw(
    runtime_error)
{
  makeIndex2w1w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"personal", L"computer"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(2, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"personal(3,20,1)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(3,20,2)", stack->pop()->toString());
}

void IndexTimeSynonymTest::testFieldTermStackIndex2w1wSearch1partial() throw(
    runtime_error)
{
  makeIndex2w1w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(tq(L"computer"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(1, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(3,20,2)", stack->pop()->toString());
}

void IndexTimeSynonymTest::
    testFieldTermStackIndex2w1wSearch1term1phrase() 
{
  makeIndex2w1w();

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(tq(L"pc"), Occur::SHOULD);
  bq->add(pqF({L"personal", L"computer"}), Occur::SHOULD);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(bq->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  assertEquals(2, stack->termList.size());
  shared_ptr<TermInfo> ti = stack->pop();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"pc(3,20,1)", ti->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"personal(3,20,1)", ti->getNext()->toString());
  assertSame(ti, ti->getNext()->getNext());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(3,20,2)", stack->pop()->toString());
}

void IndexTimeSynonymTest::testFieldPhraseListIndex1w2wSearch1phrase() throw(
    runtime_error)
{
  makeIndex1w2w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"personal", L"computer"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"personalcomputer(1.0)((3,5))",
               fpl->phraseList.get(0)->toString());
  assertEquals(3, fpl->phraseList.get(0).getStartOffset());
  assertEquals(5, fpl->phraseList.get(0).getEndOffset());
}

void IndexTimeSynonymTest::testFieldPhraseListIndex1w2wSearch1partial() throw(
    runtime_error)
{
  makeIndex1w2w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(tq(L"computer"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(1.0)((3,5))", fpl->phraseList.get(0)->toString());
  assertEquals(3, fpl->phraseList.get(0).getStartOffset());
  assertEquals(5, fpl->phraseList.get(0).getEndOffset());
}

void IndexTimeSynonymTest::
    testFieldPhraseListIndex1w2wSearch1term1phrase() 
{
  makeIndex1w2w();

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(tq(L"pc"), Occur::SHOULD);
  bq->add(pqF({L"personal", L"computer"}), Occur::SHOULD);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(bq->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(fpl->phraseList.get(0)->toString()->find(L"(1.0)((3,5))") > 0);
  assertEquals(3, fpl->phraseList.get(0).getStartOffset());
  assertEquals(5, fpl->phraseList.get(0).getEndOffset());
}

void IndexTimeSynonymTest::testFieldPhraseListIndex2w1wSearch1term() throw(
    runtime_error)
{
  makeIndex2w1w();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"pc"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"pc(1.0)((3,20))", fpl->phraseList.get(0)->toString());
  assertEquals(3, fpl->phraseList.get(0).getStartOffset());
  assertEquals(20, fpl->phraseList.get(0).getEndOffset());
}

void IndexTimeSynonymTest::testFieldPhraseListIndex2w1wSearch1phrase() throw(
    runtime_error)
{
  makeIndex2w1w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"personal", L"computer"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"personalcomputer(1.0)((3,20))",
               fpl->phraseList.get(0)->toString());
  assertEquals(3, fpl->phraseList.get(0).getStartOffset());
  assertEquals(20, fpl->phraseList.get(0).getEndOffset());
}

void IndexTimeSynonymTest::testFieldPhraseListIndex2w1wSearch1partial() throw(
    runtime_error)
{
  makeIndex2w1w();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(tq(L"computer"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"computer(1.0)((3,20))", fpl->phraseList.get(0)->toString());
  assertEquals(3, fpl->phraseList.get(0).getStartOffset());
  assertEquals(20, fpl->phraseList.get(0).getEndOffset());
}

void IndexTimeSynonymTest::
    testFieldPhraseListIndex2w1wSearch1term1phrase() 
{
  makeIndex2w1w();

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(tq(L"pc"), Occur::SHOULD);
  bq->add(pqF({L"personal", L"computer"}), Occur::SHOULD);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(bq->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(fpl->phraseList.get(0)->toString()->find(L"(1.0)((3,20))") > 0);
  assertEquals(3, fpl->phraseList.get(0).getStartOffset());
  assertEquals(20, fpl->phraseList.get(0).getEndOffset());
}

void IndexTimeSynonymTest::makeIndex1w() 
{
  //           11111111112
  // 012345678901234567890
  // I'll buy a Macintosh
  //            Mac
  //            MacBook
  // 0    1   2 3
  makeSynonymIndex(L"I'll buy a Macintosh",
                   {t(L"I'll", 0, 4), t(L"buy", 5, 8), t(L"a", 9, 10),
                    t(L"Macintosh", 11, 20), t(L"Mac", 11, 20, 0),
                    t(L"MacBook", 11, 20, 0)});
}

void IndexTimeSynonymTest::makeIndex1w2w() 
{
  //           1111111
  // 01234567890123456
  // My pc was broken
  //    personal computer
  // 0  1  2   3
  makeSynonymIndex(L"My pc was broken",
                   {t(L"My", 0, 2), t(L"pc", 3, 5), t(L"personal", 3, 5, 0),
                    t(L"computer", 3, 5), t(L"was", 6, 9),
                    t(L"broken", 10, 16)});
}

void IndexTimeSynonymTest::makeIndex2w1w() 
{
  //           1111111111222222222233
  // 01234567890123456789012345678901
  // My personal computer was broken
  //    pc
  // 0  1        2        3   4
  makeSynonymIndex(L"My personal computer was broken",
                   {t(L"My", 0, 2), t(L"personal", 3, 20), t(L"pc", 3, 20, 0),
                    t(L"computer", 3, 20), t(L"was", 21, 24),
                    t(L"broken", 25, 31)});
}

void IndexTimeSynonymTest::makeSynonymIndex(
    const wstring &value, deque<Token> &tokens) 
{
  shared_ptr<Analyzer> analyzer = make_shared<TokenArrayAnalyzer>(tokens);
  make1dmfIndex(analyzer, value);
}

shared_ptr<Token> IndexTimeSynonymTest::t(const wstring &text, int startOffset,
                                          int endOffset)
{
  return t(text, startOffset, endOffset, 1);
}

shared_ptr<Token> IndexTimeSynonymTest::t(const wstring &text, int startOffset,
                                          int endOffset, int positionIncrement)
{
  shared_ptr<Token> token = make_shared<Token>(text, startOffset, endOffset);
  token->setPositionIncrement(positionIncrement);
  return token;
}

IndexTimeSynonymTest::TokenArrayAnalyzer::TokenArrayAnalyzer(
    deque<Token> &tokens)
    : tokens(tokens)
{
}

shared_ptr<TokenStreamComponents>
IndexTimeSynonymTest::TokenArrayAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> ts = make_shared<TokenizerAnonymousInnerClass>(
      shared_from_this(), Token::TOKEN_ATTRIBUTE_FACTORY);
  return make_shared<TokenStreamComponents>(ts);
}

IndexTimeSynonymTest::TokenArrayAnalyzer::TokenizerAnonymousInnerClass::
    TokenizerAnonymousInnerClass(
        shared_ptr<TokenArrayAnalyzer> outerInstance,
        shared_ptr<org::apache::lucene::util::AttributeFactory>
            TOKEN_ATTRIBUTE_FACTORY)
    : Tokenizer(TOKEN_ATTRIBUTE_FACTORY)
{
  this->outerInstance = outerInstance;
  reusableToken = std::static_pointer_cast<AttributeImpl>(
      addAttribute(CharTermAttribute::typeid));
  p = 0;
}

bool IndexTimeSynonymTest::TokenArrayAnalyzer::TokenizerAnonymousInnerClass::
    incrementToken()
{
  if (p >= outerInstance->tokens.size()) {
    return false;
  }
  clearAttributes();
  outerInstance->tokens[p++]->copyTo(reusableToken);
  return true;
}

void IndexTimeSynonymTest::TokenArrayAnalyzer::TokenizerAnonymousInnerClass::
    reset() 
{
  outerInstance->super->reset();
  this->p = 0;
}
} // namespace org::apache::lucene::search::vectorhighlight