using namespace std;

#include "Lucene50StoredFieldsFormat.h"
#include "../../index/FieldInfos.h"
#include "../../index/SegmentInfo.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../StoredFieldsReader.h"
#include "../StoredFieldsWriter.h"
#include "../compressing/CompressingStoredFieldsFormat.h"
#include "../compressing/CompressionMode.h"

namespace org::apache::lucene::codecs::lucene50
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using CompressingStoredFieldsFormat =
    org::apache::lucene::codecs::compressing::CompressingStoredFieldsFormat;
using CompressingStoredFieldsIndexWriter = org::apache::lucene::codecs::
    compressing::CompressingStoredFieldsIndexWriter;
using CompressionMode =
    org::apache::lucene::codecs::compressing::CompressionMode;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
const wstring Lucene50StoredFieldsFormat::MODE_KEY =
    Lucene50StoredFieldsFormat::typeid->getSimpleName() + L".mode";

Lucene50StoredFieldsFormat::Lucene50StoredFieldsFormat()
    : Lucene50StoredFieldsFormat(Mode::BEST_SPEED)
{
}

Lucene50StoredFieldsFormat::Lucene50StoredFieldsFormat(Mode mode)
    : mode(Objects::requireNonNull(mode))
{
}

shared_ptr<StoredFieldsReader> Lucene50StoredFieldsFormat::fieldsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<FieldInfos> fn, shared_ptr<IOContext> context) 
{
  wstring value = si->getAttribute(MODE_KEY);
  if (value == L"") {
    throw make_shared<IllegalStateException>(L"missing value for " + MODE_KEY +
                                             L" for segment: " + si->name);
  }
  Mode mode = Mode::valueOf(value);
  return impl(mode)->fieldsReader(directory, si, fn, context);
}

shared_ptr<StoredFieldsWriter> Lucene50StoredFieldsFormat::fieldsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  wstring previous = si->putAttribute(MODE_KEY, mode.name());
  if (previous != L"" && previous == mode.name() == false) {
    throw make_shared<IllegalStateException>(
        L"found existing value for " + MODE_KEY + L" for segment: " + si->name +
        L"old=" + previous + L", new=" + mode.name());
  }
  return impl(mode)->fieldsWriter(directory, si, context);
}

shared_ptr<StoredFieldsFormat> Lucene50StoredFieldsFormat::impl(Mode mode)
{
  switch (mode) {
  case org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat::Mode::
      BEST_SPEED:
    return make_shared<CompressingStoredFieldsFormat>(
        L"Lucene50StoredFieldsFast", CompressionMode::FAST, 1 << 14, 128, 1024);
  case org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat::Mode::
      BEST_COMPRESSION:
    return make_shared<CompressingStoredFieldsFormat>(
        L"Lucene50StoredFieldsHigh", CompressionMode::HIGH_COMPRESSION, 61440,
        512, 1024);
  default:
    throw make_shared<AssertionError>();
  }
}
} // namespace org::apache::lucene::codecs::lucene50