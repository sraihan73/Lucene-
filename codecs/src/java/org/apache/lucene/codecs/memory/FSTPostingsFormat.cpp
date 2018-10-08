using namespace std;

#include "FSTPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsReaderBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/lucene50/Lucene50PostingsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/lucene50/Lucene50PostingsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "FSTTermsReader.h"
#include "FSTTermsWriter.h"

namespace org::apache::lucene::codecs::memory
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using Lucene50PostingsReader =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsReader;
using Lucene50PostingsWriter =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IOUtils = org::apache::lucene::util::IOUtils;

FSTPostingsFormat::FSTPostingsFormat()
    : org::apache::lucene::codecs::PostingsFormat(L"FST50")
{
}

wstring FSTPostingsFormat::toString() { return getName(); }

shared_ptr<FieldsConsumer> FSTPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<PostingsWriterBase> postingsWriter =
      make_shared<Lucene50PostingsWriter>(state);

  bool success = false;
  try {
    shared_ptr<FieldsConsumer> ret =
        make_shared<FSTTermsWriter>(state, postingsWriter);
    success = true;
    return ret;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({postingsWriter});
    }
  }
}

shared_ptr<FieldsProducer> FSTPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  shared_ptr<PostingsReaderBase> postingsReader =
      make_shared<Lucene50PostingsReader>(state);
  bool success = false;
  try {
    shared_ptr<FieldsProducer> ret =
        make_shared<FSTTermsReader>(state, postingsReader);
    success = true;
    return ret;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({postingsReader});
    }
  }
}
} // namespace org::apache::lucene::codecs::memory