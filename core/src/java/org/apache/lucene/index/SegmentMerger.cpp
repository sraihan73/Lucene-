using namespace std;

#include "SegmentMerger.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using InfoStream = org::apache::lucene::util::InfoStream;
using Version = org::apache::lucene::util::Version;

SegmentMerger::SegmentMerger(deque<std::shared_ptr<CodecReader>> &readers,
                             shared_ptr<SegmentInfo> segmentInfo,
                             shared_ptr<InfoStream> infoStream,
                             shared_ptr<Directory> dir,
                             shared_ptr<FieldInfos::FieldNumbers> fieldNumbers,
                             shared_ptr<IOContext> context) 
    : directory(dir), codec(segmentInfo->getCodec()), context(context),
      mergeState(make_shared<MergeState>(readers, segmentInfo, infoStream)),
      fieldInfosBuilder(make_shared<FieldInfos::Builder>(fieldNumbers))
{
  if (context->context != IOContext::Context::MERGE) {
    throw invalid_argument(L"IOContext.context should be MERGE; got: " +
                           context->context);
  }
  shared_ptr<Version> minVersion = Version::LATEST;
  for (auto reader : readers) {
    shared_ptr<Version> leafMinVersion = reader->getMetaData()->getMinVersion();
    if (leafMinVersion == nullptr) {
      minVersion.reset();
      break;
    }
    if (minVersion->onOrAfter(leafMinVersion)) {
      minVersion = leafMinVersion;
    }
  }
  assert(
      (segmentInfo->minVersion == nullptr,
       L"The min version should be set by SegmentMerger for merged segments"));
  segmentInfo->minVersion = minVersion;
  if (mergeState->infoStream->isEnabled(L"SM")) {
    if (segmentInfo->getIndexSort() != nullptr) {
      mergeState->infoStream->message(L"SM", L"index sort during merge: " +
                                                 segmentInfo->getIndexSort());
    }
  }
}

bool SegmentMerger::shouldMerge()
{
  return mergeState->segmentInfo->maxDoc() > 0;
}

shared_ptr<MergeState> SegmentMerger::merge() 
{
  if (!shouldMerge()) {
    throw make_shared<IllegalStateException>(
        L"Merge would result in 0 document segment");
  }
  mergeFieldInfos();
  int64_t t0 = 0;
  if (mergeState->infoStream->isEnabled(L"SM")) {
    t0 = System::nanoTime();
  }
  int numMerged = mergeFields();
  if (mergeState->infoStream->isEnabled(L"SM")) {
    int64_t t1 = System::nanoTime();
    mergeState->infoStream->message(L"SM",
                                    to_wstring((t1 - t0) / 1000000) +
                                        L" msec to merge stored fields [" +
                                        to_wstring(numMerged) + L" docs]");
  }
  assert((numMerged == mergeState->segmentInfo->maxDoc(),
          L"numMerged=" + to_wstring(numMerged) +
              L" vs mergeState.segmentInfo.maxDoc()=" +
              to_wstring(mergeState->segmentInfo->maxDoc())));

  shared_ptr<SegmentWriteState> *const segmentWriteState =
      make_shared<SegmentWriteState>(
          mergeState->infoStream, directory, mergeState->segmentInfo,
          mergeState->mergeFieldInfos, nullptr, context);
  if (mergeState->infoStream->isEnabled(L"SM")) {
    t0 = System::nanoTime();
  }
  mergeTerms(segmentWriteState);
  if (mergeState->infoStream->isEnabled(L"SM")) {
    int64_t t1 = System::nanoTime();
    mergeState->infoStream->message(
        L"SM", to_wstring((t1 - t0) / 1000000) + L" msec to merge postings [" +
                   to_wstring(numMerged) + L" docs]");
  }

  if (mergeState->infoStream->isEnabled(L"SM")) {
    t0 = System::nanoTime();
  }
  if (mergeState->mergeFieldInfos->hasDocValues()) {
    mergeDocValues(segmentWriteState);
  }
  if (mergeState->infoStream->isEnabled(L"SM")) {
    int64_t t1 = System::nanoTime();
    mergeState->infoStream->message(L"SM", to_wstring((t1 - t0) / 1000000) +
                                               L" msec to merge doc values [" +
                                               to_wstring(numMerged) +
                                               L" docs]");
  }

  if (mergeState->infoStream->isEnabled(L"SM")) {
    t0 = System::nanoTime();
  }
  if (mergeState->mergeFieldInfos->hasPointValues()) {
    mergePoints(segmentWriteState);
  }
  if (mergeState->infoStream->isEnabled(L"SM")) {
    int64_t t1 = System::nanoTime();
    mergeState->infoStream->message(
        L"SM", to_wstring((t1 - t0) / 1000000) + L" msec to merge points [" +
                   to_wstring(numMerged) + L" docs]");
  }

  if (mergeState->mergeFieldInfos->hasNorms()) {
    if (mergeState->infoStream->isEnabled(L"SM")) {
      t0 = System::nanoTime();
    }
    mergeNorms(segmentWriteState);
    if (mergeState->infoStream->isEnabled(L"SM")) {
      int64_t t1 = System::nanoTime();
      mergeState->infoStream->message(
          L"SM", to_wstring((t1 - t0) / 1000000) + L" msec to merge norms [" +
                     to_wstring(numMerged) + L" docs]");
    }
  }

  if (mergeState->mergeFieldInfos->hasVectors()) {
    if (mergeState->infoStream->isEnabled(L"SM")) {
      t0 = System::nanoTime();
    }
    numMerged = mergeVectors();
    if (mergeState->infoStream->isEnabled(L"SM")) {
      int64_t t1 = System::nanoTime();
      mergeState->infoStream->message(
          L"SM", to_wstring((t1 - t0) / 1000000) + L" msec to merge vectors [" +
                     to_wstring(numMerged) + L" docs]");
    }
    assert(numMerged == mergeState->segmentInfo->maxDoc());
  }

  // write the merged infos
  if (mergeState->infoStream->isEnabled(L"SM")) {
    t0 = System::nanoTime();
  }
  codec->fieldInfosFormat()->write(directory, mergeState->segmentInfo, L"",
                                   mergeState->mergeFieldInfos, context);
  if (mergeState->infoStream->isEnabled(L"SM")) {
    int64_t t1 = System::nanoTime();
    mergeState->infoStream->message(L"SM", to_wstring((t1 - t0) / 1000000) +
                                               L" msec to write field infos [" +
                                               to_wstring(numMerged) +
                                               L" docs]");
  }

  return mergeState;
}

void SegmentMerger::mergeDocValues(
    shared_ptr<SegmentWriteState> segmentWriteState) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.DocValuesConsumer
  // consumer = codec.docValuesFormat().fieldsConsumer(segmentWriteState))
  {
    org::apache::lucene::codecs::DocValuesConsumer consumer =
        codec->docValuesFormat()->fieldsConsumer(segmentWriteState);
    consumer->merge(mergeState);
  }
}

void SegmentMerger::mergePoints(
    shared_ptr<SegmentWriteState> segmentWriteState) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.PointsWriter writer
  // = codec.pointsFormat().fieldsWriter(segmentWriteState))
  {
    org::apache::lucene::codecs::PointsWriter writer =
        codec->pointsFormat()->fieldsWriter(segmentWriteState);
    writer->merge(mergeState);
  }
}

void SegmentMerger::mergeNorms(
    shared_ptr<SegmentWriteState> segmentWriteState) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.NormsConsumer
  // consumer = codec.normsFormat().normsConsumer(segmentWriteState))
  {
    org::apache::lucene::codecs::NormsConsumer consumer =
        codec->normsFormat()->normsConsumer(segmentWriteState);
    consumer->merge(mergeState);
  }
}

void SegmentMerger::mergeFieldInfos() 
{
  for (auto readerFieldInfos : mergeState->fieldInfos) {
    for (auto fi : readerFieldInfos) {
      fieldInfosBuilder->add(fi);
    }
  }
  mergeState->mergeFieldInfos = fieldInfosBuilder->finish();
}

int SegmentMerger::mergeFields() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.StoredFieldsWriter
  // fieldsWriter = codec.storedFieldsFormat().fieldsWriter(directory,
  // mergeState.segmentInfo, context))
  {
    org::apache::lucene::codecs::StoredFieldsWriter fieldsWriter =
        codec->storedFieldsFormat()->fieldsWriter(
            directory, mergeState->segmentInfo, context);
    return fieldsWriter->merge(mergeState);
  }
}

int SegmentMerger::mergeVectors() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.TermVectorsWriter
  // termVectorsWriter = codec.termVectorsFormat().vectorsWriter(directory,
  // mergeState.segmentInfo, context))
  {
    org::apache::lucene::codecs::TermVectorsWriter termVectorsWriter =
        codec->termVectorsFormat()->vectorsWriter(
            directory, mergeState->segmentInfo, context);
    return termVectorsWriter->merge(mergeState);
  }
}

void SegmentMerger::mergeTerms(
    shared_ptr<SegmentWriteState> segmentWriteState) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.FieldsConsumer
  // consumer = codec.postingsFormat().fieldsConsumer(segmentWriteState))
  {
    org::apache::lucene::codecs::FieldsConsumer consumer =
        codec->postingsFormat()->fieldsConsumer(segmentWriteState);
    consumer->merge(mergeState);
  }
}
} // namespace org::apache::lucene::index