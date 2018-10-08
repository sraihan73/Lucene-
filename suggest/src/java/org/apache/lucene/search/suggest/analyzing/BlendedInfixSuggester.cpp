using namespace std;

#include "BlendedInfixSuggester.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
double BlendedInfixSuggester::LINEAR_COEF = 0.10;
int BlendedInfixSuggester::DEFAULT_NUM_FACTOR = 10;

BlendedInfixSuggester::BlendedInfixSuggester(
    shared_ptr<Directory> dir, shared_ptr<Analyzer> analyzer) 
    : AnalyzingInfixSuggester(dir, analyzer), numFactor(DEFAULT_NUM_FACTOR),
      blenderType(BlenderType::POSITION_LINEAR)
{
}

BlendedInfixSuggester::BlendedInfixSuggester(
    shared_ptr<Directory> dir, shared_ptr<Analyzer> indexAnalyzer,
    shared_ptr<Analyzer> queryAnalyzer, int minPrefixChars,
    BlenderType blenderType, int numFactor,
    bool commitOnBuild) 
    : AnalyzingInfixSuggester(dir, indexAnalyzer, queryAnalyzer, minPrefixChars,
                              commitOnBuild),
      numFactor(numFactor), blenderType(blenderType)
{
}

BlendedInfixSuggester::BlendedInfixSuggester(
    shared_ptr<Directory> dir, shared_ptr<Analyzer> indexAnalyzer,
    shared_ptr<Analyzer> queryAnalyzer, int minPrefixChars,
    BlenderType blenderType, int numFactor, optional<double> &exponent,
    bool commitOnBuild, bool allTermsRequired,
    bool highlight) 
    : AnalyzingInfixSuggester(dir, indexAnalyzer, queryAnalyzer, minPrefixChars,
                              commitOnBuild, allTermsRequired, highlight),
      numFactor(numFactor), blenderType(blenderType)
{
  if (exponent) {
    this->exponent = exponent;
  }
}

deque<std::shared_ptr<Lookup::LookupResult>> BlendedInfixSuggester::lookup(
    shared_ptr<std::wstring> key,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, bool onlyMorePopular,
    int num) 
{
  // Don't * numFactor here since we do it down below, once, in the call chain:
  return AnalyzingInfixSuggester::lookup(key, contexts, onlyMorePopular, num);
}

deque<std::shared_ptr<Lookup::LookupResult>> BlendedInfixSuggester::lookup(
    shared_ptr<std::wstring> key,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, int num,
    bool allTermsRequired, bool doHighlight) 
{
  // Don't * numFactor here since we do it down below, once, in the call chain:
  return AnalyzingInfixSuggester::lookup(key, contexts, num, allTermsRequired,
                                         doHighlight);
}

deque<std::shared_ptr<Lookup::LookupResult>> BlendedInfixSuggester::lookup(
    shared_ptr<std::wstring> key,
    unordered_map<std::shared_ptr<BytesRef>, BooleanClause::Occur> &contextInfo,
    int num, bool allTermsRequired, bool doHighlight) 
{
  // Don't * numFactor here since we do it down below, once, in the call chain:
  return AnalyzingInfixSuggester::lookup(key, contextInfo, num,
                                         allTermsRequired, doHighlight);
}

deque<std::shared_ptr<Lookup::LookupResult>> BlendedInfixSuggester::lookup(
    shared_ptr<std::wstring> key, shared_ptr<BooleanQuery> contextQuery,
    int num, bool allTermsRequired, bool doHighlight) 
{
  /** We need to do num * numFactor here only because it is the last call in the
   * lookup chain*/
  return AnalyzingInfixSuggester::lookup(key, contextQuery, num * numFactor,
                                         allTermsRequired, doHighlight);
}

shared_ptr<FieldType> BlendedInfixSuggester::getTextFieldType()
{
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(true);
  ft->setOmitNorms(true);

  return ft;
}

deque<std::shared_ptr<Lookup::LookupResult>>
BlendedInfixSuggester::createResults(
    shared_ptr<IndexSearcher> searcher, shared_ptr<TopFieldDocs> hits, int num,
    shared_ptr<std::wstring> key, bool doHighlight,
    shared_ptr<Set<wstring>> matchedTokens,
    const wstring &prefixToken) 
{

  set<std::shared_ptr<Lookup::LookupResult>> results =
      set<std::shared_ptr<Lookup::LookupResult>>(LOOKUP_COMP);

  // we reduce the num to the one initially requested
  int actualNum = num / numFactor;

  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    shared_ptr<FieldDoc> fd =
        std::static_pointer_cast<FieldDoc>(hits->scoreDocs[i]);

    shared_ptr<BinaryDocValues> textDV = MultiDocValues::getBinaryValues(
        searcher->getIndexReader(), TEXT_FIELD_NAME);
    assert(textDV != nullptr);

    textDV->advance(fd->doc);

    const wstring text = textDV->binaryValue()->utf8ToString();
    int64_t weight = any_cast<optional<int64_t>>(fd->fields[0]);

    // This will just be null if app didn't pass payloads to build():
    // TODO: maybe just stored fields?  they compress...
    shared_ptr<BinaryDocValues> payloadsDV = MultiDocValues::getBinaryValues(
        searcher->getIndexReader(), L"payloads");

    shared_ptr<BytesRef> payload;
    if (payloadsDV != nullptr) {
      if (payloadsDV->advance(fd->doc) == fd->doc) {
        payload = BytesRef::deepCopyOf(payloadsDV->binaryValue());
      } else {
        payload = make_shared<BytesRef>(BytesRef::EMPTY_BYTES);
      }
    } else {
      payload.reset();
    }

    double coefficient;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (StringHelper::startsWith(text, key->toString())) {
      // if hit starts with the key, we don't change the score
      coefficient = 1;
    } else {
      coefficient =
          createCoefficient(searcher, fd->doc, matchedTokens, prefixToken);
    }

    int64_t score = static_cast<int64_t>(weight * coefficient);

    shared_ptr<Lookup::LookupResult> result;
    if (doHighlight) {
      result = make_shared<Lookup::LookupResult>(
          text, highlight(text, matchedTokens, prefixToken), score, payload);
    } else {
      result = make_shared<Lookup::LookupResult>(text, score, payload);
    }

    boundedTreeAdd(results, result, actualNum);
  }

  return deque<>(results.descendingSet());
}

void BlendedInfixSuggester::boundedTreeAdd(
    set<std::shared_ptr<Lookup::LookupResult>> &results,
    shared_ptr<Lookup::LookupResult> result, int num)
{

  if (results.size() >= num) {
    if (results.first().value < result->value) {
      results.pollFirst();
    } else {
      return;
    }
  }

  results.insert(result);
}

double BlendedInfixSuggester::createCoefficient(
    shared_ptr<IndexSearcher> searcher, int doc,
    shared_ptr<Set<wstring>> matchedTokens,
    const wstring &prefixToken) 
{

  shared_ptr<Terms> tv =
      searcher->getIndexReader()->getTermVector(doc, TEXT_FIELD_NAME);
  shared_ptr<TermsEnum> it = tv->begin();

  optional<int> position = numeric_limits<int>::max();
  shared_ptr<BytesRef> term;
  // find the closest token position
  while ((term = it->next()) != nullptr) {

    wstring docTerm = term->utf8ToString();

    if (matchedTokens->contains(docTerm) ||
        (prefixToken != L"" &&
         StringHelper::startsWith(docTerm, prefixToken))) {

      shared_ptr<PostingsEnum> docPosEnum =
          it->postings(nullptr, PostingsEnum::OFFSETS);
      docPosEnum->nextDoc();

      // use the first occurrence of the term
      int p = docPosEnum->nextPosition();
      if (p < position) {
        position = p;
      }
    }
  }

  // create corresponding coefficient based on position
  return calculateCoefficient(position);
}

double BlendedInfixSuggester::calculateCoefficient(int position)
{

  double coefficient;
  switch (blenderType) {
  case org::apache::lucene::search::suggest::analyzing::BlendedInfixSuggester::
      BlenderType::POSITION_LINEAR:
    coefficient = 1 - LINEAR_COEF * position;
    break;

  case org::apache::lucene::search::suggest::analyzing::BlendedInfixSuggester::
      BlenderType::POSITION_RECIPROCAL:
    coefficient = 1.0 / (position + 1);
    break;

  case org::apache::lucene::search::suggest::analyzing::BlendedInfixSuggester::
      BlenderType::POSITION_EXPONENTIAL_RECIPROCAL:
    coefficient = 1.0 / pow((position + 1.0), exponent);
    break;

  default:
    coefficient = 1;
  }

  return coefficient;
}

shared_ptr<java::util::Comparator<std::shared_ptr<
    org::apache::lucene::search::suggest::Lookup::LookupResult>>>
    BlendedInfixSuggester::LOOKUP_COMP = make_shared<LookUpComparator>();

int BlendedInfixSuggester::LookUpComparator::compare(
    shared_ptr<Lookup::LookupResult> o1, shared_ptr<Lookup::LookupResult> o2)
{
  // order on weight
  if (o1->value > o2->value) {
    return 1;
  } else if (o1->value < o2->value) {
    return -1;
  }

  // otherwise on alphabetic order
  int keyCompare = CHARSEQUENCE_COMPARATOR->compare(o1->key, o2->key);

  if (keyCompare != 0) {
    return keyCompare;
  }

  // if same weight and title, use the payload if there is one
  if (o1->payload != nullptr) {
    return o1->payload->compareTo(o2->payload);
  }

  return 0;
}
} // namespace org::apache::lucene::search::suggest::analyzing