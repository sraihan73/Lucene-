using namespace std;

#include "StandardSyntaxParser.h"

namespace org::apache::lucene::queryparser::flexible::standard::parser
{
using Message = org::apache::lucene::queryparser::flexible::messages::Message;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeParseException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeParseException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using AndQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::AndQueryNode;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using BoostQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BoostQueryNode;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using FuzzyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FuzzyQueryNode;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using GroupQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::GroupQueryNode;
using OrQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::OrQueryNode;
using RegexpQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::RegexpQueryNode;
using SlopQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::SlopQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QuotedFieldQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::QuotedFieldQueryNode;
using SyntaxParser =
    org::apache::lucene::queryparser::flexible::core::parser::SyntaxParser;
using TermRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::TermRangeQueryNode;

StandardSyntaxParser::StandardSyntaxParser()
    : StandardSyntaxParser(new FastCharStream(new StringReader(L"")))
{
}

shared_ptr<QueryNode> StandardSyntaxParser::parse(
    shared_ptr<std::wstring> query,
    shared_ptr<std::wstring> field) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  ReInit(make_shared<FastCharStream>(
      make_shared<StringReader>(query->toString())));
  try {
    // TopLevelQuery is a Query followed by the end-of-input (EOF)
    shared_ptr<QueryNode> querynode = TopLevelQuery(field);
    return querynode;
  } catch (const ParseException &tme) {
    tme->setQuery(query);
    throw tme;
  } catch (const Error &tme) {
    shared_ptr<Message> message = make_shared<MessageImpl>(
        QueryParserMessages::INVALID_SYNTAX_CANNOT_PARSE, query,
        tme->getMessage());
    shared_ptr<QueryNodeParseException> e =
        make_shared<QueryNodeParseException>(tme);
    e->setQuery(query);
    e->setNonLocalizedMessage(message);
    throw e;
  }
}

ModifierQueryNode::Modifier
StandardSyntaxParser::Modifiers() 
{
  ModifierQueryNode::Modifier ret = ModifierQueryNode::Modifier::MOD_NONE;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case NOT:
  case PLUS:
  case MINUS:
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case PLUS:
      jj_consume_token(PLUS);
      ret = ModifierQueryNode::Modifier::MOD_REQ;
      break;
    case MINUS:
      jj_consume_token(MINUS);
      ret = ModifierQueryNode::Modifier::MOD_NOT;
      break;
    case NOT:
      jj_consume_token(NOT);
      ret = ModifierQueryNode::Modifier::MOD_NOT;
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

shared_ptr<QueryNode> StandardSyntaxParser::TopLevelQuery(
    shared_ptr<std::wstring> field) 
{
  shared_ptr<QueryNode> q;
  q = Query(field);
  jj_consume_token(0);
  {
    if (true) {
      return q;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<QueryNode> StandardSyntaxParser::Query(
    shared_ptr<std::wstring> field) 
{
  deque<std::shared_ptr<QueryNode>> clauses;
  shared_ptr<QueryNode> c, first = nullptr;
  first = DisjQuery(field);
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case NOT:
    case PLUS:
    case MINUS:
    case LPAREN:
    case QUOTED:
    case TERM:
    case REGEXPTERM:
    case RANGEIN_START:
    case RANGEEX_START:
    case NUMBER:;
      break;
    default:
      jj_la1[2] = jj_gen;
      goto label_1Break;
    }
    c = DisjQuery(field);
    if (clauses.empty()) {
      clauses = deque<std::shared_ptr<QueryNode>>();
      clauses.push_back(first);
    }
    clauses.push_back(c);
  label_1Continue:;
  }
label_1Break:
  if (clauses.size() > 0) {
    {
      if (true) {
        return make_shared<BooleanQueryNode>(clauses);
      }
    }
  } else {
    // Handle the case of a "pure" negation query which
    // needs to be wrapped as a bool query, otherwise
    // the returned result drops the negation.
    if (std::dynamic_pointer_cast<ModifierQueryNode>(first) != nullptr) {
      shared_ptr<ModifierQueryNode> m =
          std::static_pointer_cast<ModifierQueryNode>(first);
      if (m->getModifier() == ModifierQueryNode::Modifier::MOD_NOT) {
        {
          if (true) {
            return make_shared<BooleanQueryNode>(
                Arrays::asList<std::shared_ptr<QueryNode>>(m));
          }
        }
      }
    }
    {
      if (true) {
        return first;
      }
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<QueryNode> StandardSyntaxParser::DisjQuery(
    shared_ptr<std::wstring> field) 
{
  shared_ptr<QueryNode> first, c;
  deque<std::shared_ptr<QueryNode>> clauses;
  first = ConjQuery(field);
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case OR:;
      break;
    default:
      jj_la1[3] = jj_gen;
      goto label_2Break;
    }
    jj_consume_token(OR);
    c = ConjQuery(field);
    if (clauses.empty()) {
      clauses = deque<std::shared_ptr<QueryNode>>();
      clauses.push_back(first);
    }
    clauses.push_back(c);
  label_2Continue:;
  }
label_2Break:
  if (clauses.size() > 0) {
    {
      if (true) {
        return make_shared<OrQueryNode>(clauses);
      }
    }
  } else {
    {
      if (true) {
        return first;
      }
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<QueryNode> StandardSyntaxParser::ConjQuery(
    shared_ptr<std::wstring> field) 
{
  shared_ptr<QueryNode> first, c;
  deque<std::shared_ptr<QueryNode>> clauses;
  first = ModClause(field);
  while (true) {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case AND:;
      break;
    default:
      jj_la1[4] = jj_gen;
      goto label_3Break;
    }
    jj_consume_token(AND);
    c = ModClause(field);
    if (clauses.empty()) {
      clauses = deque<std::shared_ptr<QueryNode>>();
      clauses.push_back(first);
    }
    clauses.push_back(c);
  label_3Continue:;
  }
label_3Break:
  if (clauses.size() > 0) {
    {
      if (true) {
        return make_shared<AndQueryNode>(clauses);
      }
    }
  } else {
    {
      if (true) {
        return first;
      }
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<QueryNode> StandardSyntaxParser::ModClause(
    shared_ptr<std::wstring> field) 
{
  shared_ptr<QueryNode> q;
  ModifierQueryNode::Modifier mods;
  mods = Modifiers();
  q = Clause(field);
  if (mods != ModifierQueryNode::Modifier::MOD_NONE) {
    q = make_shared<ModifierQueryNode>(q, mods);
  }
  {
    if (true) {
      return q;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<QueryNode> StandardSyntaxParser::Clause(
    shared_ptr<std::wstring> field) 
{
  shared_ptr<QueryNode> q;
  shared_ptr<Token> fieldToken = nullptr, boost = nullptr, operator_ = nullptr,
                    term = nullptr;
  shared_ptr<FieldQueryNode> qLower, qUpper;
  bool lowerInclusive, upperInclusive;

  bool group = false;
  if (jj_2_2(3)) {
    fieldToken = jj_consume_token(TERM);
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case OP_COLON:
    case OP_EQUAL:
      switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
      case OP_COLON:
        jj_consume_token(OP_COLON);
        break;
      case OP_EQUAL:
        jj_consume_token(OP_EQUAL);
        break;
      default:
        jj_la1[5] = jj_gen;
        jj_consume_token(-1);
        throw make_shared<ParseException>();
      }
      field = EscapeQuerySyntaxImpl::discardEscapeChar(fieldToken->image);
      q = Term(field);
      break;
    case OP_LESSTHAN:
    case OP_LESSTHANEQ:
    case OP_MORETHAN:
    case OP_MORETHANEQ:
      switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
      case OP_LESSTHAN:
        operator_ = jj_consume_token(OP_LESSTHAN);
        break;
      case OP_LESSTHANEQ:
        operator_ = jj_consume_token(OP_LESSTHANEQ);
        break;
      case OP_MORETHAN:
        operator_ = jj_consume_token(OP_MORETHAN);
        break;
      case OP_MORETHANEQ:
        operator_ = jj_consume_token(OP_MORETHANEQ);
        break;
      default:
        jj_la1[6] = jj_gen;
        jj_consume_token(-1);
        throw make_shared<ParseException>();
      }
      field = EscapeQuerySyntaxImpl::discardEscapeChar(fieldToken->image);
      switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
      case TERM:
        term = jj_consume_token(TERM);
        break;
      case QUOTED:
        term = jj_consume_token(QUOTED);
        break;
      case NUMBER:
        term = jj_consume_token(NUMBER);
        break;
      default:
        jj_la1[7] = jj_gen;
        jj_consume_token(-1);
        throw make_shared<ParseException>();
      }
      if (term->kind == QUOTED) {
        term->image = term->image.substr(1, (term->image.length() - 1) - 1);
      }
      switch (operator_->kind) {
      case OP_LESSTHAN:
        lowerInclusive = true;
        upperInclusive = false;

        qLower = make_shared<FieldQueryNode>(field, L"*", term->beginColumn,
                                             term->endColumn);
        qUpper = make_shared<FieldQueryNode>(
            field, EscapeQuerySyntaxImpl::discardEscapeChar(term->image),
            term->beginColumn, term->endColumn);

        break;
      case OP_LESSTHANEQ:
        lowerInclusive = true;
        upperInclusive = true;

        qLower = make_shared<FieldQueryNode>(field, L"*", term->beginColumn,
                                             term->endColumn);
        qUpper = make_shared<FieldQueryNode>(
            field, EscapeQuerySyntaxImpl::discardEscapeChar(term->image),
            term->beginColumn, term->endColumn);
        break;
      case OP_MORETHAN:
        lowerInclusive = false;
        upperInclusive = true;

        qLower = make_shared<FieldQueryNode>(
            field, EscapeQuerySyntaxImpl::discardEscapeChar(term->image),
            term->beginColumn, term->endColumn);
        qUpper = make_shared<FieldQueryNode>(field, L"*", term->beginColumn,
                                             term->endColumn);
        break;
      case OP_MORETHANEQ:
        lowerInclusive = true;
        upperInclusive = true;

        qLower = make_shared<FieldQueryNode>(
            field, EscapeQuerySyntaxImpl::discardEscapeChar(term->image),
            term->beginColumn, term->endColumn);
        qUpper = make_shared<FieldQueryNode>(field, L"*", term->beginColumn,
                                             term->endColumn);
        break;
      default: {
        if (true) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw make_shared<Error>(L"Unhandled case: operator=" +
                                   operator_->toString());
        }
      }
      }
      q = make_shared<TermRangeQueryNode>(qLower, qUpper, lowerInclusive,
                                          upperInclusive);
      break;
    default:
      jj_la1[8] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
  } else {
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case LPAREN:
    case QUOTED:
    case TERM:
    case REGEXPTERM:
    case RANGEIN_START:
    case RANGEEX_START:
    case NUMBER:
      if (jj_2_1(2)) {
        fieldToken = jj_consume_token(TERM);
        switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
        case OP_COLON:
          jj_consume_token(OP_COLON);
          break;
        case OP_EQUAL:
          jj_consume_token(OP_EQUAL);
          break;
        default:
          jj_la1[9] = jj_gen;
          jj_consume_token(-1);
          throw make_shared<ParseException>();
        }
        field = EscapeQuerySyntaxImpl::discardEscapeChar(fieldToken->image);
      } else {
        ;
      }
      switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
      case QUOTED:
      case TERM:
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
          jj_la1[10] = jj_gen;
          ;
        }
        group = true;
        break;
      default:
        jj_la1[11] = jj_gen;
        jj_consume_token(-1);
        throw make_shared<ParseException>();
      }
      break;
    default:
      jj_la1[12] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
  }
  if (boost != nullptr) {
    float f = static_cast<float>(1.0);
    try {
      f = stof(boost->image);
      // avoid boosting null queries, such as those caused by stop words
      if (q != nullptr) {
        q = make_shared<BoostQueryNode>(q, f);
      }
    } catch (const runtime_error &ignored) {
      /* Should this be handled somehow? (defaults to "no boost", if
       * boost number is invalid)
       */
    }
  }
  if (group) {
    q = make_shared<GroupQueryNode>(q);
  }
  {
    if (true) {
      return q;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

shared_ptr<QueryNode>
StandardSyntaxParser::Term(shared_ptr<std::wstring> field) 
{
  shared_ptr<Token> term, boost = nullptr, fuzzySlop = nullptr, goop1, goop2;
  bool fuzzy = false;
  bool regexp = false;
  bool startInc = false;
  bool endInc = false;
  shared_ptr<QueryNode> q = nullptr;
  shared_ptr<FieldQueryNode> qLower, qUpper;
  float defaultMinSimilarity =
      org::apache::lucene::search::FuzzyQuery::defaultMinSimilarity;
  switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
  case TERM:
  case REGEXPTERM:
  case NUMBER:
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case TERM:
      term = jj_consume_token(TERM);
      q = make_shared<FieldQueryNode>(
          field, EscapeQuerySyntaxImpl::discardEscapeChar(term->image),
          term->beginColumn, term->endColumn);
      break;
    case REGEXPTERM:
      term = jj_consume_token(REGEXPTERM);
      regexp = true;
      break;
    case NUMBER:
      term = jj_consume_token(NUMBER);
      break;
    default:
      jj_la1[13] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case FUZZY_SLOP:
      fuzzySlop = jj_consume_token(FUZZY_SLOP);
      fuzzy = true;
      break;
    default:
      jj_la1[14] = jj_gen;
      ;
    }
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
        jj_la1[15] = jj_gen;
        ;
      }
      break;
    default:
      jj_la1[16] = jj_gen;
      ;
    }
    if (fuzzy) {
      float fms = defaultMinSimilarity;
      try {
        fms = stof(fuzzySlop->image.substr(1));
      } catch (const runtime_error &ignored) {
      }
      if (fms < 0.0f) {
        {
          if (true) {
            throw make_shared<ParseException>(make_shared<MessageImpl>(
                QueryParserMessages::INVALID_SYNTAX_FUZZY_LIMITS));
          }
        }
      } else if (fms >= 1.0f && fms != static_cast<int>(fms)) {
        {
          if (true) {
            throw make_shared<ParseException>(make_shared<MessageImpl>(
                QueryParserMessages::INVALID_SYNTAX_FUZZY_EDITS));
          }
        }
      }
      q = make_shared<FuzzyQueryNode>(
          field, EscapeQuerySyntaxImpl::discardEscapeChar(term->image), fms,
          term->beginColumn, term->endColumn);
    } else if (regexp) {
      wstring re = term->image.substr(1, (term->image.length() - 1) - 1);
      q = make_shared<RegexpQueryNode>(field, re, 0, re.length());
    }
    break;
  case RANGEIN_START:
  case RANGEEX_START:
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case RANGEIN_START:
      jj_consume_token(RANGEIN_START);
      startInc = true;
      break;
    case RANGEEX_START:
      jj_consume_token(RANGEEX_START);
      break;
    default:
      jj_la1[17] = jj_gen;
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
      jj_la1[18] = jj_gen;
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
      jj_la1[19] = jj_gen;
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
      jj_la1[20] = jj_gen;
      jj_consume_token(-1);
      throw make_shared<ParseException>();
    }
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case CARAT:
      jj_consume_token(CARAT);
      boost = jj_consume_token(NUMBER);
      break;
    default:
      jj_la1[21] = jj_gen;
      ;
    }
    if (goop1->kind == RANGE_QUOTED) {
      goop1->image = goop1->image.substr(1, (goop1->image.length() - 1) - 1);
    }
    if (goop2->kind == RANGE_QUOTED) {
      goop2->image = goop2->image.substr(1, (goop2->image.length() - 1) - 1);
    }

    qLower = make_shared<FieldQueryNode>(
        field, EscapeQuerySyntaxImpl::discardEscapeChar(goop1->image),
        goop1->beginColumn, goop1->endColumn);
    qUpper = make_shared<FieldQueryNode>(
        field, EscapeQuerySyntaxImpl::discardEscapeChar(goop2->image),
        goop2->beginColumn, goop2->endColumn);
    q = make_shared<TermRangeQueryNode>(qLower, qUpper, startInc ? true : false,
                                        endInc ? true : false);
    break;
  case QUOTED: {
    term = jj_consume_token(QUOTED);
    q = make_shared<QuotedFieldQueryNode>(
        field,
        EscapeQuerySyntaxImpl::discardEscapeChar(
            term->image.substr(1, (term->image.length() - 1) - 1)),
        term->beginColumn + 1, term->endColumn - 1);
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case FUZZY_SLOP:
      fuzzySlop = jj_consume_token(FUZZY_SLOP);
      break;
    default:
      jj_la1[22] = jj_gen;
      ;
    }
    switch ((jj_ntk_ == -1) ? jj_ntk() : jj_ntk_) {
    case CARAT:
      jj_consume_token(CARAT);
      boost = jj_consume_token(NUMBER);
      break;
    default:
      jj_la1[23] = jj_gen;
      ;
    }
    int phraseSlop = 0;

    if (fuzzySlop != nullptr) {
      try {
        phraseSlop = static_cast<int>(stof(fuzzySlop->image.substr(1)));
        q = make_shared<SlopQueryNode>(q, phraseSlop);
      } catch (const runtime_error &ignored) {
        /* Should this be handled somehow? (defaults to "no PhraseSlop", if
         * slop number is invalid)
         */
      }
    }
    break;
  }
  default:
    jj_la1[24] = jj_gen;
    jj_consume_token(-1);
    throw make_shared<ParseException>();
  }
  if (boost != nullptr) {
    float f = static_cast<float>(1.0);
    try {
      f = stof(boost->image);
      // avoid boosting null queries, such as those caused by stop words
      if (q != nullptr) {
        q = make_shared<BoostQueryNode>(q, f);
      }
    } catch (const runtime_error &ignored) {
      /* Should this be handled somehow? (defaults to "no boost", if
       * boost number is invalid)
       */
    }
  }
  {
    if (true) {
      return q;
    }
  }
  throw make_shared<Error>(L"Missing return statement in function");
}

bool StandardSyntaxParser::jj_2_1(int xla)
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

bool StandardSyntaxParser::jj_2_2(int xla)
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

bool StandardSyntaxParser::jj_3R_12()
{
  if (jj_scan_token(RANGEIN_START)) {
    return true;
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_11()
{
  if (jj_scan_token(REGEXPTERM)) {
    return true;
  }
  return false;
}

bool StandardSyntaxParser::jj_3_1()
{
  if (jj_scan_token(TERM)) {
    return true;
  }
  shared_ptr<Token> xsp;
  xsp = jj_scanpos;
  if (jj_scan_token(15)) {
    jj_scanpos = xsp;
    if (jj_scan_token(16)) {
      return true;
    }
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_8()
{
  shared_ptr<Token> xsp;
  xsp = jj_scanpos;
  if (jj_3R_12()) {
    jj_scanpos = xsp;
    if (jj_scan_token(27)) {
      return true;
    }
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_10()
{
  if (jj_scan_token(TERM)) {
    return true;
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_7()
{
  shared_ptr<Token> xsp;
  xsp = jj_scanpos;
  if (jj_3R_10()) {
    jj_scanpos = xsp;
    if (jj_3R_11()) {
      jj_scanpos = xsp;
      if (jj_scan_token(28)) {
        return true;
      }
    }
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_9()
{
  if (jj_scan_token(QUOTED)) {
    return true;
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_5()
{
  shared_ptr<Token> xsp;
  xsp = jj_scanpos;
  if (jj_scan_token(17)) {
    jj_scanpos = xsp;
    if (jj_scan_token(18)) {
      jj_scanpos = xsp;
      if (jj_scan_token(19)) {
        jj_scanpos = xsp;
        if (jj_scan_token(20)) {
          return true;
        }
      }
    }
  }
  xsp = jj_scanpos;
  if (jj_scan_token(23)) {
    jj_scanpos = xsp;
    if (jj_scan_token(22)) {
      jj_scanpos = xsp;
      if (jj_scan_token(28)) {
        return true;
      }
    }
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_4()
{
  shared_ptr<Token> xsp;
  xsp = jj_scanpos;
  if (jj_scan_token(15)) {
    jj_scanpos = xsp;
    if (jj_scan_token(16)) {
      return true;
    }
  }
  if (jj_3R_6()) {
    return true;
  }
  return false;
}

bool StandardSyntaxParser::jj_3R_6()
{
  shared_ptr<Token> xsp;
  xsp = jj_scanpos;
  if (jj_3R_7()) {
    jj_scanpos = xsp;
    if (jj_3R_8()) {
      jj_scanpos = xsp;
      if (jj_3R_9()) {
        return true;
      }
    }
  }
  return false;
}

bool StandardSyntaxParser::jj_3_2()
{
  if (jj_scan_token(TERM)) {
    return true;
  }
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

std::deque<int> StandardSyntaxParser::jj_la1_0;
std::deque<int> StandardSyntaxParser::jj_la1_1;

StandardSyntaxParser::StaticConstructor::StaticConstructor()
{
  jj_la1_init_0();
  jj_la1_init_1();
}

StandardSyntaxParser::StaticConstructor StandardSyntaxParser::staticConstructor;

void StandardSyntaxParser::jj_la1_init_0()
{
  jj_la1_0 = std::deque<int>{
      0x1c00,     0x1c00,     0x1ec03c00, 0x200,      0x100,
      0x18000,    0x1e0000,   0x10c00000, 0x1f8000,   0x18000,
      0x200000,   0x1ec02000, 0x1ec02000, 0x12800000, 0x1000000,
      0x1000000,  0x200000,   0xc000000,  0x20000000, 0x20000000,
      0xc0000000, 0x200000,   0x1000000,  0x200000,   0x1ec00000};
}

void StandardSyntaxParser::jj_la1_init_1()
{
  jj_la1_1 = std::deque<int>{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                              0x3, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0};
}

StandardSyntaxParser::StandardSyntaxParser(shared_ptr<CharStream> stream)
{
  token_source = make_shared<StandardSyntaxParserTokenManager>(stream);
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

void StandardSyntaxParser::ReInit(shared_ptr<CharStream> stream)
{
  token_source->ReInit(stream);
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

StandardSyntaxParser::StandardSyntaxParser(
    shared_ptr<StandardSyntaxParserTokenManager> tm)
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

void StandardSyntaxParser::ReInit(
    shared_ptr<StandardSyntaxParserTokenManager> tm)
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

shared_ptr<Token>
StandardSyntaxParser::jj_consume_token(int kind) 
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

bool StandardSyntaxParser::jj_scan_token(int kind)
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

shared_ptr<Token> StandardSyntaxParser::getNextToken()
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

shared_ptr<Token> StandardSyntaxParser::getToken(int index)
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

int StandardSyntaxParser::jj_ntk()
{
  if ((jj_nt = token->next) == nullptr) {
    return (jj_ntk_ = (token->next = token_source->getNextToken()).kind);
  } else {
    return (jj_ntk_ = jj_nt->kind);
  }
}

void StandardSyntaxParser::jj_add_error_token(int kind, int pos)
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

shared_ptr<ParseException> StandardSyntaxParser::generateParseException()
{
  jj_expentries.clear();
  std::deque<bool> la1tokens(34);
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
  for (int i = 0; i < 34; i++) {
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

void StandardSyntaxParser::enable_tracing() {}

void StandardSyntaxParser::disable_tracing() {}

void StandardSyntaxParser::jj_rescan_token()
{
  jj_rescan = true;
  for (int i = 0; i < 2; i++) {
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
          }
        }
        p = p->next;
      } while (p != nullptr);
    } catch (const LookaheadSuccess &ls) {
    }
  }
  jj_rescan = false;
}

void StandardSyntaxParser::jj_save(int index, int xla)
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
} // namespace org::apache::lucene::queryparser::flexible::standard::parser