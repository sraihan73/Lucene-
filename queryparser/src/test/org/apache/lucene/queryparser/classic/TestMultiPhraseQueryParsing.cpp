using namespace std;

#include "TestMultiPhraseQueryParsing.h"

namespace org::apache::lucene::queryparser::classic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Term = org::apache::lucene::index::Term;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestMultiPhraseQueryParsing::TokenAndPos::TokenAndPos(const wstring &token,
                                                      int pos)
    : token(token), pos(pos)
{
}

TestMultiPhraseQueryParsing::CannedAnalyzer::CannedAnalyzer(
    std::deque<std::shared_ptr<TokenAndPos>> &tokens)
    : tokens(tokens)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMultiPhraseQueryParsing::CannedAnalyzer::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<CannedTokenizer>(tokens));
}

TestMultiPhraseQueryParsing::CannedTokenizer::CannedTokenizer(
    std::deque<std::shared_ptr<TokenAndPos>> &tokens)
    : org::apache::lucene::analysis::Tokenizer(), tokens(tokens)
{
}

bool TestMultiPhraseQueryParsing::CannedTokenizer::incrementToken()
{
  clearAttributes();
  if (upto < tokens.size()) {
    shared_ptr<TokenAndPos> *const token = tokens[upto++];
    termAtt->setEmpty();
    termAtt->append(token->token);
    posIncrAtt->setPositionIncrement(token->pos - lastPos);
    lastPos = token->pos;
    return true;
  } else {
    return false;
  }
}

void TestMultiPhraseQueryParsing::CannedTokenizer::reset() 
{
  Tokenizer::reset();
  this->upto = 0;
  this->lastPos = 0;
}

void TestMultiPhraseQueryParsing::testMultiPhraseQueryParsing() throw(
    runtime_error)
{
  std::deque<std::shared_ptr<TokenAndPos>> INCR_0_QUERY_TOKENS_AND = {
      make_shared<TokenAndPos>(L"a", 0), make_shared<TokenAndPos>(L"1", 0),
      make_shared<TokenAndPos>(L"b", 1), make_shared<TokenAndPos>(L"1", 1),
      make_shared<TokenAndPos>(L"c", 2)};

  shared_ptr<QueryParser> qp = make_shared<QueryParser>(
      L"field", make_shared<CannedAnalyzer>(INCR_0_QUERY_TOKENS_AND));
  shared_ptr<Query> q = qp->parse(L"\"this text is acually ignored\"");
  assertTrue(L"wrong query type!",
             std::dynamic_pointer_cast<MultiPhraseQuery>(q) != nullptr);

  shared_ptr<MultiPhraseQuery::Builder> multiPhraseQueryBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  multiPhraseQueryBuilder->add(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"a"),
                                         make_shared<Term>(L"field", L"1")},
      -1);
  multiPhraseQueryBuilder->add(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"b"),
                                         make_shared<Term>(L"field", L"1")},
      0);
  multiPhraseQueryBuilder->add(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"c")}, 1);

  assertEquals(multiPhraseQueryBuilder->build(), q);
}
} // namespace org::apache::lucene::queryparser::classic