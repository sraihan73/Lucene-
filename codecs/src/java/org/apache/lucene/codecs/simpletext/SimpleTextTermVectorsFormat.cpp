using namespace std;

#include "SimpleTextTermVectorsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermVectorsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermVectorsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "SimpleTextTermVectorsReader.h"
#include "SimpleTextTermVectorsWriter.h"

namespace org::apache::lucene::codecs::simpletext
{
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

shared_ptr<TermVectorsReader> SimpleTextTermVectorsFormat::vectorsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<FieldInfos> fieldInfos,
    shared_ptr<IOContext> context) 
{
  return make_shared<SimpleTextTermVectorsReader>(directory, segmentInfo,
                                                  context);
}

shared_ptr<TermVectorsWriter> SimpleTextTermVectorsFormat::vectorsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    shared_ptr<IOContext> context) 
{
  return make_shared<SimpleTextTermVectorsWriter>(directory, segmentInfo->name,
                                                  context);
}
} // namespace org::apache::lucene::codecs::simpletext