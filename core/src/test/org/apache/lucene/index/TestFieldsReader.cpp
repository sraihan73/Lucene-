using namespace std;

#include "TestFieldsReader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DocumentStoredFieldVisitor =
    org::apache::lucene::document::DocumentStoredFieldVisitor;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using BufferedIndexInput = org::apache::lucene::store::BufferedIndexInput;
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory> TestFieldsReader::dir;
shared_ptr<org::apache::lucene::document::Document> TestFieldsReader::testDoc;
shared_ptr<FieldInfos::Builder> TestFieldsReader::fieldInfos = nullptr;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFieldsReader::beforeClass() 
{
  testDoc = make_shared<Document>();
  fieldInfos = make_shared<FieldInfos::Builder>(
      make_shared<FieldInfos::FieldNumbers>(nullptr));
  DocHelper::setupDoc(testDoc);
  for (auto field : testDoc->getFields()) {
    shared_ptr<FieldInfo> fieldInfo = fieldInfos->getOrAdd(field->name());
    shared_ptr<IndexableFieldType> ift = field->fieldType();
    fieldInfo->setIndexOptions(ift->indexOptions());
    if (ift->omitNorms()) {
      fieldInfo->setOmitsNorms();
    }
    fieldInfo->setDocValuesType(ift->docValuesType());
  }
  dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy());
  conf->getMergePolicy()->setNoCFSRatio(0.0);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  writer->addDocument(testDoc);
  delete writer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestFieldsReader::afterClass() 
{
  delete dir;
  dir.reset();
  fieldInfos.reset();
  testDoc.reset();
}

void TestFieldsReader::test() 
{
  assertTrue(dir != nullptr);
  assertTrue(fieldInfos != nullptr);
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<Document> doc = reader->document(0);
  assertTrue(doc->size() > 0);
  assertTrue(doc->getField(DocHelper::TEXT_FIELD_1_KEY) != nullptr);

  shared_ptr<Field> field = std::static_pointer_cast<Field>(
      doc->getField(DocHelper::TEXT_FIELD_2_KEY));
  assertTrue(field != nullptr);
  assertTrue(field->fieldType()->storeTermVectors());

  assertFalse(field->fieldType()->omitNorms());
  assertTrue(field->fieldType()->indexOptions() ==
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);

  field = std::static_pointer_cast<Field>(
      doc->getField(DocHelper::TEXT_FIELD_3_KEY));
  assertTrue(field != nullptr);
  assertFalse(field->fieldType()->storeTermVectors());
  assertTrue(field->fieldType()->omitNorms());
  assertTrue(field->fieldType()->indexOptions() ==
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);

  field = std::static_pointer_cast<Field>(doc->getField(DocHelper::NO_TF_KEY));
  assertTrue(field != nullptr);
  assertFalse(field->fieldType()->storeTermVectors());
  assertFalse(field->fieldType()->omitNorms());
  assertTrue(field->fieldType()->indexOptions() == IndexOptions::DOCS);

  shared_ptr<DocumentStoredFieldVisitor> visitor =
      make_shared<DocumentStoredFieldVisitor>(DocHelper::TEXT_FIELD_3_KEY);
  reader->document(0, visitor);
  const deque<std::shared_ptr<IndexableField>> fields =
      visitor->getDocument()->getFields();
  assertEquals(1, fields.size());
  assertEquals(DocHelper::TEXT_FIELD_3_KEY, fields[0]->name());
  delete reader;
}

TestFieldsReader::FaultyFSDirectory::FaultyFSDirectory(
    shared_ptr<TestFieldsReader> outerInstance, shared_ptr<Directory> fsDir)
    : org::apache::lucene::store::FilterDirectory(fsDir),
      outerInstance(outerInstance)
{
}

shared_ptr<IndexInput> TestFieldsReader::FaultyFSDirectory::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  return make_shared<FaultyIndexInput>(outerInstance, doFail,
                                       in_->openInput(name, context));
}

void TestFieldsReader::FaultyFSDirectory::startFailing() { doFail->set(true); }

TestFieldsReader::FaultyIndexInput::FaultyIndexInput(
    shared_ptr<TestFieldsReader> outerInstance,
    shared_ptr<AtomicBoolean> doFail, shared_ptr<IndexInput> delegate_)
    : org::apache::lucene::store::BufferedIndexInput(
          L"FaultyIndexInput(" + delegate_ + L")",
          BufferedIndexInput::BUFFER_SIZE),
      doFail(doFail), outerInstance(outerInstance)
{
  this->delegate_ = delegate_;
}

void TestFieldsReader::FaultyIndexInput::simOutage() 
{
  if (doFail->get() && count++ % 2 == 1) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Simulated network outage");
  }
}

void TestFieldsReader::FaultyIndexInput::readInternal(
    std::deque<char> &b, int offset, int length) 
{
  simOutage();
  delegate_->seek(getFilePointer());
  delegate_->readBytes(b, offset, length);
}

void TestFieldsReader::FaultyIndexInput::seekInternal(int64_t pos) throw(
    IOException)
{
}

int64_t TestFieldsReader::FaultyIndexInput::length()
{
  return delegate_->length();
}

TestFieldsReader::FaultyIndexInput::~FaultyIndexInput() { delete delegate_; }

shared_ptr<FaultyIndexInput> TestFieldsReader::FaultyIndexInput::clone()
{
  shared_ptr<FaultyIndexInput> i =
      make_shared<FaultyIndexInput>(outerInstance, doFail, delegate_->clone());
  // seek the clone to our current position
  try {
    i->seek(getFilePointer());
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  return i;
}

shared_ptr<IndexInput>
TestFieldsReader::FaultyIndexInput::slice(const wstring &sliceDescription,
                                          int64_t offset,
                                          int64_t length) 
{
  shared_ptr<IndexInput> slice =
      delegate_->slice(sliceDescription, offset, length);
  return make_shared<FaultyIndexInput>(outerInstance, doFail, slice);
}

void TestFieldsReader::testExceptions() 
{
  shared_ptr<Path> indexDir = createTempDir(L"testfieldswriterexceptions");

  shared_ptr<Directory> fsDir = newFSDirectory(indexDir);
  shared_ptr<FaultyFSDirectory> dir =
      make_shared<FaultyFSDirectory>(shared_from_this(), fsDir);
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(OpenMode::CREATE);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 2; i++) {
    writer->addDocument(testDoc);
  }
  writer->forceMerge(1);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  dir->startFailing();

  bool exc = false;

  for (int i = 0; i < 2; i++) {
    try {
      reader->document(i);
    } catch (const IOException &ioe) {
      // expected
      exc = true;
    }
    try {
      reader->document(i);
    } catch (const IOException &ioe) {
      // expected
      exc = true;
    }
  }
  assertTrue(exc);
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::index