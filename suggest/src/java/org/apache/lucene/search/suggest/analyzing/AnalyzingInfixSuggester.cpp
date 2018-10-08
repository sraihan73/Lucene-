using namespace std;

#include "AnalyzingInfixSuggester.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using EdgeNGramTokenFilter =
    org::apache::lucene::analysis::ngram::EdgeNGramTokenFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using SegmentReader = org::apache::lucene::index::SegmentReader;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopFieldCollector = org::apache::lucene::search::TopFieldCollector;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const wstring AnalyzingInfixSuggester::TEXTGRAMS_FIELD_NAME = L"textgrams";
const wstring AnalyzingInfixSuggester::TEXT_FIELD_NAME = L"text";
const wstring AnalyzingInfixSuggester::EXACT_TEXT_FIELD_NAME = L"exacttext";
const wstring AnalyzingInfixSuggester::CONTEXTS_FIELD_NAME = L"contexts";
const shared_ptr<org::apache::lucene::search::Sort>
    AnalyzingInfixSuggester::SORT =
        make_shared<org::apache::lucene::search::Sort>(
            make_shared<org::apache::lucene::search::SortField>(
                L"weight", org::apache::lucene::search::SortField::Type::LONG,
                true));

AnalyzingInfixSuggester::AnalyzingInfixSuggester(
    shared_ptr<Directory> dir, shared_ptr<Analyzer> analyzer) 
    : AnalyzingInfixSuggester(dir, analyzer, analyzer, DEFAULT_MIN_PREFIX_CHARS,
                              false, DEFAULT_ALL_TERMS_REQUIRED,
                              DEFAULT_HIGHLIGHT)
{
}

AnalyzingInfixSuggester::AnalyzingInfixSuggester(
    shared_ptr<Directory> dir, shared_ptr<Analyzer> indexAnalyzer,
    shared_ptr<Analyzer> queryAnalyzer, int minPrefixChars,
    bool commitOnBuild) 
    : AnalyzingInfixSuggester(dir, indexAnalyzer, queryAnalyzer, minPrefixChars,
                              commitOnBuild, DEFAULT_ALL_TERMS_REQUIRED,
                              DEFAULT_HIGHLIGHT)
{
}

AnalyzingInfixSuggester::AnalyzingInfixSuggester(
    shared_ptr<Directory> dir, shared_ptr<Analyzer> indexAnalyzer,
    shared_ptr<Analyzer> queryAnalyzer, int minPrefixChars, bool commitOnBuild,
    bool allTermsRequired, bool highlight) 
    : AnalyzingInfixSuggester(dir, indexAnalyzer, queryAnalyzer, minPrefixChars,
                              commitOnBuild, allTermsRequired, highlight,
                              DEFAULT_CLOSE_INDEXWRITER_ON_BUILD)
{
}

AnalyzingInfixSuggester::AnalyzingInfixSuggester(
    shared_ptr<Directory> dir, shared_ptr<Analyzer> indexAnalyzer,
    shared_ptr<Analyzer> queryAnalyzer, int minPrefixChars, bool commitOnBuild,
    bool allTermsRequired, bool highlight,
    bool closeIndexWriterOnBuild) 
    : queryAnalyzer(queryAnalyzer), indexAnalyzer(indexAnalyzer), dir(dir),
      minPrefixChars(minPrefixChars), allTermsRequired(allTermsRequired),
      highlight(highlight), commitOnBuild(commitOnBuild),
      closeIndexWriterOnBuild(closeIndexWriterOnBuild)
{

  if (minPrefixChars < 0) {
    throw invalid_argument(L"minPrefixChars must be >= 0; got: " +
                           to_wstring(minPrefixChars));
  }

  if (DirectoryReader::indexExists(dir)) {
    // Already built; open it:
    searcherMgr = make_shared<SearcherManager>(dir, nullptr);
  }
}

shared_ptr<IndexWriterConfig> AnalyzingInfixSuggester::getIndexWriterConfig(
    shared_ptr<Analyzer> indexAnalyzer, IndexWriterConfig::OpenMode openMode)
{
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(indexAnalyzer);
  iwc->setOpenMode(openMode);

  // This way all merged segments will be sorted at
  // merge time, allow for per-segment early termination
  // when those segments are searched:
  iwc->setIndexSort(SORT);

  return iwc;
}

shared_ptr<Directory>
AnalyzingInfixSuggester::getDirectory(shared_ptr<Path> path) 
{
  return FSDirectory::open(path);
}

void AnalyzingInfixSuggester::build(shared_ptr<InputIterator> iter) throw(
    IOException)
{

  {
    lock_guard<mutex> lock(searcherMgrLock);
    if (searcherMgr != nullptr) {
      delete searcherMgr;
      searcherMgr.reset();
    }

    if (writer != nullptr) {
      delete writer;
      writer.reset();
    }

    bool success = false;
    try {
      // First pass: build a temporary normal Lucene index,
      // just indexing the suggestions as they iterate:
      writer = make_shared<IndexWriter>(
          dir, getIndexWriterConfig(getGramAnalyzer(),
                                    IndexWriterConfig::OpenMode::CREATE));
      // long t0 = System.nanoTime();

      // TODO: use threads?
      shared_ptr<BytesRef> text;
      while ((text = iter->next()) != nullptr) {
        shared_ptr<BytesRef> payload;
        if (iter->hasPayloads()) {
          payload = iter->payload();
        } else {
          payload.reset();
        }

        add(text, iter->contexts(), iter->weight(), payload);
      }

      // System.out.println("initial indexing time: " +
      // ((System.nanoTime()-t0)/1000000) + " msec");
      if (commitOnBuild || closeIndexWriterOnBuild) {
        commit();
      }
      searcherMgr = make_shared<SearcherManager>(writer, nullptr);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success) {
        if (closeIndexWriterOnBuild) {
          delete writer;
          writer.reset();
        }
      } else { // failure
        if (writer != nullptr) {
          writer->rollback();
          writer.reset();
        }
      }
    }
  }
}

void AnalyzingInfixSuggester::commit() 
{
  if (writer == nullptr) {
    if (searcherMgr == nullptr || closeIndexWriterOnBuild == false) {
      throw make_shared<IllegalStateException>(
          L"Cannot commit on an closed writer. Add documents first");
    }
    // else no-op: writer was committed and closed after the index was built, so
    // commit is unnecessary
  } else {
    writer->commit();
  }
}

shared_ptr<Analyzer> AnalyzingInfixSuggester::getGramAnalyzer()
{
  return make_shared<AnalyzerWrapperAnonymousInnerClass>(
      shared_from_this(), Analyzer::PER_FIELD_REUSE_STRATEGY);
}

AnalyzingInfixSuggester::AnalyzerWrapperAnonymousInnerClass::
    AnalyzerWrapperAnonymousInnerClass(
        shared_ptr<AnalyzingInfixSuggester> outerInstance,
        shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY)
    : org::apache::lucene::analysis::AnalyzerWrapper(PER_FIELD_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer>
AnalyzingInfixSuggester::AnalyzerWrapperAnonymousInnerClass::getWrappedAnalyzer(
    const wstring &fieldName)
{
  return outerInstance->indexAnalyzer;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzingInfixSuggester::AnalyzerWrapperAnonymousInnerClass::wrapComponents(
    const wstring &fieldName,
    shared_ptr<Analyzer::TokenStreamComponents> components)
{
  assert((!(fieldName == TEXTGRAMS_FIELD_NAME &&
            outerInstance->minPrefixChars == 0),
          L"no need \"textgrams\" when minPrefixChars=" +
              to_wstring(outerInstance->minPrefixChars)));
  if (fieldName == TEXTGRAMS_FIELD_NAME && outerInstance->minPrefixChars > 0) {
    // TODO: should use an EdgeNGramTokenFilterFactory here
    shared_ptr<TokenFilter> filter = make_shared<EdgeNGramTokenFilter>(
        components->getTokenStream(), 1, outerInstance->minPrefixChars, false);
    return make_shared<Analyzer::TokenStreamComponents>(
        components->getTokenizer(), filter);
  } else {
    return components;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void AnalyzingInfixSuggester::ensureOpen() 
{
  if (writer == nullptr) {
    if (DirectoryReader::indexExists(dir)) {
      // Already built; open it:
      writer = make_shared<IndexWriter>(
          dir, getIndexWriterConfig(getGramAnalyzer(),
                                    IndexWriterConfig::OpenMode::APPEND));
    } else {
      writer = make_shared<IndexWriter>(
          dir, getIndexWriterConfig(getGramAnalyzer(),
                                    IndexWriterConfig::OpenMode::CREATE));
    }
    {
      lock_guard<mutex> lock(searcherMgrLock);
      shared_ptr<SearcherManager> oldSearcherMgr = searcherMgr;
      searcherMgr = make_shared<SearcherManager>(writer, nullptr);
      if (oldSearcherMgr != nullptr) {
        delete oldSearcherMgr;
      }
    }
  }
}

void AnalyzingInfixSuggester::add(
    shared_ptr<BytesRef> text,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, int64_t weight,
    shared_ptr<BytesRef> payload) 
{
  ensureOpen();
  writer->addDocument(buildDocument(text, contexts, weight, payload));
}

void AnalyzingInfixSuggester::update(
    shared_ptr<BytesRef> text,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, int64_t weight,
    shared_ptr<BytesRef> payload) 
{
  ensureOpen();
  writer->updateDocument(
      make_shared<Term>(EXACT_TEXT_FIELD_NAME, text->utf8ToString()),
      buildDocument(text, contexts, weight, payload));
}

shared_ptr<Document> AnalyzingInfixSuggester::buildDocument(
    shared_ptr<BytesRef> text,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, int64_t weight,
    shared_ptr<BytesRef> payload) 
{
  wstring textString = text->utf8ToString();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = getTextFieldType();
  doc->push_back(make_shared<Field>(TEXT_FIELD_NAME, textString, ft));
  if (minPrefixChars > 0) {
    doc->push_back(make_shared<Field>(TEXTGRAMS_FIELD_NAME, textString, ft));
  }
  doc->push_back(make_shared<StringField>(EXACT_TEXT_FIELD_NAME, textString,
                                          Field::Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(TEXT_FIELD_NAME, text));
  doc->push_back(make_shared<NumericDocValuesField>(L"weight", weight));
  if (payload != nullptr) {
    doc->push_back(make_shared<BinaryDocValuesField>(L"payloads", payload));
  }
  if (contexts != nullptr) {
    for (auto context : contexts) {
      doc->push_back(make_shared<StringField>(CONTEXTS_FIELD_NAME, context,
                                              Field::Store::NO));
      doc->push_back(
          make_shared<SortedSetDocValuesField>(CONTEXTS_FIELD_NAME, context));
    }
  }
  return doc;
}

void AnalyzingInfixSuggester::refresh() 
{
  if (searcherMgr == nullptr) {
    throw make_shared<IllegalStateException>(L"suggester was not built");
  }
  if (writer != nullptr) {
    searcherMgr->maybeRefreshBlocking();
  }
  // else no-op: writer was committed and closed after the index was built
  //             and before searchMgr was constructed, so refresh is unnecessary
}

shared_ptr<FieldType> AnalyzingInfixSuggester::getTextFieldType()
{
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS);
  ft->setOmitNorms(true);

  return ft;
}

deque<std::shared_ptr<Lookup::LookupResult>> AnalyzingInfixSuggester::lookup(
    shared_ptr<std::wstring> key,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, bool onlyMorePopular,
    int num) 
{
  return lookup(key, contexts, num, allTermsRequired, highlight_);
}

deque<std::shared_ptr<Lookup::LookupResult>>
AnalyzingInfixSuggester::lookup(shared_ptr<std::wstring> key, int num,
                                bool allTermsRequired,
                                bool doHighlight) 
{
  return lookup(key, std::static_pointer_cast<BooleanQuery>(nullptr), num,
                allTermsRequired, doHighlight);
}

deque<std::shared_ptr<Lookup::LookupResult>> AnalyzingInfixSuggester::lookup(
    shared_ptr<std::wstring> key,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, int num,
    bool allTermsRequired, bool doHighlight) 
{
  return lookup(key, toQuery(contexts), num, allTermsRequired, doHighlight);
}

shared_ptr<Query> AnalyzingInfixSuggester::getLastTokenQuery(
    const wstring &token) 
{
  if (token.length() < minPrefixChars) {
    // The leading ngram was directly indexed:
    return make_shared<TermQuery>(
        make_shared<Term>(TEXTGRAMS_FIELD_NAME, token));
  }

  return make_shared<PrefixQuery>(make_shared<Term>(TEXT_FIELD_NAME, token));
}

deque<std::shared_ptr<Lookup::LookupResult>> AnalyzingInfixSuggester::lookup(
    shared_ptr<std::wstring> key,
    unordered_map<std::shared_ptr<BytesRef>, Occur> &contextInfo, int num,
    bool allTermsRequired, bool doHighlight) 
{
  return lookup(key, toQuery(contextInfo), num, allTermsRequired, doHighlight);
}

shared_ptr<BooleanQuery> AnalyzingInfixSuggester::toQuery(
    unordered_map<std::shared_ptr<BytesRef>, Occur> &contextInfo)
{
  if (contextInfo.empty() || contextInfo.empty()) {
    return nullptr;
  }

  shared_ptr<BooleanQuery::Builder> contextFilter =
      make_shared<BooleanQuery::Builder>();
  for (auto entry : contextInfo) {
    addContextToQuery(contextFilter, entry.first, entry.second);
  }

  return contextFilter->build();
}

shared_ptr<BooleanQuery> AnalyzingInfixSuggester::toQuery(
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contextInfo)
{
  if (contextInfo == nullptr || contextInfo->isEmpty()) {
    return nullptr;
  }

  shared_ptr<BooleanQuery::Builder> contextFilter =
      make_shared<BooleanQuery::Builder>();
  for (auto context : contextInfo) {
    addContextToQuery(contextFilter, context, Occur::SHOULD);
  }
  return contextFilter->build();
}

void AnalyzingInfixSuggester::addContextToQuery(
    shared_ptr<BooleanQuery::Builder> query, shared_ptr<BytesRef> context,
    Occur clause)
{
  // NOTE: we "should" wrap this in
  // ConstantScoreQuery, or maybe send this as a
  // Filter instead to search.

  // TODO: if we had a BinaryTermField we could fix
  // this "must be valid ut8f" limitation:
  query->add(
      make_shared<TermQuery>(make_shared<Term>(CONTEXTS_FIELD_NAME, context)),
      clause);
}

deque<std::shared_ptr<Lookup::LookupResult>> AnalyzingInfixSuggester::lookup(
    shared_ptr<std::wstring> key, shared_ptr<BooleanQuery> contextQuery,
    int num, bool allTermsRequired, bool doHighlight) 
{

  if (searcherMgr == nullptr) {
    throw make_shared<IllegalStateException>(L"suggester was not built");
  }

  constexpr Occur occur;
  if (allTermsRequired) {
    occur = Occur::MUST;
  } else {
    occur = Occur::SHOULD;
  }

  shared_ptr<BooleanQuery::Builder> query;
  shared_ptr<Set<wstring>> matchedTokens;
  wstring prefixToken = L"";

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // queryAnalyzer.tokenStream("", new java.io.StringReader(key.toString()))) C++
  // TODO: There is no native C++ equivalent to 'toString':
  {
    org::apache::lucene::analysis::TokenStream ts = queryAnalyzer->tokenStream(
        L"", make_shared<java::io::StringReader>(key->toString()));
    // long t0 = System.currentTimeMillis();
    ts->reset();
    shared_ptr<CharTermAttribute> *const termAtt =
        ts->addAttribute(CharTermAttribute::typeid);
    shared_ptr<OffsetAttribute> *const offsetAtt =
        ts->addAttribute(OffsetAttribute::typeid);
    wstring lastToken = L"";
    query = make_shared<BooleanQuery::Builder>();
    int maxEndOffset = -1;
    matchedTokens = unordered_set<>();
    while (ts->incrementToken()) {
      if (lastToken != L"") {
        matchedTokens->add(lastToken);
        query->add(make_shared<TermQuery>(
                       make_shared<Term>(TEXT_FIELD_NAME, lastToken)),
                   occur);
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      lastToken = termAtt->toString();
      if (lastToken != L"") {
        maxEndOffset = max(maxEndOffset, offsetAtt->endOffset());
      }
    }
    ts->end();

    if (lastToken != L"") {
      shared_ptr<Query> lastQuery;
      if (maxEndOffset == offsetAtt->endOffset()) {
        // Use PrefixQuery (or the ngram equivalent) when
        // there was no trailing discarded chars in the
        // string (e.g. whitespace), so that if query does
        // not end with a space we show prefix matches for
        // that token:
        lastQuery = getLastTokenQuery(lastToken);
        prefixToken = lastToken;
      } else {
        // Use TermQuery for an exact match if there were
        // trailing discarded chars (e.g. whitespace), so
        // that if query ends with a space we only show
        // exact matches for that term:
        matchedTokens->add(lastToken);
        lastQuery = make_shared<TermQuery>(
            make_shared<Term>(TEXT_FIELD_NAME, lastToken));
      }

      if (lastQuery != nullptr) {
        query->add(lastQuery, occur);
      }
    }

    if (contextQuery->size() > 0) {
      bool allMustNot = true;
      for (auto clause : contextQuery->clauses()) {
        if (clause->getOccur() != Occur::MUST_NOT) {
          allMustNot = false;
          break;
        }
      }

      if (allMustNot) {
        // All are MUST_NOT: add the contextQuery to the main query instead (not
        // as sub-query)
        for (auto clause : contextQuery->clauses()) {
          query->add(clause);
        }
      } else if (allTermsRequired == false) {
        // We must carefully upgrade the query clauses to MUST:
        shared_ptr<BooleanQuery::Builder> newQuery =
            make_shared<BooleanQuery::Builder>();
        newQuery->add(query->build(), Occur::MUST);
        newQuery->add(contextQuery, Occur::MUST);
        query = newQuery;
      } else {
        // Add contextQuery as sub-query
        query->add(contextQuery, Occur::MUST);
      }
    }
  }

  // TODO: we could allow blended sort here, combining
  // weight w/ score.  Now we ignore score and sort only
  // by weight:

  shared_ptr<Query> finalQuery = finishQuery(query, allTermsRequired);

  // System.out.println("finalQuery=" + finalQuery);

  // Sort by weight, descending:
  shared_ptr<TopFieldCollector> c =
      TopFieldCollector::create(SORT, num, true, false, false, false);
  deque<std::shared_ptr<Lookup::LookupResult>> results;
  shared_ptr<SearcherManager> mgr;
  shared_ptr<IndexSearcher> searcher;
  {
    lock_guard<mutex> lock(searcherMgrLock);
    mgr = searcherMgr; // acquire & release on same SearcherManager, via local
                       // reference
    searcher = mgr->acquire();
  }
  try {
    // System.out.println("got searcher=" + searcher);
    searcher->search(finalQuery, c);

    shared_ptr<TopFieldDocs> hits = c->topDocs();

    // Slower way if postings are not pre-sorted by weight:
    // hits = searcher.search(query, null, num, SORT);
    results = createResults(searcher, hits, num, key, doHighlight,
                            matchedTokens, prefixToken);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(searcher);
  }

  // System.out.println((System.currentTimeMillis() - t0) + " msec for infix
  // suggest"); System.out.println(results);

  return results;
}

deque<std::shared_ptr<Lookup::LookupResult>>
AnalyzingInfixSuggester::createResults(
    shared_ptr<IndexSearcher> searcher, shared_ptr<TopFieldDocs> hits, int num,
    shared_ptr<std::wstring> charSequence, bool doHighlight,
    shared_ptr<Set<wstring>> matchedTokens,
    const wstring &prefixToken) 
{

  deque<std::shared_ptr<LeafReaderContext>> leaves =
      searcher->getIndexReader()->leaves();
  deque<std::shared_ptr<Lookup::LookupResult>> results =
      deque<std::shared_ptr<Lookup::LookupResult>>();
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    shared_ptr<FieldDoc> fd =
        std::static_pointer_cast<FieldDoc>(hits->scoreDocs[i]);
    shared_ptr<BinaryDocValues> textDV = MultiDocValues::getBinaryValues(
        searcher->getIndexReader(), TEXT_FIELD_NAME);
    textDV->advance(fd->doc);
    shared_ptr<BytesRef> term = textDV->binaryValue();
    wstring text = term->utf8ToString();
    int64_t score = any_cast<optional<int64_t>>(fd->fields[0]);

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

    // Must look up sorted-set by segment:
    int segment = ReaderUtil::subIndex(fd->doc, leaves);
    shared_ptr<SortedSetDocValues> contextsDV =
        leaves[segment]->reader().getSortedSetDocValues(CONTEXTS_FIELD_NAME);
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts;
    if (contextsDV != nullptr) {
      contexts = unordered_set<std::shared_ptr<BytesRef>>();
      int targetDocID = fd->doc - leaves[segment]->docBase;
      if (contextsDV->advance(targetDocID) == targetDocID) {
        int64_t ord;
        while ((ord = contextsDV->nextOrd()) !=
               SortedSetDocValues::NO_MORE_ORDS) {
          shared_ptr<BytesRef> context =
              BytesRef::deepCopyOf(contextsDV->lookupOrd(ord));
          contexts->add(context);
        }
      }
    } else {
      contexts.reset();
    }

    shared_ptr<Lookup::LookupResult> result;

    if (doHighlight) {
      result = make_shared<Lookup::LookupResult>(
          text, highlight(text, matchedTokens, prefixToken), score, payload,
          contexts);
    } else {
      result =
          make_shared<Lookup::LookupResult>(text, score, payload, contexts);
    }

    results.push_back(result);
  }

  return results;
}

shared_ptr<Query>
AnalyzingInfixSuggester::finishQuery(shared_ptr<BooleanQuery::Builder> in_,
                                     bool allTermsRequired)
{
  return in_->build();
}

any AnalyzingInfixSuggester::highlight(
    const wstring &text, shared_ptr<Set<wstring>> matchedTokens,
    const wstring &prefixToken) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // queryAnalyzer.tokenStream("text", new java.io.StringReader(text)))
  {
    org::apache::lucene::analysis::TokenStream ts = queryAnalyzer->tokenStream(
        L"text", make_shared<java::io::StringReader>(text));
    shared_ptr<CharTermAttribute> termAtt =
        ts->addAttribute(CharTermAttribute::typeid);
    shared_ptr<OffsetAttribute> offsetAtt =
        ts->addAttribute(OffsetAttribute::typeid);
    ts->reset();
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    int upto = 0;
    while (ts->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring token = termAtt->toString();
      int startOffset = offsetAtt->startOffset();
      int endOffset = offsetAtt->endOffset();
      if (upto < startOffset) {
        addNonMatch(sb, text.substr(upto, startOffset - upto));
        upto = startOffset;
      } else if (upto > startOffset) {
        continue;
      }

      if (matchedTokens->contains(token)) {
        // Token matches.
        addWholeMatch(sb, text.substr(startOffset, endOffset - startOffset),
                      token);
        upto = endOffset;
      } else if (prefixToken != L"" &&
                 StringHelper::startsWith(token, prefixToken)) {
        addPrefixMatch(sb, text.substr(startOffset, endOffset - startOffset),
                       token, prefixToken);
        upto = endOffset;
      }
    }
    ts->end();
    int endOffset = offsetAtt->endOffset();
    if (upto < endOffset) {
      addNonMatch(sb, text.substr(upto));
    }
    return sb->toString();
  }
}

void AnalyzingInfixSuggester::addNonMatch(shared_ptr<StringBuilder> sb,
                                          const wstring &text)
{
  sb->append(text);
}

void AnalyzingInfixSuggester::addWholeMatch(shared_ptr<StringBuilder> sb,
                                            const wstring &surface,
                                            const wstring &analyzed)
{
  sb->append(L"<b>");
  sb->append(surface);
  sb->append(L"</b>");
}

void AnalyzingInfixSuggester::addPrefixMatch(shared_ptr<StringBuilder> sb,
                                             const wstring &surface,
                                             const wstring &analyzed,
                                             const wstring &prefixToken)
{
  // TODO: apps can try to invert their analysis logic
  // here, e.g. downcase the two before checking prefix:
  if (prefixToken.length() >= surface.length()) {
    addWholeMatch(sb, surface, analyzed);
    return;
  }
  sb->append(L"<b>");
  sb->append(surface.substr(0, prefixToken.length()));
  sb->append(L"</b>");
  sb->append(surface.substr(prefixToken.length()));
}

bool AnalyzingInfixSuggester::store(shared_ptr<DataOutput> in_) throw(
    IOException)
{
  return false;
}

bool AnalyzingInfixSuggester::load(shared_ptr<DataInput> out) 
{
  return false;
}

AnalyzingInfixSuggester::~AnalyzingInfixSuggester()
{
  if (searcherMgr != nullptr) {
    delete searcherMgr;
    searcherMgr.reset();
  }
  if (writer != nullptr) {
    delete writer;
    writer.reset();
  }
  if (dir != nullptr) {
    delete dir;
  }
}

int64_t AnalyzingInfixSuggester::ramBytesUsed()
{
  int64_t mem = RamUsageEstimator::shallowSizeOf(shared_from_this());
  try {
    if (searcherMgr != nullptr) {
      shared_ptr<SearcherManager> mgr;
      shared_ptr<IndexSearcher> searcher;
      {
        lock_guard<mutex> lock(searcherMgrLock);
        mgr = searcherMgr; // acquire & release on same SearcherManager, via
                           // local reference
        searcher = mgr->acquire();
      }
      try {
        for (auto context : searcher->getIndexReader()->leaves()) {
          shared_ptr<LeafReader> reader =
              FilterLeafReader::unwrap(context->reader());
          if (std::dynamic_pointer_cast<SegmentReader>(reader) != nullptr) {
            mem += (std::static_pointer_cast<SegmentReader>(context->reader()))
                       ->ramBytesUsed();
          }
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(searcher);
      }
    }
    return mem;
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AnalyzingInfixSuggester::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  try {
    if (searcherMgr != nullptr) {
      shared_ptr<SearcherManager> mgr;
      shared_ptr<IndexSearcher> searcher;
      {
        lock_guard<mutex> lock(searcherMgrLock);
        mgr = searcherMgr; // acquire & release on same SearcherManager, via
                           // local reference
        searcher = mgr->acquire();
      }
      try {
        for (auto context : searcher->getIndexReader()->leaves()) {
          shared_ptr<LeafReader> reader =
              FilterLeafReader::unwrap(context->reader());
          if (std::dynamic_pointer_cast<SegmentReader>(reader) != nullptr) {
            resources.push_back(Accountables::namedAccountable(
                L"segment", std::static_pointer_cast<SegmentReader>(reader)));
          }
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(searcher);
      }
    }
    return Collections::unmodifiableList(resources);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

int64_t AnalyzingInfixSuggester::getCount() 
{
  if (searcherMgr == nullptr) {
    return 0;
  }
  shared_ptr<SearcherManager> mgr;
  shared_ptr<IndexSearcher> searcher;
  {
    lock_guard<mutex> lock(searcherMgrLock);
    mgr = searcherMgr; // acquire & release on same SearcherManager, via local
                       // reference
    searcher = mgr->acquire();
  }
  try {
    return searcher->getIndexReader()->numDocs();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(searcher);
  }
}
} // namespace org::apache::lucene::search::suggest::analyzing