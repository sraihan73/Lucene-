using namespace std;

#include "WriteLineDocTaskTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/WriteLineDocTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using org::apache::commons::compress::compressors::CompressorStreamFactory;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Type = org::apache::lucene::benchmark::byTask::utils::StreamUtils::Type;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;

shared_ptr<Document>
WriteLineDocTaskTest::WriteLineDocMaker::makeDocument() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(BODY_FIELD, L"body", Field::Store::NO));
  doc->push_back(
      make_shared<StringField>(TITLE_FIELD, L"title", Field::Store::NO));
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date", Field::Store::NO));
  return doc;
}

shared_ptr<Document>
WriteLineDocTaskTest::NewLinesDocMaker::makeDocument() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(BODY_FIELD, L"body\r\ntext\ttwo",
                                          Field::Store::NO));
  doc->push_back(make_shared<StringField>(TITLE_FIELD, L"title\r\ntext",
                                          Field::Store::NO));
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date\r\ntext", Field::Store::NO));
  return doc;
}

shared_ptr<Document>
WriteLineDocTaskTest::NoBodyDocMaker::makeDocument() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(TITLE_FIELD, L"title", Field::Store::NO));
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date", Field::Store::NO));
  return doc;
}

shared_ptr<Document>
WriteLineDocTaskTest::NoTitleDocMaker::makeDocument() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(BODY_FIELD, L"body", Field::Store::NO));
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date", Field::Store::NO));
  return doc;
}

shared_ptr<Document>
WriteLineDocTaskTest::JustDateDocMaker::makeDocument() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date", Field::Store::NO));
  return doc;
}

shared_ptr<Document>
WriteLineDocTaskTest::LegalJustDateDocMaker::makeDocument() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date", Field::Store::NO));
  return doc;
}

shared_ptr<Document>
WriteLineDocTaskTest::EmptyDocMaker::makeDocument() 
{
  return make_shared<Document>();
}

shared_ptr<Document>
WriteLineDocTaskTest::ThreadingDocMaker::makeDocument() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  wstring name = Thread::currentThread().getName();
  doc->push_back(
      make_shared<StringField>(BODY_FIELD, L"body_" + name, Field::Store::NO));
  doc->push_back(make_shared<StringField>(TITLE_FIELD, L"title_" + name,
                                          Field::Store::NO));
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date_" + name, Field::Store::NO));
  return doc;
}

const shared_ptr<
    org::apache::commons::compress::compressors::CompressorStreamFactory>
    WriteLineDocTaskTest::csFactory = make_shared<
        org::apache::commons::compress::compressors::CompressorStreamFactory>();

shared_ptr<PerfRunData> WriteLineDocTaskTest::createPerfRunData(
    shared_ptr<Path> file, bool allowEmptyDocs,
    const wstring &docMakerName) 
{
  shared_ptr<Properties> props = make_shared<Properties>();
  props->setProperty(L"doc.maker", docMakerName);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"line.file.out", file->toAbsolutePath()->toString());
  props->setProperty(L"directory", L"RAMDirectory"); // no accidental FS dir.
  if (allowEmptyDocs) {
    props->setProperty(L"sufficient.fields", L",");
  }
  if (docMakerName == LegalJustDateDocMaker::typeid->getName()) {
    props->setProperty(L"line.fields", DocMaker::DATE_FIELD);
    props->setProperty(L"sufficient.fields", DocMaker::DATE_FIELD);
  }
  shared_ptr<Config> config = make_shared<Config>(props);
  return make_shared<PerfRunData>(config);
}

void WriteLineDocTaskTest::doReadTest(
    shared_ptr<Path> file, Type fileType, const wstring &expTitle,
    const wstring &expDate, const wstring &expBody) 
{
  shared_ptr<InputStream> in_ = Files::newInputStream(file);
  switch (fileType.innerEnumValue) {
  case Type::InnerEnum::BZIP2:
    in_ = csFactory->createCompressorInputStream(CompressorStreamFactory::BZIP2,
                                                 in_);
    break;
  case Type::InnerEnum::GZIP:
    in_ = csFactory->createCompressorInputStream(CompressorStreamFactory::GZIP,
                                                 in_);
    break;
  case Type::InnerEnum::PLAIN:
    break; // nothing to do
  default:
    assertFalse(L"Unknown file type!", true); // fail, should not happen
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader br = new
  // java.io.BufferedReader(new java.io.InputStreamReader(in,
  // java.nio.charset.StandardCharsets.UTF_8)))
  {
    java::io::BufferedReader br =
        java::io::BufferedReader(make_shared<java::io::InputStreamReader>(
            in_, java::nio::charset::StandardCharsets::UTF_8));
    wstring line = br.readLine();
    assertHeaderLine(line);
    line = br.readLine();
    assertNotNull(line);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::deque<wstring> parts =
        line.split(Character::toString(WriteLineDocTask::SEP));
    int numExpParts = expBody == L"" ? 2 : 3;
    assertEquals(numExpParts, parts.size());
    assertEquals(expTitle, parts[0]);
    assertEquals(expDate, parts[1]);
    if (expBody != L"") {
      assertEquals(expBody, parts[2]);
    }
    assertNull(br.readLine());
  }
}

void WriteLineDocTaskTest::assertHeaderLine(const wstring &line)
{
  assertTrue(L"First line should be a header line",
             StringHelper::startsWith(
                 line, WriteLineDocTask::FIELDS_HEADER_INDICATOR));
}

void WriteLineDocTaskTest::testBZip2() 
{

  // Create a document in bz2 format.
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line.bz2");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, WriteLineDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  doReadTest(file, Type::BZIP2, L"title", L"date", L"body");
}

void WriteLineDocTaskTest::testGZip() 
{

  // Create a document in gz format.
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line.gz");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, WriteLineDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  doReadTest(file, Type::GZIP, L"title", L"date", L"body");
}

void WriteLineDocTaskTest::testRegularFile() 
{

  // Create a document in regular format.
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, WriteLineDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  doReadTest(file, Type::PLAIN, L"title", L"date", L"body");
}

void WriteLineDocTaskTest::testCharsReplace() 
{
  // WriteLineDocTask replaced only \t characters w/ a space, since that's its
  // separator char. However, it didn't replace newline characters, which
  // resulted in errors in LineDocSource.
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, NewLinesDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  doReadTest(file, Type::PLAIN, L"title text", L"date text", L"body text two");
}

void WriteLineDocTaskTest::testEmptyBody() 
{
  // WriteLineDocTask threw away documents w/ no BODY element, even if they
  // had a TITLE element (LUCENE-1755). It should throw away documents if they
  // don't have BODY nor TITLE
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, NoBodyDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  doReadTest(file, Type::PLAIN, L"title", L"date", L"");
}

void WriteLineDocTaskTest::testEmptyTitle() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, NoTitleDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  doReadTest(file, Type::PLAIN, L"", L"date", L"body");
}

void WriteLineDocTaskTest::testJustDate() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, JustDateDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader br =
  // java.nio.file.Files.newBufferedReader(file,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::BufferedReader br = java::nio::file::Files::newBufferedReader(
        file, java::nio::charset::StandardCharsets::UTF_8);
    wstring line = br.readLine();
    assertHeaderLine(line);
    line = br.readLine();
    assertNull(line);
  }
}

void WriteLineDocTaskTest::testLegalJustDate() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, LegalJustDateDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader br =
  // java.nio.file.Files.newBufferedReader(file,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::BufferedReader br = java::nio::file::Files::newBufferedReader(
        file, java::nio::charset::StandardCharsets::UTF_8);
    wstring line = br.readLine();
    assertHeaderLine(line);
    line = br.readLine();
    assertNotNull(line);
  }
}

void WriteLineDocTaskTest::testEmptyDoc() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, true, EmptyDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt = make_shared<WriteLineDocTask>(runData);
  wldt->doLogic();
  wldt->close();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader br =
  // java.nio.file.Files.newBufferedReader(file,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::BufferedReader br = java::nio::file::Files::newBufferedReader(
        file, java::nio::charset::StandardCharsets::UTF_8);
    wstring line = br.readLine();
    assertHeaderLine(line);
    line = br.readLine();
    assertNotNull(line);
  }
}

void WriteLineDocTaskTest::testMultiThreaded() 
{
  shared_ptr<Path> file = getWorkDir()->resolve(L"one-line");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, false, ThreadingDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> *const wldt =
      make_shared<WriteLineDocTask>(runData);
  std::deque<std::shared_ptr<Thread>> threads(10);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), L"t" + to_wstring(i), wldt);
  }

  for (auto t : threads) {
    t->start();
  }
  for (auto t : threads) {
    t->join();
  }

  wldt->close();

  shared_ptr<Set<wstring>> ids = unordered_set<wstring>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader br =
  // java.nio.file.Files.newBufferedReader(file,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::BufferedReader br = java::nio::file::Files::newBufferedReader(
        file, java::nio::charset::StandardCharsets::UTF_8);
    wstring line = br.readLine();
    assertHeaderLine(line); // header line is written once, no matter how many
                            // threads there are
    for (int i = 0; i < threads.size(); i++) {
      line = br.readLine();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      std::deque<wstring> parts =
          line.split(Character::toString(WriteLineDocTask::SEP));
      assertEquals(3, parts.size());
      // check that all thread names written are the same in the same line
      wstring tname = parts[0].substr((int)parts[0].find(L'_'));
      ids->add(tname);
      assertEquals(tname, parts[1].substr((int)parts[1].find(L'_')));
      assertEquals(tname, parts[2].substr((int)parts[2].find(L'_')));
    }
    // only threads.length lines should exist
    assertNull(br.readLine());
    assertEquals(threads.size(), ids->size());
  }
}

WriteLineDocTaskTest::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<WriteLineDocTaskTest> outerInstance, wstring L"t" + i,
    shared_ptr<org::apache::lucene::benchmark::byTask::tasks::WriteLineDocTask>
        wldt)
    : Thread(L"t" + i)
{
  this->outerInstance = outerInstance;
  this->wldt = wldt;
}

void WriteLineDocTaskTest::ThreadAnonymousInnerClass::run()
{
  try {
    wldt->doLogic();
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::benchmark::byTask::tasks