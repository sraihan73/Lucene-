using namespace std;

#include "SimpleTextStoredFieldsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/StoredFieldsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/StoredFieldsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "SimpleTextStoredFieldsReader.h"
#include "SimpleTextStoredFieldsWriter.h"

namespace org::apache::lucene::codecs::simpletext
{
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

shared_ptr<StoredFieldsReader> SimpleTextStoredFieldsFormat::fieldsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<FieldInfos> fn, shared_ptr<IOContext> context) 
{
  ;
  return make_shared<SimpleTextStoredFieldsReader>(directory, si, fn, context);
}

shared_ptr<StoredFieldsWriter> SimpleTextStoredFieldsFormat::fieldsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  return make_shared<SimpleTextStoredFieldsWriter>(directory, si->name,
                                                   context);
}
} // namespace org::apache::lucene::codecs::simpletext