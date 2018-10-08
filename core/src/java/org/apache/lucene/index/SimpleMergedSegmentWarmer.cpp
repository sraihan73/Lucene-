using namespace std;

#include "SimpleMergedSegmentWarmer.h"

namespace org::apache::lucene::index
{
using IndexReaderWarmer =
    org::apache::lucene::index::IndexWriter::IndexReaderWarmer;
using InfoStream = org::apache::lucene::util::InfoStream;

SimpleMergedSegmentWarmer::SimpleMergedSegmentWarmer(
    shared_ptr<InfoStream> infoStream)
    : infoStream(infoStream)
{
}

void SimpleMergedSegmentWarmer::warm(shared_ptr<LeafReader> reader) throw(
    IOException)
{
  int64_t startTime = System::currentTimeMillis();
  int indexedCount = 0;
  int docValuesCount = 0;
  int normsCount = 0;
  for (auto info : reader->getFieldInfos()) {
    if (info->getIndexOptions() != IndexOptions::NONE) {
      reader->terms(info->name);
      indexedCount++;

      if (info->hasNorms()) {
        reader->getNormValues(info->name);
        normsCount++;
      }
    }

    if (info->getDocValuesType() != DocValuesType::NONE) {
      switch (info->getDocValuesType()) {
      case NUMERIC:
        reader->getNumericDocValues(info->name);
        break;
      case BINARY:
        reader->getBinaryDocValues(info->name);
        break;
      case SORTED:
        reader->getSortedDocValues(info->name);
        break;
      case SORTED_NUMERIC:
        reader->getSortedNumericDocValues(info->name);
        break;
      case SORTED_SET:
        reader->getSortedSetDocValues(info->name);
        break;
      default:
        assert(false); // unknown dv type
      }
      docValuesCount++;
    }
  }

  reader->document(0);
  reader->getTermVectors(0);

  if (infoStream->isEnabled(L"SMSW")) {
    infoStream->message(L"SMSW", L"Finished warming segment: " + reader +
                                     L", indexed=" + to_wstring(indexedCount) +
                                     L", docValues=" +
                                     to_wstring(docValuesCount) + L", norms=" +
                                     to_wstring(normsCount) + L", time=" +
                                     (System::currentTimeMillis() - startTime));
  }
}
} // namespace org::apache::lucene::index