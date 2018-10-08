using namespace std;

#include "WriteEnwikiLineDocTaskTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/WriteEnwikiLineDocTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/tasks/WriteLineDocTask.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"
#include "WriteLineDocTaskTest.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;

shared_ptr<Document>
WriteEnwikiLineDocTaskTest::WriteLineCategoryDocMaker::makeDocument() throw(
    runtime_error)
{
  bool isCategory = (flip->incrementAndGet() % 2 == 0);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(BODY_FIELD, L"body text", Field::Store::NO));
  doc->push_back(make_shared<StringField>(
      TITLE_FIELD, isCategory ? L"Category:title text" : L"title text",
      Field::Store::NO));
  doc->push_back(
      make_shared<StringField>(DATE_FIELD, L"date text", Field::Store::NO));
  return doc;
}

shared_ptr<PerfRunData> WriteEnwikiLineDocTaskTest::createPerfRunData(
    shared_ptr<Path> file, const wstring &docMakerName) 
{
  shared_ptr<Properties> props = make_shared<Properties>();
  props->setProperty(L"doc.maker", docMakerName);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"line.file.out", file->toAbsolutePath()->toString());
  props->setProperty(L"directory", L"RAMDirectory"); // no accidental FS dir.
  shared_ptr<Config> config = make_shared<Config>(props);
  return make_shared<PerfRunData>(config);
}

void WriteEnwikiLineDocTaskTest::doReadTest(
    shared_ptr<Path> file, const wstring &expTitle, const wstring &expDate,
    const wstring &expBody) 
{
  doReadTest(2, file, expTitle, expDate, expBody);
  shared_ptr<Path> categoriesFile =
      WriteEnwikiLineDocTask::categoriesLineFile(file);
  doReadTest(2, categoriesFile, L"Category:" + expTitle, expDate, expBody);
}

void WriteEnwikiLineDocTaskTest::doReadTest(
    int n, shared_ptr<Path> file, const wstring &expTitle,
    const wstring &expDate, const wstring &expBody) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader br =
  // java.nio.file.Files.newBufferedReader(file,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::BufferedReader br = java::nio::file::Files::newBufferedReader(
        file, java::nio::charset::StandardCharsets::UTF_8);
    wstring line = br.readLine();
    WriteLineDocTaskTest::assertHeaderLine(line);
    for (int i = 0; i < n; i++) {
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
    }
    assertNull(br.readLine());
  }
}

void WriteEnwikiLineDocTaskTest::testCategoryLines() 
{
  // WriteLineDocTask replaced only \t characters w/ a space, since that's its
  // separator char. However, it didn't replace newline characters, which
  // resulted in errors in LineDocSource.
  shared_ptr<Path> file = getWorkDir()->resolve(L"two-lines-each.txt");
  shared_ptr<PerfRunData> runData =
      createPerfRunData(file, WriteLineCategoryDocMaker::typeid->getName());
  shared_ptr<WriteLineDocTask> wldt =
      make_shared<WriteEnwikiLineDocTask>(runData);
  for (int i = 0; i < 4;
       i++) { // four times so that each file should have 2 lines.
    wldt->doLogic();
  }
  wldt->close();

  doReadTest(file, L"title text", L"date text", L"body text");
}
} // namespace org::apache::lucene::benchmark::byTask::tasks