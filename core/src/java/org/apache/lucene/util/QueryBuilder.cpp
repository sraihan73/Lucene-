using namespace std;

#include "QueryBuilder.h"

namespace org::apache::lucene::util
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using GraphTokenStreamFiniteStrings =
    org::apache::lucene::util::graph::GraphTokenStreamFiniteStrings;

QueryBuilder::QueryBuilder(shared_ptr<Analyzer> analyzer)
{
  this->analyzer = analyzer;
}

shared_ptr<Query> QueryBuilder::createBooleanQuery(const wstring &field,
                                                   const wstring &queryText)
{
  return createBooleanQuery(field, queryText, BooleanClause::Occur::SHOULD);
}

shared_ptr<Query>
QueryBuilder::createBooleanQuery(const wstring &field, const wstring &queryText,
                                 BooleanClause::Occur operator_)
{
  if (operator_ != BooleanClause::Occur::SHOULD &&
      operator_ != BooleanClause::Occur::MUST) {
    throw invalid_argument(
        L"invalid operator: only SHOULD or MUST are allowed");
  }
  return createFieldQuery(analyzer, operator_, field, queryText, false, 0);
}

shared_ptr<Query> QueryBuilder::createPhraseQuery(const wstring &field,
                                                  const wstring &queryText)
{
  return createPhraseQuery(field, queryText, 0);
}

shared_ptr<Query> QueryBuilder::createPhraseQuery(const wstring &field,
                                                  const wstring &queryText,
                                                  int phraseSlop)
{
  return createFieldQuery(analyzer, BooleanClause::Occur::MUST, field,
                          queryText, true, phraseSlop);
}

shared_ptr<Query> QueryBuilder::createMinShouldMatchQuery(
    const wstring &field, const wstring &queryText, float fraction)
{
  if (isnan(fraction) || fraction < 0 || fraction > 1) {
    throw invalid_argument(L"fraction should be >= 0 and <= 1");
  }

  // TODO: weird that BQ equals/rewrite/scorer doesn't handle this?
  if (fraction == 1) {
    return createBooleanQuery(field, queryText, BooleanClause::Occur::MUST);
  }

  shared_ptr<Query> query = createFieldQuery(
      analyzer, BooleanClause::Occur::SHOULD, field, queryText, false, 0);
  if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr) {
    query = addMinShouldMatchToBoolean(
        std::static_pointer_cast<BooleanQuery>(query), fraction);
  }
  return query;
}

shared_ptr<BooleanQuery>
QueryBuilder::addMinShouldMatchToBoolean(shared_ptr<BooleanQuery> query,
                                         float fraction)
{
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  builder->setMinimumNumberShouldMatch(
      static_cast<int>(fraction * query->clauses().size()));
  for (auto clause : query) {
    builder->add(clause);
  }

  return builder->build();
}

shared_ptr<Analyzer> QueryBuilder::getAnalyzer() { return analyzer; }

void QueryBuilder::setAnalyzer(shared_ptr<Analyzer> analyzer)
{
  this->analyzer = analyzer;
}

bool QueryBuilder::getEnablePositionIncrements()
{
  return enablePositionIncrements;
}

void QueryBuilder::setEnablePositionIncrements(bool enable)
{
  this->enablePositionIncrements = enable;
}

bool QueryBuilder::getAutoGenerateMultiTermSynonymsPhraseQuery()
{
  return autoGenerateMultiTermSynonymsPhraseQuery;
}

void QueryBuilder::setAutoGenerateMultiTermSynonymsPhraseQuery(bool enable)
{
  this->autoGenerateMultiTermSynonymsPhraseQuery = enable;
}

shared_ptr<Query> QueryBuilder::createFieldQuery(shared_ptr<Analyzer> analyzer,
                                                 BooleanClause::Occur operator_,
                                                 const wstring &field,
                                                 const wstring &queryText,
                                                 bool quoted, int phraseSlop)
{
  assert(operator_ == BooleanClause::Occur::SHOULD ||
         operator_ == BooleanClause::Occur::MUST);

  // Use the analyzer to get all the tokens, and then build an appropriate
  // query based on the analysis chain.
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // source = analyzer.tokenStream(field, queryText))
  {
    org::apache::lucene::analysis::TokenStream source =
        analyzer->tokenStream(field, queryText);
    try {
      return createFieldQuery(source, operator_, field, quoted, phraseSlop);
    } catch (const IOException &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Error analyzing query text",
      // e);
      throw runtime_error(L"Error analyzing query text");
    }
  }
}

void QueryBuilder::setEnableGraphQueries(bool v) { enableGraphQueries = v; }

bool QueryBuilder::getEnableGraphQueries() { return enableGraphQueries; }

shared_ptr<Query> QueryBuilder::createFieldQuery(shared_ptr<TokenStream> source,
                                                 BooleanClause::Occur operator_,
                                                 const wstring &field,
                                                 bool quoted, int phraseSlop)
{
  assert(operator_ == BooleanClause::Occur::SHOULD ||
         operator_ == BooleanClause::Occur::MUST);

  // Build an appropriate query based on the analysis chain.
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.analysis.CachingTokenFilter stream = new
  // org.apache.lucene.analysis.CachingTokenFilter(source))
  {
    org::apache::lucene::analysis::CachingTokenFilter stream =
        org::apache::lucene::analysis::CachingTokenFilter(source);
    try {

      shared_ptr<TermToBytesRefAttribute> termAtt =
          stream->getAttribute(TermToBytesRefAttribute::typeid);
      shared_ptr<PositionIncrementAttribute> posIncAtt =
          stream->addAttribute(PositionIncrementAttribute::typeid);
      shared_ptr<PositionLengthAttribute> posLenAtt =
          stream->addAttribute(PositionLengthAttribute::typeid);

      if (termAtt == nullptr) {
        return nullptr;
      }

      // phase 1: read through the stream and assess the situation:
      // counting the number of tokens/positions and marking if we have any
      // synonyms.

      int numTokens = 0;
      int positionCount = 0;
      bool hasSynonyms = false;
      bool isGraph = false;

      stream->reset();
      while (stream->incrementToken()) {
        numTokens++;
        int positionIncrement = posIncAtt->getPositionIncrement();
        if (positionIncrement != 0) {
          positionCount += positionIncrement;
        } else {
          hasSynonyms = true;
        }

        int positionLength = posLenAtt->getPositionLength();
        if (enableGraphQueries && positionLength > 1) {
          isGraph = true;
        }
      }

      // phase 2: based on token count, presence of synonyms, and options
      // formulate a single term, bool, or phrase.

      if (numTokens == 0) {
        return nullptr;
      } else if (numTokens == 1) {
        // single term
        return analyzeTerm(field, stream);
      } else if (isGraph) {
        // graph
        if (quoted) {
          return analyzeGraphPhrase(stream, field, phraseSlop);
        } else {
          return analyzeGraphBoolean(field, stream, operator_);
        }
      } else if (quoted && positionCount > 1) {
        // phrase
        if (hasSynonyms) {
          // complex phrase with synonyms
          return analyzeMultiPhrase(field, stream, phraseSlop);
        } else {
          // simple phrase
          return analyzePhrase(field, stream, phraseSlop);
        }
      } else {
        // bool
        if (positionCount == 1) {
          // only one position, with synonyms
          return analyzeBoolean(field, stream);
        } else {
          // complex case: multiple positions
          return analyzeMultiBoolean(field, stream, operator_);
        }
      }
    } catch (const IOException &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Error analyzing query text",
      // e);
      throw runtime_error(L"Error analyzing query text");
    }
  }
}

shared_ptr<SpanQuery>
QueryBuilder::createSpanQuery(shared_ptr<TokenStream> in_,
                              const wstring &field) 
{
  shared_ptr<TermToBytesRefAttribute> termAtt =
      in_->getAttribute(TermToBytesRefAttribute::typeid);
  if (termAtt == nullptr) {
    return nullptr;
  }

  deque<std::shared_ptr<SpanTermQuery>> terms =
      deque<std::shared_ptr<SpanTermQuery>>();
  while (in_->incrementToken()) {
    terms.push_back(make_shared<SpanTermQuery>(
        make_shared<Term>(field, termAtt->getBytesRef())));
  }

  if (terms.empty()) {
    return nullptr;
  } else if (terms.size() == 1) {
    return terms[0];
  } else {
    return make_shared<SpanNearQuery>(
        terms.toArray(std::deque<std::shared_ptr<SpanTermQuery>>(0)), 0, true);
  }
}

shared_ptr<Query>
QueryBuilder::analyzeTerm(const wstring &field,
                          shared_ptr<TokenStream> stream) 
{
  shared_ptr<TermToBytesRefAttribute> termAtt =
      stream->getAttribute(TermToBytesRefAttribute::typeid);

  stream->reset();
  if (!stream->incrementToken()) {
    throw make_shared<AssertionError>();
  }

  return newTermQuery(make_shared<Term>(field, termAtt->getBytesRef()));
}

shared_ptr<Query>
QueryBuilder::analyzeBoolean(const wstring &field,
                             shared_ptr<TokenStream> stream) 
{
  shared_ptr<TermToBytesRefAttribute> termAtt =
      stream->getAttribute(TermToBytesRefAttribute::typeid);

  stream->reset();
  deque<std::shared_ptr<Term>> terms = deque<std::shared_ptr<Term>>();
  while (stream->incrementToken()) {
    terms.push_back(make_shared<Term>(field, termAtt->getBytesRef()));
  }

  return newSynonymQuery(
      terms.toArray(std::deque<std::shared_ptr<Term>>(terms.size())));
}

void QueryBuilder::add(shared_ptr<BooleanQuery::Builder> q,
                       deque<std::shared_ptr<Term>> &current,
                       BooleanClause::Occur operator_)
{
  if (current.empty()) {
    return;
  }
  if (current.size() == 1) {
    q->add(newTermQuery(current[0]), operator_);
  } else {
    q->add(newSynonymQuery(current.toArray(
               std::deque<std::shared_ptr<Term>>(current.size()))),
           operator_);
  }
}

shared_ptr<Query> QueryBuilder::analyzeMultiBoolean(
    const wstring &field, shared_ptr<TokenStream> stream,
    BooleanClause::Occur operator_) 
{
  shared_ptr<BooleanQuery::Builder> q = newBooleanQuery();
  deque<std::shared_ptr<Term>> currentQuery = deque<std::shared_ptr<Term>>();

  shared_ptr<TermToBytesRefAttribute> termAtt =
      stream->getAttribute(TermToBytesRefAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> posIncrAtt =
      stream->getAttribute(PositionIncrementAttribute::typeid);

  stream->reset();
  while (stream->incrementToken()) {
    if (posIncrAtt->getPositionIncrement() != 0) {
      add(q, currentQuery, operator_);
      currentQuery.clear();
    }
    currentQuery.push_back(make_shared<Term>(field, termAtt->getBytesRef()));
  }
  add(q, currentQuery, operator_);

  return q->build();
}

shared_ptr<Query> QueryBuilder::analyzePhrase(const wstring &field,
                                              shared_ptr<TokenStream> stream,
                                              int slop) 
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->setSlop(slop);

  shared_ptr<TermToBytesRefAttribute> termAtt =
      stream->getAttribute(TermToBytesRefAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> posIncrAtt =
      stream->getAttribute(PositionIncrementAttribute::typeid);
  int position = -1;

  stream->reset();
  while (stream->incrementToken()) {
    if (enablePositionIncrements) {
      position += posIncrAtt->getPositionIncrement();
    } else {
      position += 1;
    }
    builder->add(make_shared<Term>(field, termAtt->getBytesRef()), position);
  }

  return builder->build();
}

shared_ptr<Query>
QueryBuilder::analyzeMultiPhrase(const wstring &field,
                                 shared_ptr<TokenStream> stream,
                                 int slop) 
{
  shared_ptr<MultiPhraseQuery::Builder> mpqb = newMultiPhraseQueryBuilder();
  mpqb->setSlop(slop);

  shared_ptr<TermToBytesRefAttribute> termAtt =
      stream->getAttribute(TermToBytesRefAttribute::typeid);

  shared_ptr<PositionIncrementAttribute> posIncrAtt =
      stream->getAttribute(PositionIncrementAttribute::typeid);
  int position = -1;

  deque<std::shared_ptr<Term>> multiTerms = deque<std::shared_ptr<Term>>();
  stream->reset();
  while (stream->incrementToken()) {
    int positionIncrement = posIncrAtt->getPositionIncrement();

    if (positionIncrement > 0 && multiTerms.size() > 0) {
      if (enablePositionIncrements) {
        mpqb->add(multiTerms.toArray(std::deque<std::shared_ptr<Term>>(0)),
                  position);
      } else {
        mpqb->add(multiTerms.toArray(std::deque<std::shared_ptr<Term>>(0)));
      }
      multiTerms.clear();
    }
    position += positionIncrement;
    multiTerms.push_back(make_shared<Term>(field, termAtt->getBytesRef()));
  }

  if (enablePositionIncrements) {
    mpqb->add(multiTerms.toArray(std::deque<std::shared_ptr<Term>>(0)),
              position);
  } else {
    mpqb->add(multiTerms.toArray(std::deque<std::shared_ptr<Term>>(0)));
  }
  return mpqb->build();
}

shared_ptr<Query> QueryBuilder::analyzeGraphBoolean(
    const wstring &field, shared_ptr<TokenStream> source,
    BooleanClause::Occur operator_) 
{
  source->reset();
  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(source);
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  std::deque<int> articulationPoints = graph->articulationPoints();
  int lastState = 0;
  for (int i = 0; i <= articulationPoints.size(); i++) {
    int start = lastState;
    int end = -1;
    if (i < articulationPoints.size()) {
      end = articulationPoints[i];
    }
    lastState = end;
    shared_ptr<Query> *const queryPos;
    if (graph->hasSidePath(start)) {
      const Iterator<std::shared_ptr<TokenStream>> it =
          graph->getFiniteStrings(start, end);
      Iterator<std::shared_ptr<Query>> queries =
          make_shared<IteratorAnonymousInnerClass>(shared_from_this(), field,
                                                   it);
      queryPos = newGraphSynonymQuery(queries);
    } else {
      std::deque<std::shared_ptr<Term>> terms = graph->getTerms(field, start);
      assert(terms.size() > 0);
      if (terms.size() == 1) {
        queryPos = newTermQuery(terms[0]);
      } else {
        queryPos = newSynonymQuery(terms);
      }
    }
    if (queryPos != nullptr) {
      builder->add(queryPos, operator_);
    }
  }
  return builder->build();
}

QueryBuilder::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<QueryBuilder> outerInstance, const wstring &field,
    shared_ptr<Iterator<std::shared_ptr<TokenStream>>> it)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->it = it;
}

bool QueryBuilder::IteratorAnonymousInnerClass::hasNext()
{
  return it->hasNext();
}

shared_ptr<Query> QueryBuilder::IteratorAnonymousInnerClass::next()
{
  shared_ptr<TokenStream> ts = it->next();
  return outerInstance->createFieldQuery(
      ts, BooleanClause::Occur::MUST, field,
      outerInstance->getAutoGenerateMultiTermSynonymsPhraseQuery(), 0);
}

shared_ptr<SpanQuery>
QueryBuilder::analyzeGraphPhrase(shared_ptr<TokenStream> source,
                                 const wstring &field,
                                 int phraseSlop) 
{
  source->reset();
  shared_ptr<GraphTokenStreamFiniteStrings> graph =
      make_shared<GraphTokenStreamFiniteStrings>(source);
  deque<std::shared_ptr<SpanQuery>> clauses =
      deque<std::shared_ptr<SpanQuery>>();
  std::deque<int> articulationPoints = graph->articulationPoints();
  int lastState = 0;
  for (int i = 0; i <= articulationPoints.size(); i++) {
    int start = lastState;
    int end = -1;
    if (i < articulationPoints.size()) {
      end = articulationPoints[i];
    }
    lastState = end;
    shared_ptr<SpanQuery> *const queryPos;
    if (graph->hasSidePath(start)) {
      deque<std::shared_ptr<SpanQuery>> queries =
          deque<std::shared_ptr<SpanQuery>>();
      Iterator<std::shared_ptr<TokenStream>> it =
          graph->getFiniteStrings(start, end);
      while (it->hasNext()) {
        shared_ptr<TokenStream> ts = it->next();
        shared_ptr<SpanQuery> q = createSpanQuery(ts, field);
        if (q != nullptr) {
          queries.push_back(q);
        }
        it++;
      }
      if (queries.size() > 0) {
        queryPos = make_shared<SpanOrQuery>(
            queries.toArray(std::deque<std::shared_ptr<SpanQuery>>(0)));
      } else {
        queryPos.reset();
      }
    } else {
      std::deque<std::shared_ptr<Term>> terms = graph->getTerms(field, start);
      assert(terms.size() > 0);
      if (terms.size() == 1) {
        queryPos = make_shared<SpanTermQuery>(terms[0]);
      } else {
        std::deque<std::shared_ptr<SpanTermQuery>> orClauses(terms.size());
        for (int idx = 0; idx < terms.size(); idx++) {
          orClauses[idx] = make_shared<SpanTermQuery>(terms[idx]);
        }

        queryPos = make_shared<SpanOrQuery>(orClauses);
      }
    }

    if (queryPos != nullptr) {
      clauses.push_back(queryPos);
    }
  }

  if (clauses.empty()) {
    return nullptr;
  } else if (clauses.size() == 1) {
    return clauses[0];
  } else {
    return make_shared<SpanNearQuery>(
        clauses.toArray(std::deque<std::shared_ptr<SpanQuery>>(0)), phraseSlop,
        true);
  }
}

shared_ptr<BooleanQuery::Builder> QueryBuilder::newBooleanQuery()
{
  return make_shared<BooleanQuery::Builder>();
}

shared_ptr<Query>
QueryBuilder::newSynonymQuery(std::deque<std::shared_ptr<Term>> &terms)
{
  return make_shared<SynonymQuery>(terms);
}

shared_ptr<Query> QueryBuilder::newGraphSynonymQuery(
    shared_ptr<Iterator<std::shared_ptr<Query>>> queries)
{
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  while (queries->hasNext()) {
    builder->add(queries->next(), BooleanClause::Occur::SHOULD);
    queries++;
  }
  shared_ptr<BooleanQuery> bq = builder->build();
  if (bq->clauses().size() == 1) {
    return bq->clauses()[0]->getQuery();
  }
  return bq;
}

shared_ptr<Query> QueryBuilder::newTermQuery(shared_ptr<Term> term)
{
  return make_shared<TermQuery>(term);
}

shared_ptr<MultiPhraseQuery::Builder> QueryBuilder::newMultiPhraseQueryBuilder()
{
  return make_shared<MultiPhraseQuery::Builder>();
}
} // namespace org::apache::lucene::util