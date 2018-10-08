using namespace std;

#include "ParallelLeafReader.h"

namespace org::apache::lucene::index
{
using Sort = org::apache::lucene::search::Sort;
using Bits = org::apache::lucene::util::Bits;
using Version = org::apache::lucene::util::Version;

ParallelLeafReader::ParallelLeafReader(deque<LeafReader> &readers) throw(
    IOException)
    : ParallelLeafReader(true, readers)
{
}

ParallelLeafReader::ParallelLeafReader(
    bool closeSubReaders, deque<LeafReader> &readers) 
    : ParallelLeafReader(closeSubReaders, readers, readers)
{
}

ParallelLeafReader::ParallelLeafReader(
    bool closeSubReaders, std::deque<std::shared_ptr<LeafReader>> &readers,
    std::deque<std::shared_ptr<LeafReader>>
        &storedFieldsReaders) 
    : fieldInfos(builder::finish()), parallelReaders(readers.clone()),
      storedFieldsReaders(storedFieldsReaders.clone()),
      closeSubReaders(closeSubReaders),
      metaData(
          make_shared<LeafMetaData>(createdVersionMajor, minVersion, indexSort))
{
  if (readers.empty() && storedFieldsReaders.size() > 0) {
    throw invalid_argument(L"There must be at least one main reader if "
                           L"storedFieldsReaders are used.");
  }
  if (parallelReaders.size() > 0) {
    shared_ptr<LeafReader> *const first = parallelReaders[0];
    this->maxDoc_ = first->maxDoc();
    this->numDocs_ = first->numDocs();
    this->hasDeletions = first->hasDeletions();
  } else {
    this->maxDoc_ = this->numDocs_ = 0;
    this->hasDeletions = false;
  }
  Collections::addAll(completeReaderSet, this->parallelReaders);
  Collections::addAll(completeReaderSet, this->storedFieldsReaders);

  // check compatibility:
  for (auto reader : completeReaderSet) {
    if (reader->maxDoc() != maxDoc_) {
      throw invalid_argument(L"All readers must have same maxDoc: " +
                             to_wstring(maxDoc_) + L"!=" +
                             to_wstring(reader->maxDoc()));
    }
  }
  const wstring softDeletesField =
      completeReaderSet->stream()
          .map_obj([&](any r) { r::getFieldInfos().getSoftDeletesField(); })
          .filter(Objects::nonNull)
          .findAny()
          .orElse(nullptr);
  // TODO: make this read-only in a cleaner way?
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(softDeletesField));

  shared_ptr<Sort> indexSort = nullptr;
  int createdVersionMajor = -1;

  // build FieldInfos and fieldToReader map_obj:
  for (auto reader : this->parallelReaders) {
    shared_ptr<LeafMetaData> leafMetaData = reader->getMetaData();

    shared_ptr<Sort> leafIndexSort = leafMetaData->getSort();
    if (indexSort == nullptr) {
      indexSort = leafIndexSort;
    } else if (leafIndexSort != nullptr &&
               indexSort->equals(leafIndexSort) == false) {
      throw invalid_argument(L"cannot combine LeafReaders that have different "
                             L"index sorts: saw both sort=" +
                             indexSort + L" and " + leafIndexSort);
    }

    if (createdVersionMajor == -1) {
      createdVersionMajor = leafMetaData->getCreatedVersionMajor();
    } else if (createdVersionMajor != leafMetaData->getCreatedVersionMajor()) {
      throw invalid_argument(
          L"cannot combine LeafReaders that have different creation versions: "
          L"saw both version=" +
          to_wstring(createdVersionMajor) + L" and " +
          to_wstring(leafMetaData->getCreatedVersionMajor()));
    }

    shared_ptr<FieldInfos> *const readerFieldInfos = reader->getFieldInfos();
    for (auto fieldInfo : readerFieldInfos) {
      // NOTE: first reader having a given field "wins":
      if (!fieldToReader->containsKey(fieldInfo->name)) {
        builder->add(fieldInfo, fieldInfo->getDocValuesGen());
        fieldToReader->put(fieldInfo->name, reader);
        // only add these if the reader responsible for that field name is the
        // current:
        // TODO consider populating 1st leaf with vectors even if the field name
        // has been seen on a previous leaf
        if (fieldInfo->hasVectors()) {
          tvFieldToReader->put(fieldInfo->name, reader);
        }
        // TODO consider populating 1st leaf with terms even if the field name
        // has been seen on a previous leaf
        if (fieldInfo->getIndexOptions() != IndexOptions::NONE) {
          termsFieldToReader.emplace(fieldInfo->name, reader);
        }
      }
    }
  }
  if (createdVersionMajor == -1) {
    // empty reader
    createdVersionMajor = Version::LATEST->major;
  }

  shared_ptr<Version> minVersion = Version::LATEST;
  for (auto reader : this->parallelReaders) {
    shared_ptr<Version> leafVersion = reader->getMetaData()->getMinVersion();
    if (leafVersion == nullptr) {
      minVersion.reset();
      break;
    } else if (minVersion->onOrAfter(leafVersion)) {
      minVersion = leafVersion;
    }
  }

  // do this finally so any Exceptions occurred before don't affect refcounts:
  for (auto reader : completeReaderSet) {
    if (!closeSubReaders) {
      reader->incRef();
    }
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    reader->registerParentReader(shared_from_this());
  }
}

wstring ParallelLeafReader::toString()
{
  shared_ptr<StringBuilder> *const buffer =
      make_shared<StringBuilder>(L"ParallelLeafReader(");
  for (shared_ptr<final Iterator<std::shared_ptr<LeafReader>>> iter =
           completeReaderSet->begin();
       iter->hasNext();) {
    buffer->append(iter->next());
    if (iter->hasNext()) {
      buffer->append(L", ");
    }
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return buffer->append(L')')->toString();
}

ParallelLeafReader::ParallelFields::ParallelFields() {}

void ParallelLeafReader::ParallelFields::addField(const wstring &fieldName,
                                                  shared_ptr<Terms> terms)
{
  fields.emplace(fieldName, terms);
}

shared_ptr<Iterator<wstring>> ParallelLeafReader::ParallelFields::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms>
ParallelLeafReader::ParallelFields::terms(const wstring &field)
{
  return fields[field];
}

int ParallelLeafReader::ParallelFields::size() { return fields.size(); }

shared_ptr<FieldInfos> ParallelLeafReader::getFieldInfos()
{
  return fieldInfos;
}

shared_ptr<Bits> ParallelLeafReader::getLiveDocs()
{
  ensureOpen();
  return hasDeletions ? parallelReaders[0]->getLiveDocs() : nullptr;
}

shared_ptr<Terms>
ParallelLeafReader::terms(const wstring &field) 
{
  ensureOpen();
  shared_ptr<LeafReader> leafReader = termsFieldToReader[field];
  return leafReader == nullptr ? nullptr : leafReader->terms(field);
}

int ParallelLeafReader::numDocs()
{
  // Don't call ensureOpen() here (it could affect performance)
  return numDocs_;
}

int ParallelLeafReader::maxDoc()
{
  // Don't call ensureOpen() here (it could affect performance)
  return maxDoc_;
}

void ParallelLeafReader::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  ensureOpen();
  for (auto reader : storedFieldsReaders) {
    reader->document(docID, visitor);
  }
}

shared_ptr<CacheHelper> ParallelLeafReader::getCoreCacheHelper()
{
  // ParallelReader instances can be short-lived, which would make caching
  // trappy so we do not cache on them, unless they wrap a single reader in
  // which case we delegate
  if (parallelReaders.size() == 1 && storedFieldsReaders.size() == 1 &&
      parallelReaders[0] == storedFieldsReaders[0]) {
    return parallelReaders[0]->getCoreCacheHelper();
  }
  return nullptr;
}

shared_ptr<CacheHelper> ParallelLeafReader::getReaderCacheHelper()
{
  // ParallelReader instances can be short-lived, which would make caching
  // trappy so we do not cache on them, unless they wrap a single reader in
  // which case we delegate
  if (parallelReaders.size() == 1 && storedFieldsReaders.size() == 1 &&
      parallelReaders[0] == storedFieldsReaders[0]) {
    return parallelReaders[0]->getReaderCacheHelper();
  }
  return nullptr;
}

shared_ptr<Fields>
ParallelLeafReader::getTermVectors(int docID) 
{
  ensureOpen();
  shared_ptr<ParallelFields> fields;
  for (auto ent : tvFieldToReader) {
    wstring fieldName = ent.first;
    shared_ptr<Terms> deque = ent.second::getTermVector(docID, fieldName);
    if (deque != nullptr) {
      if (fields->empty()) {
        fields = make_shared<ParallelFields>();
      }
      fields->addField(fieldName, deque);
    }
  }

  return fields;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ParallelLeafReader::doClose() 
{
  shared_ptr<IOException> ioe = nullptr;
  for (auto reader : completeReaderSet) {
    try {
      if (closeSubReaders) {
        reader->close();
      } else {
        reader->decRef();
      }
    } catch (const IOException &e) {
      if (ioe == nullptr) {
        ioe = e;
      }
    }
  }
  // throw the first exception
  if (ioe != nullptr) {
    throw ioe;
  }
}

shared_ptr<NumericDocValues>
ParallelLeafReader::getNumericDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<LeafReader> reader = fieldToReader->get(field);
  return reader == nullptr ? nullptr : reader->getNumericDocValues(field);
}

shared_ptr<BinaryDocValues>
ParallelLeafReader::getBinaryDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<LeafReader> reader = fieldToReader->get(field);
  return reader == nullptr ? nullptr : reader->getBinaryDocValues(field);
}

shared_ptr<SortedDocValues>
ParallelLeafReader::getSortedDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<LeafReader> reader = fieldToReader->get(field);
  return reader == nullptr ? nullptr : reader->getSortedDocValues(field);
}

shared_ptr<SortedNumericDocValues>
ParallelLeafReader::getSortedNumericDocValues(const wstring &field) throw(
    IOException)
{
  ensureOpen();
  shared_ptr<LeafReader> reader = fieldToReader->get(field);
  return reader == nullptr ? nullptr : reader->getSortedNumericDocValues(field);
}

shared_ptr<SortedSetDocValues> ParallelLeafReader::getSortedSetDocValues(
    const wstring &field) 
{
  ensureOpen();
  shared_ptr<LeafReader> reader = fieldToReader->get(field);
  return reader == nullptr ? nullptr : reader->getSortedSetDocValues(field);
}

shared_ptr<NumericDocValues>
ParallelLeafReader::getNormValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<LeafReader> reader = fieldToReader->get(field);
  shared_ptr<NumericDocValues> values =
      reader == nullptr ? nullptr : reader->getNormValues(field);
  return values;
}

shared_ptr<PointValues>
ParallelLeafReader::getPointValues(const wstring &fieldName) 
{
  ensureOpen();
  shared_ptr<LeafReader> reader = fieldToReader->get(fieldName);
  return reader == nullptr ? nullptr : reader->getPointValues(fieldName);
}

void ParallelLeafReader::checkIntegrity() 
{
  ensureOpen();
  for (auto reader : completeReaderSet) {
    reader->checkIntegrity();
  }
}

std::deque<std::shared_ptr<LeafReader>>
ParallelLeafReader::getParallelReaders()
{
  ensureOpen();
  return parallelReaders;
}

shared_ptr<LeafMetaData> ParallelLeafReader::getMetaData() { return metaData; }
} // namespace org::apache::lucene::index