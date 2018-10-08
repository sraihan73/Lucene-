using namespace std;

#include "MemoryIndex.h"

namespace org::apache::lucene::index::memory
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using namespace org::apache::lucene::index;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using DirectBytesStartArray =
    org::apache::lucene::util::BytesRefHash::DirectBytesStartArray;
using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;
using SliceReader = org::apache::lucene::util::IntBlockPool::SliceReader;
using SliceWriter = org::apache::lucene::util::IntBlockPool::SliceWriter;
using RecyclingByteBlockAllocator =
    org::apache::lucene::util::RecyclingByteBlockAllocator;
using RecyclingIntBlockAllocator =
    org::apache::lucene::util::RecyclingIntBlockAllocator;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;

MemoryIndex::MemoryIndex() : MemoryIndex(false) {}

MemoryIndex::MemoryIndex(bool storeOffsets) : MemoryIndex(storeOffsets, false)
{
}

MemoryIndex::MemoryIndex(bool storeOffsets, bool storePayloads)
    : MemoryIndex(storeOffsets, storePayloads, 0)
{
}

MemoryIndex::MemoryIndex(bool storeOffsets, bool storePayloads,
                         int64_t maxReusedBytes)
    : storeOffsets(storeOffsets), storePayloads(storePayloads),
      byteBlockPool(
          make_shared<ByteBlockPool>(make_shared<RecyclingByteBlockAllocator>(
              ByteBlockPool::BYTE_BLOCK_SIZE, maxBufferedByteBlocks,
              bytesUsed))),
      intBlockPool(
          make_shared<IntBlockPool>(make_shared<RecyclingIntBlockAllocator>(
              IntBlockPool::INT_BLOCK_SIZE, maxBufferedIntBlocks, bytesUsed))),
      postingsWriter(
          make_shared<org::apache::lucene::util::IntBlockPool::SliceWriter>(
              intBlockPool)),
      payloadsBytesRefs(storePayloads ? make_shared<BytesRefArray>(bytesUsed)
                                      : nullptr)
{
  this->defaultFieldType->setIndexOptions(
      storeOffsets ? IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS
                   : IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  this->defaultFieldType->setStoreTermVectors(true);
  this->bytesUsed = Counter::newCounter();
  constexpr int maxBufferedByteBlocks =
      static_cast<int>((maxReusedBytes / 2) / ByteBlockPool::BYTE_BLOCK_SIZE);
  constexpr int maxBufferedIntBlocks =
      static_cast<int>((maxReusedBytes - (maxBufferedByteBlocks *
                                          ByteBlockPool::BYTE_BLOCK_SIZE)) /
                       (IntBlockPool::INT_BLOCK_SIZE * Integer::BYTES));
  assert(maxBufferedByteBlocks * ByteBlockPool::BYTE_BLOCK_SIZE) +
          (maxBufferedIntBlocks * IntBlockPool::INT_BLOCK_SIZE *
           Integer::BYTES) <=
      maxReusedBytes;
  // TODO refactor BytesRefArray to allow us to apply maxReusedBytes option
}

void MemoryIndex::addField(const wstring &fieldName, const wstring &text,
                           shared_ptr<Analyzer> analyzer)
{
  if (fieldName == L"") {
    throw invalid_argument(L"fieldName must not be null");
  }
  if (text == L"") {
    throw invalid_argument(L"text must not be null");
  }
  if (analyzer == nullptr) {
    throw invalid_argument(L"analyzer must not be null");
  }

  shared_ptr<TokenStream> stream = analyzer->tokenStream(fieldName, text);
  storeTerms(getInfo(fieldName, defaultFieldType), stream,
             analyzer->getPositionIncrementGap(fieldName),
             analyzer->getOffsetGap(fieldName));
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static MemoryIndex fromDocument(Iterable<?
// extends IndexableField> document, org.apache.lucene.analysis.Analyzer
// analyzer)
shared_ptr<MemoryIndex> MemoryIndex::fromDocument(deque<T1> document,
                                                  shared_ptr<Analyzer> analyzer)
{
  return fromDocument(document, analyzer, false, false, 0);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static MemoryIndex fromDocument(Iterable<?
// extends IndexableField> document, org.apache.lucene.analysis.Analyzer
// analyzer, bool storeOffsets, bool storePayloads)
shared_ptr<MemoryIndex>
MemoryIndex::fromDocument(deque<T1> document, shared_ptr<Analyzer> analyzer,
                          bool storeOffsets, bool storePayloads)
{
  return fromDocument(document, analyzer, storeOffsets, storePayloads, 0);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static MemoryIndex fromDocument(Iterable<?
// extends IndexableField> document, org.apache.lucene.analysis.Analyzer
// analyzer, bool storeOffsets, bool storePayloads, long maxReusedBytes)
shared_ptr<MemoryIndex>
MemoryIndex::fromDocument(deque<T1> document, shared_ptr<Analyzer> analyzer,
                          bool storeOffsets, bool storePayloads,
                          int64_t maxReusedBytes)
{
  shared_ptr<MemoryIndex> mi =
      make_shared<MemoryIndex>(storeOffsets, storePayloads, maxReusedBytes);
  for (auto field : document) {
    mi->addField(field, analyzer);
  }
  return mi;
}

template <typename T>
shared_ptr<TokenStream>
MemoryIndex::keywordTokenStream(shared_ptr<deque<T>> keywords)
{
  // TODO: deprecate & move this method into AnalyzerUtil?
  if (keywords == nullptr) {
    throw invalid_argument(L"keywords must not be null");
  }

  return make_shared<TokenStreamAnonymousInnerClass>(shared_from_this(),
                                                     keywords);
}

MemoryIndex::TokenStreamAnonymousInnerClass::TokenStreamAnonymousInnerClass(
    shared_ptr<MemoryIndex> outerInstance,
    shared_ptr<deque<std::shared_ptr<T>>> keywords)
{
  this->outerInstance = outerInstance;
  this->keywords = keywords;
  iter = keywords->begin();
  start = 0;
  termAtt = addAttribute(CharTermAttribute::typeid);
  offsetAtt = addAttribute(OffsetAttribute::typeid);
}

bool MemoryIndex::TokenStreamAnonymousInnerClass::incrementToken()
{
  if (!iter->hasNext()) {
    return false;
  }

  shared_ptr<T> obj = iter->next();
  if (obj == nullptr) {
    throw invalid_argument(L"keyword must not be null");
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring term = obj->toString();
  clearAttributes();
  termAtt::setEmpty()->append(term);
  offsetAtt::setOffset(start, start + termAtt->length());
  start += term.length() + 1; // separate words by 1 (blank) character
  return true;
}

void MemoryIndex::addField(shared_ptr<IndexableField> field,
                           shared_ptr<Analyzer> analyzer)
{

  shared_ptr<Info> info = getInfo(field->name(), field->fieldType());

  int offsetGap;
  shared_ptr<TokenStream> tokenStream;
  int positionIncrementGap;
  if (analyzer != nullptr) {
    offsetGap = analyzer->getOffsetGap(field->name());
    tokenStream = field->tokenStream(analyzer, nullptr);
    positionIncrementGap = analyzer->getPositionIncrementGap(field->name());
  } else {
    offsetGap = 1;
    tokenStream = field->tokenStream(nullptr, nullptr);
    positionIncrementGap = 0;
  }
  if (tokenStream != nullptr) {
    storeTerms(info, tokenStream, positionIncrementGap, offsetGap);
  }

  DocValuesType docValuesType = field->fieldType()->docValuesType();
  any docValuesValue;
  switch (docValuesType) {
  case org::apache::lucene::index::DocValuesType::NONE:
    docValuesValue = nullptr;
    break;
  case org::apache::lucene::index::DocValuesType::BINARY:
  case org::apache::lucene::index::DocValuesType::SORTED:
  case org::apache::lucene::index::DocValuesType::SORTED_SET:
    docValuesValue = field->binaryValue();
    break;
  case org::apache::lucene::index::DocValuesType::NUMERIC:
  case org::apache::lucene::index::DocValuesType::SORTED_NUMERIC:
    docValuesValue = field->numericValue();
    break;
  default:
    throw make_shared<UnsupportedOperationException>(
        L"unknown doc values type [" + docValuesType + L"]");
  }
  if (docValuesValue != nullptr) {
    storeDocValues(info, docValuesType, docValuesValue);
  }

  if (field->fieldType()->pointDimensionCount() > 0) {
    storePointValues(info, field->binaryValue());
  }
}

void MemoryIndex::addField(const wstring &fieldName,
                           shared_ptr<TokenStream> stream)
{
  addField(fieldName, stream, 0);
}

void MemoryIndex::addField(const wstring &fieldName,
                           shared_ptr<TokenStream> stream,
                           int positionIncrementGap)
{
  addField(fieldName, stream, positionIncrementGap, 1);
}

void MemoryIndex::addField(const wstring &fieldName,
                           shared_ptr<TokenStream> tokenStream,
                           int positionIncrementGap, int offsetGap)
{
  shared_ptr<Info> info = getInfo(fieldName, defaultFieldType);
  storeTerms(info, tokenStream, positionIncrementGap, offsetGap);
}

shared_ptr<Info> MemoryIndex::getInfo(const wstring &fieldName,
                                      shared_ptr<IndexableFieldType> fieldType)
{
  if (frozen) {
    throw invalid_argument(
        L"Cannot call addField() when MemoryIndex is frozen");
  }
  if (fieldName == L"") {
    throw invalid_argument(L"fieldName must not be null");
  }
  shared_ptr<Info> info = fields->get(fieldName);
  if (info == nullptr) {
    fields->put(fieldName,
                info = make_shared<Info>(
                    shared_from_this(),
                    createFieldInfo(fieldName, fields->size(), fieldType),
                    byteBlockPool));
  }
  if (fieldType->pointDimensionCount() !=
      info->fieldInfo.getPointDimensionCount()) {
    if (fieldType->pointDimensionCount() > 0) {
      info->fieldInfo.setPointDimensions(fieldType->pointDimensionCount(),
                                         fieldType->pointNumBytes());
    }
  }
  if (fieldType->docValuesType() != info->fieldInfo.getDocValuesType()) {
    if (fieldType->docValuesType() != DocValuesType::NONE) {
      info->fieldInfo.setDocValuesType(fieldType->docValuesType());
    }
  }
  return info;
}

shared_ptr<FieldInfo>
MemoryIndex::createFieldInfo(const wstring &fieldName, int ord,
                             shared_ptr<IndexableFieldType> fieldType)
{
  IndexOptions indexOptions =
      storeOffsets ? IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS
                   : IndexOptions::DOCS_AND_FREQS_AND_POSITIONS;
  return make_shared<FieldInfo>(
      fieldName, ord, fieldType->storeTermVectors(), fieldType->omitNorms(),
      storePayloads, indexOptions, fieldType->docValuesType(), -1,
      Collections::emptyMap(), fieldType->pointDimensionCount(),
      fieldType->pointNumBytes(), false);
}

void MemoryIndex::storePointValues(shared_ptr<Info> info,
                                   shared_ptr<BytesRef> pointValue)
{
  if (info->pointValues == nullptr) {
    info->pointValues = std::deque<std::shared_ptr<BytesRef>>(4);
  }
  info->pointValues =
      ArrayUtil::grow(info->pointValues, info->pointValuesCount + 1);
  info->pointValues[info->pointValuesCount++] =
      BytesRef::deepCopyOf(pointValue);
}

void MemoryIndex::storeDocValues(shared_ptr<Info> info,
                                 DocValuesType docValuesType,
                                 any docValuesValue)
{
  wstring fieldName = info->fieldInfo.name;
  DocValuesType existingDocValuesType = info->fieldInfo.getDocValuesType();
  if (existingDocValuesType == DocValuesType::NONE) {
    // first time we add doc values for this field:
    info->fieldInfo = make_shared<FieldInfo>(
        info->fieldInfo.name, info->fieldInfo.number,
        info->fieldInfo.hasVectors(), info->fieldInfo.hasPayloads(),
        info->fieldInfo.hasPayloads(), info->fieldInfo.getIndexOptions(),
        docValuesType, -1, info->fieldInfo.attributes(),
        info->fieldInfo.getPointDimensionCount(),
        info->fieldInfo.getPointNumBytes(),
        info->fieldInfo.isSoftDeletesField());
  } else if (existingDocValuesType != docValuesType) {
    throw invalid_argument(L"Can't add [" + docValuesType +
                           L"] doc values field [" + fieldName +
                           L"], because [" + existingDocValuesType +
                           L"] doc values field already exists");
  }
  switch (docValuesType) {
  case org::apache::lucene::index::DocValuesType::NUMERIC:
    if (info->numericProducer.dvLongValues != nullptr) {
      throw invalid_argument(L"Only one value per field allowed for [" +
                             docValuesType + L"] doc values field [" +
                             fieldName + L"]");
    }
    info->numericProducer->dvLongValues =
        std::deque<int64_t>{any_cast<int64_t>(docValuesValue)};
    info->numericProducer.count++;
    break;
  case org::apache::lucene::index::DocValuesType::SORTED_NUMERIC:
    if (info->numericProducer->dvLongValues == nullptr) {
      info->numericProducer->dvLongValues = std::deque<int64_t>(4);
    }
    info->numericProducer->dvLongValues = ArrayUtil::grow(
        info->numericProducer.dvLongValues, info->numericProducer.count + 1);
    info->numericProducer.dvLongValues[info->numericProducer.count++] =
        any_cast<int64_t>(docValuesValue);
    break;
  case org::apache::lucene::index::DocValuesType::BINARY:
    if (info->binaryProducer.dvBytesValuesSet != nullptr) {
      throw invalid_argument(L"Only one value per field allowed for [" +
                             docValuesType + L"] doc values field [" +
                             fieldName + L"]");
    }
    info->binaryProducer->dvBytesValuesSet =
        make_shared<BytesRefHash>(byteBlockPool);
    info->binaryProducer.dvBytesValuesSet->add(
        any_cast<std::shared_ptr<BytesRef>>(docValuesValue));
    break;
  case org::apache::lucene::index::DocValuesType::SORTED:
    if (info->binaryProducer.dvBytesValuesSet != nullptr) {
      throw invalid_argument(L"Only one value per field allowed for [" +
                             docValuesType + L"] doc values field [" +
                             fieldName + L"]");
    }
    info->binaryProducer->dvBytesValuesSet =
        make_shared<BytesRefHash>(byteBlockPool);
    info->binaryProducer.dvBytesValuesSet->add(
        any_cast<std::shared_ptr<BytesRef>>(docValuesValue));
    break;
  case org::apache::lucene::index::DocValuesType::SORTED_SET:
    if (info->binaryProducer->dvBytesValuesSet == nullptr) {
      info->binaryProducer->dvBytesValuesSet =
          make_shared<BytesRefHash>(byteBlockPool);
    }
    info->binaryProducer.dvBytesValuesSet->add(
        any_cast<std::shared_ptr<BytesRef>>(docValuesValue));
    break;
  default:
    throw make_shared<UnsupportedOperationException>(
        L"unknown doc values type [" + docValuesType + L"]");
  }
}

void MemoryIndex::storeTerms(shared_ptr<Info> info,
                             shared_ptr<TokenStream> tokenStream,
                             int positionIncrementGap, int offsetGap)
{

  int pos = -1;
  int offset = 0;
  if (info->numTokens > 0) {
    pos = info->lastPosition + positionIncrementGap;
    offset = info->lastOffset + offsetGap;
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // stream = tokenStream)
  {
    org::apache::lucene::analysis::TokenStream stream = tokenStream;
    try {
      shared_ptr<TermToBytesRefAttribute> termAtt =
          stream->getAttribute(TermToBytesRefAttribute::typeid);
      shared_ptr<PositionIncrementAttribute> posIncrAttribute =
          stream->addAttribute(PositionIncrementAttribute::typeid);
      shared_ptr<OffsetAttribute> offsetAtt =
          stream->addAttribute(OffsetAttribute::typeid);
      shared_ptr<PayloadAttribute> payloadAtt =
          storePayloads ? stream->addAttribute(PayloadAttribute::typeid)
                        : nullptr;
      stream->reset();

      while (stream->incrementToken()) {
        //        if (DEBUG) System.err.println("token='" + term + "'");
        info->numTokens++;
        constexpr int posIncr = posIncrAttribute->getPositionIncrement();
        if (posIncr == 0) {
          info->numOverlapTokens++;
        }
        pos += posIncr;
        int ord = info->terms->add(termAtt->getBytesRef());
        if (ord < 0) {
          ord = (-ord) - 1;
          postingsWriter->reset(info->sliceArray.end[ord]);
        } else {
          info->sliceArray.start[ord] = postingsWriter->startNewSlice();
        }
        info->sliceArray.freq[ord]++;
        info->sumTotalTermFreq++;
        postingsWriter->writeInt(pos);
        if (storeOffsets) {
          postingsWriter->writeInt(offsetAtt->startOffset() + offset);
          postingsWriter->writeInt(offsetAtt->endOffset() + offset);
        }
        if (storePayloads) {
          shared_ptr<BytesRef> *const payload = payloadAtt->getPayload();
          constexpr int pIndex;
          if (payload == nullptr || payload->length == 0) {
            pIndex = -1;
          } else {
            pIndex = payloadsBytesRefs->append(payload);
          }
          postingsWriter->writeInt(pIndex);
        }
        info->sliceArray.end[ord] = postingsWriter->getCurrentOffset();
      }
      stream->end();
      if (info->numTokens > 0) {
        info->lastPosition = pos;
        info->lastOffset = offsetAtt->endOffset() + offset;
      }
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  }
}

void MemoryIndex::setSimilarity(shared_ptr<Similarity> similarity)
{
  if (frozen) {
    throw invalid_argument(L"Cannot set Similarity when MemoryIndex is frozen");
  }
  if (this->normSimilarity == similarity) {
    return;
  }
  this->normSimilarity = similarity;
  // invalidate any cached norms that may exist
  for (auto info : fields) {
    info->second->norm = nullptr;
  }
}

shared_ptr<IndexSearcher> MemoryIndex::createSearcher()
{
  shared_ptr<MemoryIndexReader> reader =
      make_shared<MemoryIndexReader>(shared_from_this());
  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(reader); // ensures no auto-close !!
  searcher->setSimilarity(normSimilarity);
  searcher->setQueryCache(nullptr);
  return searcher;
}

void MemoryIndex::freeze()
{
  this->frozen = true;
  for (auto info : fields) {
    info->second.freeze();
  }
}

float MemoryIndex::search(shared_ptr<Query> query)
{
  if (query == nullptr) {
    throw invalid_argument(L"query must not be null");
  }

  shared_ptr<IndexSearcher> searcher = createSearcher();
  try {
    const std::deque<float> scores =
        std::deque<float>(1); // inits to 0.0f (no match)
    searcher->search(query, make_shared<SimpleCollectorAnonymousInnerClass>(
                                shared_from_this(), scores));
    float score = scores[0];
    return score;
  } catch (const IOException &e) { // can never happen (RAMDirectory)
    throw runtime_error(e);
  }
}

MemoryIndex::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(shared_ptr<MemoryIndex> outerInstance,
                                       deque<float> &scores)
{
  this->outerInstance = outerInstance;
  this->scores = scores;
}

void MemoryIndex::SimpleCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  scores[0] = scorer::score();
}

void MemoryIndex::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

bool MemoryIndex::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

wstring MemoryIndex::toStringDebug()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>(256);
  int sumPositions = 0;
  int sumTerms = 0;
  shared_ptr<BytesRef> *const spare = make_shared<BytesRef>();
  shared_ptr<BytesRefBuilder> *const payloadBuilder =
      storePayloads ? make_shared<BytesRefBuilder>() : nullptr;
  for (auto entry : fields) {
    wstring fieldName = entry.first;
    shared_ptr<Info> info = entry.second;
    info->sortTerms();
    result->append(fieldName + L":\n");
    shared_ptr<SliceByteStartArray> sliceArray = info->sliceArray;
    int numPositions = 0;
    shared_ptr<IntBlockPool::SliceReader> postingsReader =
        make_shared<IntBlockPool::SliceReader>(intBlockPool);
    for (int j = 0; j < info->terms->size(); j++) {
      int ord = info->sortedTerms[j];
      info->terms->get(ord, spare);
      int freq = sliceArray->freq[ord];
      result->append(L"\t'" + spare + L"':" + to_wstring(freq) + L":");
      postingsReader->reset(sliceArray->start[ord], sliceArray->end[ord]);
      result->append(L" [");
      constexpr int iters = storeOffsets ? 3 : 1;
      while (!postingsReader->endOfSlice()) {
        result->append(L"(");

        for (int k = 0; k < iters; k++) {
          result->append(postingsReader->readInt());
          if (k < iters - 1) {
            result->append(L", ");
          }
        }
        if (storePayloads) {
          int payloadIndex = postingsReader->readInt();
          if (payloadIndex != -1) {
            result->append(
                L", " + payloadsBytesRefs->get(payloadBuilder, payloadIndex));
          }
        }
        result->append(L")");

        if (!postingsReader->endOfSlice()) {
          result->append(L", ");
        }
      }
      result->append(L"]");
      result->append(L"\n");
      numPositions += freq;
    }

    result->append(L"\tterms=" + info->terms->size());
    result->append(L", positions=" + to_wstring(numPositions));
    result->append(L"\n");
    sumPositions += numPositions;
    sumTerms += info->terms->size();
  }

  result->append(L"\nfields=" + fields->size());
  result->append(L", terms=" + to_wstring(sumTerms));
  result->append(L", positions=" + to_wstring(sumPositions));
  return result->toString();
}

MemoryIndex::Info::Info(shared_ptr<MemoryIndex> outerInstance,
                        shared_ptr<FieldInfo> fieldInfo,
                        shared_ptr<ByteBlockPool> byteBlockPool)
    : outerInstance(outerInstance)
{
  this->fieldInfo = fieldInfo;
  this->sliceArray =
      make_shared<SliceByteStartArray>(BytesRefHash::DEFAULT_CAPACITY);
  this->terms = make_shared<BytesRefHash>(
      byteBlockPool, BytesRefHash::DEFAULT_CAPACITY, sliceArray);
  this->binaryProducer = make_shared<BinaryDocValuesProducer>();
  this->numericProducer = make_shared<NumericDocValuesProducer>();
}

void MemoryIndex::Info::freeze()
{
  sortTerms();
  prepareDocValuesAndPointValues();
  getNormDocValues();
}

void MemoryIndex::Info::sortTerms()
{
  if (sortedTerms.empty()) {
    sortedTerms = terms->sort();
  }
}

void MemoryIndex::Info::prepareDocValuesAndPointValues()
{
  if (preparedDocValuesAndPointValues == false) {
    DocValuesType dvType = fieldInfo->getDocValuesType();
    if (dvType == DocValuesType::NUMERIC ||
        dvType == DocValuesType::SORTED_NUMERIC) {
      numericProducer->prepareForUsage();
    }
    if (dvType == DocValuesType::BINARY || dvType == DocValuesType::SORTED ||
        dvType == DocValuesType::SORTED_SET) {
      binaryProducer->prepareForUsage();
    }
    if (pointValues.size() > 0) {
      assert((pointValues[0]->bytes->size() == pointValues[0]->length,
              L"BytesRef should wrap a precise byte[], BytesRef.deepCopyOf() "
              L"should take care of this"));

      constexpr int numDimensions = fieldInfo->getPointDimensionCount();
      constexpr int numBytesPerDimension = fieldInfo->getPointNumBytes();
      if (numDimensions == 1) {
        // PointInSetQuery.MergePointVisitor expects values to be visited in
        // increasing order, this is a 1d optimization which has to be done here
        // too. Otherwise we emit values out of order which causes mismatches.
        Arrays::sort(pointValues, 0, pointValuesCount);
        minPackedValue = pointValues[0]->bytes.clone();
        maxPackedValue = pointValues[pointValuesCount - 1]->bytes.clone();
      } else {
        minPackedValue = pointValues[0]->bytes.clone();
        maxPackedValue = pointValues[0]->bytes.clone();
        for (int i = 0; i < pointValuesCount; i++) {
          shared_ptr<BytesRef> pointValue = pointValues[i];
          assert((pointValue->bytes.size() == pointValue->length,
                  L"BytesRef should wrap a precise byte[], "
                  L"BytesRef.deepCopyOf() should take care of this"));
          for (int dim = 0; dim < numDimensions; ++dim) {
            int offset = dim * numBytesPerDimension;
            if (StringHelper::compare(numBytesPerDimension, pointValue->bytes,
                                      offset, minPackedValue, offset) < 0) {
              System::arraycopy(pointValue->bytes, offset, minPackedValue,
                                offset, numBytesPerDimension);
            }
            if (StringHelper::compare(numBytesPerDimension, pointValue->bytes,
                                      offset, maxPackedValue, offset) > 0) {
              System::arraycopy(pointValue->bytes, offset, maxPackedValue,
                                offset, numBytesPerDimension);
            }
          }
        }
      }
    }
    preparedDocValuesAndPointValues = true;
  }
}

shared_ptr<NumericDocValues> MemoryIndex::Info::getNormDocValues()
{
  if (!norm) {
    shared_ptr<FieldInvertState> invertState = make_shared<FieldInvertState>(
        Version::LATEST->major, fieldInfo->name, fieldInfo->number, numTokens,
        numOverlapTokens, 0);
    constexpr int64_t value =
        outerInstance->normSimilarity->computeNorm(invertState);
    if (DEBUG) {
      System::err::println(L"MemoryIndexReader.norms: " + fieldInfo->name +
                           L":" + to_wstring(value) + L":" +
                           to_wstring(numTokens));
    }

    norm = value;
  }
  return numericDocValues(norm);
}

int MemoryIndex::MemoryDocValuesIterator::advance(int doc)
{
  this->doc = doc;
  return docId();
}

int MemoryIndex::MemoryDocValuesIterator::nextDoc()
{
  doc++;
  return docId();
}

int MemoryIndex::MemoryDocValuesIterator::docId()
{
  return doc > 0 ? NumericDocValues::NO_MORE_DOCS : doc;
}

shared_ptr<SortedNumericDocValues>
MemoryIndex::numericDocValues(std::deque<int64_t> &values, int count)
{
  shared_ptr<MemoryDocValuesIterator> it =
      make_shared<MemoryDocValuesIterator>();
  return make_shared<SortedNumericDocValuesAnonymousInnerClass>(values, count,
                                                                it);
}

MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass(
        deque<int64_t> &values, int count,
        shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                       MemoryDocValuesIterator>
            it)
{
  this->values = values;
  this->count = count;
  this->it = it;
}

int64_t
MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::nextValue() throw(
    IOException)
{
  return values[ord++];
}

int MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::docValueCount()
{
  return count;
}

bool MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  ord = 0;
  return it->advance(target) == target;
}

int MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::docID()
{
  return it->docId();
}

int MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  return it->nextDoc();
}

int MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::advance(
    int target) 
{
  return it->advance(target);
}

int64_t MemoryIndex::SortedNumericDocValuesAnonymousInnerClass::cost()
{
  return 1;
}

shared_ptr<NumericDocValues> MemoryIndex::numericDocValues(int64_t value)
{
  shared_ptr<MemoryDocValuesIterator> it =
      make_shared<MemoryDocValuesIterator>();
  return make_shared<NumericDocValuesAnonymousInnerClass>(value, it);
}

MemoryIndex::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass(
        int64_t value, shared_ptr<org::apache::lucene::index::memory::
                                        MemoryIndex::MemoryDocValuesIterator>
                             it)
{
  this->value = value;
  this->it = it;
}

int64_t
MemoryIndex::NumericDocValuesAnonymousInnerClass::longValue() 
{
  return value;
}

bool MemoryIndex::NumericDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  return advance(target) == target;
}

int MemoryIndex::NumericDocValuesAnonymousInnerClass::docID()
{
  return it->docId();
}

int MemoryIndex::NumericDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  return it->nextDoc();
}

int MemoryIndex::NumericDocValuesAnonymousInnerClass::advance(int target) throw(
    IOException)
{
  return it->advance(target);
}

int64_t MemoryIndex::NumericDocValuesAnonymousInnerClass::cost() { return 1; }

shared_ptr<SortedDocValues>
MemoryIndex::sortedDocValues(shared_ptr<BytesRef> value)
{
  shared_ptr<MemoryDocValuesIterator> it =
      make_shared<MemoryDocValuesIterator>();
  return make_shared<SortedDocValuesAnonymousInnerClass>(value, it);
}

MemoryIndex::SortedDocValuesAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass(
        shared_ptr<BytesRef> value,
        shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                       MemoryDocValuesIterator>
            it)
{
  this->value = value;
  this->it = it;
}

int MemoryIndex::SortedDocValuesAnonymousInnerClass::ordValue() { return 0; }

shared_ptr<BytesRef> MemoryIndex::SortedDocValuesAnonymousInnerClass::lookupOrd(
    int ord) 
{
  return value;
}

int MemoryIndex::SortedDocValuesAnonymousInnerClass::getValueCount()
{
  return 1;
}

bool MemoryIndex::SortedDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  return it->advance(target) == target;
}

int MemoryIndex::SortedDocValuesAnonymousInnerClass::docID()
{
  return it->docId();
}

int MemoryIndex::SortedDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  return it->nextDoc();
}

int MemoryIndex::SortedDocValuesAnonymousInnerClass::advance(int target) throw(
    IOException)
{
  return it->advance(target);
}

int64_t MemoryIndex::SortedDocValuesAnonymousInnerClass::cost() { return 1; }

shared_ptr<SortedSetDocValues>
MemoryIndex::sortedSetDocValues(shared_ptr<BytesRefHash> values,
                                std::deque<int> &bytesIds)
{
  shared_ptr<MemoryDocValuesIterator> it =
      make_shared<MemoryDocValuesIterator>();
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  return make_shared<SortedSetDocValuesAnonymousInnerClass>(values, bytesIds,
                                                            it, scratch);
}

MemoryIndex::SortedSetDocValuesAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass(
        shared_ptr<BytesRefHash> values, deque<int> &bytesIds,
        shared_ptr<org::apache::lucene::index::memory::MemoryIndex::
                       MemoryDocValuesIterator>
            it,
        shared_ptr<BytesRef> scratch)
{
  this->values = values;
  this->bytesIds = bytesIds;
  this->it = it;
  this->scratch = scratch;
}

int64_t
MemoryIndex::SortedSetDocValuesAnonymousInnerClass::nextOrd() 
{
  if (ord >= values->size()) {
    return NO_MORE_ORDS;
  }
  return ord++;
}

shared_ptr<BytesRef>
MemoryIndex::SortedSetDocValuesAnonymousInnerClass::lookupOrd(
    int64_t ord) 
{
  return values->get(bytesIds[static_cast<int>(ord)], scratch);
}

int64_t MemoryIndex::SortedSetDocValuesAnonymousInnerClass::getValueCount()
{
  return values->size();
}

bool MemoryIndex::SortedSetDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  ord = 0;
  return it->advance(target) == target;
}

int MemoryIndex::SortedSetDocValuesAnonymousInnerClass::docID()
{
  return it->docId();
}

int MemoryIndex::SortedSetDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  return it->nextDoc();
}

int MemoryIndex::SortedSetDocValuesAnonymousInnerClass::advance(
    int target) 
{
  return it->advance(target);
}

int64_t MemoryIndex::SortedSetDocValuesAnonymousInnerClass::cost()
{
  return 1;
}

void MemoryIndex::BinaryDocValuesProducer::prepareForUsage()
{
  bytesIds = dvBytesValuesSet->sort();
}

void MemoryIndex::NumericDocValuesProducer::prepareForUsage()
{
  Arrays::sort(dvLongValues, 0, count);
}

MemoryIndex::MemoryIndexReader::MemoryIndexReader(
    shared_ptr<MemoryIndex> outerInstance)
    : LeafReader(), outerInstance(outerInstance)
{
  for (auto info : outerInstance->fields) {
    info->second.prepareDocValuesAndPointValues();
  }
}

shared_ptr<Info>
MemoryIndex::MemoryIndexReader::getInfoForExpectedDocValuesType(
    const wstring &fieldName, DocValuesType expectedType)
{
  if (expectedType == DocValuesType::NONE) {
    return nullptr;
  }
  shared_ptr<Info> info = outerInstance->fields->get(fieldName);
  if (info == nullptr) {
    return nullptr;
  }
  if (info->fieldInfo.getDocValuesType() != expectedType) {
    return nullptr;
  }
  return info;
}

shared_ptr<Bits> MemoryIndex::MemoryIndexReader::getLiveDocs()
{
  return nullptr;
}

shared_ptr<FieldInfos> MemoryIndex::MemoryIndexReader::getFieldInfos()
{
  std::deque<std::shared_ptr<FieldInfo>> fieldInfos(
      outerInstance->fields->size());
  int i = 0;
  for (auto info : outerInstance->fields) {
    fieldInfos[i++] = info->second.fieldInfo;
  }
  return make_shared<FieldInfos>(fieldInfos);
}

shared_ptr<NumericDocValues>
MemoryIndex::MemoryIndexReader::getNumericDocValues(const wstring &field) throw(
    IOException)
{
  shared_ptr<Info> info =
      getInfoForExpectedDocValuesType(field, DocValuesType::NUMERIC);
  if (info == nullptr) {
    return nullptr;
  }
  return numericDocValues(info->numericProducer.dvLongValues[0]);
}

shared_ptr<BinaryDocValues>
MemoryIndex::MemoryIndexReader::getBinaryDocValues(const wstring &field)
{
  return getSortedDocValues(field, DocValuesType::BINARY);
}

shared_ptr<SortedDocValues>
MemoryIndex::MemoryIndexReader::getSortedDocValues(const wstring &field)
{
  return getSortedDocValues(field, DocValuesType::SORTED);
}

shared_ptr<SortedDocValues>
MemoryIndex::MemoryIndexReader::getSortedDocValues(const wstring &field,
                                                   DocValuesType docValuesType)
{
  shared_ptr<Info> info = getInfoForExpectedDocValuesType(field, docValuesType);
  if (info != nullptr) {
    shared_ptr<BytesRef> value =
        info->binaryProducer.dvBytesValuesSet->get(0, make_shared<BytesRef>());
    return sortedDocValues(value);
  } else {
    return nullptr;
  }
}

shared_ptr<SortedNumericDocValues>
MemoryIndex::MemoryIndexReader::getSortedNumericDocValues(const wstring &field)
{
  shared_ptr<Info> info =
      getInfoForExpectedDocValuesType(field, DocValuesType::SORTED_NUMERIC);
  if (info != nullptr) {
    return numericDocValues(info->numericProducer.dvLongValues,
                            info->numericProducer.count);
  } else {
    return nullptr;
  }
}

shared_ptr<SortedSetDocValues>
MemoryIndex::MemoryIndexReader::getSortedSetDocValues(const wstring &field)
{
  shared_ptr<Info> info =
      getInfoForExpectedDocValuesType(field, DocValuesType::SORTED_SET);
  if (info != nullptr) {
    return sortedSetDocValues(info->binaryProducer.dvBytesValuesSet,
                              info->binaryProducer.bytesIds);
  } else {
    return nullptr;
  }
}

shared_ptr<PointValues>
MemoryIndex::MemoryIndexReader::getPointValues(const wstring &fieldName)
{
  shared_ptr<Info> info = outerInstance->fields->get(fieldName);
  if (info == nullptr || info->pointValues == nullptr) {
    return nullptr;
  }
  return make_shared<MemoryIndexPointValues>(shared_from_this(), info);
}

void MemoryIndex::MemoryIndexReader::checkIntegrity() 
{
  // no-op
}

shared_ptr<Terms>
MemoryIndex::MemoryIndexReader::terms(const wstring &field) 
{
  return memoryFields->terms(field);
}

MemoryIndex::MemoryIndexReader::MemoryFields::MemoryFields(
    shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance,
    unordered_map<wstring, std::shared_ptr<Info>> &fields)
    : fields(fields), outerInstance(outerInstance)
{
}

shared_ptr<Iterator<wstring>>
MemoryIndex::MemoryIndexReader::MemoryFields::iterator()
{
  return fields.entrySet()
      .stream()
      .filter([&](any e) { return e::getValue().numTokens > 0; })
      .map_obj(unordered_map::Entry::getKey)
      .begin();
}

shared_ptr<Terms>
MemoryIndex::MemoryIndexReader::MemoryFields::terms(const wstring &field)
{
  shared_ptr<Info> *const info = fields[field];
  if (info == nullptr || info->numTokens <= 0) {
    return nullptr;
  }

  return make_shared<TermsAnonymousInnerClass>(shared_from_this(), info);
}

MemoryIndex::MemoryIndexReader::MemoryFields::TermsAnonymousInnerClass::
    TermsAnonymousInnerClass(
        shared_ptr<MemoryFields> outerInstance,
        shared_ptr<org::apache::lucene::index::memory::MemoryIndex::Info> info)
{
  this->outerInstance = outerInstance;
  this->info = info;
}

shared_ptr<TermsEnum> MemoryIndex::MemoryIndexReader::MemoryFields::
    TermsAnonymousInnerClass::iterator()
{
  return make_shared<MemoryTermsEnum>(outerInstance->outerInstance, info);
}

int64_t
MemoryIndex::MemoryIndexReader::MemoryFields::TermsAnonymousInnerClass::size()
{
  return info->terms->size();
}

int64_t MemoryIndex::MemoryIndexReader::MemoryFields::
    TermsAnonymousInnerClass::getSumTotalTermFreq()
{
  return info->sumTotalTermFreq;
}

int64_t MemoryIndex::MemoryIndexReader::MemoryFields::
    TermsAnonymousInnerClass::getSumDocFreq()
{
  // each term has df=1
  return info->terms->size();
}

int MemoryIndex::MemoryIndexReader::MemoryFields::TermsAnonymousInnerClass::
    getDocCount()
{
  return outerInstance->size() > 0 ? 1 : 0;
}

bool MemoryIndex::MemoryIndexReader::MemoryFields::TermsAnonymousInnerClass::
    hasFreqs()
{
  return true;
}

bool MemoryIndex::MemoryIndexReader::MemoryFields::TermsAnonymousInnerClass::
    hasOffsets()
{
  return outerInstance->outerInstance->outerInstance->storeOffsets;
}

bool MemoryIndex::MemoryIndexReader::MemoryFields::TermsAnonymousInnerClass::
    hasPositions()
{
  return true;
}

bool MemoryIndex::MemoryIndexReader::MemoryFields::TermsAnonymousInnerClass::
    hasPayloads()
{
  return outerInstance->outerInstance->outerInstance->storePayloads;
}

int MemoryIndex::MemoryIndexReader::MemoryFields::size()
{
  int size = 0;
  for (auto fieldName : shared_from_this()) {
    size++;
  }
  return size;
}

MemoryIndex::MemoryIndexReader::MemoryTermsEnum::MemoryTermsEnum(
    shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance,
    shared_ptr<Info> info)
    : info(info), outerInstance(outerInstance)
{
  info->sortTerms();
}

int MemoryIndex::MemoryIndexReader::MemoryTermsEnum::binarySearch(
    shared_ptr<BytesRef> b, shared_ptr<BytesRef> bytesRef, int low, int high,
    shared_ptr<BytesRefHash> hash, std::deque<int> &ords)
{
  int mid = 0;
  while (low <= high) {
    mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
    hash->get(ords[mid], bytesRef);
    constexpr int cmp = bytesRef->compareTo(b);
    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      return mid;
    }
  }
  assert(bytesRef->compareTo(b) != 0);
  return -(low + 1);
}

bool MemoryIndex::MemoryIndexReader::MemoryTermsEnum::seekExact(
    shared_ptr<BytesRef> text)
{
  termUpto = binarySearch(text, br, 0, info->terms->size() - 1, info->terms,
                          info->sortedTerms);
  return termUpto >= 0;
}

SeekStatus MemoryIndex::MemoryIndexReader::MemoryTermsEnum::seekCeil(
    shared_ptr<BytesRef> text)
{
  termUpto = binarySearch(text, br, 0, info->terms->size() - 1, info->terms,
                          info->sortedTerms);
  if (termUpto < 0) { // not found; choose successor
    termUpto = -termUpto - 1;
    if (termUpto >= info->terms->size()) {
      return SeekStatus::END;
    } else {
      info->terms->get(info->sortedTerms[termUpto], br);
      return SeekStatus::NOT_FOUND;
    }
  } else {
    return SeekStatus::FOUND;
  }
}

void MemoryIndex::MemoryIndexReader::MemoryTermsEnum::seekExact(int64_t ord)
{
  assert(ord < info->terms->size());
  termUpto = static_cast<int>(ord);
  info->terms->get(info->sortedTerms[termUpto], br);
}

shared_ptr<BytesRef> MemoryIndex::MemoryIndexReader::MemoryTermsEnum::next()
{
  termUpto++;
  if (termUpto >= info->terms->size()) {
    return nullptr;
  } else {
    info->terms->get(info->sortedTerms[termUpto], br);
    return br;
  }
}

shared_ptr<BytesRef> MemoryIndex::MemoryIndexReader::MemoryTermsEnum::term()
{
  return br;
}

int64_t MemoryIndex::MemoryIndexReader::MemoryTermsEnum::ord()
{
  return termUpto;
}

int MemoryIndex::MemoryIndexReader::MemoryTermsEnum::docFreq() { return 1; }

int64_t MemoryIndex::MemoryIndexReader::MemoryTermsEnum::totalTermFreq()
{
  return info->sliceArray.freq[info->sortedTerms[termUpto]];
}

shared_ptr<PostingsEnum>
MemoryIndex::MemoryIndexReader::MemoryTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags)
{
  if (reuse == nullptr ||
      !(std::dynamic_pointer_cast<MemoryPostingsEnum>(reuse) != nullptr)) {
    reuse = make_shared<MemoryPostingsEnum>(outerInstance);
  }
  constexpr int ord = info->sortedTerms[termUpto];
  return (std::static_pointer_cast<MemoryPostingsEnum>(reuse))
      ->reset(info->sliceArray.start[ord], info->sliceArray.end[ord],
              info->sliceArray.freq[ord]);
}

void MemoryIndex::MemoryIndexReader::MemoryTermsEnum::seekExact(
    shared_ptr<BytesRef> term, shared_ptr<TermState> state) 
{
  assert(state != nullptr);
  this->seekExact((std::static_pointer_cast<OrdTermState>(state))->ord);
}

shared_ptr<TermState>
MemoryIndex::MemoryIndexReader::MemoryTermsEnum::termState() 
{
  shared_ptr<OrdTermState> ts = make_shared<OrdTermState>();
  ts->ord = termUpto;
  return ts;
}

MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::MemoryPostingsEnum(
    shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance)
    : sliceReader(
          make_shared<org::apache::lucene::util::IntBlockPool::SliceReader>(
              outerInstance->outerInstance->intBlockPool)),
      payloadBuilder(outerInstance->outerInstance->storePayloads
                         ? make_shared<BytesRefBuilder>()
                         : nullptr),
      outerInstance(outerInstance)
{
}

shared_ptr<PostingsEnum>
MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::reset(int start, int end,
                                                          int freq)
{
  this->sliceReader->reset(start, end);
  posUpto = 0; // for assert
  hasNext = true;
  doc = -1;
  this->freq_ = freq;
  return shared_from_this();
}

int MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::docID() { return doc; }

int MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::nextDoc()
{
  pos = -1;
  if (hasNext) {
    hasNext = false;
    return doc = 0;
  } else {
    return doc = NO_MORE_DOCS;
  }
}

int MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::advance(
    int target) 
{
  return slowAdvance(target);
}

int MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::freq() throw(
    IOException)
{
  return freq_;
}

int MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::nextPosition()
{
  posUpto++;
  assert(posUpto <= freq_);
  assert((!sliceReader->endOfSlice(),
          L" stores offsets : " + to_wstring(startOffset_)));
  int pos = sliceReader->readInt();
  if (outerInstance->outerInstance->storeOffsets) {
    // pos = sliceReader.readInt();
    startOffset_ = sliceReader->readInt();
    endOffset_ = sliceReader->readInt();
  }
  if (outerInstance->outerInstance->storePayloads) {
    payloadIndex = sliceReader->readInt();
  }
  return pos;
}

int MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::startOffset()
{
  return startOffset_;
}

int MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::endOffset()
{
  return endOffset_;
}

shared_ptr<BytesRef>
MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::getPayload()
{
  if (payloadBuilder == nullptr || payloadIndex == -1) {
    return nullptr;
  }
  return outerInstance->outerInstance->payloadsBytesRefs->get(payloadBuilder,
                                                              payloadIndex);
}

int64_t MemoryIndex::MemoryIndexReader::MemoryPostingsEnum::cost()
{
  return 1;
}

MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::MemoryIndexPointValues(
    shared_ptr<MemoryIndex::MemoryIndexReader> outerInstance,
    shared_ptr<Info> info)
    : info(Objects::requireNonNull(info)), outerInstance(outerInstance)
{
  Objects::requireNonNull(info->pointValues, L"Field does not have points");
}

void MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::intersect(
    shared_ptr<IntersectVisitor> visitor) 
{
  std::deque<std::shared_ptr<BytesRef>> values = info->pointValues;

  visitor->grow(info->pointValuesCount);
  for (int i = 0; i < info->pointValuesCount; i++) {
    visitor->visit(0, values[i]->bytes);
  }
}

int64_t
MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::estimatePointCount(
    shared_ptr<IntersectVisitor> visitor)
{
  return 1LL;
}

std::deque<char> MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::
    getMinPackedValue() 
{
  return info->minPackedValue;
}

std::deque<char> MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::
    getMaxPackedValue() 
{
  return info->maxPackedValue;
}

int MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::
    getNumDimensions() 
{
  return info->fieldInfo.getPointDimensionCount();
}

int MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::
    getBytesPerDimension() 
{
  return info->fieldInfo.getPointNumBytes();
}

int64_t MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::size()
{
  return info->pointValuesCount;
}

int MemoryIndex::MemoryIndexReader::MemoryIndexPointValues::getDocCount()
{
  return 1;
}

shared_ptr<Fields> MemoryIndex::MemoryIndexReader::getTermVectors(int docID)
{
  if (docID == 0) {
    return memoryFields;
  } else {
    return nullptr;
  }
}

int MemoryIndex::MemoryIndexReader::numDocs()
{
  if (DEBUG) {
    System::err::println(L"MemoryIndexReader.numDocs");
  }
  return 1;
}

int MemoryIndex::MemoryIndexReader::maxDoc()
{
  if (DEBUG) {
    System::err::println(L"MemoryIndexReader.maxDoc");
  }
  return 1;
}

void MemoryIndex::MemoryIndexReader::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor)
{
  if (DEBUG) {
    System::err::println(L"MemoryIndexReader.document");
  }
  // no-op: there are no stored fields
}

void MemoryIndex::MemoryIndexReader::doClose()
{
  if (DEBUG) {
    System::err::println(L"MemoryIndexReader.doClose");
  }
}

shared_ptr<NumericDocValues>
MemoryIndex::MemoryIndexReader::getNormValues(const wstring &field)
{
  shared_ptr<Info> info = outerInstance->fields->get(field);
  if (info == nullptr || info->fieldInfo.omitsNorms()) {
    return nullptr;
  }
  return info->getNormDocValues();
}

shared_ptr<LeafMetaData> MemoryIndex::MemoryIndexReader::getMetaData()
{
  return make_shared<LeafMetaData>(Version::LATEST->major, Version::LATEST,
                                   nullptr);
}

shared_ptr<CacheHelper> MemoryIndex::MemoryIndexReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> MemoryIndex::MemoryIndexReader::getReaderCacheHelper()
{
  return nullptr;
}

void MemoryIndex::reset()
{
  fields->clear();
  this->normSimilarity = IndexSearcher::getDefaultSimilarity();
  byteBlockPool->reset(false, false); // no need to 0-fill the buffers
  intBlockPool->reset(true, false); // here must must 0-fill since we use slices
  if (payloadsBytesRefs != nullptr) {
    payloadsBytesRefs->clear();
  }
  this->frozen = false;
}

MemoryIndex::SliceByteStartArray::SliceByteStartArray(int initSize)
    : org::apache::lucene::util::BytesRefHash::DirectBytesStartArray(initSize)
{
}

std::deque<int> MemoryIndex::SliceByteStartArray::init()
{
  const std::deque<int> ord = BytesRefHash::DirectBytesStartArray::init();
  start = std::deque<int>(ArrayUtil::oversize(ord.size(), Integer::BYTES));
  end = std::deque<int>(ArrayUtil::oversize(ord.size(), Integer::BYTES));
  freq = std::deque<int>(ArrayUtil::oversize(ord.size(), Integer::BYTES));
  assert(start.size() >= ord.size());
  assert(end.size() >= ord.size());
  assert(freq.size() >= ord.size());
  return ord;
}

std::deque<int> MemoryIndex::SliceByteStartArray::grow()
{
  const std::deque<int> ord = BytesRefHash::DirectBytesStartArray::grow();
  if (start.size() < ord.size()) {
    start = ArrayUtil::grow(start, ord.size());
    end = ArrayUtil::grow(end, ord.size());
    freq = ArrayUtil::grow(freq, ord.size());
  }
  assert(start.size() >= ord.size());
  assert(end.size() >= ord.size());
  assert(freq.size() >= ord.size());
  return ord;
}

std::deque<int> MemoryIndex::SliceByteStartArray::clear()
{
  start = end.clear();
  return BytesRefHash::DirectBytesStartArray::clear();
}
} // namespace org::apache::lucene::index::memory