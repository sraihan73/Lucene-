using namespace std;

#include "TermVectorLeafReader.h"

namespace org::apache::lucene::search::highlight
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using LeafMetaData = org::apache::lucene::index::LeafMetaData;
using LeafReader = org::apache::lucene::index::LeafReader;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PointValues = org::apache::lucene::index::PointValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Terms = org::apache::lucene::index::Terms;
using Bits = org::apache::lucene::util::Bits;
using Version = org::apache::lucene::util::Version;

TermVectorLeafReader::TermVectorLeafReader(const wstring &field,
                                           shared_ptr<Terms> terms)
    : fields(make_shared<Fields>()),
      fieldInfos(make_shared<FieldInfos>(
          std::deque<std::shared_ptr<FieldInfo>>{fieldInfo}))
{
  {Iterator<wstring> iterator(){
      return Collections::singletonList(field).begin();
}

Terms this->terms(wstring fld) throws IOException
{
  if (field != fld) {
    return nullptr;
  }
  return terms;
}

int size() { return 1; }
}; // namespace org::apache::lucene::search::highlight

IndexOptions indexOptions;
if (!terms->hasFreqs()) {
  indexOptions = IndexOptions::DOCS;
} else if (!terms->hasPositions()) {
  indexOptions = IndexOptions::DOCS_AND_FREQS;
} else if (!terms->hasOffsets()) {
  indexOptions = IndexOptions::DOCS_AND_FREQS_AND_POSITIONS;
} else {
  indexOptions = IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS;
}
shared_ptr<FieldInfo> fieldInfo = make_shared<FieldInfo>(
    field, 0, true, true, terms->hasPayloads(), indexOptions,
    DocValuesType::NONE, -1, Collections::emptyMap(), 0, 0, false);
}

void TermVectorLeafReader::doClose()  {}

shared_ptr<Terms>
TermVectorLeafReader::terms(const wstring &field) 
{
  return fields->terms(field);
}

shared_ptr<NumericDocValues> TermVectorLeafReader::getNumericDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<BinaryDocValues> TermVectorLeafReader::getBinaryDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<SortedDocValues> TermVectorLeafReader::getSortedDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<SortedNumericDocValues>
TermVectorLeafReader::getSortedNumericDocValues(const wstring &field) throw(
    IOException)
{
  return nullptr;
}

shared_ptr<SortedSetDocValues> TermVectorLeafReader::getSortedSetDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<NumericDocValues>
TermVectorLeafReader::getNormValues(const wstring &field) 
{
  return nullptr; // Is this needed?  See MemoryIndex for a way to do it.
}

shared_ptr<FieldInfos> TermVectorLeafReader::getFieldInfos()
{
  return fieldInfos;
}

shared_ptr<Bits> TermVectorLeafReader::getLiveDocs() { return nullptr; }

shared_ptr<PointValues>
TermVectorLeafReader::getPointValues(const wstring &fieldName)
{
  return nullptr;
}

void TermVectorLeafReader::checkIntegrity()  {}

shared_ptr<Fields>
TermVectorLeafReader::getTermVectors(int docID) 
{
  if (docID != 0) {
    return nullptr;
  }
  return fields;
}

int TermVectorLeafReader::numDocs() { return 1; }

int TermVectorLeafReader::maxDoc() { return 1; }

void TermVectorLeafReader::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
}

shared_ptr<LeafMetaData> TermVectorLeafReader::getMetaData()
{
  return make_shared<LeafMetaData>(Version::LATEST->major, nullptr, nullptr);
}

shared_ptr<CacheHelper> TermVectorLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> TermVectorLeafReader::getReaderCacheHelper()
{
  return nullptr;
}
}