using namespace std;

#include "UnifiedHighlighter.h"

namespace org::apache::lucene::search::uhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using FieldType = org::apache::lucene::document::FieldType;
using BaseCompositeReader = org::apache::lucene::index::BaseCompositeReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiFields = org::apache::lucene::index::MultiFields;
using MultiReader = org::apache::lucene::index::MultiReader;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using Weight = org::apache::lucene::search::Weight;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
const shared_ptr<org::apache::lucene::search::IndexSearcher>
    UnifiedHighlighter::EMPTY_INDEXSEARCHER;

UnifiedHighlighter::StaticConstructor::StaticConstructor()
{
  try {
    shared_ptr<IndexReader> emptyReader = make_shared<MultiReader>();
    EMPTY_INDEXSEARCHER = make_shared<IndexSearcher>(emptyReader);
    EMPTY_INDEXSEARCHER->setQueryCache(nullptr);
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

UnifiedHighlighter::StaticConstructor UnifiedHighlighter::staticConstructor;
std::deque<std::shared_ptr<
    org::apache::lucene::util::automaton::CharacterRunAutomaton>> const
    UnifiedHighlighter::ZERO_LEN_AUTOMATA_ARRAY = std::deque<std::shared_ptr<
        org::apache::lucene::util::automaton::CharacterRunAutomaton>>(0);

shared_ptr<Set<std::shared_ptr<Term>>>
UnifiedHighlighter::extractTerms(shared_ptr<Query> query) 
{
  shared_ptr<Set<std::shared_ptr<Term>>> queryTerms =
      unordered_set<std::shared_ptr<Term>>();
  EMPTY_INDEXSEARCHER
      ->createWeight(EMPTY_INDEXSEARCHER->rewrite(query), false, 1)
      ->extractTerms(queryTerms);
  return queryTerms;
}

UnifiedHighlighter::UnifiedHighlighter(shared_ptr<IndexSearcher> indexSearcher,
                                       shared_ptr<Analyzer> indexAnalyzer)
    : searcher(indexSearcher) / *TODO : make non nullable * /
, indexAnalyzer(Objects::requireNonNull(
      indexAnalyzer, wstring(L"indexAnalyzer is required") +
                         L" (even if in some circumstances it isn't used)"))
{
}

void UnifiedHighlighter::setHandleMultiTermQuery(bool handleMtq)
{
  this->defaultHandleMtq = handleMtq;
}

void UnifiedHighlighter::setHighlightPhrasesStrictly(
    bool highlightPhrasesStrictly)
{
  this->defaultHighlightPhrasesStrictly = highlightPhrasesStrictly;
}

void UnifiedHighlighter::setMaxLength(int maxLength)
{
  if (maxLength < 0 || maxLength == numeric_limits<int>::max()) {
    // two reasons: no overflow problems in BreakIterator.preceding(offset+1),
    // our sentinel in the offsets queue uses this value to terminate.
    throw invalid_argument(L"maxLength must be < Integer.MAX_VALUE");
  }
  this->maxLength = maxLength;
}

void UnifiedHighlighter::setBreakIterator(
    function<BreakIterator *()> &breakIterator)
{
  this->defaultBreakIterator = breakIterator;
}

void UnifiedHighlighter::setScorer(shared_ptr<PassageScorer> scorer)
{
  this->defaultScorer = scorer;
}

void UnifiedHighlighter::setFormatter(shared_ptr<PassageFormatter> formatter)
{
  this->defaultFormatter = formatter;
}

void UnifiedHighlighter::setMaxNoHighlightPassages(
    int defaultMaxNoHighlightPassages)
{
  this->defaultMaxNoHighlightPassages = defaultMaxNoHighlightPassages;
}

void UnifiedHighlighter::setCacheFieldValCharsThreshold(
    int cacheFieldValCharsThreshold)
{
  this->cacheFieldValCharsThreshold = cacheFieldValCharsThreshold;
}

void UnifiedHighlighter::setFieldMatcher(
    function<bool(const wstring &)> &predicate)
{
  this->defaultFieldMatcher = predicate;
}

bool UnifiedHighlighter::shouldHandleMultiTermQuery(const wstring &field)
{
  return defaultHandleMtq;
}

bool UnifiedHighlighter::shouldHighlightPhrasesStrictly(const wstring &field)
{
  return defaultHighlightPhrasesStrictly;
}

bool UnifiedHighlighter::shouldPreferPassageRelevancyOverSpeed(
    const wstring &field)
{
  return defaultPassageRelevancyOverSpeed;
}

shared_ptr<protected> function < UnifiedHighlighter::bool(shared_ptr<const> std)
{
  if (defaultFieldMatcher != nullptr) {
    return defaultFieldMatcher;
  } else {
    // requireFieldMatch = true
    return [&](qf) { field::equals(qf); };
  }
}

int UnifiedHighlighter::getMaxLength() { return maxLength; }

shared_ptr<BreakIterator>
UnifiedHighlighter::getBreakIterator(const wstring &field)
{
  return defaultBreakIterator->get();
}

shared_ptr<PassageScorer> UnifiedHighlighter::getScorer(const wstring &field)
{
  return defaultScorer;
}

shared_ptr<PassageFormatter>
UnifiedHighlighter::getFormatter(const wstring &field)
{
  return defaultFormatter;
}

int UnifiedHighlighter::getMaxNoHighlightPassages(const wstring &field)
{
  return defaultMaxNoHighlightPassages;
}

int UnifiedHighlighter::getCacheFieldValCharsThreshold()
{ // question: should we size by bytes instead?
  return cacheFieldValCharsThreshold;
}

shared_ptr<IndexSearcher> UnifiedHighlighter::getIndexSearcher()
{
  return searcher;
}

shared_ptr<Analyzer> UnifiedHighlighter::getIndexAnalyzer()
{
  return indexAnalyzer;
}

UnifiedHighlighter::OffsetSource
UnifiedHighlighter::getOffsetSource(const wstring &field)
{
  shared_ptr<FieldInfo> fieldInfo = getFieldInfo(field);
  if (fieldInfo != nullptr) {
    if (fieldInfo->getIndexOptions() ==
        IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) {
      return fieldInfo->hasVectors() ? OffsetSource::POSTINGS_WITH_TERM_VECTORS
                                     : OffsetSource::POSTINGS;
    }
    if (fieldInfo->hasVectors()) { // unfortunately we can't also check if the
                                   // TV has offsets
      return OffsetSource::TERM_VECTORS;
    }
  }
  return OffsetSource::ANALYSIS;
}

shared_ptr<FieldInfo> UnifiedHighlighter::getFieldInfo(const wstring &field)
{
  if (searcher == nullptr) {
    return nullptr;
  }
  // Need thread-safety for lazy-init but lets avoid 'synchronized' by using
  // double-check locking idiom
  shared_ptr<FieldInfos> fieldInfos =
      this->fieldInfos; // note: it's volatile; read once
  if (fieldInfos->empty()) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      fieldInfos = this->fieldInfos;
      if (fieldInfos->empty()) {
        fieldInfos =
            MultiFields::getMergedFieldInfos(searcher->getIndexReader());
        this->fieldInfos = fieldInfos;
      }
    }
  }
  return fieldInfos->fieldInfo(field);
}

std::deque<wstring>
UnifiedHighlighter::highlight(const wstring &field, shared_ptr<Query> query,
                              shared_ptr<TopDocs> topDocs) 
{
  return highlight(field, query, topDocs, 1);
}

std::deque<wstring>
UnifiedHighlighter::highlight(const wstring &field, shared_ptr<Query> query,
                              shared_ptr<TopDocs> topDocs,
                              int maxPassages) 
{
  unordered_map<wstring, std::deque<wstring>> res =
      highlightFields(std::deque<wstring>{field}, query, topDocs,
                      std::deque<int>{maxPassages});
  return res[field];
}

unordered_map<wstring, std::deque<wstring>>
UnifiedHighlighter::highlightFields(
    std::deque<wstring> &fields, shared_ptr<Query> query,
    shared_ptr<TopDocs> topDocs) 
{
  std::deque<int> maxPassages(fields.size());
  Arrays::fill(maxPassages, 1);
  return highlightFields(fields, query, topDocs, maxPassages);
}

unordered_map<wstring, std::deque<wstring>>
UnifiedHighlighter::highlightFields(
    std::deque<wstring> &fields, shared_ptr<Query> query,
    shared_ptr<TopDocs> topDocs,
    std::deque<int> &maxPassages) 
{
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs = topDocs->scoreDocs;
  std::deque<int> docids(scoreDocs.size());
  for (int i = 0; i < docids.size(); i++) {
    docids[i] = scoreDocs[i]->doc;
  }

  return highlightFields(fields, query, docids, maxPassages);
}

unordered_map<wstring, std::deque<wstring>>
UnifiedHighlighter::highlightFields(
    std::deque<wstring> &fieldsIn, shared_ptr<Query> query,
    std::deque<int> &docidsIn,
    std::deque<int> &maxPassagesIn) 
{
  unordered_map<wstring, std::deque<wstring>> snippets =
      unordered_map<wstring, std::deque<wstring>>();
  for (auto ent :
       highlightFieldsAsObjects(fieldsIn, query, docidsIn, maxPassagesIn)) {
    std::deque<any> snippetObjects = ent.second;
    std::deque<wstring> snippetStrings(snippetObjects.size());
    snippets.emplace(ent.first, snippetStrings);
    for (int i = 0; i < snippetObjects.size(); i++) {
      any snippet = snippetObjects[i];
      if (snippet != nullptr) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        snippetStrings[i] = snippet.toString();
      }
    }
  }

  return snippets;
}

unordered_map<wstring, std::deque<any>>
UnifiedHighlighter::highlightFieldsAsObjects(
    std::deque<wstring> &fieldsIn, shared_ptr<Query> query,
    std::deque<int> &docIdsIn,
    std::deque<int> &maxPassagesIn) 
{
  if (fieldsIn.size() < 1) {
    throw invalid_argument(L"fieldsIn must not be empty");
  }
  if (fieldsIn.size() != maxPassagesIn.size()) {
    throw invalid_argument(L"invalid number of maxPassagesIn");
  }
  if (searcher == nullptr) {
    throw make_shared<IllegalStateException>(
        wstring(
            L"This method requires that an indexSearcher was passed in the ") +
        L"constructor.  Perhaps you mean to call highlightWithoutSearcher?");
  }

  // Sort docs & fields for sequential i/o

  // Sort doc IDs w/ index to original order: (copy input arrays since we sort
  // in-place)
  std::deque<int> docIds(docIdsIn.size());
  std::deque<int> docInIndexes(
      docIds.size()); // fill in ascending order; points into docIdsIn[]
  copyAndSortDocIdsWithIndex(docIdsIn, docIds,
                             docInIndexes); // latter 2 are "out" params

  // Sort fields w/ maxPassages pair: (copy input arrays since we sort in-place)
  const std::deque<wstring> fields = std::deque<wstring>(fieldsIn.size());
  const std::deque<int> maxPassages = std::deque<int>(maxPassagesIn.size());
  copyAndSortFieldsWithMaxPassages(fieldsIn, maxPassagesIn, fields,
                                   maxPassages); // latter 2 are "out" params

  // Init field highlighters (where most of the highlight logic lives, and on a
  // per field basis)
  shared_ptr<Set<std::shared_ptr<Term>>> queryTerms = extractTerms(query);
  std::deque<std::shared_ptr<FieldHighlighter>> fieldHighlighters(
      fields.size());
  int numTermVectors = 0;
  int numPostings = 0;
  for (int f = 0; f < fields.size(); f++) {
    shared_ptr<FieldHighlighter> fieldHighlighter =
        getFieldHighlighter(fields[f], query, queryTerms, maxPassages[f]);
    fieldHighlighters[f] = fieldHighlighter;

    switch (fieldHighlighter->getOffsetSource()) {
    case TERM_VECTORS:
      numTermVectors++;
      break;
    case POSTINGS:
      numPostings++;
      break;
    case POSTINGS_WITH_TERM_VECTORS:
      numTermVectors++;
      numPostings++;
      break;
    case ANALYSIS:
    case NONE_NEEDED:
    default:
      // do nothing
      break;
    }
  }

  int cacheCharsThreshold =
      calculateOptimalCacheCharsThreshold(numTermVectors, numPostings);

  shared_ptr<IndexReader> indexReaderWithTermVecCache =
      (numTermVectors >= 2)
          ? TermVectorReusingLeafReader::wrap(searcher->getIndexReader())
          : nullptr;

  // [fieldIdx][docIdInIndex] of highlightDoc result
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: Object[][] highlightDocsInByField = new
  // Object[fields.length][docIds.length];
  std::deque<std::deque<any>> highlightDocsInByField =
      RectangularVectors::ReturnRectangularStdanyVector(fields.size(),
                                                        docIds.size());
  // Highlight in doc batches determined by loadFieldValues (consumes from
  // docIdIter)
  shared_ptr<DocIdSetIterator> docIdIter = asDocIdSetIterator(docIds);
  for (int batchDocIdx = 0; batchDocIdx < docIds.size();) {
    // Load the field values of the first batch of document(s) (note: commonly
    // all docs are in this batch)
    deque<std::deque<std::shared_ptr<std::wstring>>> fieldValsByDoc =
        loadFieldValues(fields, docIdIter, cacheCharsThreshold);
    //    the size of the above deque is the size of the batch (num of docs in
    //    the batch)

    // Highlight in per-field order first, then by doc (better I/O pattern)
    for (int fieldIdx = 0; fieldIdx < fields.size(); fieldIdx++) {
      std::deque<any> resultByDocIn =
          highlightDocsInByField[fieldIdx]; // parallel to docIdsIn
      shared_ptr<FieldHighlighter> fieldHighlighter =
          fieldHighlighters[fieldIdx];
      for (int docIdx = batchDocIdx;
           docIdx - batchDocIdx < fieldValsByDoc.size(); docIdx++) {
        int docId = docIds[docIdx]; // sorted order
        shared_ptr<std::wstring> content =
            fieldValsByDoc[docIdx - batchDocIdx][fieldIdx];
        if (content == nullptr) {
          continue;
        }
        shared_ptr<IndexReader> indexReader =
            (fieldHighlighter->getOffsetSource() ==
                 OffsetSource::TERM_VECTORS &&
             indexReaderWithTermVecCache != nullptr)
                ? indexReaderWithTermVecCache
                : searcher->getIndexReader();
        int docInIndex = docInIndexes[docIdx]; // original input order
        assert(resultByDocIn[docInIndex] == nullptr);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        resultByDocIn[docInIndex] = fieldHighlighter->highlightFieldForDoc(
            indexReader, docId, content->toString());
      }
    }

    batchDocIdx += fieldValsByDoc.size();
  }
  assert(docIdIter->docID() == DocIdSetIterator::NO_MORE_DOCS ||
         docIdIter->nextDoc() == DocIdSetIterator::NO_MORE_DOCS);

  // TODO reconsider the return type; since this is an "advanced" method, lets
  // not return a Map?  Notice the only
  //    caller simply iterates it to build another structure.

  // field -> object highlights parallel to docIdsIn
  unordered_map<wstring, std::deque<any>> resultMap =
      unordered_map<wstring, std::deque<any>>(fields.size());
  for (int f = 0; f < fields.size(); f++) {
    resultMap.emplace(fields[f], highlightDocsInByField[f]);
  }
  return resultMap;
}

int UnifiedHighlighter::calculateOptimalCacheCharsThreshold(int numTermVectors,
                                                            int numPostings)
{
  if (numPostings == 0 && numTermVectors == 0) {
    // (1) When all fields are ANALYSIS there's no point in caching a batch of
    // documents because no other info on disk is needed to highlight it.
    return 0;
  } else if (numTermVectors >= 2) {
    // (2) When two or more fields have term vectors, given the field-then-doc
    // algorithm, the underlying term vectors will be fetched in a terrible
    // access pattern unless we highlight a doc at a time and use a special
    // current-doc TV cache.  So we do that.  Hopefully one day TVs will be
    // improved to make this pointless.
    return 0;
  } else {
    return getCacheFieldValCharsThreshold();
  }
}

void UnifiedHighlighter::copyAndSortFieldsWithMaxPassages(
    std::deque<wstring> &fieldsIn, std::deque<int> &maxPassagesIn,
    std::deque<wstring> &fields, std::deque<int> &maxPassages)
{
  System::arraycopy(fieldsIn, 0, fields, 0, fieldsIn.size());
  System::arraycopy(maxPassagesIn, 0, maxPassages, 0, maxPassagesIn.size());
  make_shared<InPlaceMergeSorterAnonymousInnerClass>(shared_from_this(), fields,
                                                     maxPassages)
      .sort(0, fields.size());
}

UnifiedHighlighter::InPlaceMergeSorterAnonymousInnerClass::
    InPlaceMergeSorterAnonymousInnerClass(
        shared_ptr<UnifiedHighlighter> outerInstance, deque<wstring> &fields,
        deque<int> &maxPassages)
{
  this->outerInstance = outerInstance;
  this->fields = fields;
  this->maxPassages = maxPassages;
}

void UnifiedHighlighter::InPlaceMergeSorterAnonymousInnerClass::swap(int i,
                                                                     int j)
{
  wstring tmp = fields[i];
  fields[i] = fields[j];
  fields[j] = tmp;
  int tmp2 = maxPassages[i];
  maxPassages[i] = maxPassages[j];
  maxPassages[j] = tmp2;
}

int UnifiedHighlighter::InPlaceMergeSorterAnonymousInnerClass::compare(int i,
                                                                       int j)
{
  return fields[i].compare(fields[j]);
}

void UnifiedHighlighter::copyAndSortDocIdsWithIndex(
    std::deque<int> &docIdsIn, std::deque<int> &docIds,
    std::deque<int> &docInIndexes)
{
  System::arraycopy(docIdsIn, 0, docIds, 0, docIdsIn.size());
  for (int i = 0; i < docInIndexes.size(); i++) {
    docInIndexes[i] = i;
  }
  make_shared<InPlaceMergeSorterAnonymousInnerClass2>(shared_from_this(),
                                                      docIds, docInIndexes)
      .sort(0, docIds.size());
}

UnifiedHighlighter::InPlaceMergeSorterAnonymousInnerClass2::
    InPlaceMergeSorterAnonymousInnerClass2(
        shared_ptr<UnifiedHighlighter> outerInstance, deque<int> &docIds,
        deque<int> &docInIndexes)
{
  this->outerInstance = outerInstance;
  this->docIds = docIds;
  this->docInIndexes = docInIndexes;
}

void UnifiedHighlighter::InPlaceMergeSorterAnonymousInnerClass2::swap(int i,
                                                                      int j)
{
  int tmp = docIds[i];
  docIds[i] = docIds[j];
  docIds[j] = tmp;
  tmp = docInIndexes[i];
  docInIndexes[i] = docInIndexes[j];
  docInIndexes[j] = tmp;
}

int UnifiedHighlighter::InPlaceMergeSorterAnonymousInnerClass2::compare(int i,
                                                                        int j)
{
  return Integer::compare(docIds[i], docIds[j]);
}

any UnifiedHighlighter::highlightWithoutSearcher(
    const wstring &field, shared_ptr<Query> query, const wstring &content,
    int maxPassages) 
{
  if (this->searcher != nullptr) {
    throw make_shared<IllegalStateException>(
        L"highlightWithoutSearcher should only be called on a " +
        getClass().getSimpleName() + L" without an IndexSearcher.");
  }
  Objects::requireNonNull(content, L"content is required");
  shared_ptr<Set<std::shared_ptr<Term>>> queryTerms = extractTerms(query);
  return getFieldHighlighter(field, query, queryTerms, maxPassages)
      ->highlightFieldForDoc(nullptr, -1, content);
}

shared_ptr<FieldHighlighter> UnifiedHighlighter::getFieldHighlighter(
    const wstring &field, shared_ptr<Query> query,
    shared_ptr<Set<std::shared_ptr<Term>>> allTerms, int maxPassages)
{
  std::deque<std::shared_ptr<BytesRef>> terms =
      filterExtractedTerms(getFieldMatcher(field), allTerms);
  shared_ptr<Set<HighlightFlag>> highlightFlags = getFlags(field);
  shared_ptr<PhraseHelper> phraseHelper =
      getPhraseHelper(field, query, highlightFlags);
  std::deque<std::shared_ptr<CharacterRunAutomaton>> automata =
      getAutomata(field, query, highlightFlags);
  OffsetSource offsetSource =
      getOptimizedOffsetSource(field, terms, phraseHelper, automata);
  return make_shared<FieldHighlighter>(
      field,
      getOffsetStrategy(offsetSource, field, terms, phraseHelper, automata,
                        highlightFlags),
      make_shared<SplittingBreakIterator>(
          getBreakIterator(field), UnifiedHighlighter::MULTIVAL_SEP_CHAR),
      getScorer(field), maxPassages, getMaxNoHighlightPassages(field),
      getFormatter(field));
}

std::deque<std::shared_ptr<BytesRef>> UnifiedHighlighter::filterExtractedTerms(
    function<bool(const wstring &)> &fieldMatcher,
    shared_ptr<Set<std::shared_ptr<Term>>> queryTerms)
{
  // Strip off the redundant field and sort the remaining terms
  shared_ptr<SortedSet<std::shared_ptr<BytesRef>>> filteredTerms =
      set<std::shared_ptr<BytesRef>>();
  for (auto term : queryTerms) {
    if (fieldMatcher(term->field())) {
      filteredTerms->add(term->bytes());
    }
  }
  return filteredTerms->toArray(
      std::deque<std::shared_ptr<BytesRef>>(filteredTerms->size()));
}

shared_ptr<Set<HighlightFlag>>
UnifiedHighlighter::getFlags(const wstring &field)
{
  shared_ptr<Set<HighlightFlag>> highlightFlags =
      EnumSet::noneOf(HighlightFlag::typeid);
  if (shouldHandleMultiTermQuery(field)) {
    highlightFlags->add(HighlightFlag::MULTI_TERM_QUERY);
  }
  if (shouldHighlightPhrasesStrictly(field)) {
    highlightFlags->add(HighlightFlag::PHRASES);
  }
  if (shouldPreferPassageRelevancyOverSpeed(field)) {
    highlightFlags->add(HighlightFlag::PASSAGE_RELEVANCY_OVER_SPEED);
  }
  return highlightFlags;
}

shared_ptr<PhraseHelper> UnifiedHighlighter::getPhraseHelper(
    const wstring &field, shared_ptr<Query> query,
    shared_ptr<Set<HighlightFlag>> highlightFlags)
{
  bool highlightPhrasesStrictly =
      highlightFlags->contains(HighlightFlag::PHRASES);
  bool handleMultiTermQuery =
      highlightFlags->contains(HighlightFlag::MULTI_TERM_QUERY);
  return highlightPhrasesStrictly ? make_shared<PhraseHelper>(
                                        query, field, getFieldMatcher(field),
                                        shared_from_this()::requiresRewrite,
                                        shared_from_this()::preSpanQueryRewrite,
                                        !handleMultiTermQuery)
                                  : PhraseHelper::NONE;
}

std::deque<std::shared_ptr<CharacterRunAutomaton>>
UnifiedHighlighter::getAutomata(const wstring &field, shared_ptr<Query> query,
                                shared_ptr<Set<HighlightFlag>> highlightFlags)
{
  return highlightFlags->contains(HighlightFlag::MULTI_TERM_QUERY)
             ? MultiTermHighlighting::extractAutomata(
                   query, getFieldMatcher(field),
                   !highlightFlags->contains(HighlightFlag::PHRASES),
                   shared_from_this()::preMultiTermQueryRewrite)
             : ZERO_LEN_AUTOMATA_ARRAY;
}

UnifiedHighlighter::OffsetSource UnifiedHighlighter::getOptimizedOffsetSource(
    const wstring &field, std::deque<std::shared_ptr<BytesRef>> &terms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata)
{
  OffsetSource offsetSource = getOffsetSource(field);

  if (terms.empty() && automata.empty() && !phraseHelper->willRewrite()) {
    return OffsetSource::NONE_NEEDED; // nothing to highlight
  }

  switch (offsetSource) {
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::POSTINGS:
    if (phraseHelper->willRewrite()) {
      // We can't choose the postings offset source when there is "rewriting" in
      // the strict phrase processing (rare but possible). Postings requires
      // knowing all the terms (except wildcards) up front.
      return OffsetSource::ANALYSIS;
    } else if (automata.size() > 0) {
      return OffsetSource::ANALYSIS;
    }
    break;
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::POSTINGS_WITH_TERM_VECTORS:
    if (!phraseHelper->willRewrite() && automata.empty()) {
      return OffsetSource::POSTINGS; // We don't need term vectors
    }
    break;
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::ANALYSIS:
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::TERM_VECTORS:
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::NONE_NEEDED:
  default:
    // stick with the original offset source
    break;
  }

  return offsetSource;
}

shared_ptr<FieldOffsetStrategy> UnifiedHighlighter::getOffsetStrategy(
    OffsetSource offsetSource, const wstring &field,
    std::deque<std::shared_ptr<BytesRef>> &terms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
    shared_ptr<Set<HighlightFlag>> highlightFlags)
{
  switch (offsetSource) {
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::ANALYSIS:
    if (!phraseHelper->hasPositionSensitivity() &&
        !highlightFlags->contains(
            HighlightFlag::PASSAGE_RELEVANCY_OVER_SPEED)) {
      // skip using a memory index since it's pure term filtering
      return make_shared<TokenStreamOffsetStrategy>(
          field, terms, phraseHelper, automata, getIndexAnalyzer());
    } else {
      return make_shared<MemoryIndexOffsetStrategy>(
          field, getFieldMatcher(field), terms, phraseHelper, automata,
          getIndexAnalyzer(), shared_from_this()::preMultiTermQueryRewrite);
    }
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::NONE_NEEDED:
    return NoOpOffsetStrategy::INSTANCE;
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::TERM_VECTORS:
    return make_shared<TermVectorOffsetStrategy>(field, terms, phraseHelper,
                                                 automata);
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::POSTINGS:
    return make_shared<PostingsOffsetStrategy>(field, terms, phraseHelper,
                                               automata);
  case org::apache::lucene::search::uhighlight::UnifiedHighlighter::
      OffsetSource::POSTINGS_WITH_TERM_VECTORS:
    return make_shared<PostingsWithTermVectorsOffsetStrategy>(
        field, terms, phraseHelper, automata);
  default:
    throw invalid_argument(L"Unrecognized offset source " + offsetSource);
  }
}

optional<bool>
UnifiedHighlighter::requiresRewrite(shared_ptr<SpanQuery> spanQuery)
{
  return nullopt;
}

shared_ptr<deque<std::shared_ptr<Query>>>
UnifiedHighlighter::preSpanQueryRewrite(shared_ptr<Query> query)
{
  return nullptr;
}

shared_ptr<deque<std::shared_ptr<Query>>>
UnifiedHighlighter::preMultiTermQueryRewrite(shared_ptr<Query> query)
{
  return nullptr;
}

shared_ptr<DocIdSetIterator>
UnifiedHighlighter::asDocIdSetIterator(std::deque<int> &sortedDocIds)
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this(),
                                                          sortedDocIds);
}

UnifiedHighlighter::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<UnifiedHighlighter> outerInstance, deque<int> &sortedDocIds)
{
  this->outerInstance = outerInstance;
  this->sortedDocIds = sortedDocIds;
  idx = -1;
}

int UnifiedHighlighter::DocIdSetIteratorAnonymousInnerClass::docID()
{
  if (idx < 0 || idx >= sortedDocIds.size()) {
    return DocIdSetIterator::NO_MORE_DOCS;
  }
  return sortedDocIds[idx];
}

int UnifiedHighlighter::DocIdSetIteratorAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  idx++;
  return docID();
}

int UnifiedHighlighter::DocIdSetIteratorAnonymousInnerClass::advance(
    int target) 
{
  return DocIdSetIterator::slowAdvance(target); // won't be called, so whatever
}

int64_t UnifiedHighlighter::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return max(0, sortedDocIds.size() - (idx + 1)); // remaining docs
}

deque<std::deque<std::shared_ptr<std::wstring>>>
UnifiedHighlighter::loadFieldValues(std::deque<wstring> &fields,
                                    shared_ptr<DocIdSetIterator> docIter,
                                    int cacheCharsThreshold) 
{
  deque<std::deque<std::shared_ptr<std::wstring>>> docListOfFields =
      deque<std::deque<std::shared_ptr<std::wstring>>>(
          cacheCharsThreshold == 0
              ? 1
              : static_cast<int>(min(64, docIter->cost())));

  shared_ptr<LimitedStoredFieldVisitor> visitor =
      newLimitedStoredFieldsVisitor(fields);
  int sumChars = 0;
  do {
    int docId = docIter->nextDoc();
    if (docId == DocIdSetIterator::NO_MORE_DOCS) {
      break;
    }
    visitor->init();
    searcher->doc(docId, visitor);
    std::deque<std::shared_ptr<std::wstring>> valuesByField =
        visitor->getValuesByField();
    docListOfFields.push_back(valuesByField);
    for (auto val : valuesByField) {
      sumChars += (val == nullptr ? 0 : val->length());
    }
  } while (sumChars <= cacheCharsThreshold && cacheCharsThreshold != 0);
  return docListOfFields;
}

shared_ptr<LimitedStoredFieldVisitor>
UnifiedHighlighter::newLimitedStoredFieldsVisitor(std::deque<wstring> &fields)
{
  return make_shared<LimitedStoredFieldVisitor>(fields, MULTIVAL_SEP_CHAR,
                                                getMaxLength());
}

UnifiedHighlighter::LimitedStoredFieldVisitor::LimitedStoredFieldVisitor(
    std::deque<wstring> &fields, wchar_t valueSeparator, int maxLength)
    : fields(fields), valueSeparator(valueSeparator), maxLength(maxLength)
{
}

void UnifiedHighlighter::LimitedStoredFieldVisitor::init()
{
  values = std::deque<std::shared_ptr<std::wstring>>(fields.size());
  currentField = -1;
}

void UnifiedHighlighter::LimitedStoredFieldVisitor::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &byteValue) 
{
  wstring value = wstring(byteValue, StandardCharsets::UTF_8);
  assert(currentField >= 0);
  shared_ptr<std::wstring> curValue = values[currentField];
  if (curValue == nullptr) {
    // question: if truncate due to maxLength, should we try and avoid keeping
    // the other chars in-memory on
    //  the backing char[]?
    values[currentField] = value.substr(
        0, min(maxLength, value.length())); // note: may return 'this'
    return;
  }
  constexpr int lengthBudget = maxLength - curValue->length();
  if (lengthBudget <= 0) {
    return;
  }
  shared_ptr<StringBuilder> curValueBuilder;
  if (std::dynamic_pointer_cast<StringBuilder>(curValue) != nullptr) {
    curValueBuilder = std::static_pointer_cast<StringBuilder>(curValue);
  } else {
    // upgrade std::wstring to StringBuilder. Choose a good initial size.
    curValueBuilder = make_shared<StringBuilder>(
        curValue->length() + min(lengthBudget, value.length() + 256));
    curValueBuilder->append(curValue);
  }
  curValueBuilder->append(valueSeparator);
  curValueBuilder->append(
      value.substr(0, min(lengthBudget - 1, value.length())));
  values[currentField] = curValueBuilder;
}

Status UnifiedHighlighter::LimitedStoredFieldVisitor::needsField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  currentField = Arrays::binarySearch(fields, fieldInfo->name);
  if (currentField < 0) {
    return Status::NO;
  }
  shared_ptr<std::wstring> curVal = values[currentField];
  if (curVal != nullptr && curVal->length() >= maxLength) {
    return fields.size() == 1 ? Status::STOP : Status::NO;
  }
  return Status::YES;
}

std::deque<std::shared_ptr<std::wstring>>
UnifiedHighlighter::LimitedStoredFieldVisitor::getValuesByField()
{
  return this->values;
}

shared_ptr<IndexReader> UnifiedHighlighter::TermVectorReusingLeafReader::wrap(
    shared_ptr<IndexReader> reader) 
{
  std::deque<std::shared_ptr<LeafReader>> leafReaders =
      reader->leaves()
          .stream()
          .map_obj(LeafReaderContext::reader)
          .map_obj(TermVectorReusingLeafReader::new)
          .toArray(LeafReader[] ::new);
  return make_shared<BaseCompositeReaderAnonymousInnerClass>(leafReaders,
                                                             reader);
}

UnifiedHighlighter::TermVectorReusingLeafReader::
    BaseCompositeReaderAnonymousInnerClass::
        BaseCompositeReaderAnonymousInnerClass(
            deque<std::shared_ptr<LeafReader>> &leafReaders,
            shared_ptr<IndexReader> reader)
    : org::apache::lucene::index::BaseCompositeReader<
          org::apache::lucene::index::IndexReader>(leafReaders)
{
  this->reader = reader;
}

void UnifiedHighlighter::TermVectorReusingLeafReader::
    BaseCompositeReaderAnonymousInnerClass::doClose() 
{
  delete reader;
}

shared_ptr<IndexReader::CacheHelper>
UnifiedHighlighter::TermVectorReusingLeafReader::
    BaseCompositeReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return nullptr;
}

UnifiedHighlighter::TermVectorReusingLeafReader::TermVectorReusingLeafReader(
    shared_ptr<LeafReader> in_)
    : org::apache::lucene::index::FilterLeafReader(in_)
{
}

shared_ptr<Fields>
UnifiedHighlighter::TermVectorReusingLeafReader::getTermVectors(
    int docID) 
{
  if (docID != lastDocId) {
    lastDocId = docID;
    tvFields = in_->getTermVectors(docID);
  }
  return tvFields;
}

shared_ptr<IndexReader::CacheHelper>
UnifiedHighlighter::TermVectorReusingLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<IndexReader::CacheHelper>
UnifiedHighlighter::TermVectorReusingLeafReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::search::uhighlight