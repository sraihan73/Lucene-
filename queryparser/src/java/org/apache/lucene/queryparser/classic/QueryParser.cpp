using namespace std;

#include "QueryParser.h"

namespace org::apache::lucene::queryparser::classic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Query = org::apache::lucene::search::Query;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;

QueryParser::QueryParser(const wstring &f, shared_ptr<Analyzer> a)
    : QueryParser(new FastCharStream(new StringReader(L"")))
{
  init(f, a);
}

void QueryParser::setAutoGeneratePhraseQueries(bool value)
{
  if (splitOnWhitespace == false && value == true) {
    throw invalid_argument(L"setAutoGeneratePhraseQueries(true) is disallowed "
                           L"when getSplitOnWhitespace() == false");
  }
  this->autoGeneratePhraseQueries = value;
}

bool QueryParser::getSplitOnWhitespace() { return splitOnWhitespace; }

void QueryParser::setSplitOnWhitespace(bool splitOnWhitespace)
{
  if (splitOnWhitespace == false && getAutoGeneratePhraseQueries() == true) {
    throw invalid_argument(L"setSplitOnWhitespace(false) is disallowed when "
                           L"getAutoGeneratePhraseQueries() == true");
  }
  this->splitOnWhitespace = splitOnWhitespace;
}

shared_ptr<java::util::Set<int>> QueryParser::disallowedPostMultiTerm =
    unordered_set<int>(
        java::util::Arrays::asList(COLON, STAR, FUZZY_SLOP, CARAT, AND, OR));

bool QueryParser::allowedPostMultiTerm(int tokenKind)
{
  return disallowedPostMultiTerm->contains(tokenKind) == false;
}

int QueryParser::Conjunction() 
{
  int ret = CONJ_NONE;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case AND:
  case OR:
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case AND:
      jj_consume_token(AND);
      ret = CONJ_AND;
      break;
    case OR:
      jj_consume_token(OR);
      ret = CONJ_OR;
      break;
    default:
      jj_la1[0] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    break;
  default:
    jj_la1[1] = jj_gen;
    ;
  }
  {
    if (true) {
      return ret;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

int QueryParser::Modifiers() 
{
  int ret = MOD_NONE;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case NOT:
  case PLUS:
  case MINUS:
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case PLUS:
      jj_consume_token(PLUS);
      ret = MOD_REQ;
      break;
    case MINUS:
      jj_consume_token(MINUS);
      ret = MOD_NOT;
      break;
    case NOT:
      jj_consume_token(NOT);
      ret = MOD_NOT;
      break;
    default:
      jj_la1[2] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    break;
  default:
    jj_la1[3] = jj_gen;
    ;
  }
  {
    if (true) {
      return ret;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<Query>
QueryParser::TopLevelQuery(const wstring &field) 
{
  shared_ptr<Query> q;
  q = Query(field);
  jj_consume_token(0);
  {
    if (true) {
      return q;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<Query> QueryParser::Query(const wstring &field) 
{
  deque<std::shared_ptr<BooleanClause>> clauses =
      deque<std::shared_ptr<BooleanClause>>();
  shared_ptr<Query> q, firstQuery = nullptr;
  int conj, mods;
  if (jj_2_1(2)) {
    firstQuery = MultiTerm(field, clauses);
  } else {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case NOT:
    case PLUS:
    case MINUS:
    case BAREOPER:
    case LPAREN:
    case STAR:
    case QUOTED:
    case TERM:
    case PREFIXTERM:
    case WILDTERM:
    case REGEXPTERM:
    case RANGEIN_START:
    case RANGEEX_START:
    case NUMBER:
      mods = Modifiers();
      q = Clause(field);
      addClause(clauses, CONJ_NONE, mods, q);
      if (mods == MOD_NONE) {
        firstQuery = q;
      }
      break;
    default:
      jj_la1[4] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
  }
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case AND:
    case OR:
    case NOT:
    case PLUS:
    case MINUS:
    case BAREOPER:
    case LPAREN:
    case STAR:
    case QUOTED:
    case TERM:
    case PREFIXTERM:
    case WILDTERM:
    case REGEXPTERM:
    case RANGEIN_START:
    case RANGEEX_START:
    case NUMBER:;
      break;
    default:
      jj_la1[5] = jj_gen;
      goto label_1Break;
    }
    if (jj_2_2(2)) {
      MultiTerm(field, clauses);
    } else {
      switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
      case AND:
      case OR:
      case NOT:
      case PLUS:
      case MINUS:
      case BAREOPER:
      case LPAREN:
      case STAR:
      case QUOTED:
      case TERM:
      case PREFIXTERM:
      case WILDTERM:
      case REGEXPTERM:
      case RANGEIN_START:
      case RANGEEX_START:
      case NUMBER:
        conj = Conjunction();
        mods = Modifiers();
        q = Clause(field);
        addClause(clauses, conj, mods, q);
        break;
      default:
        jj_la1[6] = jj_gen;
        jj_consume_token(-1);
        throw make_shared<ParseException>();
      }
    }
  label_1Continue:;
  }
label_1Break:
  if (clauses.size() == 1 && firstQuery != nullptr) {
    {
      if (true) {
        return firstQuery;
      }
    }
  } else {
    {
      if (true) {
        return getBooleanQuery(clauses);
      }
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<Query>
QueryParser::Clause(const wstring &field) 
{
  shared_ptr<Query> q;
  shared_ptr<Token> fieldToken = nullptr, boost = nullptr;
  if (jj_2_3(2)) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case TERM:
      fieldToken = jj_consume_token(TERM);
      jj_consume_token(COLON);
      field = discardEscapeChar(fieldToken->image);
      break;
    case STAR:
      jj_consume_token(STAR);
      jj_consume_token(COLON);
      field = L"*";
      break;
    default:
      jj_la1[7] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
  } else {
    ;
  }
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case BAREOPER:
  case STAR:
  case QUOTED:
  case TERM:
  case PREFIXTERM:
  case WILDTERM:
  case REGEXPTERM:
  case RANGEIN_START:
  case RANGEEX_START:
  case NUMBER:
    q = Term(field);
    break;
  case LPAREN:
    jj_consume_token(LPAREN);
    q = Query(field);
    jj_consume_token(RPAREN);
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case CARAT:
      jj_consume_token(CARAT);
      boost = jj_consume_token(NUMBER);
      break;
    default:
      jj_la1[8] = jj_gen;
      ;
    }
    break;
  default:
    jj_la1[9] = jj_gen;
    jj_consume_token(-1);
    throw make_shared<ParseException>();
  }
  {
    if (true) {
      return handleBoost(q, boost);
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<Query> QueryParser::Term(const wstring &field) 
{
  shared_ptr<Token> term, boost = nullptr, fuzzySlop = nullptr, goop1, goop2;
  bool prefix = false;
  bool wildcard = false;
  bool fuzzy = false;
  bool regexp = false;
  bool startInc = false;
  bool endInc = false;
  shared_ptr<Query> q;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case BAREOPER:
  case STAR:
  case TERM:
  case PREFIXTERM:
  case WILDTERM:
  case REGEXPTERM:
  case NUMBER:
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case TERM:
      term = jj_consume_token(TERM);
      break;
    case STAR:
      term = jj_consume_token(STAR);
      wildcard = true;
      break;
    case PREFIXTERM:
      term = jj_consume_token(PREFIXTERM);
      prefix = true;
      break;
    case WILDTERM:
      term = jj_consume_token(WILDTERM);
      wildcard = true;
      break;
    case REGEXPTERM:
      term = jj_consume_token(REGEXPTERM);
      regexp = true;
      break;
    case NUMBER:
      term = jj_consume_token(NUMBER);
      break;
    case BAREOPER:
      term = jj_consume_token(BAREOPER);
      term->image = term->image.substr(0, 1);
      break;
    default:
      jj_la1[10] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case CARAT:
    case FUZZY_SLOP:
      switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
      case CARAT:
        jj_consume_token(CARAT);
        boost = jj_consume_token(NUMBER);
        switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
        case FUZZY_SLOP:
          fuzzySlop = jj_consume_token(FUZZY_SLOP);
          fuzzy = true;
          break;
        default:
          jj_la1[11] = jj_gen;
          ;
        }
        break;
      case FUZZY_SLOP:
        fuzzySlop = jj_consume_token(FUZZY_SLOP);
        fuzzy = true;
        switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
        case CARAT:
          jj_consume_token(CARAT);
          boost = jj_consume_token(NUMBER);
          break;
        default:
          jj_la1[12] = jj_gen;
          ;
        }
        break;
      default:
        jj_la1[13] = jj_gen;
        jj_consume_token(-1);
        throw make_shared<ParseException>();
      }
      break;
    default:
      jj_la1[14] = jj_gen;
      ;
    }
    q = handleBareTokenQuery(field, term, fuzzySlop, prefix, wildcard, fuzzy,
                             regexp);
    break;
  case RANGEIN_START:
  case RANGEEX_START: {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case RANGEIN_START:
      jj_consume_token(RANGEIN_START);
      startInc = true;
      break;
    case RANGEEX_START:
      jj_consume_token(RANGEEX_START);
      break;
    default:
      jj_la1[15] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case RANGE_GOOP:
      goop1 = jj_consume_token(RANGE_GOOP);
      break;
    case RANGE_QUOTED:
      goop1 = jj_consume_token(RANGE_QUOTED);
      break;
    case RANGE_TO:
      goop1 = jj_consume_token(RANGE_TO);
      break;
    default:
      jj_la1[16] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    jj_consume_token(RANGE_TO);
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case RANGE_GOOP:
      goop2 = jj_consume_token(RANGE_GOOP);
      break;
    case RANGE_QUOTED:
      goop2 = jj_consume_token(RANGE_QUOTED);
      break;
    case RANGE_TO:
      goop2 = jj_consume_token(RANGE_TO);
      break;
    default:
      jj_la1[17] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case RANGEIN_END:
      jj_consume_token(RANGEIN_END);
      endInc = true;
      break;
    case RANGEEX_END:
      jj_consume_token(RANGEEX_END);
      break;
    default:
      jj_la1[18] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case CARAT:
      jj_consume_token(CARAT);
      boost = jj_consume_token(NUMBER);
      break;
    default:
      jj_la1[19] = jj_gen;
      ;
    }
    bool startOpen = false;
    bool endOpen = false;
    if (goop1->kind == RANGE_QUOTED) {
      goop1->image = goop1->image.substr(1, (goop1->image.length() - 1) - 1);
    } else if (L"*" == goop1->image) {
      startOpen = true;
    }
    if (goop2->kind == RANGE_QUOTED) {
      goop2->image = goop2->image.substr(1, (goop2->image.length() - 1) - 1);
    } else if (L"*" == goop2->image) {
      endOpen = true;
    }
    q = getRangeQuery(field, startOpen ? L"" : discardEscapeChar(goop1->image),
                      endOpen ? L"" : discardEscapeChar(goop2->image), startInc,
                      endInc);
    break;
  }
  case QUOTED:
    term = jj_consume_token(QUOTED);
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case CARAT:
    case FUZZY_SLOP:
      switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
      case CARAT:
        jj_consume_token(CARAT);
        boost = jj_consume_token(NUMBER);
        switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
        case FUZZY_SLOP:
          fuzzySlop = jj_consume_token(FUZZY_SLOP);
          fuzzy = true;
          break;
        default:
          jj_la1[20] = jj_gen;
          ;
        }
        break;
      case FUZZY_SLOP:
        fuzzySlop = jj_consume_token(FUZZY_SLOP);
        fuzzy = true;
        switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
        case CARAT:
          jj_consume_token(CARAT);
          boost = jj_consume_token(NUMBER);
          break;
        default:
          jj_la1[21] = jj_gen;
          ;
        }
        break;
      default:
        jj_la1[22] = jj_gen;
        jj_consume_token(-1);
        throw make_shared<ParseException>();
      }
      break;
    default:
      jj_la1[23] = jj_gen;
      ;
    }
    q = handleQuotedTerm(field, term, fuzzySlop);
    break;
  default:
    jj_la1[24] = jj_gen;
    jj_consume_token(-1);
    throw make_shared<ParseException>();
  }
  {
    if (true) {
      return handleBoost(q, boost);
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<Query> QueryParser::MultiTerm(
    const wstring &field,
    deque<std::shared_ptr<BooleanClause>> &clauses) 
{
  shared_ptr<Token> text, whitespace, followingText;
  shared_ptr<Query> firstQuery = nullptr;
  text = jj_consume_token(TERM);
  if (splitOnWhitespace) {
    firstQuery = getFieldQuery(field, discardEscapeChar(text->image), false);
    addClause(clauses, CONJ_NONE, MOD_NONE, firstQuery);
  }
  if (getToken(1)->kind == TERM && allowedPostMultiTerm(getToken(2)->kind)) {

  } else {
    jj_consume_token(-1);
    throw make_shared<ParseException>();
  }
  while (true) {
    followingText = jj_consume_token(TERM);
    if (splitOnWhitespace) {
      shared_ptr<Query> q =
          getFieldQuery(field, discardEscapeChar(followingText->image), false);
      addClause(clauses, CONJ_NONE, MOD_NONE, q);
    } else { // build up the text to send to analysis
      text->image += L" " + followingText->image;
    }
    if (getToken(1)->kind == TERM && allowedPostMultiTerm(getToken(2)->kind)) {
      ;
    } else {
      goto label_2Break;
    }
  label_2Continue:;
  }
label_2Break:
  if (splitOnWhitespace == false) {
    firstQuery = getFieldQuery(field, discardEscapeChar(text->image), false);
    addMultiTermClauses(clauses, firstQuery);
  }
  {
    if (true) {
      return firstQuery;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
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

bool QueryParser::jj_2_2(int xla)
{
  jj_la = xla;
  jj_lastpos = jj_scanpos = token;
  try {
    return !jj_3_2();
  } catch (const LookaheadSuccess &ls) {
    return true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    jj_save(1, xla);
  }
}

bool QueryParser::jj_2_3(int xla)
{
  jj_la = xla;
  jj_lastpos = jj_scanpos = token;
  try {
    return !jj_3_3();
  } catch (const LookaheadSuccess &ls) {
    return true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    jj_save(2, xla);
  }
}

bool QueryParser::jj_3R_3()
{
  if (jj_scan_token(TERM)) {
    return true;
  }
  jj_lookingAhead = true;
  jj_semLA =
      getToken(1)->kind == TERM && allowedPostMultiTerm(getToken(2)->kind);
  jj_lookingAhead = false;
  if (!jj_semLA || jj_3R_6()) {
    return true;
  }
  shared_ptr<Token> xsp;
  if (jj_3R_7()) {
    return true;
  }
  while (true) {
    xsp = jj_scanpos;
    if (jj_3R_7()) {
      jj_scanpos = xsp;
      break;
    }
  }
  return false;
}

bool QueryParser::jj_3R_6() { return false; }

bool QueryParser::jj_3R_5()
{
  if (jj_scan_token(STAR)) {
    return true;
  }
  if (jj_scan_token(COLON)) {
    return true;
  }
  return false;
}

bool QueryParser::jj_3R_4()
{
  if (jj_scan_token(TERM)) {
    return true;
  }
  if (jj_scan_token(COLON)) {
    return true;
  }
  return false;
}

bool QueryParser::jj_3_2()
{
  if (jj_3R_3()) {
    return true;
  }
  return false;
}

bool QueryParser::jj_3_1()
{
  if (jj_3R_3()) {
    return true;
  }
  return false;
}

bool QueryParser::jj_3R_7()
{
  if (jj_scan_token(TERM)) {
    return true;
  }
  return false;
}

bool QueryParser::jj_3_3()
{
  shared_ptr<Token> xsp;
  xsp = jj_scanpos;
  if (jj_3R_4()) {
    jj_scanpos = xsp;
    if (jj_3R_5()) {
      return true;
    }
  }
  return false;
}

std::deque<int> QueryParser::jj_la1_0;
std::deque<int> QueryParser::jj_la1_1;

QueryParser::StaticConstructor::StaticConstructor()
{
  jj_la1_init_0();
  jj_la1_init_1();
}

QueryParser::StaticConstructor QueryParser::staticConstructor;

void QueryParser::jj_la1_init_0()
{
  jj_la1_0 = std::deque<int>{
      0x300,     0x300,      0x1c00,     0x1c00,     0xfda7c00,
      0xfda7f00, 0xfda7f00,  0x120000,   0x40000,    0xfda6000,
      0x9d22000, 0x200000,   0x40000,    0x240000,   0x240000,
      0x6000000, 0x90000000, 0x90000000, 0x60000000, 0x40000,
      0x200000,  0x40000,    0x240000,   0x240000,   0xfda2000};
}

void QueryParser::jj_la1_init_1()
{
  jj_la1_1 = std::deque<int>{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1,
                              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
}

QueryParser::QueryParser(shared_ptr<CharStream> stream)
{
  token_source = make_shared<QueryParserTokenManager>(stream);
  token = make_shared<Token>();
  jj_ntk_ = -1;
  jj_gen = 0;
  for (int i = 0; i < 25; i++) {
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
  jj_lookingAhead = false;
  jj_gen = 0;
  for (int i = 0; i < 25; i++) {
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
  for (int i = 0; i < 25; i++) {
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
  for (int i = 0; i < 25; i++) {
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
  shared_ptr<Token> t = jj_lookingAhead ? jj_scanpos : token;
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
  std::deque<bool> la1tokens(33);
  if (jj_kind >= 0) {
    la1tokens[jj_kind] = true;
    jj_kind = -1;
  }
  for (int i = 0; i < 25; i++) {
    if (jj_la1[i] == jj_gen) {
      for (int j = 0; j < 32; j++) {
        if ((jj_la1_0[i] & (1 << j)) != 0) {
          la1tokens[j] = true;
        }
        if ((jj_la1_1[i] & (1 << j)) != 0) {
          la1tokens[32 + j] = true;
        }
      }
    }
  }
  for (int i = 0; i < 33; i++) {
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
  for (int i = 0; i < 3; i++) {
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
          case 1:
            jj_3_2();
            break;
          case 2:
            jj_3_3();
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
} // namespace org::apache::lucene::queryparser::classic