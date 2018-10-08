using namespace std;

#include "MoreLikeThis.h"

namespace org::apache::lucene::queries::mlt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Document = org::apache::lucene::document::Document;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
std::deque<wstring> const MoreLikeThis::DEFAULT_FIELD_NAMES =
    std::deque<wstring>{L"contents"};
const shared_ptr < java::util::Set < ? >> MoreLikeThis::DEFAULT_STOP_WORDS =
                                           nullptr;

float MoreLikeThis::getBoostFactor() { return boostFactor; }

void MoreLikeThis::setBoostFactor(float boostFactor)
{
  this->boostFactor = boostFactor;
}

MoreLikeThis::MoreLikeThis(shared_ptr<IndexReader> ir)
    : MoreLikeThis(ir, new ClassicSimilarity())
{
}

MoreLikeThis::MoreLikeThis(shared_ptr<IndexReader> ir,
                           shared_ptr<TFIDFSimilarity> sim)
    : ir(ir)
{
  this->similarity = sim;
}

shared_ptr<TFIDFSimilarity> MoreLikeThis::getSimilarity() { return similarity; }

void MoreLikeThis::setSimilarity(shared_ptr<TFIDFSimilarity> similarity)
{
  this->similarity = similarity;
}

shared_ptr<Analyzer> MoreLikeThis::getAnalyzer() { return analyzer; }

void MoreLikeThis::setAnalyzer(shared_ptr<Analyzer> analyzer)
{
  this->analyzer = analyzer;
}

int MoreLikeThis::getMinTermFreq() { return minTermFreq; }

void MoreLikeThis::setMinTermFreq(int minTermFreq)
{
  this->minTermFreq = minTermFreq;
}

int MoreLikeThis::getMinDocFreq() { return minDocFreq; }

void MoreLikeThis::setMinDocFreq(int minDocFreq)
{
  this->minDocFreq = minDocFreq;
}

int MoreLikeThis::getMaxDocFreq() { return maxDocFreq; }

void MoreLikeThis::setMaxDocFreq(int maxFreq) { this->maxDocFreq = maxFreq; }

void MoreLikeThis::setMaxDocFreqPct(int maxPercentage)
{
  setMaxDocFreq(Math::toIntExact(static_cast<int64_t>(maxPercentage) *
                                 ir->numDocs() / 100));
}

bool MoreLikeThis::isBoost() { return boost; }

void MoreLikeThis::setBoost(bool boost) { this->boost = boost; }

std::deque<wstring> MoreLikeThis::getFieldNames() { return fieldNames; }

void MoreLikeThis::setFieldNames(std::deque<wstring> &fieldNames)
{
  this->fieldNames = fieldNames;
}

int MoreLikeThis::getMinWordLen() { return minWordLen; }

void MoreLikeThis::setMinWordLen(int minWordLen)
{
  this->minWordLen = minWordLen;
}

int MoreLikeThis::getMaxWordLen() { return maxWordLen; }

void MoreLikeThis::setMaxWordLen(int maxWordLen)
{
  this->maxWordLen = maxWordLen;
}

template <typename T1>
void MoreLikeThis::setStopWords(shared_ptr<Set<T1>> stopWords)
{
  this->stopWords = stopWords;
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: public std::unordered_set<?> getStopWords()
shared_ptr < Set < ? >> MoreLikeThis::getStopWords() { return stopWords; }

int MoreLikeThis::getMaxQueryTerms() { return maxQueryTerms; }

void MoreLikeThis::setMaxQueryTerms(int maxQueryTerms)
{
  this->maxQueryTerms = maxQueryTerms;
}

int MoreLikeThis::getMaxNumTokensParsed() { return maxNumTokensParsed; }

void MoreLikeThis::setMaxNumTokensParsed(int i) { maxNumTokensParsed = i; }

shared_ptr<Query> MoreLikeThis::like(int docNum) 
{
  if (fieldNames.empty()) {
    // gather deque of valid fields from lucene
    shared_ptr<deque<wstring>> fields = MultiFields::getIndexedFields(ir);
    fieldNames = fields->toArray(std::deque<wstring>(fields->size()));
  }

  return createQuery(retrieveTerms(docNum));
}

shared_ptr<Query> MoreLikeThis::like(unordered_map<wstring, deque<any>>
                                         &filteredDocument) 
{
  if (fieldNames.empty()) {
    // gather deque of valid fields from lucene
    shared_ptr<deque<wstring>> fields = MultiFields::getIndexedFields(ir);
    fieldNames = fields->toArray(std::deque<wstring>(fields->size()));
  }
  return createQuery(retrieveTerms(filteredDocument));
}

shared_ptr<Query> MoreLikeThis::like(const wstring &fieldName,
                                     deque<Reader> &readers) 
{
  unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
      perFieldTermFrequencies =
          unordered_map<wstring,
                        unordered_map<wstring, std::shared_ptr<Int>>>();
  for (shared_ptr<Reader> r : readers) {
    addTermFrequencies(r, perFieldTermFrequencies, fieldName);
  }
  return createQuery(createQueue(perFieldTermFrequencies));
}

shared_ptr<Query> MoreLikeThis::createQuery(
    shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> q)
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<ScoreTerm> scoreTerm;
  float bestScore = -1;

  while ((scoreTerm = q->pop()) != nullptr) {
    shared_ptr<Query> tq = make_shared<TermQuery>(
        make_shared<Term>(scoreTerm->topField, scoreTerm->word));

    if (boost) {
      if (bestScore == -1) {
        bestScore = (scoreTerm->score);
      }
      float myScore = (scoreTerm->score);
      tq = make_shared<BoostQuery>(tq, boostFactor * myScore / bestScore);
    }

    try {
      query->add(tq, BooleanClause::Occur::SHOULD);
    } catch (const BooleanQuery::TooManyClauses &ignore) {
      break;
    }
  }
  return query->build();
}

shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> MoreLikeThis::createQueue(
    unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
        &perFieldTermFrequencies) 
{
  // have collected all words in doc and their freqs
  int numDocs = ir->numDocs();
  constexpr int limit =
      min(maxQueryTerms, this->getTermsCount(perFieldTermFrequencies));
  shared_ptr<FreqQ> queue =
      make_shared<FreqQ>(limit); // will order words by score
  for (auto entry : perFieldTermFrequencies) {
    unordered_map<wstring, std::shared_ptr<Int>> perWordTermFrequencies =
        entry.second;
    wstring fieldName = entry.first;

    for (auto tfEntry : perWordTermFrequencies) { // for every word
      wstring word = tfEntry.first;
      int tf = tfEntry.second::x; // term freq in the source doc
      if (minTermFreq > 0 && tf < minTermFreq) {
        continue; // filter out words that don't occur enough times in the
                  // source
      }

      int docFreq = ir->docFreq(make_shared<Term>(fieldName, word));

      if (minDocFreq > 0 && docFreq < minDocFreq) {
        continue; // filter out words that don't occur in enough docs
      }

      if (docFreq > maxDocFreq) {
        continue; // filter out words that occur in too many docs
      }

      if (docFreq == 0) {
        continue; // index update problem?
      }

      float idf = similarity->idf(docFreq, numDocs);
      float score = tf * idf;

      if (queue->size() < limit) {
        // there is still space in the queue
        queue->push_back(
            make_shared<ScoreTerm>(word, fieldName, score, idf, docFreq, tf));
      } else {
        shared_ptr<ScoreTerm> term = queue->top();
        if (term->score < score) { // update the smallest in the queue in place
                                   // and update the queue.
          term->update(word, fieldName, score, idf, docFreq, tf);
          queue->updateTop();
        }
      }
    }
  }
  return queue;
}

int MoreLikeThis::getTermsCount(
    unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
        &perFieldTermFrequencies)
{
  int totalTermsCount = 0;
  shared_ptr<deque<unordered_map<wstring, std::shared_ptr<Int>>>> values =
      perFieldTermFrequencies.values();
  for (auto perWordTermFrequencies : values) {
    totalTermsCount += perWordTermFrequencies.size();
  }
  return totalTermsCount;
}

wstring MoreLikeThis::describeParams()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"\t")
      ->append(L"maxQueryTerms  : ")
      ->append(maxQueryTerms)
      ->append(L"\n");
  sb->append(L"\t")
      ->append(L"minWordLen     : ")
      ->append(minWordLen)
      ->append(L"\n");
  sb->append(L"\t")
      ->append(L"maxWordLen     : ")
      ->append(maxWordLen)
      ->append(L"\n");
  sb->append(L"\t")->append(L"fieldNames     : ");
  wstring delim = L"";
  for (auto fieldName : fieldNames) {
    sb->append(delim)->append(fieldName);
    delim = L", ";
  }
  sb->append(L"\n");
  sb->append(L"\t")->append(L"boost          : ")->append(boost)->append(L"\n");
  sb->append(L"\t")
      ->append(L"minTermFreq    : ")
      ->append(minTermFreq)
      ->append(L"\n");
  sb->append(L"\t")
      ->append(L"minDocFreq     : ")
      ->append(minDocFreq)
      ->append(L"\n");
  return sb->toString();
}

shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>>
MoreLikeThis::retrieveTerms(int docNum) 
{
  unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
      field2termFreqMap =
          unordered_map<wstring,
                        unordered_map<wstring, std::shared_ptr<Int>>>();
  for (auto fieldName : fieldNames) {
    shared_ptr<Fields> *const vectors = ir->getTermVectors(docNum);
    shared_ptr<Terms> *const deque;
    if (vectors->size() > 0) {
      deque = vectors->terms(fieldName);
    } else {
      deque.reset();
    }

    // field does not store term deque info
    if (deque == nullptr) {
      shared_ptr<Document> d = ir->document(docNum);
      std::deque<std::shared_ptr<IndexableField>> fields =
          d->getFields(fieldName);
      for (auto field : fields) {
        const wstring stringValue = field->stringValue();
        if (stringValue != L"") {
          addTermFrequencies(make_shared<StringReader>(stringValue),
                             field2termFreqMap, fieldName);
        }
      }
    } else {
      addTermFrequencies(field2termFreqMap, deque, fieldName);
    }
  }

  return createQueue(field2termFreqMap);
}

shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>>
MoreLikeThis::retrieveTerms(unordered_map<wstring, deque<any>>
                                &field2fieldValues) 
{
  unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
      field2termFreqMap =
          unordered_map<wstring,
                        unordered_map<wstring, std::shared_ptr<Int>>>();
  for (auto fieldName : fieldNames) {
    for (auto field : field2fieldValues) {
      shared_ptr<deque<any>> fieldValues = field2fieldValues[field.first];
      if (fieldValues == nullptr) {
        continue;
      }
      for (auto fieldValue : fieldValues) {
        if (fieldValue != nullptr) {
          addTermFrequencies(
              make_shared<StringReader>(wstring::valueOf(fieldValue)),
              field2termFreqMap, fieldName);
        }
      }
    }
  }
  return createQueue(field2termFreqMap);
}

void MoreLikeThis::addTermFrequencies(
    unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
        &field2termFreqMap,
    shared_ptr<Terms> deque, const wstring &fieldName) 
{
  unordered_map<wstring, std::shared_ptr<Int>> termFreqMap =
      field2termFreqMap[fieldName];
  if (termFreqMap.empty()) {
    termFreqMap = unordered_map<>();
    field2termFreqMap.emplace(fieldName, termFreqMap);
  }
  shared_ptr<TermsEnum> *const termsEnum = deque->begin();
  shared_ptr<CharsRefBuilder> *const spare = make_shared<CharsRefBuilder>();
  shared_ptr<BytesRef> text;
  while ((text = termsEnum->next()) != nullptr) {
    spare->copyUTF8Bytes(text);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring term = spare->toString();
    if (isNoiseWord(term)) {
      continue;
    }
    constexpr int freq = static_cast<int>(termsEnum->totalTermFreq());

    // increment frequency
    shared_ptr<Int> cnt = termFreqMap[term];
    if (cnt == nullptr) {
      cnt = make_shared<Int>();
      termFreqMap.emplace(term, cnt);
      cnt->x = freq;
    } else {
      cnt->x += freq;
    }
  }
}

void MoreLikeThis::addTermFrequencies(
    shared_ptr<Reader> r,
    unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
        &perFieldTermFrequencies,
    const wstring &fieldName) 
{
  if (analyzer == nullptr) {
    throw make_shared<UnsupportedOperationException>(
        wstring(L"To use MoreLikeThis without ") +
        L"term vectors, you must provide an Analyzer");
  }
  unordered_map<wstring, std::shared_ptr<Int>> termFreqMap =
      perFieldTermFrequencies[fieldName];
  if (termFreqMap.empty()) {
    termFreqMap = unordered_map<>();
    perFieldTermFrequencies.emplace(fieldName, termFreqMap);
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream(fieldName, r))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(fieldName, r);
    int tokenCount = 0;
    // for every token
    shared_ptr<CharTermAttribute> termAtt =
        ts->addAttribute(CharTermAttribute::typeid);
    ts->reset();
    while (ts->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring word = termAtt->toString();
      tokenCount++;
      if (tokenCount > maxNumTokensParsed) {
        break;
      }
      if (isNoiseWord(word)) {
        continue;
      }

      // increment frequency
      shared_ptr<Int> cnt = termFreqMap[word];
      if (cnt == nullptr) {
        termFreqMap.emplace(word, make_shared<Int>());
      } else {
        cnt->x++;
      }
    }
    ts->end();
  }
}

bool MoreLikeThis::isNoiseWord(const wstring &term)
{
  int len = term.length();
  if (minWordLen > 0 && len < minWordLen) {
    return true;
  }
  if (maxWordLen > 0 && len > maxWordLen) {
    return true;
  }
  return stopWords != nullptr && stopWords->contains(term);
}

shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>>
MoreLikeThis::retrieveTerms(shared_ptr<Reader> r,
                            const wstring &fieldName) 
{
  unordered_map<wstring, unordered_map<wstring, std::shared_ptr<Int>>>
      field2termFreqMap =
          unordered_map<wstring,
                        unordered_map<wstring, std::shared_ptr<Int>>>();
  addTermFrequencies(r, field2termFreqMap, fieldName);
  return createQueue(field2termFreqMap);
}

std::deque<wstring>
MoreLikeThis::retrieveInterestingTerms(int docNum) 
{
  deque<any> al = deque<any>(maxQueryTerms);
  shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> pq =
      retrieveTerms(docNum);
  shared_ptr<ScoreTerm> scoreTerm;
  int lim =
      maxQueryTerms; // have to be careful, retrieveTerms returns all words but
                     // that's probably not useful to our caller...
  // we just want to return the top words
  while (((scoreTerm = pq->pop()) != nullptr) && lim-- > 0) {
    al.push_back(scoreTerm->word); // the 1st entry is the interesting word
  }
  std::deque<wstring> res(al.size());
  return al.toArray(res);
}

std::deque<wstring> MoreLikeThis::retrieveInterestingTerms(
    shared_ptr<Reader> r, const wstring &fieldName) 
{
  deque<any> al = deque<any>(maxQueryTerms);
  shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> pq =
      retrieveTerms(r, fieldName);
  shared_ptr<ScoreTerm> scoreTerm;
  int lim =
      maxQueryTerms; // have to be careful, retrieveTerms returns all words but
                     // that's probably not useful to our caller...
  // we just want to return the top words
  while (((scoreTerm = pq->pop()) != nullptr) && lim-- > 0) {
    al.push_back(scoreTerm->word); // the 1st entry is the interesting word
  }
  std::deque<wstring> res(al.size());
  return al.toArray(res);
}

MoreLikeThis::FreqQ::FreqQ(int maxSize)
    : org::apache::lucene::util::PriorityQueue<ScoreTerm>(maxSize)
{
}

bool MoreLikeThis::FreqQ::lessThan(shared_ptr<ScoreTerm> a,
                                   shared_ptr<ScoreTerm> b)
{
  return a->score < b->score;
}

MoreLikeThis::ScoreTerm::ScoreTerm(const wstring &word, const wstring &topField,
                                   float score, float idf, int docFreq, int tf)
{
  this->word = word;
  this->topField = topField;
  this->score = score;
  this->idf = idf;
  this->docFreq = docFreq;
  this->tf = tf;
}

void MoreLikeThis::ScoreTerm::update(const wstring &word,
                                     const wstring &topField, float score,
                                     float idf, int docFreq, int tf)
{
  this->word = word;
  this->topField = topField;
  this->score = score;
  this->idf = idf;
  this->docFreq = docFreq;
  this->tf = tf;
}

MoreLikeThis::Int::Int() { x = 1; }
} // namespace org::apache::lucene::queries::mlt