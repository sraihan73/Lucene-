using namespace std;

#include "WeightedSpanTermExtractor.h"

namespace org::apache::lucene::search::highlight
{
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using MemoryIndex = org::apache::lucene::index::memory::MemoryIndex;
using CommonTermsQuery = org::apache::lucene::queries::CommonTermsQuery;
using CustomScoreQuery = org::apache::lucene::queries::CustomScoreQuery;
using FunctionScoreQuery =
    org::apache::lucene::queries::function::FunctionScoreQuery;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using ToChildBlockJoinQuery =
    org::apache::lucene::search::join::ToChildBlockJoinQuery;
using ToParentBlockJoinQuery =
    org::apache::lucene::search::join::ToParentBlockJoinQuery;
using FieldMaskingSpanQuery =
    org::apache::lucene::search::spans::FieldMaskingSpanQuery;
using SpanFirstQuery = org::apache::lucene::search::spans::SpanFirstQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanNotQuery = org::apache::lucene::search::spans::SpanNotQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Spans = org::apache::lucene::search::spans::Spans;
using Bits = org::apache::lucene::util::Bits;
using IOUtils = org::apache::lucene::util::IOUtils;

WeightedSpanTermExtractor::WeightedSpanTermExtractor() {}

WeightedSpanTermExtractor::WeightedSpanTermExtractor(
    const wstring &defaultField)
{
  if (defaultField != L"") {
    this->defaultField = defaultField;
  }
}

void WeightedSpanTermExtractor::extract(
    shared_ptr<Query> query, float boost,
    unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
        &terms) 
{
  if (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
    shared_ptr<BoostQuery> boostQuery =
        std::static_pointer_cast<BoostQuery>(query);
    extract(boostQuery->getQuery(), boost * boostQuery->getBoost(), terms);
  } else if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr) {
    for (auto clause : std::static_pointer_cast<BooleanQuery>(query)) {
      if (!clause->isProhibited()) {
        extract(clause->getQuery(), boost, terms);
      }
    }
  } else if (std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr) {
    shared_ptr<PhraseQuery> phraseQuery =
        (std::static_pointer_cast<PhraseQuery>(query));
    std::deque<std::shared_ptr<Term>> phraseQueryTerms =
        phraseQuery->getTerms();
    if (phraseQueryTerms.size() == 1) {
      extractWeightedSpanTerms(
          terms, make_shared<SpanTermQuery>(phraseQueryTerms[0]), boost);
    } else {
      std::deque<std::shared_ptr<SpanQuery>> clauses(phraseQueryTerms.size());
      for (int i = 0; i < phraseQueryTerms.size(); i++) {
        clauses[i] = make_shared<SpanTermQuery>(phraseQueryTerms[i]);
      }

      // sum position increments beyond 1
      int positionGaps = 0;
      std::deque<int> positions = phraseQuery->getPositions();
      if (positions.size() >= 2) {
        // positions are in increasing order.   max(0,...) is just a safeguard.
        positionGaps = max(0, positions[positions.size() - 1] - positions[0] -
                                  positions.size() + 1);
      }

      // if original slop is 0 then require inOrder
      bool inorder = (phraseQuery->getSlop() == 0);

      shared_ptr<SpanNearQuery> sp = make_shared<SpanNearQuery>(
          clauses, phraseQuery->getSlop() + positionGaps, inorder);
      extractWeightedSpanTerms(terms, sp, boost);
    }
  } else if (std::dynamic_pointer_cast<TermQuery>(query) != nullptr ||
             std::dynamic_pointer_cast<SynonymQuery>(query) != nullptr) {
    extractWeightedTerms(terms, query, boost);
  } else if (std::dynamic_pointer_cast<SpanQuery>(query) != nullptr) {
    extractWeightedSpanTerms(terms, std::static_pointer_cast<SpanQuery>(query),
                             boost);
  } else if (std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr) {
    shared_ptr<Query> *const q =
        (std::static_pointer_cast<ConstantScoreQuery>(query))->getQuery();
    if (q != nullptr) {
      extract(q, boost, terms);
    }
  } else if (std::dynamic_pointer_cast<CommonTermsQuery>(query) != nullptr) {
    // specialized since rewriting would change the result query
    // this query is TermContext sensitive.
    extractWeightedTerms(terms, query, boost);
  } else if (std::dynamic_pointer_cast<DisjunctionMaxQuery>(query) != nullptr) {
    for (auto clause : (std::static_pointer_cast<DisjunctionMaxQuery>(query))) {
      extract(clause, boost, terms);
    }
  } else if (std::dynamic_pointer_cast<ToParentBlockJoinQuery>(query) !=
             nullptr) {
    extract((std::static_pointer_cast<ToParentBlockJoinQuery>(query))
                ->getChildQuery(),
            boost, terms);
  } else if (std::dynamic_pointer_cast<ToChildBlockJoinQuery>(query) !=
             nullptr) {
    extract((std::static_pointer_cast<ToChildBlockJoinQuery>(query))
                ->getParentQuery(),
            boost, terms);
  } else if (std::dynamic_pointer_cast<MultiPhraseQuery>(query) != nullptr) {
    shared_ptr<MultiPhraseQuery> *const mpq =
        std::static_pointer_cast<MultiPhraseQuery>(query);
    std::deque<std::deque<std::shared_ptr<Term>>> termArrays =
        mpq->getTermArrays();
    const std::deque<int> positions = mpq->getPositions();
    if (positions.size() > 0) {

      int maxPosition = positions[positions.size() - 1];
      for (int i = 0; i < positions.size() - 1; ++i) {
        if (positions[i] > maxPosition) {
          maxPosition = positions[i];
        }
      }

      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
      // java.util.List<org.apache.lucene.search.spans.SpanQuery>[] disjunctLists
      // = new java.util.List[maxPosition + 1];
      const std::deque<deque<std::shared_ptr<SpanQuery>>> disjunctLists =
          std::deque<deque>(maxPosition + 1);
      int distinctPositions = 0;

      for (int i = 0; i < termArrays.size(); ++i) {
        std::deque<std::shared_ptr<Term>> termArray = termArrays[i];
        deque<std::shared_ptr<SpanQuery>> disjuncts =
            disjunctLists[positions[i]];
        if (disjuncts.empty()) {
          disjuncts =
              (disjunctLists[positions[i]] = deque<>(termArray.size()));
          ++distinctPositions;
        }
        for (auto aTermArray : termArray) {
          disjuncts.push_back(make_shared<SpanTermQuery>(aTermArray));
        }
      }

      int positionGaps = 0;
      int position = 0;
      std::deque<std::shared_ptr<SpanQuery>> clauses(distinctPositions);
      for (auto disjuncts : disjunctLists) {
        if (disjuncts.size() > 0) {
          clauses[position++] = make_shared<SpanOrQuery>(disjuncts.toArray(
              std::deque<std::shared_ptr<SpanQuery>>(disjuncts.size())));
        } else {
          ++positionGaps;
        }
      }

      if (clauses.size() == 1) {
        extractWeightedSpanTerms(terms, clauses[0], boost);
      } else {
        constexpr int slop = mpq->getSlop();
        constexpr bool inorder = (slop == 0);

        shared_ptr<SpanNearQuery> sp =
            make_shared<SpanNearQuery>(clauses, slop + positionGaps, inorder);
        extractWeightedSpanTerms(terms, sp, boost);
      }
    }
  } else if (std::dynamic_pointer_cast<MatchAllDocsQuery>(query) != nullptr) {
    // nothing
  } else if (std::dynamic_pointer_cast<CustomScoreQuery>(query) != nullptr) {
    extract((std::static_pointer_cast<CustomScoreQuery>(query))->getSubQuery(),
            boost, terms);
  } else if (std::dynamic_pointer_cast<FunctionScoreQuery>(query) != nullptr) {
    extract((std::static_pointer_cast<FunctionScoreQuery>(query))
                ->getWrappedQuery(),
            boost, terms);
  } else if (isQueryUnsupported(query->getClass())) {
    // nothing
  } else {
    if (std::dynamic_pointer_cast<MultiTermQuery>(query) != nullptr &&
        (!expandMultiTermQuery ||
         !fieldNameComparator(
             (std::static_pointer_cast<MultiTermQuery>(query))->getField()))) {
      return;
    }
    shared_ptr<Query> origQuery = query;
    shared_ptr<IndexReader> *const reader = getLeafContext()->reader();
    shared_ptr<Query> rewritten;
    if (std::dynamic_pointer_cast<MultiTermQuery>(query) != nullptr) {
      rewritten = MultiTermQuery::SCORING_BOOLEAN_REWRITE->rewrite(
          reader, std::static_pointer_cast<MultiTermQuery>(query));
    } else {
      rewritten = origQuery->rewrite(reader);
    }
    if (rewritten != origQuery) {
      // only rewrite once and then flatten again - the rewritten query could
      // have a special treatment if this method is overwritten in a subclass or
      // above in the next recursion
      extract(rewritten, boost, terms);
    } else {
      extractUnknownQuery(query, terms);
    }
  }
}

bool WeightedSpanTermExtractor::isQueryUnsupported(type_info clazz)
{
  // spatial queries do not support highlighting:
  if (clazz.getName()->startsWith(L"org.apache.lucene.spatial.")) {
    return true;
  }
  // spatial3d queries are also not supported:
  if (clazz.getName()->startsWith(L"org.apache.lucene.spatial3d.")) {
    return true;
  }
  return false;
}

void WeightedSpanTermExtractor::extractUnknownQuery(
    shared_ptr<Query> query,
    unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
        &terms) 
{

  // for sub-classing to extract custom queries
}

void WeightedSpanTermExtractor::extractWeightedSpanTerms(
    unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>> &terms,
    shared_ptr<SpanQuery> spanQuery, float boost) 
{
  shared_ptr<Set<wstring>> fieldNames;

  if (fieldName == L"") {
    fieldNames = unordered_set<>();
    collectSpanQueryFields(spanQuery, fieldNames);
  } else {
    fieldNames = unordered_set<>(1);
    fieldNames->add(fieldName);
  }
  // To support the use of the default field name
  if (defaultField != L"") {
    fieldNames->add(defaultField);
  }

  unordered_map<wstring, std::shared_ptr<SpanQuery>> queries =
      unordered_map<wstring, std::shared_ptr<SpanQuery>>();

  shared_ptr<Set<std::shared_ptr<Term>>> nonWeightedTerms =
      unordered_set<std::shared_ptr<Term>>();
  constexpr bool mustRewriteQuery = this->mustRewriteQuery(spanQuery);
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(getLeafContext());
  searcher->setQueryCache(nullptr);
  if (mustRewriteQuery) {
    for (auto field : fieldNames) {
      shared_ptr<SpanQuery> *const rewrittenQuery =
          std::static_pointer_cast<SpanQuery>(
              spanQuery->rewrite(getLeafContext()->reader()));
      queries.emplace(field, rewrittenQuery);
      rewrittenQuery->createWeight(searcher, false, boost)
          ->extractTerms(nonWeightedTerms);
    }
  } else {
    spanQuery->createWeight(searcher, false, boost)
        ->extractTerms(nonWeightedTerms);
  }

  deque<std::shared_ptr<PositionSpan>> spanPositions =
      deque<std::shared_ptr<PositionSpan>>();

  for (auto field : fieldNames) {
    shared_ptr<SpanQuery> *const q;
    if (mustRewriteQuery) {
      q = queries[field];
    } else {
      q = spanQuery;
    }
    shared_ptr<LeafReaderContext> context = getLeafContext();
    shared_ptr<SpanWeight> w = std::static_pointer_cast<SpanWeight>(
        searcher->createWeight(searcher->rewrite(q), false, 1));
    shared_ptr<Bits> acceptDocs = context->reader()->getLiveDocs();
    shared_ptr<Spans> *const spans =
        w->getSpans(context, SpanWeight::Postings::POSITIONS);
    if (spans == nullptr) {
      return;
    }

    // collect span positions
    while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
      if (acceptDocs != nullptr && acceptDocs->get(spans->docID()) == false) {
        continue;
      }
      while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
        spanPositions.push_back(make_shared<PositionSpan>(
            spans->startPosition(), spans->endPosition() - 1));
      }
    }
  }

  if (spanPositions.empty()) {
    // no spans found
    return;
  }

  for (auto queryTerm : nonWeightedTerms) {

    if (fieldNameComparator(queryTerm->field())) {
      shared_ptr<WeightedSpanTerm> weightedSpanTerm = terms[queryTerm->text()];

      if (weightedSpanTerm == nullptr) {
        weightedSpanTerm =
            make_shared<WeightedSpanTerm>(boost, queryTerm->text());
        weightedSpanTerm->addPositionSpans(spanPositions);
        weightedSpanTerm->positionSensitive = true;
        terms.emplace(queryTerm->text(), weightedSpanTerm);
      } else {
        if (spanPositions.size() > 0) {
          weightedSpanTerm->addPositionSpans(spanPositions);
        }
      }
    }
  }
}

void WeightedSpanTermExtractor::extractWeightedTerms(
    unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>> &terms,
    shared_ptr<Query> query, float boost) 
{
  shared_ptr<Set<std::shared_ptr<Term>>> nonWeightedTerms =
      unordered_set<std::shared_ptr<Term>>();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(getLeafContext());
  searcher->createWeight(searcher->rewrite(query), false, 1)
      ->extractTerms(nonWeightedTerms);

  for (auto queryTerm : nonWeightedTerms) {

    if (fieldNameComparator(queryTerm->field())) {
      shared_ptr<WeightedSpanTerm> weightedSpanTerm =
          make_shared<WeightedSpanTerm>(boost, queryTerm->text());
      terms.emplace(queryTerm->text(), weightedSpanTerm);
    }
  }
}

bool WeightedSpanTermExtractor::fieldNameComparator(
    const wstring &fieldNameToCheck)
{
  bool rv = fieldName == L"" || fieldName == fieldNameToCheck ||
            (defaultField != L"" && defaultField == fieldNameToCheck);
  return rv;
}

shared_ptr<LeafReaderContext>
WeightedSpanTermExtractor::getLeafContext() 
{
  if (internalReader == nullptr) {
    bool cacheIt =
        wrapToCaching && !(std::dynamic_pointer_cast<CachingTokenFilter>(
                               tokenStream) != nullptr);

    // If it's from term vectors, simply wrap the underlying Terms in a reader
    if (std::dynamic_pointer_cast<TokenStreamFromTermVector>(tokenStream) !=
        nullptr) {
      cacheIt = false;
      shared_ptr<Terms> termVectorTerms =
          (std::static_pointer_cast<TokenStreamFromTermVector>(tokenStream))
              ->getTermVectorTerms();
      if (termVectorTerms->hasPositions() && termVectorTerms->hasOffsets()) {
        internalReader = make_shared<TermVectorLeafReader>(
            DelegatingLeafReader::FIELD_NAME, termVectorTerms);
      }
    }

    // Use MemoryIndex (index/invert this tokenStream now)
    if (internalReader == nullptr) {
      shared_ptr<MemoryIndex> *const indexer =
          make_shared<MemoryIndex>(true, usePayloads); // offsets and payloads
      if (cacheIt) {
        assert(!cachedTokenStream);
        tokenStream =
            make_shared<CachingTokenFilter>(make_shared<OffsetLimitTokenFilter>(
                tokenStream, maxDocCharsToAnalyze));
        cachedTokenStream = true;
        indexer->addField(DelegatingLeafReader::FIELD_NAME, tokenStream);
      } else {
        indexer->addField(DelegatingLeafReader::FIELD_NAME,
                          make_shared<OffsetLimitTokenFilter>(
                              tokenStream, maxDocCharsToAnalyze));
      }
      shared_ptr<IndexSearcher> *const searcher = indexer->createSearcher();
      // MEM index has only atomic ctx
      internalReader = (std::static_pointer_cast<LeafReaderContext>(
                            searcher->getTopReaderContext()))
                           ->reader();
    }

    // Now wrap it so we always use a common field.
    this->internalReader = make_shared<DelegatingLeafReader>(internalReader);
  }

  return internalReader->getContext();
}

const wstring WeightedSpanTermExtractor::DelegatingLeafReader::FIELD_NAME =
    L"shadowed_field";

WeightedSpanTermExtractor::DelegatingLeafReader::DelegatingLeafReader(
    shared_ptr<LeafReader> in_)
    : org::apache::lucene::index::FilterLeafReader(in_)
{
}

shared_ptr<FieldInfos>
WeightedSpanTermExtractor::DelegatingLeafReader::getFieldInfos()
{
  throw make_shared<UnsupportedOperationException>(); // TODO merge them
}

shared_ptr<Terms> WeightedSpanTermExtractor::DelegatingLeafReader::terms(
    const wstring &field) 
{
  return FilterLeafReader::terms(DelegatingLeafReader::FIELD_NAME);
}

shared_ptr<NumericDocValues>
WeightedSpanTermExtractor::DelegatingLeafReader::getNumericDocValues(
    const wstring &field) 
{
  return FilterLeafReader::getNumericDocValues(FIELD_NAME);
}

shared_ptr<BinaryDocValues>
WeightedSpanTermExtractor::DelegatingLeafReader::getBinaryDocValues(
    const wstring &field) 
{
  return FilterLeafReader::getBinaryDocValues(FIELD_NAME);
}

shared_ptr<SortedDocValues>
WeightedSpanTermExtractor::DelegatingLeafReader::getSortedDocValues(
    const wstring &field) 
{
  return FilterLeafReader::getSortedDocValues(FIELD_NAME);
}

shared_ptr<NumericDocValues>
WeightedSpanTermExtractor::DelegatingLeafReader::getNormValues(
    const wstring &field) 
{
  return FilterLeafReader::getNormValues(FIELD_NAME);
}

shared_ptr<IndexReader::CacheHelper>
WeightedSpanTermExtractor::DelegatingLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<IndexReader::CacheHelper>
WeightedSpanTermExtractor::DelegatingLeafReader::getReaderCacheHelper()
{
  return nullptr;
}

unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
WeightedSpanTermExtractor::getWeightedSpanTerms(
    shared_ptr<Query> query, float boost,
    shared_ptr<TokenStream> tokenStream) 
{
  return getWeightedSpanTerms(query, boost, tokenStream, L"");
}

unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
WeightedSpanTermExtractor::getWeightedSpanTerms(
    shared_ptr<Query> query, float boost, shared_ptr<TokenStream> tokenStream,
    const wstring &fieldName) 
{
  this->fieldName = fieldName;

  unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>> terms = make_shared<
      PositionCheckingMap<wstring, std::shared_ptr<WeightedSpanTerm>>>();
  this->tokenStream = tokenStream;
  try {
    extract(query, boost, terms);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({internalReader});
  }

  return terms;
}

unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
WeightedSpanTermExtractor::getWeightedSpanTermsWithScores(
    shared_ptr<Query> query, float boost, shared_ptr<TokenStream> tokenStream,
    const wstring &fieldName, shared_ptr<IndexReader> reader) 
{
  if (fieldName != L"") {
    this->fieldName = fieldName;
  } else {
    this->fieldName = L"";
  }
  this->tokenStream = tokenStream;

  unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>> terms = make_shared<
      PositionCheckingMap<wstring, std::shared_ptr<WeightedSpanTerm>>>();
  extract(query, boost, terms);

  int totalNumDocs = reader->maxDoc();
  shared_ptr<Set<wstring>> weightedTerms = terms.keySet();
  Set<wstring>::const_iterator it = weightedTerms->begin();

  try {
    while (it != weightedTerms->end()) {
      shared_ptr<WeightedSpanTerm> weightedSpanTerm = terms[*it];
      int docFreq =
          reader->docFreq(make_shared<Term>(fieldName, weightedSpanTerm->term));
      // IDF algorithm taken from ClassicSimilarity class
      float idf = static_cast<float>(
          log(totalNumDocs / static_cast<double>(docFreq + 1)) + 1.0);
      weightedSpanTerm->weight *= idf;
      it++;
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({internalReader});
  }

  return terms;
}

void WeightedSpanTermExtractor::collectSpanQueryFields(
    shared_ptr<SpanQuery> spanQuery, shared_ptr<Set<wstring>> fieldNames)
{
  if (std::dynamic_pointer_cast<FieldMaskingSpanQuery>(spanQuery) != nullptr) {
    collectSpanQueryFields(
        (std::static_pointer_cast<FieldMaskingSpanQuery>(spanQuery))
            ->getMaskedQuery(),
        fieldNames);
  } else if (std::dynamic_pointer_cast<SpanFirstQuery>(spanQuery) != nullptr) {
    collectSpanQueryFields(
        (std::static_pointer_cast<SpanFirstQuery>(spanQuery))->getMatch(),
        fieldNames);
  } else if (std::dynamic_pointer_cast<SpanNearQuery>(spanQuery) != nullptr) {
    for (auto clause :
         (std::static_pointer_cast<SpanNearQuery>(spanQuery))->getClauses()) {
      collectSpanQueryFields(clause, fieldNames);
    }
  } else if (std::dynamic_pointer_cast<SpanNotQuery>(spanQuery) != nullptr) {
    collectSpanQueryFields(
        (std::static_pointer_cast<SpanNotQuery>(spanQuery))->getInclude(),
        fieldNames);
  } else if (std::dynamic_pointer_cast<SpanOrQuery>(spanQuery) != nullptr) {
    for (auto clause :
         (std::static_pointer_cast<SpanOrQuery>(spanQuery))->getClauses()) {
      collectSpanQueryFields(clause, fieldNames);
    }
  } else {
    fieldNames->add(spanQuery->getField());
  }
}

bool WeightedSpanTermExtractor::mustRewriteQuery(
    shared_ptr<SpanQuery> spanQuery)
{
  if (!expandMultiTermQuery) {
    return false; // Will throw UnsupportedOperationException in case of a
                  // SpanRegexQuery.
  } else if (std::dynamic_pointer_cast<FieldMaskingSpanQuery>(spanQuery) !=
             nullptr) {
    return mustRewriteQuery(
        (std::static_pointer_cast<FieldMaskingSpanQuery>(spanQuery))
            ->getMaskedQuery());
  } else if (std::dynamic_pointer_cast<SpanFirstQuery>(spanQuery) != nullptr) {
    return mustRewriteQuery(
        (std::static_pointer_cast<SpanFirstQuery>(spanQuery))->getMatch());
  } else if (std::dynamic_pointer_cast<SpanNearQuery>(spanQuery) != nullptr) {
    for (auto clause :
         (std::static_pointer_cast<SpanNearQuery>(spanQuery))->getClauses()) {
      if (mustRewriteQuery(clause)) {
        return true;
      }
    }
    return false;
  } else if (std::dynamic_pointer_cast<SpanNotQuery>(spanQuery) != nullptr) {
    shared_ptr<SpanNotQuery> spanNotQuery =
        std::static_pointer_cast<SpanNotQuery>(spanQuery);
    return mustRewriteQuery(spanNotQuery->getInclude()) ||
           mustRewriteQuery(spanNotQuery->getExclude());
  } else if (std::dynamic_pointer_cast<SpanOrQuery>(spanQuery) != nullptr) {
    for (auto clause :
         (std::static_pointer_cast<SpanOrQuery>(spanQuery))->getClauses()) {
      if (mustRewriteQuery(clause)) {
        return true;
      }
    }
    return false;
  } else if (std::dynamic_pointer_cast<SpanTermQuery>(spanQuery) != nullptr) {
    return false;
  } else {
    return true;
  }
}

bool WeightedSpanTermExtractor::getExpandMultiTermQuery()
{
  return expandMultiTermQuery;
}

void WeightedSpanTermExtractor::setExpandMultiTermQuery(
    bool expandMultiTermQuery)
{
  this->expandMultiTermQuery = expandMultiTermQuery;
}

bool WeightedSpanTermExtractor::isUsePayloads() { return usePayloads; }

void WeightedSpanTermExtractor::setUsePayloads(bool usePayloads)
{
  this->usePayloads = usePayloads;
}

bool WeightedSpanTermExtractor::isCachedTokenStream()
{
  return cachedTokenStream;
}

shared_ptr<TokenStream> WeightedSpanTermExtractor::getTokenStream()
{
  assert(tokenStream != nullptr);
  return tokenStream;
}

void WeightedSpanTermExtractor::setWrapIfNotCachingTokenFilter(bool wrap)
{
  this->wrapToCaching = wrap;
}

void WeightedSpanTermExtractor::setMaxDocCharsToAnalyze(
    int maxDocCharsToAnalyze)
{
  this->maxDocCharsToAnalyze = maxDocCharsToAnalyze;
}
} // namespace org::apache::lucene::search::highlight