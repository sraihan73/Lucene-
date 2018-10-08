using namespace std;

#include "NearestFuzzyQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BoostAttribute.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/BoostQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/FuzzyTermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/MaxNonCompetitiveBoostAttribute.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/LevenshteinAutomata.h"

namespace org::apache::lucene::classification::utils
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostAttribute = org::apache::lucene::search::BoostAttribute;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using FuzzyTermsEnum = org::apache::lucene::search::FuzzyTermsEnum;
using MaxNonCompetitiveBoostAttribute =
    org::apache::lucene::search::MaxNonCompetitiveBoostAttribute;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;

NearestFuzzyQuery::NearestFuzzyQuery(shared_ptr<Analyzer> analyzer)
    : analyzer(analyzer)
{
}

NearestFuzzyQuery::FieldVals::FieldVals(const wstring &name, int maxEdits,
                                        const wstring &queryString)
    : queryString(queryString), fieldName(name), maxEdits(maxEdits),
      prefixLength(NearestFuzzyQuery::PREFIX_LENGTH)
{
}

int NearestFuzzyQuery::FieldVals::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + ((fieldName == L"") ? 0 : fieldName.hashCode());
  result = prime * result + maxEdits;
  result = prime * result + prefixLength;
  result = prime * result + ((queryString == L"") ? 0 : queryString.hashCode());
  return result;
}

bool NearestFuzzyQuery::FieldVals::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<FieldVals> other = any_cast<std::shared_ptr<FieldVals>>(obj);
  if (fieldName == L"") {
    if (other->fieldName != L"") {
      return false;
    }
  } else if (fieldName != other->fieldName) {
    return false;
  }
  if (maxEdits != other->maxEdits) {
    return false;
  }
  if (prefixLength != other->prefixLength) {
    return false;
  }
  if (queryString == L"") {
    if (other->queryString != L"") {
      return false;
    }
  } else if (queryString != other->queryString) {
    return false;
  }
  return true;
}

void NearestFuzzyQuery::addTerms(const wstring &queryString,
                                 const wstring &fieldName)
{
  int maxEdits = static_cast<int>(MIN_SIMILARITY);
  if (maxEdits != MIN_SIMILARITY) {
    throw invalid_argument(
        L"MIN_SIMILARITY must integer value between 0 and " +
        to_wstring(LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) +
        L", inclusive; got " + to_wstring(MIN_SIMILARITY));
  }
  fieldVals.push_back(make_shared<FieldVals>(fieldName, maxEdits, queryString));
}

void NearestFuzzyQuery::addTerms(
    shared_ptr<IndexReader> reader, shared_ptr<FieldVals> f,
    shared_ptr<ScoreTermQueue> q) 
{
  if (f->queryString == L"") {
    return;
  }
  shared_ptr<Terms> *const terms = MultiFields::getTerms(reader, f->fieldName);
  if (terms == nullptr) {
    return;
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream(f.fieldName, f.queryString))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(f->fieldName, f->queryString);
    shared_ptr<CharTermAttribute> termAtt =
        ts->addAttribute(CharTermAttribute::typeid);

    int corpusNumDocs = reader->numDocs();
    unordered_set<wstring> processedTerms = unordered_set<wstring>();
    ts->reset();
    while (ts->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring term = termAtt->toString();
      if (!find(processedTerms.begin(), processedTerms.end(), term) !=
          processedTerms.end()) {
        processedTerms.insert(term);
        shared_ptr<ScoreTermQueue> variantsQ = make_shared<ScoreTermQueue>(
            MAX_VARIANTS_PER_TERM); // maxNum variants considered for any one
                                    // term
        float minScore = 0;
        shared_ptr<Term> startTerm = make_shared<Term>(f->fieldName, term);
        shared_ptr<AttributeSource> atts = make_shared<AttributeSource>();
        shared_ptr<MaxNonCompetitiveBoostAttribute> maxBoostAtt =
            atts->addAttribute(MaxNonCompetitiveBoostAttribute::typeid);
        shared_ptr<FuzzyTermsEnum> fe = make_shared<FuzzyTermsEnum>(
            terms, atts, startTerm, f->maxEdits, f->prefixLength, true);
        // store the df so all variants use same idf
        int df = reader->docFreq(startTerm);
        int numVariants = 0;
        int totalVariantDocFreqs = 0;
        shared_ptr<BytesRef> possibleMatch;
        shared_ptr<BoostAttribute> boostAtt =
            fe->attributes()->addAttribute(BoostAttribute::typeid);
        while ((possibleMatch = fe->next()) != nullptr) {
          numVariants++;
          totalVariantDocFreqs += fe->docFreq();
          float score = boostAtt->getBoost();
          if (variantsQ->size() < MAX_VARIANTS_PER_TERM || score > minScore) {
            shared_ptr<ScoreTerm> st = make_shared<ScoreTerm>(
                make_shared<Term>(startTerm->field(),
                                  BytesRef::deepCopyOf(possibleMatch)),
                score, startTerm);
            variantsQ->insertWithOverflow(st);
            minScore = variantsQ->top()->score; // maintain minScore
          }
          maxBoostAtt->setMaxNonCompetitiveBoost(
              variantsQ->size() >= MAX_VARIANTS_PER_TERM
                  ? minScore
                  : -numeric_limits<float>::infinity());
        }

        if (numVariants > 0) {
          int avgDf = totalVariantDocFreqs / numVariants;
          if (df == 0) // no direct match we can use as df for all variants
          {
            df = avgDf; // use avg df of all variants
          }

          // take the top variants (scored by edit distance) and reset the score
          // to include an IDF factor then add to the global queue for ranking
          // overall top query terms
          int size = variantsQ->size();
          for (int i = 0; i < size; i++) {
            shared_ptr<ScoreTerm> st = variantsQ->pop();
            if (st != nullptr) {
              st->score = (st->score * st->score) * idf(df, corpusNumDocs);
              q->insertWithOverflow(st);
            }
          }
        }
      }
    }
    ts->end();
  }
}

float NearestFuzzyQuery::idf(int docFreq, int docCount)
{
  return static_cast<float>(
      log((docCount + 1) / static_cast<double>(docFreq + 1)) + 1.0);
}

shared_ptr<Query>
NearestFuzzyQuery::newTermQuery(shared_ptr<IndexReader> reader,
                                shared_ptr<Term> term) 
{
  // we build an artificial TermContext that will give an overall df and ttf
  // equal to 1
  shared_ptr<TermContext> context =
      make_shared<TermContext>(reader->getContext());
  for (auto leafContext : reader->leaves()) {
    shared_ptr<Terms> terms = leafContext->reader()->terms(term->field());
    if (terms != nullptr) {
      shared_ptr<TermsEnum> termsEnum = terms->begin();
      if (termsEnum->seekExact(term->bytes())) {
        int freq =
            1 - context->docFreq(); // we want the total df and ttf to be 1
        context->register_(termsEnum->termState(), leafContext->ord, freq,
                           freq);
      }
    }
  }
  return make_shared<TermQuery>(term, context);
}

shared_ptr<Query>
NearestFuzzyQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<ScoreTermQueue> q = make_shared<ScoreTermQueue>(MAX_NUM_TERMS);
  // load up the deque of possible terms
  for (auto f : fieldVals) {
    addTerms(reader, f, q);
  }

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();

  // create BooleanQueries to hold the variants for each token/field pair and
  // ensure it
  // has no coord factor
  // Step 1: sort the termqueries by term/field
  unordered_map<std::shared_ptr<Term>, deque<std::shared_ptr<ScoreTerm>>>
      variantQueries = unordered_map<std::shared_ptr<Term>,
                                     deque<std::shared_ptr<ScoreTerm>>>();
  int size = q->size();
  for (int i = 0; i < size; i++) {
    shared_ptr<ScoreTerm> st = q->pop();
    if (st != nullptr) {
      deque<std::shared_ptr<ScoreTerm>> l = variantQueries.computeIfAbsent(
          st->fuzziedSourceTerm, [&](any k) { deque<>(); });
      l.push_back(st);
    }
  }
  // Step 2: Organize the sorted termqueries into zero-coord scoring bool
  // queries
  for (auto variants : variantQueries) {
    if (variants.second->size() == 1) {
      // optimize where only one selected variant
      shared_ptr<ScoreTerm> st = variants.second->get(0);
      shared_ptr<Query> tq = newTermQuery(reader, st->term);
      // set the boost to a mix of IDF and score
      bq->add(make_shared<BoostQuery>(tq, st->score),
              BooleanClause::Occur::SHOULD);
    } else {
      shared_ptr<BooleanQuery::Builder> termVariants =
          make_shared<BooleanQuery::Builder>();
      for (shared_ptr<ScoreTerm> st : variants.second) {
        // found a match
        shared_ptr<Query> tq = newTermQuery(reader, st->term);
        // set the boost using the ScoreTerm's score
        termVariants->add(make_shared<BoostQuery>(tq, st->score),
                          BooleanClause::Occur::SHOULD); // add to query
      }
      bq->add(termVariants->build(),
              BooleanClause::Occur::SHOULD); // add to query
    }
  }
  // TODO possible alternative step 3 - organize above booleans into a new layer
  // of field-based
  // booleans with a minimum-should-match of NumFields-1?
  return bq->build();
}

NearestFuzzyQuery::ScoreTerm::ScoreTerm(shared_ptr<Term> term, float score,
                                        shared_ptr<Term> fuzziedSourceTerm)
    : term(term), fuzziedSourceTerm(fuzziedSourceTerm)
{
  this->score = score;
}

NearestFuzzyQuery::ScoreTermQueue::ScoreTermQueue(int size)
    : org::apache::lucene::util::PriorityQueue<ScoreTerm>(size)
{
}

bool NearestFuzzyQuery::ScoreTermQueue::lessThan(shared_ptr<ScoreTerm> termA,
                                                 shared_ptr<ScoreTerm> termB)
{
  if (termA->score == termB->score) {
    return termA->term->compareTo(termB->term) > 0;
  } else {
    return termA->score < termB->score;
  }
}

wstring NearestFuzzyQuery::toString(const wstring &field) { return L""; }

int NearestFuzzyQuery::hashCode()
{
  int prime = 31;
  int result = classHash();
  result = prime * result + Objects::hashCode(analyzer);
  result = prime * result + Objects::hashCode(fieldVals);
  return result;
}

bool NearestFuzzyQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool NearestFuzzyQuery::equalsTo(shared_ptr<NearestFuzzyQuery> other)
{
  return Objects::equals(analyzer, other->analyzer) &&
         Objects::equals(fieldVals, other->fieldVals);
}
} // namespace org::apache::lucene::classification::utils