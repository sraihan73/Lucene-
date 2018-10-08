using namespace std;

#include "FilterCodecReader.h"

namespace org::apache::lucene::index
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;

FilterCodecReader::FilterCodecReader(shared_ptr<CodecReader> in_)
    : in_(Objects::requireNonNull(in_))
{
}

shared_ptr<StoredFieldsReader> FilterCodecReader::getFieldsReader()
{
  return in_->getFieldsReader();
}

shared_ptr<TermVectorsReader> FilterCodecReader::getTermVectorsReader()
{
  return in_->getTermVectorsReader();
}

shared_ptr<NormsProducer> FilterCodecReader::getNormsReader()
{
  return in_->getNormsReader();
}

shared_ptr<DocValuesProducer> FilterCodecReader::getDocValuesReader()
{
  return in_->getDocValuesReader();
}

shared_ptr<FieldsProducer> FilterCodecReader::getPostingsReader()
{
  return in_->getPostingsReader();
}

shared_ptr<Bits> FilterCodecReader::getLiveDocs() { return in_->getLiveDocs(); }

shared_ptr<FieldInfos> FilterCodecReader::getFieldInfos()
{
  return in_->getFieldInfos();
}

shared_ptr<PointsReader> FilterCodecReader::getPointsReader()
{
  return in_->getPointsReader();
}

int FilterCodecReader::numDocs() { return in_->numDocs(); }

int FilterCodecReader::maxDoc() { return in_->maxDoc(); }

shared_ptr<LeafMetaData> FilterCodecReader::getMetaData()
{
  return in_->getMetaData();
}

void FilterCodecReader::doClose()  { in_->doClose(); }

int64_t FilterCodecReader::ramBytesUsed() { return in_->ramBytesUsed(); }

shared_ptr<deque<std::shared_ptr<Accountable>>>
FilterCodecReader::getChildResources()
{
  return in_->getChildResources();
}

void FilterCodecReader::checkIntegrity() 
{
  in_->checkIntegrity();
}
} // namespace org::apache::lucene::index