using namespace std;

#include "QueryParser.h"

namespace org::apache::lucene::queryparser::surround::parser
{
using SrndQuery = org::apache::lucene::queryparser::surround::query::SrndQuery;
using FieldsQuery =
    org::apache::lucene::queryparser::surround::query::FieldsQuery;
using OrQuery = org::apache::lucene::queryparser::surround::query::OrQuery;
using AndQuery = org::apache::lucene::queryparser::surround::query::AndQuery;
using NotQuery = org::apache::lucene::queryparser::surround::query::NotQuery;
using DistanceQuery =
    org::apache::lucene::queryparser::surround::query::DistanceQuery;
using SrndTermQuery =
    org::apache::lucene::queryparser::surround::query::SrndTermQuery;
using SrndPrefixQuery =
    org::apache::lucene::queryparser::surround::query::SrndPrefixQuery;
using SrndTruncQuery =
    org::apache::lucene::queryparser::surround::query::SrndTruncQuery;
const wstring QueryParser::TRUNCATION_ERROR_MESSAGE =
    L"Too unrestrictive truncation: ";
const wstring QueryParser::BOOST_ERROR_MESSAGE = L"Cannot handle boost value: ";

shared_ptr<SrndQuery>
QueryParser::parse(const wstring &query) 
{
  shared_ptr<QueryParser> parser = make_shared<QueryParser>();
  return parser->parse2(query);
}

QueryParser::QueryParser()
    : QueryParser(new FastCharStream(new StringReader(L"")))
{
}

shared_ptr<SrndQuery>
QueryParser::parse2(const wstring &query) 
{
  ReInit(make_shared<FastCharStream>(make_shared<StringReader>(query)));
  try {
    return TopSrndQuery();
  } catch (const TokenMgrError &tme) {
    throw make_shared<ParseException>(tme->getMessage());
  }
}

shared_ptr<SrndQuery> QueryParser::getFieldsQuery(shared_ptr<SrndQuery> q,
                                                  deque<wstring> &fieldNames)
{
  /* FIXME: check acceptable subquery: at least one subquery should not be
   * a fields query.
   */
  return make_shared<FieldsQuery>(q, fieldNames, FIELD_OPERATOR);
}

shared_ptr<SrndQuery>
QueryParser::getOrQuery(deque<std::shared_ptr<SrndQuery>> &queries, bool infix,
                        shared_ptr<Token> orToken)
{
  return make_shared<OrQuery>(queries, infix, orToken->image);
}

shared_ptr<SrndQuery>
QueryParser::getAndQuery(deque<std::shared_ptr<SrndQuery>> &queries,
                         bool infix, shared_ptr<Token> andToken)
{
  return make_shared<AndQuery>(queries, infix, andToken->image);
}

shared_ptr<SrndQuery>
QueryParser::getNotQuery(deque<std::shared_ptr<SrndQuery>> &queries,
                         shared_ptr<Token> notToken)
{
  return make_shared<NotQuery>(queries, notToken->image);
}

int QueryParser::getOpDistance(const wstring &distanceOp)
{
  /* W, 2W, 3W etc -> 1, 2 3, etc. Same for N, 2N ... */
  return distanceOp.length() == 1
             ? 1
             : stoi(distanceOp.substr(0, distanceOp.length() - 1));
}

void QueryParser::checkDistanceSubQueries(
    shared_ptr<DistanceQuery> distq,
    const wstring &opName) 
{
  wstring m = distq->distanceSubQueryNotAllowed();
  if (m != L"") {
    throw make_shared<ParseException>(L"Operator " + opName + L": " + m);
  }
}

shared_ptr<SrndQuery>
QueryParser::getDistanceQuery(deque<std::shared_ptr<SrndQuery>> &queries,
                              bool infix, shared_ptr<Token> dToken,
                              bool ordered) 
{
  shared_ptr<DistanceQuery> dq = make_shared<DistanceQuery>(
      queries, infix, getOpDistance(dToken->image), dToken->image, ordered);
  checkDistanceSubQueries(dq, dToken->image);
  return dq;
}

shared_ptr<SrndQuery> QueryParser::getTermQuery(const wstring &term,
                                                bool quoted)
{
  return make_shared<SrndTermQuery>(term, quoted);
}

bool QueryParser::allowedSuffix(const wstring &suffixed)
{
  return (suffixed.length() - 1) >= MINIMUM_PREFIX_LENGTH;
}

shared_ptr<SrndQuery> QueryParser::getPrefixQuery(const wstring &prefix,
                                                  bool quoted)
{
  return make_shared<SrndPrefixQuery>(prefix, quoted, TRUNCATOR);
}

bool QueryParser::allowedTruncation(const wstring &truncated)
{
  /* At least 3 normal characters needed. */
  int nrNormalChars = 0;
  for (int i = 0; i < truncated.length(); i++) {
    wchar_t c = truncated[i];
    if ((c != TRUNCATOR) && (c != ANY_CHAR)) {
      nrNormalChars++;
    }
  }
  return nrNormalChars >= MINIMUM_CHARS_IN_TRUNC;
}

shared_ptr<SrndQuery> QueryParser::getTruncQuery(const wstring &truncated)
{
  return make_shared<SrndTruncQuery>(truncated, TRUNCATOR, ANY_CHAR);
}

shared_ptr<SrndQuery> QueryParser::TopSrndQuery() 
{
  shared_ptr<SrndQuery> q;
  q = FieldsQuery();
  jj_consume_token(0);
  {
    if (true) {
      return q;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::FieldsQuery() 
{
  shared_ptr<SrndQuery> q;
  deque<wstring> fieldNames;
  fieldNames = OptionalFields();
  q = OrQuery();
  {
    if (true) {
      return (fieldNames.empty()) ? q : getFieldsQuery(q, fieldNames);
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

deque<wstring> QueryParser::OptionalFields() 
{
  shared_ptr<Token> fieldName;
  deque<wstring> fieldNames;
  while (true) {
    if (jj_2_1(2)) {
      ;
    } else {
      goto label_1Break;
    }
    // to the colon
    fieldName = jj_consume_token(TERM);
    jj_consume_token(COLON);
    if (fieldNames.empty()) {
      fieldNames = deque<wstring>();
    }
    fieldNames.push_back(fieldName->image);
  label_1Continue:;
  }
label_1Break : {
  if (true) {
    return fieldNames;
  }
}
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::OrQuery() 
{
  shared_ptr<SrndQuery> q;
  deque<std::shared_ptr<SrndQuery>> queries;
  shared_ptr<Token> oprt = nullptr;
  q = AndQuery();
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case OR:;
      break;
    default:
      jj_la1[0] = jj_gen;
      goto label_2Break;
    }
    oprt = jj_consume_token(OR);
    /* keep only last used operator */
    if (queries.empty()) {
      queries = deque<std::shared_ptr<SrndQuery>>();
      queries.push_back(q);
    }
    q = AndQuery();
    queries.push_back(q);
  label_2Continue:;
  }
label_2Break : {
  if (true) {
    return (queries.empty()) ? q : getOrQuery(queries, true, oprt);
  }
}
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::AndQuery() 
{
  shared_ptr<SrndQuery> q;
  deque<std::shared_ptr<SrndQuery>> queries;
  shared_ptr<Token> oprt = nullptr;
  q = NotQuery();
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case AND:;
      break;
    default:
      jj_la1[1] = jj_gen;
      goto label_3Break;
    }
    oprt = jj_consume_token(AND);
    /* keep only last used operator */
    if (queries.empty()) {
      queries = deque<std::shared_ptr<SrndQuery>>();
      queries.push_back(q);
    }
    q = NotQuery();
    queries.push_back(q);
  label_3Continue:;
  }
label_3Break : {
  if (true) {
    return (queries.empty()) ? q : getAndQuery(queries, true, oprt);
  }
}
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::NotQuery() 
{
  shared_ptr<SrndQuery> q;
  deque<std::shared_ptr<SrndQuery>> queries;
  shared_ptr<Token> oprt = nullptr;
  q = NQuery();
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case NOT:;
      break;
    default:
      jj_la1[2] = jj_gen;
      goto label_4Break;
    }
    oprt = jj_consume_token(NOT);
    /* keep only last used operator */
    if (queries.empty()) {
      queries = deque<std::shared_ptr<SrndQuery>>();
      queries.push_back(q);
    }
    q = NQuery();
    queries.push_back(q);
  label_4Continue:;
  }
label_4Break : {
  if (true) {
    return (queries.empty()) ? q : getNotQuery(queries, oprt);
  }
}
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::NQuery() 
{
  shared_ptr<SrndQuery> q;
  deque<std::shared_ptr<SrndQuery>> queries;
  shared_ptr<Token> dt;
  q = WQuery();
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case N:;
      break;
    default:
      jj_la1[3] = jj_gen;
      goto label_5Break;
    }
    dt = jj_consume_token(N);
    queries = deque<std::shared_ptr<SrndQuery>>();
    queries.push_back(q); // left associative

    q = WQuery();
    queries.push_back(q);
    q = getDistanceQuery(queries, true, dt, false);
  label_5Continue:;
  }
label_5Break : {
  if (true) {
    return q;
  }
}
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::WQuery() 
{
  shared_ptr<SrndQuery> q;
  deque<std::shared_ptr<SrndQuery>> queries;
  shared_ptr<Token> wt;
  q = PrimaryQuery();
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case W:;
      break;
    default:
      jj_la1[4] = jj_gen;
      goto label_6Break;
    }
    wt = jj_consume_token(W);
    queries = deque<std::shared_ptr<SrndQuery>>();
    queries.push_back(q); // left associative

    q = PrimaryQuery();
    queries.push_back(q);
    q = getDistanceQuery(queries, true, wt, true);
  label_6Continue:;
  }
label_6Break : {
  if (true) {
    return q;
  }
}
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::PrimaryQuery() 
{
  /* bracketed weighted query or weighted term */
  shared_ptr<SrndQuery> q;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case LPAREN:
    jj_consume_token(LPAREN);
    q = FieldsQuery();
    jj_consume_token(RPAREN);
    break;
  case OR:
  case AND:
  case W:
  case N:
    q = PrefixOperatorQuery();
    break;
  case TRUNCQUOTED:
  case QUOTED:
  case SUFFIXTERM:
  case TRUNCTERM:
  case TERM:
    q = SimpleTerm();
    break;
  default:
    jj_la1[5] = jj_gen;
    jj_consume_token(-1);
    throw make_shared<ParseException>();
  }
  OptionalWeights(q);
  {
    if (true) {
      return q;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::PrefixOperatorQuery() 
{
  shared_ptr<Token> oprt;
  deque<std::shared_ptr<SrndQuery>> queries;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case OR:
    oprt = jj_consume_token(OR);
    /* prefix OR */
    queries = FieldsQueryList();
    {
      if (true) {
        return getOrQuery(queries, false, oprt);
      }
    }
    break;
  case AND:
    oprt = jj_consume_token(AND);
    /* prefix AND */
    queries = FieldsQueryList();
    {
      if (true) {
        return getAndQuery(queries, false, oprt);
      }
    }
    break;
  case N:
    oprt = jj_consume_token(N);
    /* prefix N */
    queries = FieldsQueryList();
    {
      if (true) {
        return getDistanceQuery(queries, false, oprt, false);
      }
    }
    break;
  case W:
    oprt = jj_consume_token(W);
    /* prefix W */
    queries = FieldsQueryList();
    {
      if (true) {
        return getDistanceQuery(queries, false, oprt, true);
      }
    }
    break;
  default:
    jj_la1[6] = jj_gen;
    jj_consume_token(-1);
    throw make_shared<ParseException>();
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

deque<std::shared_ptr<SrndQuery>>
QueryParser::FieldsQueryList() 
{
  shared_ptr<SrndQuery> q;
  deque<std::shared_ptr<SrndQuery>> queries =
      deque<std::shared_ptr<SrndQuery>>();
  jj_consume_token(LPAREN);
  q = FieldsQuery();
  queries.push_back(q);
  while (true) {
    jj_consume_token(COMMA);
    q = FieldsQuery();
    queries.push_back(q);
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case COMMA:;
      break;
    default:
      jj_la1[7] = jj_gen;
      goto label_7Break;
    }
  label_7Continue:;
  }
label_7Break:
  jj_consume_token(RPAREN);
  {
    if (true) {
      return queries;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<SrndQuery> QueryParser::SimpleTerm() 
{
  shared_ptr<Token> term;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case TERM:
    term = jj_consume_token(TERM);
    {
      if (true) {
        return getTermQuery(term->image, false);
      }
    }
    break;
  case QUOTED:
    term = jj_consume_token(QUOTED);
    {
      if (true) {
        return getTermQuery(
            term->image.substr(1, (term->image.length() - 1) - 1), true);
      }
    }
    break;
  case SUFFIXTERM:
    term = jj_consume_token(SUFFIXTERM);
    /* ending in * */
    if (!allowedSuffix(term->image)) {
      {
        if (true) {
          throw make_shared<ParseException>(TRUNCATION_ERROR_MESSAGE +
                                            term->image);
        }
      }
    }
    {
      if (true) {
        return getPrefixQuery(term->image.substr(0, term->image.length() - 1),
                              false);
      }
    }
    break;
  case TRUNCTERM:
    term = jj_consume_token(TRUNCTERM);
    /* with at least one * or ? */
    if (!allowedTruncation(term->image)) {
      {
        if (true) {
          throw make_shared<ParseException>(TRUNCATION_ERROR_MESSAGE +
                                            term->image);
        }
      }
    }
    {
      if (true) {
        return getTruncQuery(term->image);
      }
    }
    break;
  case TRUNCQUOTED:
    term = jj_consume_token(TRUNCQUOTED);
    /* eg. "9b-b,m"* */
    if ((term->image.length() - 3) < MINIMUM_PREFIX_LENGTH) {
      {
        if (true) {
          throw make_shared<ParseException>(TRUNCATION_ERROR_MESSAGE +
                                            term->image);
        }
      }
    }
    {
      if (true) {
        return getPrefixQuery(
            term->image.substr(1, (term->image.length() - 2) - 1), true);
      }
    }
    break;
  default:
    jj_la1[8] = jj_gen;
    jj_consume_token(-1);
    throw make_shared<ParseException>();
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

void QueryParser::OptionalWeights(shared_ptr<SrndQuery> q) 
{
  shared_ptr<Token> weight = nullptr;
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case CARAT:;
      break;
    default:
      jj_la1[9] = jj_gen;
      goto label_8Break;
    }
    jj_consume_token(CARAT);
    weight = jj_consume_token(NUMBER);
    float f;
    try {
      f = stof(weight->image);
    } catch (const runtime_error &floatExc) {
      {
        if (true) {
          throw make_shared<ParseException>(
              BOOST_ERROR_MESSAGE + weight->image + L" (" + floatExc + L")");
        }
      }
    }
    if (f <= 0.0) {
      {
        if (true) {
          throw make_shared<ParseException>(BOOST_ERROR_MESSAGE +
                                            weight->image);
        }
      }
    }
    q->setWeight(f * q->getWeight()); // left associative, fwiw

  label_8Continue:;
  }
label_8Break:;
}

bool QueryParser::jj_2_1(int xla)
{
  jj_la = xla;
  jj_lastpos = jj_scanpos = token;
  try {
    return !jj_3_1();
  } catch (const LookaheadSuccess &ls) {
    return true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    jj_save(0, xla);
  }
}

bool QueryParser::jj_3_1()
{
  if (jj_scan_token(TERM)) {
    return true;
  }
  if (jj_scan_token(COLON)) {
    return true;
  }
  return false;
}

std::deque<int> QueryParser::jj_la1_0;

QueryParser::StaticConstructor::StaticConstructor() { jj_la1_init_0(); }

QueryParser::StaticConstructor QueryParser::staticConstructor;

void QueryParser::jj_la1_init_0()
{
  jj_la1_0 = std::deque<int>{0x100,    0x200,  0x400,  0x1000,   0x800,
                              0x7c3b00, 0x1b00, 0x8000, 0x7c0000, 0x20000};
}

QueryParser::QueryParser(shared_ptr<CharStream> stream)
{
  token_source = make_shared<QueryParserTokenManager>(stream);
  token = make_shared<Token>();
  jj_ntk_ = -1;
  jj_gen = 0;
  for (int i = 0; i < 10; i++) {
    jj_la1[i] = -1;
  }
  for (int i = 0; i < jj_2_rtns.size(); i++) {
    jj_2_rtns[i] = make_shared<JJCalls>();
  }
}

void QueryParser::ReInit(shared_ptr<CharStream> stream)
{
  token_source->ReInit(stream);
  token = make_shared<Token>();
  jj_ntk_ = -1;
  jj_gen = 0;
  for (int i = 0; i < 10; i++) {
    jj_la1[i] = -1;
  }
  for (int i = 0; i < jj_2_rtns.size(); i++) {
    jj_2_rtns[i] = make_shared<JJCalls>();
  }
}

QueryParser::QueryParser(shared_ptr<QueryParserTokenManager> tm)
{
  token_source = tm;
  token = make_shared<Token>();
  jj_ntk_ = -1;
  jj_gen = 0;
  for (int i = 0; i < 10; i++) {
    jj_la1[i] = -1;
  }
  for (int i = 0; i < jj_2_rtns.size(); i++) {
    jj_2_rtns[i] = make_shared<JJCalls>();
  }
}

void QueryParser::ReInit(shared_ptr<QueryParserTokenManager> tm)
{
  token_source = tm;
  token = make_shared<Token>();
  jj_ntk_ = -1;
  jj_gen = 0;
  for (int i = 0; i < 10; i++) {
    jj_la1[i] = -1;
  }
  for (int i = 0; i < jj_2_rtns.size(); i++) {
    jj_2_rtns[i] = make_shared<JJCalls>();
  }
}

shared_ptr<Token> QueryParser::jj_consume_token(int kind) 
{
  shared_ptr<Token> oldToken;
  if ((oldToken = token)->next != nullptr) {
    token = token->next;
  } else {
    token = token->next = token_source->getNextToken();
  }
  jj_ntk_ = -1;
  if (token->kind == kind) {
    jj_gen++;
    if (++jj_gc > 100) {
      jj_gc = 0;
      for (int i = 0; i < jj_2_rtns.size(); i++) {
        shared_ptr<JJCalls> c = jj_2_rtns[i];
        while (c != nullptr) {
          if (c->gen < jj_gen) {
            c->first.reset();
          }
          c = c->next;
        }
      }
    }
    return token;
  }
  token = oldToken;
  jj_kind = kind;
  throw generateParseException();
}

bool QueryParser::jj_scan_token(int kind)
{
  if (jj_scanpos == jj_lastpos) {
    jj_la--;
    if (jj_scanpos->next == nullptr) {
      jj_lastpos = jj_scanpos = jj_scanpos->next = token_source->getNextToken();
    } else {
      jj_lastpos = jj_scanpos = jj_scanpos->next;
    }
  } else {
    jj_scanpos = jj_scanpos->next;
  }
  if (jj_rescan) {
    int i = 0;
    shared_ptr<Token> tok = token;
    while (tok != nullptr && tok != jj_scanpos) {
      i++;
      tok = tok->next;
    }
    if (tok != nullptr) {
      jj_add_error_token(kind, i);
    }
  }
  if (jj_scanpos->kind != kind) {
    return true;
  }
  if (jj_la == 0 && jj_scanpos == jj_lastpos) {
    throw jj_ls;
  }
  return false;
}

shared_ptr<Token> QueryParser::getNextToken()
{
  if (token->next != nullptr) {
    token = token->next;
  } else {
    token = token->next = token_source->getNextToken();
  }
  jj_ntk_ = -1;
  jj_gen++;
  return token;
}

shared_ptr<Token> QueryParser::getToken(int index)
{
  shared_ptr<Token> t = token;
  for (int i = 0; i < index; i++) {
    if (t->next != nullptr) {
      t = t->next;
    } else {
      t = t->next = token_source->getNextToken();
    }
  }
  return t;
}

int QueryParser::jj_ntk()
{
  if ((jj_nt = token->next) == nullptr) {
    return (jj_ntk_ = (token->next = token_source->getNextToken()).kind);
  } else {
    return (jj_ntk_ = jj_nt->kind);
  }
}

void QueryParser::jj_add_error_token(int kind, int pos)
{
  if (pos >= 100) {
    return;
  }
  if (pos == jj_endpos + 1) {
    jj_lasttokens[jj_endpos++] = kind;
  } else if (jj_endpos != 0) {
    jj_expentry = std::deque<int>(jj_endpos);
    for (int i = 0; i < jj_endpos; i++) {
      jj_expentry[i] = jj_lasttokens[i];
    }
    for (deque<std::deque<int>>::const_iterator it = jj_expentries.begin();
         it != jj_expentries.end(); ++it) {
      std::deque<int> oldentry = static_cast<std::deque<int>>(*it);
      if (oldentry.size() == jj_expentry.size()) {
        for (int i = 0; i < jj_expentry.size(); i++) {
          if (oldentry[i] != jj_expentry[i]) {
            goto jj_entries_loopContinue;
          }
        }
        jj_expentries.push_back(jj_expentry);
        goto jj_entries_loopBreak;
      }
    jj_entries_loopContinue:;
    }
  jj_entries_loopBreak:
    if (pos != 0) {
      jj_lasttokens[(jj_endpos = pos) - 1] = kind;
    }
  }
}

shared_ptr<ParseException> QueryParser::generateParseException()
{
  jj_expentries.clear();
  std::deque<bool> la1tokens(24);
  if (jj_kind >= 0) {
    la1tokens[jj_kind] = true;
    jj_kind = -1;
  }
  for (int i = 0; i < 10; i++) {
    if (jj_la1[i] == jj_gen) {
      for (int j = 0; j < 32; j++) {
        if ((jj_la1_0[i] & (1 << j)) != 0) {
          la1tokens[j] = true;
        }
      }
    }
  }
  for (int i = 0; i < 24; i++) {
    if (la1tokens[i]) {
      jj_expentry = std::deque<int>(1);
      jj_expentry[0] = i;
      jj_expentries.push_back(jj_expentry);
    }
  }
  jj_endpos = 0;
  jj_rescan_token();
  jj_add_error_token(0, 0);
  std::deque<std::deque<int>> exptokseq(jj_expentries.size());
  for (int i = 0; i < jj_expentries.size(); i++) {
    exptokseq[i] = jj_expentries[i];
  }
  return make_shared<ParseException>(token, exptokseq, tokenImage);
}

void QueryParser::enable_tracing() {}

void QueryParser::disable_tracing() {}

void QueryParser::jj_rescan_token()
{
  jj_rescan = true;
  for (int i = 0; i < 1; i++) {
    try {
      shared_ptr<JJCalls> p = jj_2_rtns[i];
      do {
        if (p->gen > jj_gen) {
          jj_la = p->arg;
          jj_lastpos = jj_scanpos = p->first;
          switch (i) {
          case 0:
            jj_3_1();
            break;
          }
        }
        p = p->next;
      } while (p != nullptr);
    } catch (const LookaheadSuccess &ls) {
    }
  }
  jj_rescan = false;
}

void QueryParser::jj_save(int index, int xla)
{
  shared_ptr<JJCalls> p = jj_2_rtns[index];
  while (p->gen > jj_gen) {
    if (p->next == nullptr) {
      p = p->next = make_shared<JJCalls>();
      break;
    }
    p = p->next;
  }
  p->gen = jj_gen + xla - jj_la;
  p->first = token;
  p->arg = xla;
}
} // namespace org::apache::lucene::queryparser::surround::parser