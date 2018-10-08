using namespace std;

#include "DefaultIndexingChain.h"

namespace org::apache::lucene::index
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldType = org::apache::lucene::document::FieldType;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using IOContext = org::apache::lucene::store::IOContext;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using MaxBytesLengthExceededException =
    org::apache::lucene::util::BytesRefHash::MaxBytesLengthExceededException;
using Counter = org::apache::lucene::util::Counter;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

DefaultIndexingChain::DefaultIndexingChain(
    shared_ptr<DocumentsWriterPerThread> docWriter) 
    : bytesUsed(docWriter->bytesUsed_), docState(docWriter->docState),
      docWriter(docWriter), fieldInfos(docWriter->getFieldInfosBuilder()),
      termsHash(make_shared<FreqProxTermsWriter>(docWriter, termVectorsWriter))
{

  shared_ptr<TermsHash> *const termVectorsWriter;
  if (docWriter->getSegmentInfo()->getIndexSort() == nullptr) {
    storedFieldsConsumer = make_shared<StoredFieldsConsumer>(docWriter);
    termVectorsWriter = make_shared<TermVectorsConsumer>(docWriter);
  } else {
    storedFieldsConsumer = make_shared<SortingStoredFieldsConsumer>(docWriter);
    termVectorsWriter = make_shared<SortingTermVectorsConsumer>(docWriter);
  }
}

shared_ptr<Sorter::DocMap> DefaultIndexingChain::maybeSortSegment(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<Sort> indexSort = state->segmentInfo->getIndexSort();
  if (indexSort == nullptr) {
    return nullptr;
  }

  deque<std::shared_ptr<Sorter::DocComparator>> comparators =
      deque<std::shared_ptr<Sorter::DocComparator>>();
  for (int i = 0; i < indexSort->getSort().size(); i++) {
    shared_ptr<SortField> sortField = indexSort->getSort()[i];
    shared_ptr<PerField> perField = getPerField(sortField->getField());
    if (perField != nullptr && perField->docValuesWriter != nullptr &&
        finishedDocValues->contains(perField->fieldInfo->name) == false) {
      perField->docValuesWriter->finish(state->segmentInfo->maxDoc());
      shared_ptr<Sorter::DocComparator> cmp =
          perField->docValuesWriter->getDocComparator(
              state->segmentInfo->maxDoc(), sortField);
      comparators.push_back(cmp);
      finishedDocValues->add(perField->fieldInfo->name);
    } else {
      // safe to ignore, sort field with no values or already seen before
    }
  }
  shared_ptr<Sorter> sorter = make_shared<Sorter>(indexSort);
  // returns null if the documents are already sorted
  return sorter->sort(
      state->segmentInfo->maxDoc(),
      comparators.toArray(std::deque<std::shared_ptr<Sorter::DocComparator>>(
          comparators.size())));
}

shared_ptr<Sorter::DocMap> DefaultIndexingChain::flush(
    shared_ptr<SegmentWriteState> state) 
{

  // NOTE: caller (DocumentsWriterPerThread) handles
  // aborting on any exception from this method
  shared_ptr<Sorter::DocMap> sortMap = maybeSortSegment(state);
  int maxDoc = state->segmentInfo->maxDoc();
  int64_t t0 = System::nanoTime();
  writeNorms(state, sortMap);
  if (docState->infoStream->isEnabled(L"IW")) {
    docState->infoStream->message(L"IW", ((System::nanoTime() - t0) / 1000000) +
                                             L" msec to write norms");
  }

  t0 = System::nanoTime();
  writeDocValues(state, sortMap);
  if (docState->infoStream->isEnabled(L"IW")) {
    docState->infoStream->message(L"IW", ((System::nanoTime() - t0) / 1000000) +
                                             L" msec to write docValues");
  }

  t0 = System::nanoTime();
  writePoints(state, sortMap);
  if (docState->infoStream->isEnabled(L"IW")) {
    docState->infoStream->message(L"IW", ((System::nanoTime() - t0) / 1000000) +
                                             L" msec to write points");
  }

  // it's possible all docs hit non-aborting exceptions...
  t0 = System::nanoTime();
  storedFieldsConsumer->finish(maxDoc);
  storedFieldsConsumer->flush(state, sortMap);
  if (docState->infoStream->isEnabled(L"IW")) {
    docState->infoStream->message(L"IW", ((System::nanoTime() - t0) / 1000000) +
                                             L" msec to finish stored fields");
  }

  t0 = System::nanoTime();
  unordered_map<wstring, std::shared_ptr<TermsHashPerField>> fieldsToFlush =
      unordered_map<wstring, std::shared_ptr<TermsHashPerField>>();
  for (int i = 0; i < fieldHash.size(); i++) {
    shared_ptr<PerField> perField = fieldHash[i];
    while (perField != nullptr) {
      if (perField->invertState != nullptr) {
        fieldsToFlush.emplace(perField->fieldInfo->name,
                              perField->termsHashPerField);
      }
      perField = perField->next;
    }
  }

  termsHash->flush(fieldsToFlush, state, sortMap);
  if (docState->infoStream->isEnabled(L"IW")) {
    docState->infoStream->message(
        L"IW", ((System::nanoTime() - t0) / 1000000) +
                   L" msec to write postings and finish vectors");
  }

  // Important to save after asking consumer to flush so
  // consumer can alter the FieldInfo* if necessary.  EG,
  // FreqProxTermsWriter does this with
  // FieldInfo.storePayload.
  t0 = System::nanoTime();
  docWriter->codec->fieldInfosFormat()->write(
      state->directory, state->segmentInfo, L"", state->fieldInfos,
      IOContext::DEFAULT);
  if (docState->infoStream->isEnabled(L"IW")) {
    docState->infoStream->message(L"IW", ((System::nanoTime() - t0) / 1000000) +
                                             L" msec to write fieldInfos");
  }

  return sortMap;
}

void DefaultIndexingChain::writePoints(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  shared_ptr<PointsWriter> pointsWriter = nullptr;
  bool success = false;
  try {
    for (int i = 0; i < fieldHash.size(); i++) {
      shared_ptr<PerField> perField = fieldHash[i];
      while (perField != nullptr) {
        if (perField->pointValuesWriter != nullptr) {
          if (perField->fieldInfo->getPointDimensionCount() == 0) {
            // BUG
            throw make_shared<AssertionError>(
                L"segment=" + state->segmentInfo + L": field=\"" +
                perField->fieldInfo->name + L"\" has no points but wrote them");
          }
          if (pointsWriter == nullptr) {
            // lazy init
            shared_ptr<PointsFormat> fmt =
                state->segmentInfo->getCodec()->pointsFormat();
            if (fmt == nullptr) {
              throw make_shared<IllegalStateException>(
                  L"field=\"" + perField->fieldInfo->name +
                  L"\" was indexed as points but codec does not support "
                  L"points");
            }
            pointsWriter = fmt->fieldsWriter(state);
          }

          perField->pointValuesWriter->flush(state, sortMap, pointsWriter);
          perField->pointValuesWriter.reset();
        } else if (perField->fieldInfo->getPointDimensionCount() != 0) {
          // BUG
          throw make_shared<AssertionError>(
              L"segment=" + state->segmentInfo + L": field=\"" +
              perField->fieldInfo->name +
              L"\" has points but did not write them");
        }
        perField = perField->next;
      }
    }
    if (pointsWriter != nullptr) {
      pointsWriter->finish();
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({pointsWriter});
    } else {
      IOUtils::closeWhileHandlingException({pointsWriter});
    }
  }
}

void DefaultIndexingChain::writeDocValues(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  int maxDoc = state->segmentInfo->maxDoc();
  shared_ptr<DocValuesConsumer> dvConsumer = nullptr;
  bool success = false;
  try {
    for (int i = 0; i < fieldHash.size(); i++) {
      shared_ptr<PerField> perField = fieldHash[i];
      while (perField != nullptr) {
        if (perField->docValuesWriter != nullptr) {
          if (perField->fieldInfo->getDocValuesType() == DocValuesType::NONE) {
            // BUG
            throw make_shared<AssertionError>(
                L"segment=" + state->segmentInfo + L": field=\"" +
                perField->fieldInfo->name +
                L"\" has no docValues but wrote them");
          }
          if (dvConsumer == nullptr) {
            // lazy init
            shared_ptr<DocValuesFormat> fmt =
                state->segmentInfo->getCodec()->docValuesFormat();
            dvConsumer = fmt->fieldsConsumer(state);
          }

          if (finishedDocValues->contains(perField->fieldInfo->name) == false) {
            perField->docValuesWriter->finish(maxDoc);
          }
          perField->docValuesWriter->flush(state, sortMap, dvConsumer);
          perField->docValuesWriter.reset();
        } else if (perField->fieldInfo->getDocValuesType() !=
                   DocValuesType::NONE) {
          // BUG
          throw make_shared<AssertionError>(
              L"segment=" + state->segmentInfo + L": field=\"" +
              perField->fieldInfo->name +
              L"\" has docValues but did not write them");
        }
        perField = perField->next;
      }
    }

    // TODO: catch missing DV fields here?  else we have
    // null/"" depending on how docs landed in segments?
    // but we can't detect all cases, and we should leave
    // this behavior undefined. dv is not "schemaless": it's column-stride.
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({dvConsumer});
    } else {
      IOUtils::closeWhileHandlingException({dvConsumer});
    }
  }

  if (state->fieldInfos->hasDocValues() == false) {
    if (dvConsumer != nullptr) {
      // BUG
      throw make_shared<AssertionError>(
          L"segment=" + state->segmentInfo +
          L": fieldInfos has no docValues but wrote them");
    }
  } else if (dvConsumer == nullptr) {
    // BUG
    throw make_shared<AssertionError>(
        L"segment=" + state->segmentInfo +
        L": fieldInfos has docValues but did not wrote them");
  }
}

void DefaultIndexingChain::writeNorms(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  bool success = false;
  shared_ptr<NormsConsumer> normsConsumer = nullptr;
  try {
    if (state->fieldInfos->hasNorms()) {
      shared_ptr<NormsFormat> normsFormat =
          state->segmentInfo->getCodec()->normsFormat();
      assert(normsFormat != nullptr);
      normsConsumer = normsFormat->normsConsumer(state);

      for (auto fi : state->fieldInfos) {
        shared_ptr<PerField> perField = getPerField(fi->name);
        assert(perField != nullptr);

        // we must check the final value of omitNorms for the fieldinfo: it
        // could have changed for this field since the first time we added it.
        if (fi->omitsNorms() == false &&
            fi->getIndexOptions() != IndexOptions::NONE) {
          assert((perField->norms != nullptr, L"field=" + fi->name));
          perField->norms->finish(state->segmentInfo->maxDoc());
          perField->norms->flush(state, sortMap, normsConsumer);
        }
      }
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({normsConsumer});
    } else {
      IOUtils::closeWhileHandlingException({normsConsumer});
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings("try") public void abort() throws
// java.io.IOException
void DefaultIndexingChain::abort() 
{
  // finalizer will e.g. close any open files in the term vectors writer:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.Closeable finalizer =
  // termsHash::abort)
  {
    java::io::Closeable finalizer = termsHash::abort;
    try {
      storedFieldsConsumer->abort();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      Arrays::fill(fieldHash, nullptr);
    }
  }
}

void DefaultIndexingChain::rehash()
{
  int newHashSize = (fieldHash.size() * 2);
  assert(newHashSize > fieldHash.size());

  std::deque<std::shared_ptr<PerField>> newHashArray(newHashSize);

  // Rehash
  int newHashMask = newHashSize - 1;
  for (int j = 0; j < fieldHash.size(); j++) {
    shared_ptr<PerField> fp0 = fieldHash[j];
    while (fp0 != nullptr) {
      constexpr int hashPos2 = fp0->fieldInfo->name.hashCode() & newHashMask;
      shared_ptr<PerField> nextFP0 = fp0->next;
      fp0->next = newHashArray[hashPos2];
      newHashArray[hashPos2] = fp0;
      fp0 = nextFP0;
    }
  }

  fieldHash = newHashArray;
  hashMask = newHashMask;
}

void DefaultIndexingChain::startStoredFields(int docID) 
{
  try {
    storedFieldsConsumer->startDocument(docID);
  } catch (const runtime_error &th) {
    docWriter->onAbortingException(th);
    throw th;
  }
}

void DefaultIndexingChain::finishStoredFields() 
{
  try {
    storedFieldsConsumer->finishDocument();
  } catch (const runtime_error &th) {
    docWriter->onAbortingException(th);
    throw th;
  }
}

void DefaultIndexingChain::processDocument() 
{

  // How many indexed field names we've seen (collapses
  // multiple field instances by the same name):
  int fieldCount = 0;

  int64_t fieldGen = nextFieldGen++;

  // NOTE: we need two passes here, in case there are
  // multi-valued fields, because we must process all
  // instances of a given field at once, since the
  // analyzer is free to reuse TokenStream across fields
  // (i.e., we cannot have more than one TokenStream
  // running "at once"):

  termsHash->startDocument();

  startStoredFields(docState->docID);
  try {
    for (auto field : docState->doc) {
      fieldCount = processField(field, fieldGen, fieldCount);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (docWriter->hasHitAbortingException() == false) {
      // Finish each indexed field name seen in the document:
      for (int i = 0; i < fieldCount; i++) {
        fields[i]->finish();
      }
      finishStoredFields();
    }
  }

  try {
    termsHash->finishDocument();
  } catch (const runtime_error &th) {
    // Must abort, on the possibility that on-disk term
    // vectors are now corrupt:
    docWriter->onAbortingException(th);
    throw th;
  }
}

int DefaultIndexingChain::processField(shared_ptr<IndexableField> field,
                                       int64_t fieldGen,
                                       int fieldCount) 
{
  wstring fieldName = field->name();
  shared_ptr<IndexableFieldType> fieldType = field->fieldType();

  shared_ptr<PerField> fp = nullptr;

  if (fieldType->indexOptions() == nullptr) {
    throw make_shared<NullPointerException>(
        L"IndexOptions must not be null (field: \"" + field->name() + L"\")");
  }

  // Invert indexed fields:
  if (fieldType->indexOptions() != IndexOptions::NONE) {
    fp = getOrAddField(fieldName, fieldType, true);
    bool first = fp->fieldGen != fieldGen;
    fp->invert(field, first);

    if (first) {
      fields[fieldCount++] = fp;
      fp->fieldGen = fieldGen;
    }
  } else {
    verifyUnIndexedFieldType(fieldName, fieldType);
  }

  // Add stored fields:
  if (fieldType->stored()) {
    if (fp == nullptr) {
      fp = getOrAddField(fieldName, fieldType, false);
    }
    if (fieldType->stored()) {
      wstring value = field->stringValue();
      if (value != L"" &&
          value.length() > IndexWriter::MAX_STORED_STRING_LENGTH) {
        throw invalid_argument(
            L"stored field \"" + field->name() + L"\" is too large (" +
            to_wstring(value.length()) + L" characters) to store");
      }
      try {
        storedFieldsConsumer->writeField(fp->fieldInfo, field);
      } catch (const runtime_error &th) {
        docWriter->onAbortingException(th);
        throw th;
      }
    }
  }

  DocValuesType dvType = fieldType->docValuesType();
  if (dvType == nullptr) {
    throw make_shared<NullPointerException>(
        L"docValuesType must not be null (field: \"" + fieldName + L"\")");
  }
  if (dvType != DocValuesType::NONE) {
    if (fp == nullptr) {
      fp = getOrAddField(fieldName, fieldType, false);
    }
    indexDocValue(fp, dvType, field);
  }
  if (fieldType->pointDimensionCount() != 0) {
    if (fp == nullptr) {
      fp = getOrAddField(fieldName, fieldType, false);
    }
    indexPoint(fp, field);
  }

  return fieldCount;
}

void DefaultIndexingChain::verifyUnIndexedFieldType(
    const wstring &name, shared_ptr<IndexableFieldType> ft)
{
  if (ft->storeTermVectors()) {
    throw invalid_argument(wstring(L"cannot store term vectors ") +
                           L"for a field that is not indexed (field=\"" + name +
                           L"\")");
  }
  if (ft->storeTermVectorPositions()) {
    throw invalid_argument(wstring(L"cannot store term deque positions ") +
                           L"for a field that is not indexed (field=\"" + name +
                           L"\")");
  }
  if (ft->storeTermVectorOffsets()) {
    throw invalid_argument(wstring(L"cannot store term deque offsets ") +
                           L"for a field that is not indexed (field=\"" + name +
                           L"\")");
  }
  if (ft->storeTermVectorPayloads()) {
    throw invalid_argument(wstring(L"cannot store term deque payloads ") +
                           L"for a field that is not indexed (field=\"" + name +
                           L"\")");
  }
}

void DefaultIndexingChain::indexPoint(
    shared_ptr<PerField> fp,
    shared_ptr<IndexableField> field) 
{
  int pointDimensionCount = field->fieldType()->pointDimensionCount();

  int dimensionNumBytes = field->fieldType()->pointNumBytes();

  // Record dimensions for this field; this setter will throw IllegalArgExc if
  // the dimensions were already set to something different:
  if (fp->fieldInfo->getPointDimensionCount() == 0) {
    fieldInfos->globalFieldNumbers->setDimensions(
        fp->fieldInfo->number, fp->fieldInfo->name, pointDimensionCount,
        dimensionNumBytes);
  }

  fp->fieldInfo->setPointDimensions(pointDimensionCount, dimensionNumBytes);

  if (fp->pointValuesWriter == nullptr) {
    fp->pointValuesWriter =
        make_shared<PointValuesWriter>(docWriter, fp->fieldInfo);
  }
  fp->pointValuesWriter->addPackedValue(docState->docID, field->binaryValue());
}

void DefaultIndexingChain::indexDocValue(
    shared_ptr<PerField> fp, DocValuesType dvType,
    shared_ptr<IndexableField> field) 
{

  if (fp->fieldInfo->getDocValuesType() == DocValuesType::NONE) {
    // This is the first time we are seeing this field indexed with doc values,
    // so we now record the DV type so that any future attempt to (illegally)
    // change the DV type of this field, will throw an IllegalArgExc:
    fieldInfos->globalFieldNumbers->setDocValuesType(
        fp->fieldInfo->number, fp->fieldInfo->name, dvType);
  }
  fp->fieldInfo->setDocValuesType(dvType);

  int docID = docState->docID;

  switch (dvType) {

  case org::apache::lucene::index::DocValuesType::NUMERIC:
    if (fp->docValuesWriter == nullptr) {
      fp->docValuesWriter =
          make_shared<NumericDocValuesWriter>(fp->fieldInfo, bytesUsed);
    }
    if (field->numericValue() == nullptr) {
      throw invalid_argument(L"field=\"" + fp->fieldInfo->name +
                             L"\": null value not allowed");
    }
    (std::static_pointer_cast<NumericDocValuesWriter>(fp->docValuesWriter))
        ->addValue(docID, field->numericValue()->longValue());
    break;

  case org::apache::lucene::index::DocValuesType::BINARY:
    if (fp->docValuesWriter == nullptr) {
      fp->docValuesWriter =
          make_shared<BinaryDocValuesWriter>(fp->fieldInfo, bytesUsed);
    }
    (std::static_pointer_cast<BinaryDocValuesWriter>(fp->docValuesWriter))
        ->addValue(docID, field->binaryValue());
    break;

  case org::apache::lucene::index::DocValuesType::SORTED:
    if (fp->docValuesWriter == nullptr) {
      fp->docValuesWriter =
          make_shared<SortedDocValuesWriter>(fp->fieldInfo, bytesUsed);
    }
    (std::static_pointer_cast<SortedDocValuesWriter>(fp->docValuesWriter))
        ->addValue(docID, field->binaryValue());
    break;

  case org::apache::lucene::index::DocValuesType::SORTED_NUMERIC:
    if (fp->docValuesWriter == nullptr) {
      fp->docValuesWriter =
          make_shared<SortedNumericDocValuesWriter>(fp->fieldInfo, bytesUsed);
    }
    (std::static_pointer_cast<SortedNumericDocValuesWriter>(
         fp->docValuesWriter))
        ->addValue(docID, field->numericValue()->longValue());
    break;

  case org::apache::lucene::index::DocValuesType::SORTED_SET:
    if (fp->docValuesWriter == nullptr) {
      fp->docValuesWriter =
          make_shared<SortedSetDocValuesWriter>(fp->fieldInfo, bytesUsed);
    }
    (std::static_pointer_cast<SortedSetDocValuesWriter>(fp->docValuesWriter))
        ->addValue(docID, field->binaryValue());
    break;

  default:
    throw make_shared<AssertionError>(L"unrecognized DocValues.Type: " +
                                      dvType);
  }
}

shared_ptr<PerField> DefaultIndexingChain::getPerField(const wstring &name)
{
  constexpr int hashPos = name.hashCode() & hashMask;
  shared_ptr<PerField> fp = fieldHash[hashPos];
  while (fp != nullptr && fp->fieldInfo->name != name) {
    fp = fp->next;
  }
  return fp;
}

shared_ptr<PerField> DefaultIndexingChain::getOrAddField(
    const wstring &name, shared_ptr<IndexableFieldType> fieldType, bool invert)
{

  // Make sure we have a PerField allocated
  constexpr int hashPos = name.hashCode() & hashMask;
  shared_ptr<PerField> fp = fieldHash[hashPos];
  while (fp != nullptr && fp->fieldInfo->name != name) {
    fp = fp->next;
  }

  if (fp == nullptr) {
    // First time we are seeing this field in this segment

    shared_ptr<FieldInfo> fi = fieldInfos->getOrAdd(name);
    // Messy: must set this here because e.g. FreqProxTermsWriterPerField looks
    // at the initial IndexOptions to decide what arrays it must create).  Then,
    // we also must set it in PerField.invert to allow for later downgrading of
    // the index options:
    fi->setIndexOptions(fieldType->indexOptions());

    fp = make_shared<PerField>(shared_from_this(),
                               docWriter->getIndexCreatedVersionMajor(), fi,
                               invert);
    fp->next = fieldHash[hashPos];
    fieldHash[hashPos] = fp;
    totalFieldCount++;

    // At most 50% load factor:
    if (totalFieldCount >= fieldHash.size() / 2) {
      rehash();
    }

    if (totalFieldCount > fields.size()) {
      std::deque<std::shared_ptr<PerField>> newFields(ArrayUtil::oversize(
          totalFieldCount, RamUsageEstimator::NUM_BYTES_OBJECT_REF));
      System::arraycopy(fields, 0, newFields, 0, fields.size());
      fields = newFields;
    }

  } else if (invert && fp->invertState == nullptr) {
    // Messy: must set this here because e.g. FreqProxTermsWriterPerField looks
    // at the initial IndexOptions to decide what arrays it must create).  Then,
    // we also must set it in PerField.invert to allow for later downgrading of
    // the index options:
    fp->fieldInfo->setIndexOptions(fieldType->indexOptions());
    fp->setInvertState();
  }

  return fp;
}

DefaultIndexingChain::PerField::PerField(
    shared_ptr<DefaultIndexingChain> outerInstance,
    int indexCreatedVersionMajor, shared_ptr<FieldInfo> fieldInfo, bool invert)
    : indexCreatedVersionMajor(indexCreatedVersionMajor), fieldInfo(fieldInfo),
      similarity(outerInstance->docState->similarity),
      outerInstance(outerInstance)
{
  if (invert) {
    setInvertState();
  }
}

void DefaultIndexingChain::PerField::setInvertState()
{
  invertState =
      make_shared<FieldInvertState>(indexCreatedVersionMajor, fieldInfo->name);
  termsHashPerField =
      outerInstance->termsHash->addField(invertState, fieldInfo);
  if (fieldInfo->omitsNorms() == false) {
    assert(norms == nullptr);
    // Even if no documents actually succeed in setting a norm, we still write
    // norms for this segment:
    norms = make_shared<NormValuesWriter>(
        fieldInfo, outerInstance->docState->docWriter->bytesUsed_);
  }
}

int DefaultIndexingChain::PerField::compareTo(shared_ptr<PerField> other)
{
  return this->fieldInfo->name.compare(other->fieldInfo->name);
}

void DefaultIndexingChain::PerField::finish() 
{
  if (fieldInfo->omitsNorms() == false) {
    int64_t normValue;
    if (invertState->length == 0) {
      // the field exists in this document, but it did not have
      // any indexed tokens, so we assign a default value of zero
      // to the norm
      normValue = 0;
    } else {
      normValue = similarity->computeNorm(invertState);
    }
    norms->addValue(outerInstance->docState->docID, normValue);
  }

  termsHashPerField->finish();
}

void DefaultIndexingChain::PerField::invert(shared_ptr<IndexableField> field,
                                            bool first) 
{
  if (first) {
    // First time we're seeing this field (indexed) in
    // this document:
    invertState->reset();
  }

  shared_ptr<IndexableFieldType> fieldType = field->fieldType();

  IndexOptions indexOptions = fieldType->indexOptions();
  fieldInfo->setIndexOptions(indexOptions);

  if (fieldType->omitNorms()) {
    fieldInfo->setOmitsNorms();
  }

  constexpr bool analyzed =
      fieldType->tokenized() && outerInstance->docState->analyzer != nullptr;

  /*
   * To assist people in tracking down problems in analysis components, we wish
   * to write the field name to the infostream when we fail. We expect some
   * caller to eventually deal with the real exception, so we don't want any
   * 'catch' clauses, but rather a finally that takes note of the problem.
   */
  bool succeededInProcessingField = false;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // stream = tokenStream = field.tokenStream(docState.analyzer, tokenStream))
  {
    org::apache::lucene::analysis::TokenStream stream = tokenStream =
        field->tokenStream(outerInstance->docState->analyzer, tokenStream);
    try {
      // reset the TokenStream to the first token
      stream->reset();
      invertState->setAttributeSource(stream);
      termsHashPerField->start(field, first);

      while (stream->incrementToken()) {

        // If we hit an exception in stream.next below
        // (which is fairly common, e.g. if analyzer
        // chokes on a given document), then it's
        // non-aborting and (above) this one document
        // will be marked as deleted, but still
        // consume a docID

        int posIncr = invertState->posIncrAttribute->getPositionIncrement();
        invertState->position += posIncr;
        if (invertState->position < invertState->lastPosition) {
          if (posIncr == 0) {
            throw invalid_argument(
                L"first position increment must be > 0 (got 0) for field '" +
                field->name() + L"'");
          } else if (posIncr < 0) {
            throw invalid_argument(L"position increment must be >= 0 (got " +
                                   to_wstring(posIncr) + L") for field '" +
                                   field->name() + L"'");
          } else {
            throw invalid_argument(
                L"position overflowed Integer.MAX_VALUE (got posIncr=" +
                to_wstring(posIncr) + L" lastPosition=" +
                to_wstring(invertState->lastPosition) + L" position=" +
                to_wstring(invertState->position) + L") for field '" +
                field->name() + L"'");
          }
        } else if (invertState->position > IndexWriter::MAX_POSITION) {
          throw invalid_argument(L"position " +
                                 to_wstring(invertState->position) +
                                 L" is too large for field '" + field->name() +
                                 L"': max allowed position is " +
                                 to_wstring(IndexWriter::MAX_POSITION));
        }
        invertState->lastPosition = invertState->position;
        if (posIncr == 0) {
          invertState->numOverlap++;
        }

        int startOffset =
            invertState->offset + invertState->offsetAttribute->startOffset();
        int endOffset =
            invertState->offset + invertState->offsetAttribute->endOffset();
        if (startOffset < invertState->lastStartOffset ||
            endOffset < startOffset) {
          throw invalid_argument(
              wstring(
                  L"startOffset must be non-negative, and endOffset must be >= "
                  L"startOffset, and offsets must not go backwards ") +
              L"startOffset=" + to_wstring(startOffset) + L",endOffset=" +
              to_wstring(endOffset) + L",lastStartOffset=" +
              to_wstring(invertState->lastStartOffset) + L" for field '" +
              field->name() + L"'");
        }
        invertState->lastStartOffset = startOffset;

        try {
          invertState->length = Math::addExact(
              invertState->length,
              invertState->termFreqAttribute->getTermFrequency());
        } catch (const ArithmeticException &ae) {
          throw invalid_argument(L"too many tokens for field \"" +
                                 field->name() + L"\"");
        }

        // System.out.println("  term=" + invertState.termAttribute);

        // If we hit an exception in here, we abort
        // all buffered documents since the last
        // flush, on the likelihood that the
        // internal state of the terms hash is now
        // corrupt and should not be flushed to a
        // new segment:
        try {
          termsHashPerField->add();
        } catch (const MaxBytesLengthExceededException &e) {
          std::deque<char> prefix(30);
          shared_ptr<BytesRef> bigTerm =
              invertState->termAttribute->getBytesRef();
          System::arraycopy(bigTerm->bytes, bigTerm->offset, prefix, 0, 30);
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wstring msg =
              L"Document contains at least one immense term in field=\"" +
              fieldInfo->name +
              L"\" (whose UTF8 encoding is longer than the max length " +
              to_wstring(DocumentsWriterPerThread::MAX_TERM_LENGTH_UTF8) +
              L"), all of which were skipped.  Please correct the analyzer to "
              L"not produce such terms.  The prefix of the first immense term "
              L"is: '" +
              Arrays->toString(prefix) + L"...', original message: " +
              e->what();
          if (outerInstance->docState->infoStream->isEnabled(L"IW")) {
            outerInstance->docState->infoStream->message(L"IW",
                                                         L"ERROR: " + msg);
          }
          // Document will be deleted above:
          // C++ TODO: This exception's constructor requires only one argument:
          // ORIGINAL LINE: throw new IllegalArgumentException(msg, e);
          throw invalid_argument(msg);
        } catch (const runtime_error &th) {
          outerInstance->docWriter->onAbortingException(th);
          throw th;
        }
      }

      // trigger streams to perform end-of-stream operations
      stream->end();

      // TODO: maybe add some safety? then again, it's already checked
      // when we come back around to the field...
      invertState->position +=
          invertState->posIncrAttribute->getPositionIncrement();
      invertState->offset += invertState->offsetAttribute->endOffset();

      /* if there is an exception coming through, we won't set this to true
       * here:*/
      succeededInProcessingField = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!succeededInProcessingField &&
          outerInstance->docState->infoStream->isEnabled(L"DW")) {
        outerInstance->docState->infoStream->message(
            L"DW", L"An exception was thrown while processing field " +
                       fieldInfo->name);
      }
    }
  }

  if (analyzed) {
    invertState->position +=
        outerInstance->docState->analyzer->getPositionIncrementGap(
            fieldInfo->name);
    invertState->offset +=
        outerInstance->docState->analyzer->getOffsetGap(fieldInfo->name);
  }
}

shared_ptr<DocIdSetIterator>
DefaultIndexingChain::getHasDocValues(const wstring &field)
{
  shared_ptr<PerField> perField = getPerField(field);
  if (perField != nullptr) {
    if (perField->docValuesWriter != nullptr) {
      if (perField->fieldInfo->getDocValuesType() == DocValuesType::NONE) {
        return nullptr;
      }

      return perField->docValuesWriter->getDocIdSet();
    }
  }
  return nullptr;
}
} // namespace org::apache::lucene::index