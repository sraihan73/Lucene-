using namespace std;

#include "SpellChecker.h"

namespace org::apache::lucene::search::spell
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
const wstring SpellChecker::F_WORD = L"word";

SpellChecker::SpellChecker(shared_ptr<Directory> spellIndex,
                           shared_ptr<StringDistance> sd) 
    : SpellChecker(spellIndex, sd, SuggestWordQueue::DEFAULT_COMPARATOR)
{
}

SpellChecker::SpellChecker(shared_ptr<Directory> spellIndex) 
    : SpellChecker(spellIndex, new LevenshteinDistance())
{
}

SpellChecker::SpellChecker(shared_ptr<Directory> spellIndex,
                           shared_ptr<StringDistance> sd,
                           shared_ptr<Comparator<std::shared_ptr<SuggestWord>>>
                               comparator) 
{
  setSpellIndex(spellIndex);
  setStringDistance(sd);
  this->comparator = comparator;
}

void SpellChecker::setSpellIndex(shared_ptr<Directory> spellIndexDir) throw(
    IOException)
{
  // this could be the same directory as the current spellIndex
  // modifications to the directory should be synchronized
  {
    lock_guard<mutex> lock(modifyCurrentIndexLock);
    ensureOpen();
    if (!DirectoryReader::indexExists(spellIndexDir)) {
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          spellIndexDir, make_shared<IndexWriterConfig>(nullptr));
      delete writer;
    }
    swapSearcher(spellIndexDir);
  }
}

void SpellChecker::setComparator(
    shared_ptr<Comparator<std::shared_ptr<SuggestWord>>> comparator)
{
  this->comparator = comparator;
}

shared_ptr<Comparator<std::shared_ptr<SuggestWord>>>
SpellChecker::getComparator()
{
  return comparator;
}

void SpellChecker::setStringDistance(shared_ptr<StringDistance> sd)
{
  this->sd = sd;
}

shared_ptr<StringDistance> SpellChecker::getStringDistance() { return sd; }

void SpellChecker::setAccuracy(float acc) { this->accuracy = acc; }

float SpellChecker::getAccuracy() { return accuracy; }

std::deque<wstring> SpellChecker::suggestSimilar(const wstring &word,
                                                  int numSug) 
{
  return this->suggestSimilar(word, numSug, nullptr, L"",
                              SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
}

std::deque<wstring>
SpellChecker::suggestSimilar(const wstring &word, int numSug,
                             float accuracy) 
{
  return this->suggestSimilar(word, numSug, nullptr, L"",
                              SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX, accuracy);
}

std::deque<wstring>
SpellChecker::suggestSimilar(const wstring &word, int numSug,
                             shared_ptr<IndexReader> ir, const wstring &field,
                             SuggestMode suggestMode) 
{
  return suggestSimilar(word, numSug, ir, field, suggestMode, this->accuracy);
}

std::deque<wstring>
SpellChecker::suggestSimilar(const wstring &word, int numSug,
                             shared_ptr<IndexReader> ir, const wstring &field,
                             SuggestMode suggestMode,
                             float accuracy) 
{
  // obtainSearcher calls ensureOpen
  shared_ptr<IndexSearcher> *const indexSearcher = obtainSearcher();
  try {
    if (ir == nullptr || field == L"") {
      suggestMode = SuggestMode::SUGGEST_ALWAYS;
    }
    if (suggestMode == SuggestMode::SUGGEST_ALWAYS) {
      ir.reset();
      field = L"";
    }

    constexpr int lengthWord = word.length();

    constexpr int freq = (ir != nullptr && field != L"")
                             ? ir->docFreq(make_shared<Term>(field, word))
                             : 0;
    constexpr int goalFreq =
        suggestMode == SuggestMode::SUGGEST_MORE_POPULAR ? freq : 0;
    // if the word exists in the real index and we don't care for word
    // frequency, return the word itself
    if (suggestMode == SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX && freq > 0) {
      return std::deque<wstring>{word};
    }

    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    std::deque<wstring> grams;
    wstring key;

    for (int ng = getMin(lengthWord); ng <= getMax(lengthWord); ng++) {

      key = L"gram" + to_wstring(ng); // form key

      grams = formGrams(word, ng); // form word into ngrams (allow dups too)

      if (grams.empty()) {
        continue; // hmm
      }

      if (bStart > 0) { // should we boost prefixes?
        add(query, L"start" + to_wstring(ng), grams[0],
            bStart); // matches start of word
      }
      if (bEnd > 0) { // should we boost suffixes
        add(query, L"end" + to_wstring(ng), grams[grams.size() - 1],
            bEnd); // matches end of word
      }
      for (int i = 0; i < grams.size(); i++) {
        add(query, key, grams[i]);
      }
    }

    int maxHits = 10 * numSug;

    //    System.out.println("Q: " + query);
    std::deque<std::shared_ptr<ScoreDoc>> hits =
        indexSearcher->search(query->build(), maxHits)->scoreDocs;
    //    System.out.println("HITS: " + hits.length());
    shared_ptr<SuggestWordQueue> sugQueue =
        make_shared<SuggestWordQueue>(numSug, comparator);

    // go thru more than 'maxr' matches in case the distance filter triggers
    int stop = min(hits.size(), maxHits);
    shared_ptr<SuggestWord> sugWord = make_shared<SuggestWord>();
    for (int i = 0; i < stop; i++) {

      sugWord->string =
          indexSearcher->doc(hits[i]->doc)[F_WORD]; // get orig word

      // don't suggest a word for itself, that would be silly
      if (sugWord->string == word) {
        continue;
      }

      // edit distance
      sugWord->score = sd->getDistance(word, sugWord->string);
      if (sugWord->score < accuracy) {
        continue;
      }

      if (ir != nullptr && field != L"") { // use the user index
        sugWord->freq = ir->docFreq(
            make_shared<Term>(field, sugWord->string)); // freq in the index
        // don't suggest a word that is not present in the field
        if ((suggestMode == SuggestMode::SUGGEST_MORE_POPULAR &&
             goalFreq > sugWord->freq) ||
            sugWord->freq < 1) {
          continue;
        }
      }
      sugQueue->insertWithOverflow(sugWord);
      if (sugQueue->size() == numSug) {
        // if queue full, maintain the minScore score
        accuracy = sugQueue->top()->score;
      }
      sugWord = make_shared<SuggestWord>();
    }

    // convert to array string
    std::deque<wstring> deque(sugQueue->size());
    for (int i = sugQueue->size() - 1; i >= 0; i--) {
      deque[i] = sugQueue->pop()->string;
    }

    return deque;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    releaseSearcher(indexSearcher);
  }
}

void SpellChecker::add(shared_ptr<BooleanQuery::Builder> q, const wstring &name,
                       const wstring &value, float boost)
{
  shared_ptr<Query> tq = make_shared<TermQuery>(make_shared<Term>(name, value));
  q->add(make_shared<BooleanClause>(make_shared<BoostQuery>(tq, boost),
                                    BooleanClause::Occur::SHOULD));
}

void SpellChecker::add(shared_ptr<BooleanQuery::Builder> q, const wstring &name,
                       const wstring &value)
{
  q->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(name, value)),
      BooleanClause::Occur::SHOULD));
}

std::deque<wstring> SpellChecker::formGrams(const wstring &text, int ng)
{
  int len = text.length();
  std::deque<wstring> res(len - ng + 1);
  for (int i = 0; i < len - ng + 1; i++) {
    res[i] = text.substr(i, ng);
  }
  return res;
}

void SpellChecker::clearIndex() 
{
  {
    lock_guard<mutex> lock(modifyCurrentIndexLock);
    ensureOpen();
    shared_ptr<Directory> *const dir = this->spellIndex;
    shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
        dir,
        make_shared<IndexWriterConfig>(nullptr).setOpenMode(OpenMode::CREATE));
    delete writer;
    swapSearcher(dir);
  }
}

bool SpellChecker::exist(const wstring &word) 
{
  // obtainSearcher calls ensureOpen
  shared_ptr<IndexSearcher> *const indexSearcher = obtainSearcher();
  try {
    // TODO: we should use ReaderUtil+seekExact, we dont care about the docFreq
    // this is just an existence check
    return indexSearcher->getIndexReader()->docFreq(
               make_shared<Term>(F_WORD, word)) > 0;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    releaseSearcher(indexSearcher);
  }
}

void SpellChecker::indexDictionary(shared_ptr<Dictionary> dict,
                                   shared_ptr<IndexWriterConfig> config,
                                   bool fullMerge) 
{
  {
    lock_guard<mutex> lock(modifyCurrentIndexLock);
    ensureOpen();
    shared_ptr<Directory> *const dir = this->spellIndex;
    shared_ptr<IndexWriter> *const writer =
        make_shared<IndexWriter>(dir, config);
    shared_ptr<IndexSearcher> indexSearcher = obtainSearcher();
    const deque<std::shared_ptr<TermsEnum>> termsEnums =
        deque<std::shared_ptr<TermsEnum>>();

    shared_ptr<IndexReader> *const reader = searcher->getIndexReader();
    if (reader->maxDoc() > 0) {
      for (auto ctx : reader->leaves()) {
        shared_ptr<Terms> terms = ctx->reader()->terms(F_WORD);
        if (terms != nullptr) {
          termsEnums.push_back(terms->begin());
        }
      }
    }

    bool isEmpty = termsEnums.empty();

    try {
      shared_ptr<BytesRefIterator> iter = dict->getEntryIterator();
      shared_ptr<BytesRef> currentTerm;

      while ((currentTerm = iter->next()) != nullptr) {

        wstring word = currentTerm->utf8ToString();
        int len = word.length();
        if (len < 3) {
          continue; // too short we bail but "too long" is fine...
        }

        if (!isEmpty) {
          for (auto te : termsEnums) {
            if (te->seekExact(currentTerm)) {
              goto termsContinue;
            }
          }
        }

        // ok index the word
        shared_ptr<Document> doc =
            createDocument(word, getMin(len), getMax(len));
        writer->addDocument(doc);
      termsContinue:;
      }
    termsBreak:;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      releaseSearcher(indexSearcher);
    }
    if (fullMerge) {
      writer->forceMerge(1);
    }
    // close writer
    delete writer;
    // TODO: this isn't that great, maybe in the future SpellChecker should take
    // IWC in its ctor / keep its writer open?

    // also re-open the spell index to see our own changes when the next
    // suggestion is fetched:
    swapSearcher(dir);
  }
}

int SpellChecker::getMin(int l)
{
  if (l > 5) {
    return 3;
  }
  if (l == 5) {
    return 2;
  }
  return 1;
}

int SpellChecker::getMax(int l)
{
  if (l > 5) {
    return 4;
  }
  if (l == 5) {
    return 3;
  }
  return 2;
}

shared_ptr<Document> SpellChecker::createDocument(const wstring &text, int ng1,
                                                  int ng2)
{
  shared_ptr<Document> doc = make_shared<Document>();
  // the word field is never queried on... it's indexed so it can be quickly
  // checked for rebuild (and stored for retrieval). Doesn't need norms or
  // TF/pos
  shared_ptr<Field> f =
      make_shared<StringField>(F_WORD, text, Field::Store::YES);
  doc->push_back(f); // orig term
  addGram(text, doc, ng1, ng2);
  return doc;
}

void SpellChecker::addGram(const wstring &text, shared_ptr<Document> doc,
                           int ng1, int ng2)
{
  int len = text.length();
  for (int ng = ng1; ng <= ng2; ng++) {
    wstring key = L"gram" + to_wstring(ng);
    wstring end = L"";
    for (int i = 0; i < len - ng + 1; i++) {
      wstring gram = text.substr(i, ng);
      shared_ptr<FieldType> ft =
          make_shared<FieldType>(StringField::TYPE_NOT_STORED);
      ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
      shared_ptr<Field> ngramField = make_shared<Field>(key, gram, ft);
      // spellchecker does not use positional queries, but we want freqs
      // for scoring these multivalued n-gram fields.
      doc->push_back(ngramField);
      if (i == 0) {
        // only one term possible in the startXXField, TF/pos and norms aren't
        // needed.
        shared_ptr<Field> startField = make_shared<StringField>(
            L"start" + to_wstring(ng), gram, Field::Store::NO);
        doc->push_back(startField);
      }
      end = gram;
    }
    if (end != L"") { // may not be present if len==ng1
      // only one term possible in the endXXField, TF/pos and norms aren't
      // needed.
      shared_ptr<Field> endField = make_shared<StringField>(
          L"end" + to_wstring(ng), end, Field::Store::NO);
      doc->push_back(endField);
    }
  }
}

shared_ptr<IndexSearcher> SpellChecker::obtainSearcher()
{
  {
    lock_guard<mutex> lock(searcherLock);
    ensureOpen();
    searcher->getIndexReader()->incRef();
    return searcher;
  }
}

void SpellChecker::releaseSearcher(shared_ptr<IndexSearcher> aSearcher) throw(
    IOException)
{
  // don't check if open - always decRef
  // don't decrement the private searcher - could have been swapped
  aSearcher->getIndexReader()->decRef();
}

void SpellChecker::ensureOpen()
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(L"Spellchecker has been closed");
  }
}

SpellChecker::~SpellChecker()
{
  {
    lock_guard<mutex> lock(searcherLock);
    ensureOpen();
    closed = true;
    if (searcher != nullptr) {
      delete searcher->getIndexReader();
    }
    searcher.reset();
  }
}

void SpellChecker::swapSearcher(shared_ptr<Directory> dir) 
{
  /*
   * opening a searcher is possibly very expensive.
   * We rather close it again if the Spellchecker was closed during
   * this operation than block access to the current searcher while opening.
   */
  shared_ptr<IndexSearcher> *const indexSearcher = createSearcher(dir);
  {
    lock_guard<mutex> lock(searcherLock);
    if (closed) {
      delete indexSearcher->getIndexReader();
      throw make_shared<AlreadyClosedException>(
          L"Spellchecker has been closed");
    }
    if (searcher != nullptr) {
      delete searcher->getIndexReader();
    }
    // set the spellindex in the sync block - ensure consistency.
    searcher = indexSearcher;
    this->spellIndex = dir;
  }
}

shared_ptr<IndexSearcher>
SpellChecker::createSearcher(shared_ptr<Directory> dir) 
{
  return make_shared<IndexSearcher>(DirectoryReader::open(dir));
}

bool SpellChecker::isClosed() { return closed; }
} // namespace org::apache::lucene::search::spell