using namespace std;

#include "BaseSegmentInfoFormatTestCase.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using StoredField = org::apache::lucene::document::StoredField;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSortField =
    org::apache::lucene::search::SortedNumericSortField;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using Failure = org::apache::lucene::store::MockDirectoryWrapper::Failure;
using FakeIOException =
    org::apache::lucene::store::MockDirectoryWrapper::FakeIOException;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

bool BaseSegmentInfoFormatTestCase::supportsMinVersion() { return true; }

void BaseSegmentInfoFormatTestCase::testFiles() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(),
      nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  shared_ptr<SegmentInfo> info2 =
      codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
  assertEquals(info->files(), info2->files());
  delete dir;
}

void BaseSegmentInfoFormatTestCase::testAddsSelfToFiles() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(),
      nullptr);
  shared_ptr<Set<wstring>> originalFiles = Collections::singleton(L"_123.a");
  info->setFiles(originalFiles);
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);

  shared_ptr<Set<wstring>> modifiedFiles = info->files();
  assertTrue(modifiedFiles->containsAll(originalFiles));
  assertTrue(L"did you forget to add yourself to files()",
             modifiedFiles->size() > originalFiles->size());

  shared_ptr<SegmentInfo> info2 =
      codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
  assertEquals(info->files(), info2->files());

  // files set should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { info2->files()->add(L"bogus"); });

  delete dir;
}

void BaseSegmentInfoFormatTestCase::testDiagnostics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  unordered_map<wstring, wstring> diagnostics =
      unordered_map<wstring, wstring>();
  diagnostics.emplace(L"key1", L"value1");
  diagnostics.emplace(L"key2", L"value2");
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      diagnostics, id, unordered_map<>(), nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  shared_ptr<SegmentInfo> info2 =
      codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
  assertEquals(diagnostics, info2->getDiagnostics());

  // diagnostics map_obj should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { info2->getDiagnostics().emplace(L"bogus", L"bogus"); });

  delete dir;
}

void BaseSegmentInfoFormatTestCase::testAttributes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  unordered_map<wstring, wstring> attributes =
      unordered_map<wstring, wstring>();
  attributes.emplace(L"key1", L"value1");
  attributes.emplace(L"key2", L"value2");
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap(), id, attributes, nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  shared_ptr<SegmentInfo> info2 =
      codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
  assertEquals(attributes, info2->getAttributes());

  // attributes map_obj should be immutable
  expectThrows(UnsupportedOperationException::typeid,
               [&]() { info2->getAttributes().emplace(L"bogus", L"bogus"); });

  delete dir;
}

void BaseSegmentInfoFormatTestCase::testUniqueID() 
{
  shared_ptr<Codec> codec = getCodec();
  shared_ptr<Directory> dir = newDirectory();
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(),
      nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  shared_ptr<SegmentInfo> info2 =
      codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
  assertIDEquals(id, info2->getId());
  delete dir;
}

void BaseSegmentInfoFormatTestCase::testVersions() 
{
  shared_ptr<Codec> codec = getCodec();
  for (auto v : getVersions()) {
    for (auto minV : std::deque<std::shared_ptr<Version>>{v, nullptr}) {
      shared_ptr<Directory> dir = newDirectory();
      std::deque<char> id = StringHelper::randomId();
      shared_ptr<SegmentInfo> info =
          make_shared<SegmentInfo>(dir, v, minV, L"_123", 1, false, codec,
                                   Collections::emptyMap<wstring, wstring>(),
                                   id, unordered_map<>(), nullptr);
      info->setFiles(Collections::emptySet<wstring>());
      codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
      shared_ptr<SegmentInfo> info2 = codec->segmentInfoFormat()->read(
          dir, L"_123", id, IOContext::DEFAULT);
      assertEquals(info2->getVersion(), v);
      if (supportsMinVersion()) {
        assertEquals(info2->getMinVersion(), minV);
      } else {
        assertEquals(info2->getMinVersion(), nullptr);
      }
      delete dir;
    }
  }
}

bool BaseSegmentInfoFormatTestCase::supportsIndexSort() { return true; }

shared_ptr<SortField> BaseSegmentInfoFormatTestCase::randomIndexSortField()
{
  bool reversed = random()->nextBoolean();
  shared_ptr<SortField> sortField;
  switch (random()->nextInt(10)) {
  case 0:
    sortField = make_shared<SortField>(TestUtil::randomSimpleString(random()),
                                       SortField::Type::INT, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextInt());
    }
    break;
  case 1:
    sortField = make_shared<SortedNumericSortField>(
        TestUtil::randomSimpleString(random()), SortField::Type::INT, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextInt());
    }
    break;

  case 2:
    sortField = make_shared<SortField>(TestUtil::randomSimpleString(random()),
                                       SortField::Type::LONG, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextLong());
    }
    break;
  case 3:
    sortField = make_shared<SortedNumericSortField>(
        TestUtil::randomSimpleString(random()), SortField::Type::LONG,
        reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextLong());
    }
    break;
  case 4:
    sortField = make_shared<SortField>(TestUtil::randomSimpleString(random()),
                                       SortField::Type::FLOAT, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextFloat());
    }
    break;
  case 5:
    sortField = make_shared<SortedNumericSortField>(
        TestUtil::randomSimpleString(random()), SortField::Type::FLOAT,
        reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextFloat());
    }
    break;
  case 6:
    sortField = make_shared<SortField>(TestUtil::randomSimpleString(random()),
                                       SortField::Type::DOUBLE, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextDouble());
    }
    break;
  case 7:
    sortField = make_shared<SortedNumericSortField>(
        TestUtil::randomSimpleString(random()), SortField::Type::DOUBLE,
        reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(random()->nextDouble());
    }
    break;
  case 8:
    sortField = make_shared<SortField>(TestUtil::randomSimpleString(random()),
                                       SortField::Type::STRING, reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(SortField::STRING_LAST);
    }
    break;
  case 9:
    sortField = make_shared<SortedSetSortField>(
        TestUtil::randomSimpleString(random()), reversed);
    if (random()->nextBoolean()) {
      sortField->setMissingValue(SortField::STRING_LAST);
    }
    break;
  default:
    sortField.reset();
    fail();
  }
  return sortField;
}

void BaseSegmentInfoFormatTestCase::testSort() 
{
  assumeTrue(L"test requires a codec that can read/write index sort",
             supportsIndexSort());

  constexpr int iters = atLeast(5);
  for (int i = 0; i < iters; ++i) {
    shared_ptr<Sort> sort;
    if (i == 0) {
      sort.reset();
    } else {
      constexpr int numSortFields = TestUtil::nextInt(random(), 1, 3);
      std::deque<std::shared_ptr<SortField>> sortFields(numSortFields);
      for (int j = 0; j < numSortFields; ++j) {
        sortFields[j] = randomIndexSortField();
      }
      sort = make_shared<Sort>(sortFields);
    }

    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<Codec> codec = getCodec();
    std::deque<char> id = StringHelper::randomId();
    shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
        dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
        Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(), sort);
    info->setFiles(Collections::emptySet<wstring>());
    codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
    shared_ptr<SegmentInfo> info2 =
        codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
    assertEquals(sort, info2->getIndexSort());
    delete dir;
  }
}

void BaseSegmentInfoFormatTestCase::testExceptionOnCreateOutput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(),
      nullptr);
  info->setFiles(Collections::emptySet<wstring>());

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass::
    FailureAnonymousInnerClass(
        shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"createOutput" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseSegmentInfoFormatTestCase::testExceptionOnCloseOutput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass2>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(),
      nullptr);
  info->setFiles(Collections::emptySet<wstring>());

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass2::
    FailureAnonymousInnerClass2(
        shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass2::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"close" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseSegmentInfoFormatTestCase::testExceptionOnOpenInput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass3>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(),
      nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass3::
    FailureAnonymousInnerClass3(
        shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass3::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"openInput" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseSegmentInfoFormatTestCase::testExceptionOnCloseInput() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper::Failure> fail =
      make_shared<FailureAnonymousInnerClass4>(shared_from_this());

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(fail);
  shared_ptr<Codec> codec = getCodec();
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, getVersions()[0], getVersions()[0], L"_123", 1, false, codec,
      Collections::emptyMap<wstring, wstring>(), id, unordered_map<>(),
      nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);

  fail->setDoFail();
  expectThrows(MockDirectoryWrapper::FakeIOException::typeid, [&]() {
    codec->segmentInfoFormat()->read(dir, L"_123", id, IOContext::DEFAULT);
  });
  fail->clearDoFail();

  delete dir;
}

BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass4::
    FailureAnonymousInnerClass4(
        shared_ptr<BaseSegmentInfoFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BaseSegmentInfoFormatTestCase::FailureAnonymousInnerClass4::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    if (doFail && L"close" == e->getMethodName()) {
      throw make_shared<MockDirectoryWrapper::FakeIOException>();
    }
  }
}

void BaseSegmentInfoFormatTestCase::testRandom() 
{
  shared_ptr<Codec> codec = getCodec();
  std::deque<std::shared_ptr<Version>> versions = getVersions();
  for (int i = 0; i < 10; i++) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<Version> version = versions[random()->nextInt(versions.size())];
    int64_t randomSegmentIndex = abs(random()->nextLong());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring name =
        L"_" +
        Long::toString(randomSegmentIndex != numeric_limits<int64_t>::min()
                           ? randomSegmentIndex
                           : random()->nextInt(numeric_limits<int>::max()),
                       Character::MAX_RADIX);
    int docCount = TestUtil::nextInt(random(), 1, IndexWriter::MAX_DOCS);
    bool isCompoundFile = random()->nextBoolean();
    shared_ptr<Set<wstring>> files = unordered_set<wstring>();
    int numFiles = random()->nextInt(10);
    for (int j = 0; j < numFiles; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring file =
          IndexFileNames::segmentFileName(name, L"", Integer::toString(j));
      files->add(file);
      delete dir->createOutput(file, IOContext::DEFAULT);
    }
    unordered_map<wstring, wstring> diagnostics =
        unordered_map<wstring, wstring>();
    int numDiags = random()->nextInt(10);
    for (int j = 0; j < numDiags; j++) {
      diagnostics.emplace(TestUtil::randomUnicodeString(random()),
                          TestUtil::randomUnicodeString(random()));
    }
    std::deque<char> id(StringHelper::ID_LENGTH);
    random()->nextBytes(id);

    unordered_map<wstring, wstring> attributes =
        unordered_map<wstring, wstring>();
    int numAttributes = random()->nextInt(10);
    for (int j = 0; j < numAttributes; j++) {
      attributes.emplace(TestUtil::randomUnicodeString(random()),
                         TestUtil::randomUnicodeString(random()));
    }

    shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
        dir, version, nullptr, name, docCount, isCompoundFile, codec,
        diagnostics, id, attributes, nullptr);
    info->setFiles(files);
    codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
    shared_ptr<SegmentInfo> info2 =
        codec->segmentInfoFormat()->read(dir, name, id, IOContext::DEFAULT);
    assertEquals(info, info2);

    delete dir;
  }
}

void BaseSegmentInfoFormatTestCase::assertEquals(
    shared_ptr<SegmentInfo> expected, shared_ptr<SegmentInfo> actual)
{
  assertSame(expected->dir, actual->dir);
  assertEquals(expected->name, actual->name);
  assertEquals(expected->files(), actual->files());
  // we don't assert this, because SI format has nothing to do with it... set by
  // SIS assertSame(expected.getCodec(), actual.getCodec());
  assertEquals(expected->getDiagnostics(), actual->getDiagnostics());
  assertEquals(expected->maxDoc(), actual->maxDoc());
  assertIDEquals(expected->getId(), actual->getId());
  assertEquals(expected->getUseCompoundFile(), actual->getUseCompoundFile());
  assertEquals(expected->getVersion(), actual->getVersion());
  assertEquals(expected->getAttributes(), actual->getAttributes());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated protected void assertIDEquals(byte expected[],
// byte actual[])
void BaseSegmentInfoFormatTestCase::assertIDEquals(std::deque<char> &expected,
                                                   std::deque<char> &actual)
{
  assertArrayEquals(expected, actual);
}

void BaseSegmentInfoFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  doc->push_back(make_shared<StoredField>(
      L"foobar", TestUtil::randomSimpleString(random())));
}

void BaseSegmentInfoFormatTestCase::testRamBytesUsed() 
{
  assumeTrue(L"not applicable for this format", true);
}
} // namespace org::apache::lucene::index