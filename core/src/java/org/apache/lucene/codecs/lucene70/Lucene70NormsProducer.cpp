using namespace std;

#include "Lucene70NormsProducer.h"

namespace org::apache::lucene::codecs::lucene70
{
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70NormsFormat.VERSION_CURRENT;
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70NormsFormat.VERSION_START;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DocValues = org::apache::lucene::index::DocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using IOUtils = org::apache::lucene::util::IOUtils;

Lucene70NormsProducer::Lucene70NormsProducer(
    shared_ptr<SegmentReadState> state, const wstring &dataCodec,
    const wstring &dataExtension, const wstring &metaCodec,
    const wstring &metaExtension) 
    : data(state->directory->openInput(dataName, state->context)),
      maxDoc(state->segmentInfo->maxDoc())
{
  wstring metaName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, metaExtension);
  int version = -1;

  // read in the entries from the metadata file.
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // in = state.directory.openChecksumInput(metaName, state.context))
  {
    org::apache::lucene::store::ChecksumIndexInput in_ =
        state->directory->openChecksumInput(metaName, state->context);
    runtime_error priorE = nullptr;
    try {
      version = CodecUtil::checkIndexHeader(
          in_, metaCodec, VERSION_START, VERSION_CURRENT,
          state->segmentInfo->getId(), state->segmentSuffix);
      readFields(in_, state->fieldInfos);
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(in_, priorE);
    }
  }

  wstring dataName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, dataExtension);
  bool success = false;
  try {
    constexpr int version2 = CodecUtil::checkIndexHeader(
        data, dataCodec, VERSION_START, VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);
    if (version != version2) {
      throw make_shared<CorruptIndexException>(
          L"Format versions mismatch: meta=" + to_wstring(version) + L",data=" +
              to_wstring(version2),
          data);
    }

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(data);

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({this->data});
    }
  }
}

Lucene70NormsProducer::DenseNormsIterator::DenseNormsIterator(int maxDoc)
    : maxDoc(maxDoc)
{
}

int Lucene70NormsProducer::DenseNormsIterator::docID() { return doc; }

int Lucene70NormsProducer::DenseNormsIterator::nextDoc() 
{
  return advance(doc + 1);
}

int Lucene70NormsProducer::DenseNormsIterator::advance(int target) throw(
    IOException)
{
  if (target >= maxDoc) {
    return doc = NO_MORE_DOCS;
  }
  return doc = target;
}

bool Lucene70NormsProducer::DenseNormsIterator::advanceExact(int target) throw(
    IOException)
{
  this->doc = target;
  return true;
}

int64_t Lucene70NormsProducer::DenseNormsIterator::cost() { return maxDoc; }

Lucene70NormsProducer::SparseNormsIterator::SparseNormsIterator(
    shared_ptr<IndexedDISI> disi)
    : disi(disi)
{
}

int Lucene70NormsProducer::SparseNormsIterator::docID()
{
  return disi->docID();
}

int Lucene70NormsProducer::SparseNormsIterator::nextDoc() 
{
  return disi->nextDoc();
}

int Lucene70NormsProducer::SparseNormsIterator::advance(int target) throw(
    IOException)
{
  return disi->advance(target);
}

bool Lucene70NormsProducer::SparseNormsIterator::advanceExact(int target) throw(
    IOException)
{
  return disi->advanceExact(target);
}

int64_t Lucene70NormsProducer::SparseNormsIterator::cost()
{
  return disi->cost();
}

void Lucene70NormsProducer::readFields(
    shared_ptr<IndexInput> meta,
    shared_ptr<FieldInfos> infos) 
{
  for (int fieldNumber = meta->readInt(); fieldNumber != -1;
       fieldNumber = meta->readInt()) {
    shared_ptr<FieldInfo> info = infos->fieldInfo(fieldNumber);
    if (info == nullptr) {
      throw make_shared<CorruptIndexException>(
          L"Invalid field number: " + to_wstring(fieldNumber), meta);
    } else if (!info->hasNorms()) {
      throw make_shared<CorruptIndexException>(L"Invalid field: " + info->name,
                                               meta);
    }
    shared_ptr<NormsEntry> entry = make_shared<NormsEntry>();
    entry->docsWithFieldOffset = meta->readLong();
    entry->docsWithFieldLength = meta->readLong();
    entry->numDocsWithField = meta->readInt();
    entry->bytesPerNorm = meta->readByte();
    switch (entry->bytesPerNorm) {
    case 0:
    case 1:
    case 2:
    case 4:
    case 8:
      break;
    default:
      throw make_shared<CorruptIndexException>(
          L"Invalid bytesPerValue: " + to_wstring(entry->bytesPerNorm) +
              L", field: " + info->name,
          meta);
    }
    entry->normsOffset = meta->readLong();
    norms.emplace(info->number, entry);
  }
}

shared_ptr<NumericDocValues>
Lucene70NormsProducer::getNorms(shared_ptr<FieldInfo> field) 
{
  shared_ptr<NormsEntry> *const entry = norms[field->number];
  if (entry->docsWithFieldOffset == -2) {
    // empty
    return DocValues::emptyNumeric();
  } else if (entry->docsWithFieldOffset == -1) {
    // dense
    if (entry->bytesPerNorm == 0) {
      return make_shared<DenseNormsIteratorAnonymousInnerClass>(
          shared_from_this(), maxDoc, entry);
    }
    shared_ptr<RandomAccessInput> *const slice = data->randomAccessSlice(
        entry->normsOffset,
        entry->numDocsWithField * static_cast<int64_t>(entry->bytesPerNorm));
    switch (entry->bytesPerNorm) {
    case 1:
      return make_shared<DenseNormsIteratorAnonymousInnerClass2>(
          shared_from_this(), maxDoc, slice);
    case 2:
      return make_shared<DenseNormsIteratorAnonymousInnerClass3>(
          shared_from_this(), maxDoc, slice);
    case 4:
      return make_shared<DenseNormsIteratorAnonymousInnerClass4>(
          shared_from_this(), maxDoc, slice);
    case 8:
      return make_shared<DenseNormsIteratorAnonymousInnerClass5>(
          shared_from_this(), maxDoc, slice);
    default:
      // should not happen, we already validate bytesPerNorm in readFields
      throw make_shared<AssertionError>();
    }
  } else {
    // sparse
    shared_ptr<IndexedDISI> *const disi = make_shared<IndexedDISI>(
        data, entry->docsWithFieldOffset, entry->docsWithFieldLength,
        entry->numDocsWithField);
    if (entry->bytesPerNorm == 0) {
      return make_shared<SparseNormsIteratorAnonymousInnerClass>(
          shared_from_this(), entry);
    }
    shared_ptr<RandomAccessInput> *const slice = data->randomAccessSlice(
        entry->normsOffset,
        entry->numDocsWithField * static_cast<int64_t>(entry->bytesPerNorm));
    switch (entry->bytesPerNorm) {
    case 1:
      return make_shared<SparseNormsIteratorAnonymousInnerClass2>(
          shared_from_this(), disi, slice);
    case 2:
      return make_shared<SparseNormsIteratorAnonymousInnerClass3>(
          shared_from_this(), disi, slice);
    case 4:
      return make_shared<SparseNormsIteratorAnonymousInnerClass4>(
          shared_from_this(), disi, slice);
    case 8:
      return make_shared<SparseNormsIteratorAnonymousInnerClass5>(
          shared_from_this(), disi, slice);
    default:
      // should not happen, we already validate bytesPerNorm in readFields
      throw make_shared<AssertionError>();
    }
  }
}

Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass::
    DenseNormsIteratorAnonymousInnerClass(
        shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70NormsProducer::NormsEntry>
            entry)
    : DenseNormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
}

int64_t
Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass::longValue() throw(
    IOException)
{
  return entry->normsOffset;
}

Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass2::
    DenseNormsIteratorAnonymousInnerClass2(
        shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : DenseNormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass2::
    longValue() 
{
  return slice->readByte(doc);
}

Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass3::
    DenseNormsIteratorAnonymousInnerClass3(
        shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : DenseNormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass3::
    longValue() 
{
  return slice->readShort((static_cast<int64_t>(doc)) << 1);
}

Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass4::
    DenseNormsIteratorAnonymousInnerClass4(
        shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : DenseNormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass4::
    longValue() 
{
  return slice->readInt((static_cast<int64_t>(doc)) << 2);
}

Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass5::
    DenseNormsIteratorAnonymousInnerClass5(
        shared_ptr<Lucene70NormsProducer> outerInstance, int maxDoc,
        shared_ptr<RandomAccessInput> slice)
    : DenseNormsIterator(maxDoc)
{
  this->outerInstance = outerInstance;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::DenseNormsIteratorAnonymousInnerClass5::
    longValue() 
{
  return slice->readLong((static_cast<int64_t>(doc)) << 3);
}

Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass::
    SparseNormsIteratorAnonymousInnerClass(
        shared_ptr<Lucene70NormsProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::
                       Lucene70NormsProducer::NormsEntry>
            entry)
    : SparseNormsIterator(disi)
{
  this->outerInstance = outerInstance;
  this->entry = entry;
}

int64_t Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass::
    longValue() 
{
  return entry->normsOffset;
}

Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass2::
    SparseNormsIteratorAnonymousInnerClass2(
        shared_ptr<Lucene70NormsProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<RandomAccessInput> slice)
    : SparseNormsIterator(disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass2::
    longValue() 
{
  return slice->readByte(disi->index());
}

Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass3::
    SparseNormsIteratorAnonymousInnerClass3(
        shared_ptr<Lucene70NormsProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<RandomAccessInput> slice)
    : SparseNormsIterator(disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass3::
    longValue() 
{
  return slice->readShort((static_cast<int64_t>(disi->index())) << 1);
}

Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass4::
    SparseNormsIteratorAnonymousInnerClass4(
        shared_ptr<Lucene70NormsProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<RandomAccessInput> slice)
    : SparseNormsIterator(disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass4::
    longValue() 
{
  return slice->readInt((static_cast<int64_t>(disi->index())) << 2);
}

Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass5::
    SparseNormsIteratorAnonymousInnerClass5(
        shared_ptr<Lucene70NormsProducer> outerInstance,
        shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi,
        shared_ptr<RandomAccessInput> slice)
    : SparseNormsIterator(disi)
{
  this->outerInstance = outerInstance;
  this->disi = disi;
  this->slice = slice;
}

int64_t Lucene70NormsProducer::SparseNormsIteratorAnonymousInnerClass5::
    longValue() 
{
  return slice->readLong((static_cast<int64_t>(disi->index())) << 3);
}

Lucene70NormsProducer::~Lucene70NormsProducer() { delete data; }

int64_t Lucene70NormsProducer::ramBytesUsed()
{
  return 64LL * norms.size(); // good enough
}

void Lucene70NormsProducer::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(data);
}

wstring Lucene70NormsProducer::toString()
{
  return getClass().getSimpleName() + L"(fields=" + norms.size() + L")";
}
} // namespace org::apache::lucene::codecs::lucene70