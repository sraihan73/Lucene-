using namespace std;

#include "SimpleQueryParser.h"

namespace org::apache::lucene::queryparser::simple
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using QueryBuilder = org::apache::lucene::util::QueryBuilder;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;

SimpleQueryParser::SimpleQueryParser(shared_ptr<Analyzer> analyzer,
                                     const wstring &field)
    : SimpleQueryParser(analyzer, Collections::singletonMap(field, 1.0F))
{
}

SimpleQueryParser::SimpleQueryParser(shared_ptr<Analyzer> analyzer,
                                     unordered_map<wstring, float> &weights)
    : SimpleQueryParser(analyzer, weights, -1)
{
}

SimpleQueryParser::SimpleQueryParser(shared_ptr<Analyzer> analyzer,
                                     unordered_map<wstring, float> &weights,
                                     int flags)
    : org::apache::lucene::util::QueryBuilder(analyzer), weights(weights),
      flags(flags)
{
}

shared_ptr<Query> SimpleQueryParser::parse(const wstring &queryText)
{
  if (L"*" == StringHelper::trim(queryText)) {
    return make_shared<MatchAllDocsQuery>();
  }

  std::deque<wchar_t> data = queryText.toCharArray();
  std::deque<wchar_t> buffer(data.size());

  shared_ptr<State> state = make_shared<State>(data, buffer, 0, data.size());
  parseSubQuery(state);
  if (state->top == nullptr) {
    return make_shared<MatchNoDocsQuery>(
        L"empty string passed to query parser");
  } else {
    return state->top;
  }
}

void SimpleQueryParser::parseSubQuery(shared_ptr<State> state)
{
  while (state->index < state->length) {
    if (state->data[state->index] == L'(' &&
        (flags & PRECEDENCE_OPERATORS) != 0) {
      // the beginning of a subquery has been found
      consumeSubQuery(state);
    } else if (state->data[state->index] == L')' &&
               (flags & PRECEDENCE_OPERATORS) != 0) {
      // this is an extraneous character so it is ignored
      ++state->index;
    } else if (state->data[state->index] == L'"' &&
               (flags & PHRASE_OPERATOR) != 0) {
      // the beginning of a phrase has been found
      consumePhrase(state);
    } else if (state->data[state->index] == L'+' &&
               (flags & AND_OPERATOR) != 0) {
      // an and operation has been explicitly set
      // if an operation has already been set this one is ignored
      // if a term (or phrase or subquery) has not been found yet the
      // operation is also ignored since there is no previous
      // term (or phrase or subquery) to and with
      if (state->currentOperation == nullptr && state->top != nullptr) {
        state->currentOperation = BooleanClause::Occur::MUST;
      }

      ++state->index;
    } else if (state->data[state->index] == L'|' &&
               (flags & OR_OPERATOR) != 0) {
      // an or operation has been explicitly set
      // if an operation has already been set this one is ignored
      // if a term (or phrase or subquery) has not been found yet the
      // operation is also ignored since there is no previous
      // term (or phrase or subquery) to or with
      if (state->currentOperation == nullptr && state->top != nullptr) {
        state->currentOperation = BooleanClause::Occur::SHOULD;
      }

      ++state->index;
    } else if (state->data[state->index] == L'-' &&
               (flags & NOT_OPERATOR) != 0) {
      // a not operator has been found, so increase the not count
      // two not operators in a row negate each other
      ++state->not;
      ++state->index;

      // continue so the not operator is not reset
      // before the next character is determined
      continue;
    } else if ((state->data[state->index] == L' ' ||
                state->data[state->index] == L'\t' ||
                state->data[state->index] == L'\n' ||
                state->data[state->index] == L'\r') &&
               (flags & WHITESPACE_OPERATOR) != 0) {
      // ignore any whitespace found as it may have already been
      // used a delimiter across a term (or phrase or subquery)
      // or is simply extraneous
      ++state->index;
    } else {
      // the beginning of a token has been found
      consumeToken(state);
    }

    // reset the not operator as even whitespace is not allowed when
    // specifying the not operation for a term (or phrase or subquery)
    state->not = 0;
  }
}

void SimpleQueryParser::consumeSubQuery(shared_ptr<State> state)
{
  assert(flags & PRECEDENCE_OPERATORS) != 0;
  int start = ++state->index;
  int precedence = 1;
  bool escaped = false;

  while (state->index < state->length) {
    if (!escaped) {
      if (state->data[state->index] == L'\\' &&
          (flags & ESCAPE_OPERATOR) != 0) {
        // an escape character has been found so
        // whatever character is next will become
        // part of the subquery unless the escape
        // character is the last one in the data
        escaped = true;
        ++state->index;

        continue;
      } else if (state->data[state->index] == L'(') {
        // increase the precedence as there is a
        // subquery in the current subquery
        ++precedence;
      } else if (state->data[state->index] == L')') {
        --precedence;

        if (precedence == 0) {
          // this should be the end of the subquery
          // all characters found will used for
          // creating the subquery
          break;
        }
      }
    }

    escaped = false;
    ++state->index;
  }

  if (state->index == state->length) {
    // a closing parenthesis was never found so the opening
    // parenthesis is considered extraneous and will be ignored
    state->index = start;
  } else if (state->index == start) {
    // a closing parenthesis was found immediately after the opening
    // parenthesis so the current operation is reset since it would
    // have been applied to this subquery
    state->currentOperation = nullptr;

    ++state->index;
  } else {
    // a complete subquery has been found and is recursively parsed by
    // starting over with a new state object
    shared_ptr<State> subState =
        make_shared<State>(state->data, state->buffer, start, state->index);
    parseSubQuery(subState);
    buildQueryTree(state, subState->top);

    ++state->index;
  }
}

void SimpleQueryParser::consumePhrase(shared_ptr<State> state)
{
  assert(flags & PHRASE_OPERATOR) != 0;
  int start = ++state->index;
  int copied = 0;
  bool escaped = false;
  bool hasSlop = false;

  while (state->index < state->length) {
    if (!escaped) {
      if (state->data[state->index] == L'\\' &&
          (flags & ESCAPE_OPERATOR) != 0) {
        // an escape character has been found so
        // whatever character is next will become
        // part of the phrase unless the escape
        // character is the last one in the data
        escaped = true;
        ++state->index;

        continue;
      } else if (state->data[state->index] == L'"') {
        // if there are still characters after the closing ", check for a
        // tilde
        if (state->length > (state->index + 1) &&
            state->data[state->index + 1] == L'~' &&
            (flags & NEAR_OPERATOR) != 0) {
          state->index++;
          // check for characters after the tilde
          if (state->length > (state->index + 1)) {
            hasSlop = true;
          }
          break;
        } else {
          // this should be the end of the phrase
          // all characters found will used for
          // creating the phrase query
          break;
        }
      }
    }

    escaped = false;
    state->buffer[copied++] = state->data[state->index++];
  }

  if (state->index == state->length) {
    // a closing double quote was never found so the opening
    // double quote is considered extraneous and will be ignored
    state->index = start;
  } else if (state->index == start) {
    // a closing double quote was found immediately after the opening
    // double quote so the current operation is reset since it would
    // have been applied to this phrase
    state->currentOperation = nullptr;

    ++state->index;
  } else {
    // a complete phrase has been found and is parsed through
    // through the analyzer from the given field
    wstring phrase = wstring(state->buffer, 0, copied);
    shared_ptr<Query> branch;
    if (hasSlop) {
      branch = newPhraseQuery(phrase, parseFuzziness(state));
    } else {
      branch = newPhraseQuery(phrase, 0);
    }
    buildQueryTree(state, branch);

    ++state->index;
  }
}

void SimpleQueryParser::consumeToken(shared_ptr<State> state)
{
  int copied = 0;
  bool escaped = false;
  bool prefix = false;
  bool fuzzy = false;

  while (state->index < state->length) {
    if (!escaped) {
      if (state->data[state->index] == L'\\' &&
          (flags & ESCAPE_OPERATOR) != 0) {
        // an escape character has been found so
        // whatever character is next will become
        // part of the term unless the escape
        // character is the last one in the data
        escaped = true;
        prefix = false;
        ++state->index;

        continue;
      } else if (tokenFinished(state)) {
        // this should be the end of the term
        // all characters found will used for
        // creating the term query
        break;
      } else if (copied > 0 && state->data[state->index] == L'~' &&
                 (flags & FUZZY_OPERATOR) != 0) {
        fuzzy = true;
        break;
      }

      // wildcard tracks whether or not the last character
      // was a '*' operator that hasn't been escaped
      // there must be at least one valid character before
      // searching for a prefixed set of terms
      prefix = copied > 0 && state->data[state->index] == L'*' &&
               (flags & PREFIX_OPERATOR) != 0;
    }

    escaped = false;
    state->buffer[copied++] = state->data[state->index++];
  }

  if (copied > 0) {
    shared_ptr<Query> *const branch;

    if (fuzzy && (flags & FUZZY_OPERATOR) != 0) {
      wstring token = wstring(state->buffer, 0, copied);
      int fuzziness = parseFuzziness(state);
      // edit distance has a maximum, limit to the maximum supported
      fuzziness =
          min(fuzziness, LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE);
      if (fuzziness == 0) {
        branch = newDefaultQuery(token);
      } else {
        branch = newFuzzyQuery(token, fuzziness);
      }
    } else if (prefix) {
      // if a term is found with a closing '*' it is considered to be a prefix
      // query and will have prefix added as an option
      wstring token = wstring(state->buffer, 0, copied - 1);
      branch = newPrefixQuery(token);
    } else {
      // a standard term has been found so it will be run through
      // the entire analysis chain from the specified schema field
      wstring token = wstring(state->buffer, 0, copied);
      branch = newDefaultQuery(token);
    }

    buildQueryTree(state, branch);
  }
}

shared_ptr<BooleanQuery>
SimpleQueryParser::addClause(shared_ptr<BooleanQuery> bq,
                             shared_ptr<Query> query,
                             BooleanClause::Occur occur)
{
  shared_ptr<BooleanQuery::Builder> newBq =
      make_shared<BooleanQuery::Builder>();
  newBq->setMinimumNumberShouldMatch(bq->getMinimumNumberShouldMatch());
  for (auto clause : bq) {
    newBq->add(clause);
  }
  newBq->add(query, occur);
  return newBq->build();
}

void SimpleQueryParser::buildQueryTree(shared_ptr<State> state,
                                       shared_ptr<Query> branch)
{
  if (branch != nullptr) {
    // modify our branch to a BooleanQuery wrapper for not
    // this is necessary any time a term, phrase, or subquery is negated
    if (state->not % 2 == 1) {
      shared_ptr<BooleanQuery::Builder> nq =
          make_shared<BooleanQuery::Builder>();
      nq->add(branch, BooleanClause::Occur::MUST_NOT);
      nq->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD);
      branch = nq->build();
    }

    // first term (or phrase or subquery) found and will begin our query tree
    if (state->top == nullptr) {
      state->top = branch;
    } else {
      // more than one term (or phrase or subquery) found
      // set currentOperation to the default if no other operation is explicitly
      // set
      if (state->currentOperation == nullptr) {
        state->currentOperation = defaultOperator;
      }

      // operational change requiring a new parent node
      // this occurs if the previous operation is not the same as current
      // operation because the previous operation must be evaluated separately
      // to preserve the proper precedence and the current operation will take
      // over as the top of the tree
      if (state->previousOperation != state->currentOperation) {
        shared_ptr<BooleanQuery::Builder> bq =
            make_shared<BooleanQuery::Builder>();
        bq->add(state->top, state->currentOperation);
        state->top = bq->build();
      }

      // reset all of the state for reuse
      state->top = addClause(std::static_pointer_cast<BooleanQuery>(state->top),
                             branch, state->currentOperation);
      state->previousOperation = state->currentOperation;
    }

    // reset the current operation as it was intended to be applied to
    // the incoming term (or phrase or subquery) even if branch was null
    // due to other possible errors
    state->currentOperation = nullptr;
  }
}

int SimpleQueryParser::parseFuzziness(shared_ptr<State> state)
{
  std::deque<wchar_t> slopText(state->length);
  int slopLength = 0;

  if (state->data[state->index] == L'~') {
    while (state->index < state->length) {
      state->index++;
      // it's possible that the ~ was at the end, so check after incrementing
      // to make sure we don't go out of bounds
      if (state->index < state->length) {
        if (tokenFinished(state)) {
          break;
        }
        slopText[slopLength] = state->data[state->index];
        slopLength++;
      }
    }
    int fuzziness = 0;
    try {
      wstring fuzzyString = wstring(slopText, 0, slopLength);
      if (L"" == fuzzyString) {
        // Use automatic fuzziness, ~2
        fuzziness = 2;
      } else {
        fuzziness = stoi(fuzzyString);
      }
    } catch (const NumberFormatException &e) {
      // swallow number format exceptions parsing fuzziness
    }
    // negative -> 0
    if (fuzziness < 0) {
      fuzziness = 0;
    }
    return fuzziness;
  }
  return 0;
}

bool SimpleQueryParser::tokenFinished(shared_ptr<State> state)
{
  if ((state->data[state->index] == L'"' && (flags & PHRASE_OPERATOR) != 0) ||
      (state->data[state->index] == L'|' && (flags & OR_OPERATOR) != 0) ||
      (state->data[state->index] == L'+' && (flags & AND_OPERATOR) != 0) ||
      (state->data[state->index] == L'(' &&
       (flags & PRECEDENCE_OPERATORS) != 0) ||
      (state->data[state->index] == L')' &&
       (flags & PRECEDENCE_OPERATORS) != 0) ||
      ((state->data[state->index] == L' ' ||
        state->data[state->index] == L'\t' ||
        state->data[state->index] == L'\n' ||
        state->data[state->index] == L'\r') &&
       (flags & WHITESPACE_OPERATOR) != 0)) {
    return true;
  }
  return false;
}

shared_ptr<Query> SimpleQueryParser::newDefaultQuery(const wstring &text)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  for (auto entry : weights) {
    shared_ptr<Query> q =
        createBooleanQuery(entry.first, text, defaultOperator);
    if (q != nullptr) {
      float boost = entry.second;
      if (boost != 1.0f) {
        q = make_shared<BoostQuery>(q, boost);
      }
      bq->add(q, BooleanClause::Occur::SHOULD);
    }
  }
  return simplify(bq->build());
}

shared_ptr<Query> SimpleQueryParser::newFuzzyQuery(const wstring &text,
                                                   int fuzziness)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  for (auto entry : weights) {
    const wstring fieldName = entry.first;
    shared_ptr<BytesRef> *const term =
        getAnalyzer()->normalize(fieldName, text);
    shared_ptr<Query> q =
        make_shared<FuzzyQuery>(make_shared<Term>(fieldName, term), fuzziness);
    float boost = entry.second;
    if (boost != 1.0f) {
      q = make_shared<BoostQuery>(q, boost);
    }
    bq->add(q, BooleanClause::Occur::SHOULD);
  }
  return simplify(bq->build());
}

shared_ptr<Query> SimpleQueryParser::newPhraseQuery(const wstring &text,
                                                    int slop)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  for (auto entry : weights) {
    shared_ptr<Query> q = createPhraseQuery(entry.first, text, slop);
    if (q != nullptr) {
      float boost = entry.second;
      if (boost != 1.0f) {
        q = make_shared<BoostQuery>(q, boost);
      }
      bq->add(q, BooleanClause::Occur::SHOULD);
    }
  }
  return simplify(bq->build());
}

shared_ptr<Query> SimpleQueryParser::newPrefixQuery(const wstring &text)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  for (auto entry : weights) {
    const wstring fieldName = entry.first;
    shared_ptr<BytesRef> *const term =
        getAnalyzer()->normalize(fieldName, text);
    shared_ptr<Query> q =
        make_shared<PrefixQuery>(make_shared<Term>(fieldName, term));
    float boost = entry.second;
    if (boost != 1.0f) {
      q = make_shared<BoostQuery>(q, boost);
    }
    bq->add(q, BooleanClause::Occur::SHOULD);
  }
  return simplify(bq->build());
}

shared_ptr<Query> SimpleQueryParser::simplify(shared_ptr<BooleanQuery> bq)
{
  if (bq->clauses().empty()) {
    return nullptr;
  } else if (bq->clauses().size() == 1) {
    return bq->clauses().begin()->next().getQuery();
  } else {
    return bq;
  }
}

BooleanClause::Occur SimpleQueryParser::getDefaultOperator()
{
  return defaultOperator;
}

void SimpleQueryParser::setDefaultOperator(BooleanClause::Occur operator_)
{
  if (operator_ != BooleanClause::Occur::SHOULD &&
      operator_ != BooleanClause::Occur::MUST) {
    throw invalid_argument(
        L"invalid operator: only SHOULD or MUST are allowed");
  }
  this->defaultOperator = operator_;
}

SimpleQueryParser::State::State(std::deque<wchar_t> &data,
                                std::deque<wchar_t> &buffer, int index,
                                int length)
    : data(data), buffer(buffer)
{
  this->index = index;
  this->length = length;
}
} // namespace org::apache::lucene::queryparser::simple