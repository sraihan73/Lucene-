using namespace std;

#include "BaseFieldInfoFormatTestCase.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using Failure = org::apache::lucene::store::MockDirectoryWrapper::Failure;
using FakeIOException =
    org::apache::lucene::store::MockDirectoryWrapper::FakeIOException;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void BaseFieldInfoFormatTestCase::testOneField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<SegmentInfo> segmentInfo = newSegmentInfo(dir, L"_123");
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  shared_ptr<FieldInfo> fi = builder->getOrAdd(L"field");
  fi->setIndexOptions(TextField::TYPE_STORED->indexOptions());
  addAttributes(fi);
  shared_ptr<FieldInfos> infos = builder->finish();
  codec->fieldInfosFormat()->write(dir, segmentInfo, L"", infos,
                                   IOContext::DEFAULT);
  shared_ptr<FieldInfos> infos2 = codec->fieldInfosFormat()->read(
      dir, segmentInfo, L"", IOContext::DEFAULT);
  assertEquals(1, infos2->size());
  assertNotNull(infos2->fieldInfo(L"field"));
  assertTrue(infos2->fieldInfo(L"field")->getIndexOptions() !=
             IndexOptions::NONE);
  assertFalse(infos2->fieldInfo(L"field")->getDocValuesType() !=
              DocValuesType::NONE);
  assertFalse(infos2->fieldInfo(L"field")->omitsNorms());
  assertFalse(infos2->fieldInfo(L"field")->hasPayloads());
  assertFalse(infos2->fieldInfo(L"field")->hasVectors());
  delete dir;
}

void BaseFieldInfoFormatTestCase::testImmutableAttributes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<SegmentInfo> segmentInfo = newSegmentInfo(dir, L"_123");
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  shared_ptr<FieldInfo> fi = builder->getOrAdd(L"field");
  fi->setIndexOptions(TextField::TYPE_STORED->indexOptions());
  addAttributes(fi);
  fi->putAttribute(L"foo", L"bar");
  fi->putAttribute(L"bar", L"baz");
  shared_ptr<FieldInfos> infos = builder->finish();
  codec->fieldInfosFormat()->write(dir, segmentInfo, L"", infos,
                                   IOContext::DEFAULT);
  shared_ptr<FieldInfos> infos2 = codec->fieldInfosFormat()->read(
      dir, segmentInfo, L"", IOContext::DEFAULT);
  assertEquals(1, infos2->size());
  assertNotNull(infos2->fieldInfo(L"field"));
  unordered_map<wstring, wstring> attributes =
      infos2->fieldInfo(L"field")->attributes();
  // shouldn't be able to modify attributes
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { attributes.put(L"bogus", L"bogus"); });

  delete dir;
}

void BaseFieldInfoFormatTestCase::testExceptionOnCreateOutput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<SegmentInfo> segmentInfo = newSegmentInfo(dir, L"_123");
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  shared_ptr<FieldInfo> fi = builder->getOrAdd(L"field");
  fi->setIndexOptions(TextField::TYPE_STORED->indexOptions());
  addAttributes(fi);
  shared_ptr<FieldInfos> infos = builder->finish();

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->fieldInfosFormat()->write(dir, segmentInfo, L"", infos,
                                     IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass::
    FailureAnonymousInnerClass(
        shared_ptr<BaseFieldInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"createOutput" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseFieldInfoFormatTestCase::testExceptionOnCloseOutput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass2>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<SegmentInfo> segmentInfo = newSegmentInfo(dir, L"_123");
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  shared_ptr<FieldInfo> fi = builder->getOrAdd(L"field");
  fi->setIndexOptions(TextField::TYPE_STORED->indexOptions());
  addAttributes(fi);
  shared_ptr<FieldInfos> infos = builder->finish();

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->fieldInfosFormat()->write(dir, segmentInfo, L"", infos,
                                     IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass2::
    FailureAnonymousInnerClass2(
        shared_ptr<BaseFieldInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass2::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"close" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseFieldInfoFormatTestCase::testExceptionOnOpenInput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass3>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<SegmentInfo> segmentInfo = newSegmentInfo(dir, L"_123");
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  shared_ptr<FieldInfo> fi = builder->getOrAdd(L"field");
  fi->setIndexOptions(TextField::TYPE_STORED->indexOptions());
  addAttributes(fi);
  shared_ptr<FieldInfos> infos = builder->finish();
  codec->fieldInfosFormat()->write(dir, segmentInfo, L"", infos,
                                   IOContext::DEFAULT);

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->fieldInfosFormat()->read(dir, segmentInfo, L"", IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass3::
    FailureAnonymousInnerClass3(
        shared_ptr<BaseFieldInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass3::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"openInput" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseFieldInfoFormatTestCase::testExceptionOnCloseInput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass4>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<SegmentInfo> segmentInfo = newSegmentInfo(dir, L"_123");
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  shared_ptr<FieldInfo> fi = builder->getOrAdd(L"field");
  fi->setIndexOptions(TextField::TYPE_STORED->indexOptions());
  addAttributes(fi);
  shared_ptr<FieldInfos> infos = builder->finish();
  codec->fieldInfosFormat()->write(dir, segmentInfo, L"", infos,
                                   IOContext::DEFAULT);

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->fieldInfosFormat()->read(dir, segmentInfo, L"", IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass4::
    FailureAnonymousInnerClass4(
        shared_ptr<BaseFieldInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseFieldInfoFormatTestCase::FailureAnonymousInnerClass4::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"close" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseFieldInfoFormatTestCase::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<SegmentInfo> segmentInfo = newSegmentInfo(dir, L"_123");

  // generate a bunch of fields
  int numFields = atLeast(2000);
  shared_ptr<Set<wstring>> fieldNames = unordered_set<wstring>();
  for (int i = 0; i < numFields; i++) {
    fieldNames->add(TestUtil::randomUnicodeString(random()));
  }
  shared_ptr<FieldInfos::Builder> builder = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  for (auto field : fieldNames) {
    shared_ptr<IndexableFieldType> fieldType = randomFieldType(random());
    shared_ptr<FieldInfo> fi = builder->getOrAdd(field);
    IndexOptions indexOptions = fieldType->indexOptions();
    if (indexOptions != IndexOptions::NONE) {
      fi->setIndexOptions(indexOptions);
      if (fieldType->omitNorms()) {
        fi->setOmitsNorms();
      }
    }
    fi->setDocValuesType(fieldType->docValuesType());
    if (fieldType->indexOptions() != IndexOptions::NONE &&
        fieldType->indexOptions().compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0) {
      if (random()->nextBoolean()) {
        fi->setStorePayloads();
      }
    }
    addAttributes(fi);
  }
  shared_ptr<FieldInfos> infos = builder->finish();
  codec->fieldInfosFormat()->write(dir, segmentInfo, L"", infos,
                                   IOContext::DEFAULT);
  shared_ptr<FieldInfos> infos2 = codec->fieldInfosFormat()->read(
      dir, segmentInfo, L"", IOContext::DEFAULT);
  assertEquals(infos, infos2);
  delete dir;
}

shared_ptr<IndexableFieldType>
BaseFieldInfoFormatTestCase::randomFieldType(shared_ptr<Random> r)
{
  shared_ptr<FieldType> type = make_shared<FieldType>();

  if (r->nextBoolean()) {
    std::deque<IndexOptions> values = IndexOptions::values();
    type->setIndexOptions(values[r->nextInt(values.size())]);
    type->setOmitNorms(r->nextBoolean());

    if (r->nextBoolean()) {
      type->setStoreTermVectors(true);
      if (type->indexOptions().compareTo(
              IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0) {
        type->setStoreTermVectorPositions(r->nextBoolean());
        type->setStoreTermVectorOffsets(r->nextBoolean());
        if (type->storeTermVectorPositions()) {
          type->setStoreTermVectorPayloads(r->nextBoolean());
        }
      }
    }
  }

  if (r->nextBoolean()) {
    std::deque<DocValuesType> values = getDocValuesTypes();
    type->setDocValuesType(values[r->nextInt(values.size())]);
  }

  return type;
}

void BaseFieldInfoFormatTestCase::addAttributes(shared_ptr<FieldInfo> fi) {}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated protected DocValuesType[] getDocValuesTypes()
std::deque<DocValuesType> BaseFieldInfoFormatTestCase::getDocValuesTypes()
{
  return DocValuesType::values();
}

void BaseFieldInfoFormatTestCase::assertEquals(shared_ptr<FieldInfos> expected,
                                               shared_ptr<FieldInfos> actual)
{
  assertEquals(expected->size(), actual->size());
  for (auto expectedField : expected) {
    shared_ptr<FieldInfo> actualField =
        actual->fieldInfo(expectedField->number);
    assertNotNull(actualField);
    assertEquals(expectedField, actualField);
  }
}

void BaseFieldInfoFormatTestCase::assertEquals(shared_ptr<FieldInfo> expected,
                                               shared_ptr<FieldInfo> actual)
{
  assertEquals(expected->number, actual->number);
  assertEquals(expected->name, actual->name);
  assertEquals(expected->getDocValuesType(), actual->getDocValuesType());
  assertEquals(expected->getIndexOptions(), actual->getIndexOptions());
  assertEquals(expected->hasNorms(), actual->hasNorms());
  assertEquals(expected->hasPayloads(), actual->hasPayloads());
  assertEquals(expected->hasVectors(), actual->hasVectors());
  assertEquals(expected->omitsNorms(), actual->omitsNorms());
  assertEquals(expected->getDocValuesGen(), actual->getDocValuesGen());
}

shared_ptr<SegmentInfo>
BaseFieldInfoFormatTestCase::newSegmentInfo(shared_ptr<Directory> dir,
                                            const wstring &name)
{
  shared_ptr<Version> minVersion =
      random()->nextBoolean() ? nullptr : Version::LATEST;
  return make_shared<SegmentInfo>(
      dir, Version::LATEST, minVersion, name, 10000, false, Codec::getDefault(),
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);
}

void BaseFieldInfoFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  doc->push_back(make_shared<StoredField>(
      L"foobar", TestUtil::randomSimpleString(random())));
}

void BaseFieldInfoFormatTestCase::testRamBytesUsed() 
{
  assumeTrue(L"not applicable for this format", true);
}
} // namespace org::apache::lucene::index