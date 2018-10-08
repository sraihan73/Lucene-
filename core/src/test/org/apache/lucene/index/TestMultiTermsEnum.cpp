using namespace std;

#include "TestMultiTermsEnum.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using CodecReader = org::apache::lucene::index::CodecReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using FilterCodecReader = org::apache::lucene::index::FilterCodecReader;
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMultiTermsEnum::testNoTermsInField() 
{
  shared_ptr<Directory> directory = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory,
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"deleted", L"0", Field::Store::YES));
  writer->addDocument(document);

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer);
  delete writer;

  shared_ptr<Directory> directory2 = make_shared<RAMDirectory>();
  writer = make_shared<IndexWriter>(
      directory2,
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  std::deque<std::shared_ptr<CodecReader>> codecReaders(leaves.size());
  for (int i = 0; i < leaves.size(); i++) {
    codecReaders[i] = make_shared<MigratingCodecReader>(
        std::static_pointer_cast<CodecReader>(leaves[i]->reader()));
  }

  writer->addIndexes(codecReaders); // <- bang

  IOUtils::close({writer, reader, directory});
}

TestMultiTermsEnum::MigratingCodecReader::MigratingCodecReader(
    shared_ptr<CodecReader> in_)
    : org::apache::lucene::index::FilterCodecReader(in_)
{
}

shared_ptr<FieldsProducer>
TestMultiTermsEnum::MigratingCodecReader::getPostingsReader()
{
  return make_shared<MigratingFieldsProducer>(
      FilterCodecReader::getPostingsReader(), getFieldInfos());
}

TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    MigratingFieldsProducer(shared_ptr<FieldsProducer> delegate_,
                            shared_ptr<FieldInfos> newFieldInfo)
    : BaseMigratingFieldsProducer(delegate_, newFieldInfo)
{
}

shared_ptr<Terms>
TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::terms(
    const wstring &field) 
{
  if (L"deleted" == field) {
    shared_ptr<Terms> deletedTerms =
        BaseMigratingFieldsProducer::terms(L"deleted");
    if (deletedTerms != nullptr) {
      return make_shared<ValueFilteredTerms>(deletedTerms,
                                             make_shared<BytesRef>(L"1"));
    }
    return nullptr;
  } else {
    return BaseMigratingFieldsProducer::terms(field);
  }
}

shared_ptr<FieldsProducer>
TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::create(
    shared_ptr<FieldsProducer> delegate_, shared_ptr<FieldInfos> newFieldInfo)
{
  return make_shared<MigratingFieldsProducer>(delegate_, newFieldInfo);
}

TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::ValueFilteredTerms(shared_ptr<Terms> delegate_,
                                           shared_ptr<BytesRef> value)
    : delegate_(delegate_), value(value)
{
}

shared_ptr<TermsEnum> TestMultiTermsEnum::MigratingCodecReader::
    MigratingFieldsProducer::ValueFilteredTerms::iterator() 
{
  return make_shared<FilteredTermsEnumAnonymousInnerClass>(shared_from_this(),
                                                           delegate_->begin());
}

TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::FilteredTermsEnumAnonymousInnerClass::
        FilteredTermsEnumAnonymousInnerClass(
            shared_ptr<ValueFilteredTerms> outerInstance,
            shared_ptr<TermsEnum> iterator)
    : org::apache::lucene::index::FilteredTermsEnum(iterator)
{
  this->outerInstance = outerInstance;
}

AcceptStatus TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::FilteredTermsEnumAnonymousInnerClass::accept(
        shared_ptr<BytesRef> term)
{

  int comparison = term->compareTo(outerInstance->value);
  if (comparison < 0) {
    // I don't think it will actually get here because they are supposed to call
    // nextSeekTerm to get the initial term to seek to.
    return AcceptStatus::NO_AND_SEEK;
  } else if (comparison > 0) {
    return AcceptStatus::END;
  } else { // comparison == 0
    return AcceptStatus::YES;
  }
}

shared_ptr<BytesRef>
TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::FilteredTermsEnumAnonymousInnerClass::nextSeekTerm(
        shared_ptr<BytesRef> currentTerm)
{
  if (currentTerm == nullptr ||
      currentTerm->compareTo(outerInstance->value) < 0) {
    return outerInstance->value;
  }

  return nullptr;
}

int64_t TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::size() 
{
  // Docs say we can return -1 if we don't know.
  return -1;
}

int64_t TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::getSumTotalTermFreq() 
{
  // Docs say we can return -1 if we don't know.
  return -1;
}

int64_t TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::getSumDocFreq() 
{
  // Docs say we can return -1 if we don't know.
  return -1;
}

int TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::getDocCount() 
{
  // Docs say we can return -1 if we don't know.
  return -1;
}

bool TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::hasFreqs()
{
  return delegate_->hasFreqs();
}

bool TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::hasOffsets()
{
  return delegate_->hasOffsets();
}

bool TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::hasPositions()
{
  return delegate_->hasPositions();
}

bool TestMultiTermsEnum::MigratingCodecReader::MigratingFieldsProducer::
    ValueFilteredTerms::hasPayloads()
{
  return delegate_->hasPayloads();
}

TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    BaseMigratingFieldsProducer(shared_ptr<FieldsProducer> delegate_,
                                shared_ptr<FieldInfos> newFieldInfo)
    : delegate_(delegate_), newFieldInfo(newFieldInfo)
{
}

shared_ptr<Iterator<wstring>> TestMultiTermsEnum::MigratingCodecReader::
    BaseMigratingFieldsProducer::iterator()
{
  constexpr FieldInfos::const_iterator fieldInfoIterator =
      newFieldInfo->begin();
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this(),
                                                  fieldInfoIterator);
}

TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
        shared_ptr<BaseMigratingFieldsProducer> outerInstance,
        shared_ptr<FieldInfos::const_iterator> fieldInfoIterator)
{
  this->outerInstance = outerInstance;
  this->fieldInfoIterator = fieldInfoIterator;
}

bool TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    IteratorAnonymousInnerClass::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return fieldInfoIterator->hasNext();
}

void TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

wstring TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    IteratorAnonymousInnerClass::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return fieldInfoIterator->next().name;
}

int TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    size()
{
  return newFieldInfo->size();
}

shared_ptr<Terms>
TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::terms(
    const wstring &field) 
{
  return delegate_->terms(field);
}

shared_ptr<FieldsProducer> TestMultiTermsEnum::MigratingCodecReader::
    BaseMigratingFieldsProducer::getMergeInstance() 
{
  return create(delegate_->getMergeInstance(), newFieldInfo);
}

shared_ptr<FieldsProducer>
TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::create(
    shared_ptr<FieldsProducer> delegate_, shared_ptr<FieldInfos> newFieldInfo)
{
  return make_shared<BaseMigratingFieldsProducer>(delegate_, newFieldInfo);
}

void TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    checkIntegrity() 
{
  delegate_->checkIntegrity();
}

int64_t TestMultiTermsEnum::MigratingCodecReader::
    BaseMigratingFieldsProducer::ramBytesUsed()
{
  return delegate_->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>> TestMultiTermsEnum::
    MigratingCodecReader::BaseMigratingFieldsProducer::getChildResources()
{
  return delegate_->getChildResources();
}

TestMultiTermsEnum::MigratingCodecReader::BaseMigratingFieldsProducer::
    ~BaseMigratingFieldsProducer()
{
  delegate_->close();
}

shared_ptr<CacheHelper>
TestMultiTermsEnum::MigratingCodecReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper>
TestMultiTermsEnum::MigratingCodecReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index