using namespace std;

#include "FuzzyLikeThisQuery.h"

namespace org::apache::lucene::sandbox::queries
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
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using FuzzyTermsEnum = org::apache::lucene::search::FuzzyTermsEnum;
using MaxNonCompetitiveBoostAttribute =
    org::apache::lucene::search::MaxNonCompetitiveBoostAttribute;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;
shared_ptr<org::apache::lucene::search::similarities::TFIDFSimilarity>
    FuzzyLikeThisQuery::sim = make_shared<
        org::apache::lucene::search::similarities::ClassicSimilarity>();

int FuzzyLikeThisQuery::hashCode()
{
  int prime = 31;
  int result = classHash();
  result = prime * result + Objects::hashCode(analyzer);
  result = prime * result + Objects::hashCode(fieldVals);
  result = prime * result + (ignoreTF ? 1231 : 1237);
  result = prime * result + maxNumTerms;
  return result;
}

bool FuzzyLikeThisQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool FuzzyLikeThisQuery::equalsTo(shared_ptr<FuzzyLikeThisQuery> other)
{
  return Objects::equals(analyzer, other->analyzer) &&
         Objects::equals(fieldVals, other->fieldVals) &&
         ignoreTF == other->ignoreTF && maxNumTerms == other->maxNumTerms;
}

FuzzyLikeThisQuery::FuzzyLikeThisQuery(int maxNumTerms,
                                       shared_ptr<Analyzer> analyzer)
{
  this->analyzer = analyzer;
  this->maxNumTerms = maxNumTerms;
}

FuzzyLikeThisQuery::FieldVals::FieldVals(const wstring &name, int maxEdits,
                                         int length, const wstring &queryString)
{
  fieldName = name;
  this->maxEdits = maxEdits;
  prefixLength = length;
  this->queryString = queryString;
}

int FuzzyLikeThisQuery::FieldVals::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + ((fieldName == L"") ? 0 : fieldName.hashCode());
  result = prime * result + maxEdits;
  result = prime * result + prefixLength;
  result = prime * result + ((queryString == L"") ? 0 : queryString.hashCode());
  return result;
}

bool FuzzyLikeThisQuery::FieldVals::equals(any obj)
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

void FuzzyLikeThisQuery::addTerms(const wstring &queryString,
                                  const wstring &fieldName, float minSimilarity,
                                  int prefixLength)
{
  int maxEdits = static_cast<int>(minSimilarity);
  if (maxEdits != minSimilarity || maxEdits < 0 ||
      maxEdits > LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) {
    throw invalid_argument(
        L"minSimilarity must integer value between 0 and " +
        to_wstring(LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) +
        L", inclusive; got " + to_wstring(minSimilarity));
  }
  fieldVals.push_back(
      make_shared<FieldVals>(fieldName, maxEdits, prefixLength, queryString));
}

void FuzzyLikeThisQuery::addTerms(
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
            st->score = (st->score * st->score) * sim->idf(df, corpusNumDocs);
            q->insertWithOverflow(st);
          }
        }
      }
    }
    ts->end();
  }
}

shared_ptr<Query>
FuzzyLikeThisQuery::newTermQuery(shared_ptr<IndexReader> reader,
                                 shared_ptr<Term> term) 
{
  if (ignoreTF) {
    return make_shared<ConstantScoreQuery>(make_shared<TermQuery>(term));
  } else {
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
}

shared_ptr<Query>
FuzzyLikeThisQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<ScoreTermQueue> q = make_shared<ScoreTermQueue>(maxNumTerms);
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
    deque<std::shared_ptr<ScoreTerm>> l =
        variantQueries[st->fuzziedSourceTerm];
    if (l.empty()) {
      l = deque<std::shared_ptr<ScoreTerm>>();
      variantQueries.emplace(st->fuzziedSourceTerm, l);
    }
    l.push_back(st);
  }
  // Step 2: Organize the sorted termqueries into zero-coord scoring bool
  // queries
  for (shared_ptr<Iterator<deque<std::shared_ptr<ScoreTerm>>>> iter =
           variantQueries.values().begin();
       iter->hasNext();) {
    deque<std::shared_ptr<ScoreTerm>> variants = iter->next();
    if (variants.size() == 1) {
      // optimize where only one selected variant
      shared_ptr<ScoreTerm> st = variants[0];
      shared_ptr<Query> tq = newTermQuery(reader, st->term);
      // set the boost to a mix of IDF and score
      bq->add(make_shared<BoostQuery>(tq, st->score),
              BooleanClause::Occur::SHOULD);
    } else {
      shared_ptr<BooleanQuery::Builder> termVariants =
          make_shared<BooleanQuery::Builder>();
      for (deque<std::shared_ptr<ScoreTerm>>::const_iterator iterator2 =
               variants.begin();
           iterator2 != variants.end(); ++iterator2) {
        shared_ptr<ScoreTerm> st = *iterator2;
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

FuzzyLikeThisQuery::ScoreTerm::ScoreTerm(shared_ptr<Term> term, float score,
                                         shared_ptr<Term> fuzziedSourceTerm)
{
  this->term = term;
  this->score = score;
  this->fuzziedSourceTerm = fuzziedSourceTerm;
}

FuzzyLikeThisQuery::ScoreTermQueue::ScoreTermQueue(int size)
    : org::apache::lucene::util::PriorityQueue<ScoreTerm>(size)
{
}

bool FuzzyLikeThisQuery::ScoreTermQueue::lessThan(shared_ptr<ScoreTerm> termA,
                                                  shared_ptr<ScoreTerm> termB)
{
  if (termA->score == termB->score) {
    return termA->term->compareTo(termB->term) > 0;
  } else {
    return termA->score < termB->score;
  }
}

wstring FuzzyLikeThisQuery::toString(const wstring &field) { return L""; }

bool FuzzyLikeThisQuery::isIgnoreTF() { return ignoreTF; }

void FuzzyLikeThisQuery::setIgnoreTF(bool ignoreTF)
{
  this->ignoreTF = ignoreTF;
}
} // namespace org::apache::lucene::sandbox::queries