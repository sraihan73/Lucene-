using namespace std;

#include "Lucene50TermVectorsFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using CompressingStoredFieldsIndexWriter = org::apache::lucene::codecs::
    compressing::CompressingStoredFieldsIndexWriter;
using CompressingTermVectorsFormat =
    org::apache::lucene::codecs::compressing::CompressingTermVectorsFormat;
using CompressionMode =
    org::apache::lucene::codecs::compressing::CompressionMode;
using DataOutput = org::apache::lucene::store::DataOutput;
using BlockPackedWriter = org::apache::lucene::util::packed::BlockPackedWriter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

Lucene50TermVectorsFormat::Lucene50TermVectorsFormat()
    : org::apache::lucene::codecs::compressing::CompressingTermVectorsFormat(
          L"Lucene50TermVectors", L"", CompressionMode::FAST, 1 << 12, 1024)
{
}
} // namespace org::apache::lucene::codecs::lucene50