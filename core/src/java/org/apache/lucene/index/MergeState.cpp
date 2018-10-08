using namespace std;

#include "MergeState.h"

namespace org::apache::lucene::index
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Sort = org::apache::lucene::search::Sort;
using Bits = org::apache::lucene::util::Bits;
using InfoStream = org::apache::lucene::util::InfoStream;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

MergeState::MergeState(deque<std::shared_ptr<CodecReader>> &originalReaders,
                       shared_ptr<SegmentInfo> segmentInfo,
                       shared_ptr<InfoStream> infoStream) 
    : docMaps(buildDocMaps(readers, indexSort)),
      leafDocMaps(std::deque<std::shared_ptr<DocMap>>(numReaders)),
      segmentInfo(segmentInfo),
      storedFieldsReaders(
          std::deque<std::shared_ptr<StoredFieldsReader>>(numReaders)),
      termVectorsReaders(
          std::deque<std::shared_ptr<TermVectorsReader>>(numReaders)),
      normsProducers(std::deque<std::shared_ptr<NormsProducer>>(numReaders)),
      docValuesProducers(
          std::deque<std::shared_ptr<DocValuesProducer>>(numReaders)),
      fieldInfos(std::deque<std::shared_ptr<FieldInfos>>(numReaders)),
      liveDocs(std::deque<std::shared_ptr<Bits>>(numReaders)),
      fieldsProducers(std::deque<std::shared_ptr<FieldsProducer>>(numReaders)),
      pointsReaders(std::deque<std::shared_ptr<PointsReader>>(numReaders)),
      maxDocs(std::deque<int>(numReaders)), infoStream(infoStream)
{

  shared_ptr<Sort> *const indexSort = segmentInfo->getIndexSort();
  int numReaders = originalReaders.size();
  deque<std::shared_ptr<CodecReader>> readers =
      maybeSortReaders(originalReaders, segmentInfo);

  int numDocs = 0;
  for (int i = 0; i < numReaders; i++) {
    shared_ptr<CodecReader> *const reader = readers[i];

    maxDocs[i] = reader->maxDoc();
    liveDocs[i] = reader->getLiveDocs();
    fieldInfos[i] = reader->getFieldInfos();

    normsProducers[i] = reader->getNormsReader();
    if (normsProducers[i] != nullptr) {
      normsProducers[i] = normsProducers[i]->getMergeInstance();
    }

    docValuesProducers[i] = reader->getDocValuesReader();
    if (docValuesProducers[i] != nullptr) {
      docValuesProducers[i] = docValuesProducers[i]->getMergeInstance();
    }

    storedFieldsReaders[i] = reader->getFieldsReader();
    if (storedFieldsReaders[i] != nullptr) {
      storedFieldsReaders[i] = storedFieldsReaders[i]->getMergeInstance();
    }

    termVectorsReaders[i] = reader->getTermVectorsReader();
    if (termVectorsReaders[i] != nullptr) {
      termVectorsReaders[i] = termVectorsReaders[i]->getMergeInstance();
    }

    fieldsProducers[i] = reader->getPostingsReader()->getMergeInstance();
    pointsReaders[i] = reader->getPointsReader();
    if (pointsReaders[i] != nullptr) {
      pointsReaders[i] = pointsReaders[i]->getMergeInstance();
    }
    numDocs += reader->numDocs();
  }

  segmentInfo->setMaxDoc(numDocs);
}

std::deque<std::shared_ptr<DocMap>>
MergeState::buildDeletionDocMaps(deque<std::shared_ptr<CodecReader>> &readers)
{

  int totalDocs = 0;
  int numReaders = readers.size();
  std::deque<std::shared_ptr<DocMap>> docMaps(numReaders);

  for (int i = 0; i < numReaders; i++) {
    shared_ptr<LeafReader> reader = readers[i];
    shared_ptr<Bits> liveDocs = reader->getLiveDocs();

    shared_ptr<PackedLongValues> *const delDocMap;
    if (liveDocs != nullptr) {
      delDocMap = removeDeletes(reader->maxDoc(), liveDocs);
    } else {
      delDocMap.reset();
    }

    constexpr int docBase = totalDocs;
    docMaps[i] = make_shared<DocMapAnonymousInnerClass>(
        shared_from_this(), liveDocs, delDocMap, docBase);
    totalDocs += reader->numDocs();
  }

  return docMaps;
}

MergeState::DocMapAnonymousInnerClass::DocMapAnonymousInnerClass(
    shared_ptr<MergeState> outerInstance, shared_ptr<Bits> liveDocs,
    shared_ptr<PackedLongValues> delDocMap, int docBase)
{
  this->outerInstance = outerInstance;
  this->liveDocs = liveDocs;
  this->delDocMap = delDocMap;
  this->docBase = docBase;
}

int MergeState::DocMapAnonymousInnerClass::get(int docID)
{
  if (liveDocs == nullptr) {
    return docBase + docID;
  } else if (liveDocs->get(docID)) {
    return docBase + static_cast<int>(delDocMap->get(docID));
  } else {
    return -1;
  }
}

std::deque<std::shared_ptr<DocMap>>
MergeState::buildDocMaps(deque<std::shared_ptr<CodecReader>> &readers,
                         shared_ptr<Sort> indexSort) 
{

  if (indexSort == nullptr) {
    // no index sort ... we only must map_obj around deletions, and rebase to the
    // merged segment's docID space
    return buildDeletionDocMaps(readers);
  } else {
    // do a merge sort of the incoming leaves:
    int64_t t0 = System::nanoTime();
    std::deque<std::shared_ptr<DocMap>> result =
        MultiSorter::sort(indexSort, readers);
    if (result.empty()) {
      // already sorted so we can switch back to map_obj around deletions
      return buildDeletionDocMaps(readers);
    } else {
      needsIndexSort = true;
    }
    int64_t t1 = System::nanoTime();
    if (infoStream->isEnabled(L"SM")) {
      infoStream->message(
          L"SM", wstring::format(Locale::ROOT,
                                 L"%.2f msec to build merge sorted DocMaps",
                                 (t1 - t0) / 1000000.0));
    }
    return result;
  }
}

deque<std::shared_ptr<CodecReader>> MergeState::maybeSortReaders(
    deque<std::shared_ptr<CodecReader>> &originalReaders,
    shared_ptr<SegmentInfo> segmentInfo) 
{

  // Default to identity:
  for (int i = 0; i < originalReaders.size(); i++) {
    leafDocMaps[i] =
        make_shared<DocMapAnonymousInnerClass2>(shared_from_this());
  }

  shared_ptr<Sort> indexSort = segmentInfo->getIndexSort();
  if (indexSort == nullptr) {
    return originalReaders;
  }

  /** If an incoming reader is not sorted, because it was flushed by IW older
   * than {@link Version.LUCENE_7_0_0} or because we add unsorted segments from
   * another index {@link IndexWriter#addIndexes(CodecReader...)} , we sort it
   * here:
   */
  shared_ptr<Sorter> *const sorter = make_shared<Sorter>(indexSort);
  deque<std::shared_ptr<CodecReader>> readers =
      deque<std::shared_ptr<CodecReader>>(originalReaders.size());

  for (auto leaf : originalReaders) {
    shared_ptr<Sort> segmentSort = leaf->getMetaData()->getSort();

    if (segmentSort == nullptr) {
      // This segment was written by flush, so documents are not yet sorted, so
      // we sort them now:
      int64_t t0 = System::nanoTime();
      shared_ptr<Sorter::DocMap> sortDocMap = sorter->sort(leaf);
      int64_t t1 = System::nanoTime();
      double msec = (t1 - t0) / 1000000.0;

      if (sortDocMap != nullptr) {
        if (infoStream->isEnabled(L"SM")) {
          infoStream->message(
              L"SM", wstring::format(Locale::ROOT,
                                     L"segment %s is not sorted; wrapping for "
                                     L"sort %s now (%.2f msec to sort)",
                                     leaf, indexSort, msec));
        }
        needsIndexSort = true;
        leaf = SlowCodecReaderWrapper::wrap(SortingLeafReader::wrap(
            make_shared<MergeReaderWrapper>(leaf), sortDocMap));
        leafDocMaps[readers.size()] = make_shared<DocMapAnonymousInnerClass3>(
            shared_from_this(), sortDocMap);
      } else {
        if (infoStream->isEnabled(L"SM")) {
          infoStream->message(
              L"SM", wstring::format(
                         Locale::ROOT,
                         L"segment %s is not sorted, but is already "
                         L"accidentally in sort %s order (%.2f msec to sort)",
                         leaf, indexSort, msec));
        }
      }

    } else {
      if (segmentSort->equals(indexSort) == false) {
        throw invalid_argument(
            L"index sort mismatch: merged segment has sort=" + indexSort +
            L" but to-be-merged segment has sort=" + segmentSort);
      }
      if (infoStream->isEnabled(L"SM")) {
        infoStream->message(L"SM", L"segment " + leaf + L" already sorted");
      }
    }

    readers.push_back(leaf);
  }

  return readers;
}

MergeState::DocMapAnonymousInnerClass2::DocMapAnonymousInnerClass2(
    shared_ptr<MergeState> outerInstance)
{
  this->outerInstance = outerInstance;
}

int MergeState::DocMapAnonymousInnerClass2::get(int docID) { return docID; }

MergeState::DocMapAnonymousInnerClass3::DocMapAnonymousInnerClass3(
    shared_ptr<MergeState> outerInstance,
    shared_ptr<org::apache::lucene::index::Sorter::DocMap> sortDocMap)
{
  this->outerInstance = outerInstance;
  this->sortDocMap = sortDocMap;
}

int MergeState::DocMapAnonymousInnerClass3::get(int docID)
{
  return sortDocMap->oldToNew(docID);
}

MergeState::DocMap::DocMap() {}

shared_ptr<PackedLongValues>
MergeState::removeDeletes(int const maxDoc, shared_ptr<Bits> liveDocs)
{
  shared_ptr<PackedLongValues::Builder> *const docMapBuilder =
      PackedLongValues::monotonicBuilder(PackedInts::COMPACT);
  int del = 0;
  for (int i = 0; i < maxDoc; ++i) {
    docMapBuilder->add(i - del);
    if (liveDocs->get(i) == false) {
      ++del;
    }
  }
  return docMapBuilder->build();
}
} // namespace org::apache::lucene::index