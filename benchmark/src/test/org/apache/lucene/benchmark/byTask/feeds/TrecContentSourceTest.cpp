using namespace std;

#include "TrecContentSourceTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/DateTools.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DemoHTMLParser.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/TrecDocParser.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/TrecParserByPath.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using ParsePathType =
    org::apache::lucene::benchmark::byTask::feeds::TrecDocParser::ParsePathType;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using DateTools = org::apache::lucene::document::DateTools;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

TrecContentSourceTest::StringableTrecSource::StringableTrecSource(
    const wstring &docs, bool forever)
{
  this->docs = docs;
  this->forever = forever;
}

void TrecContentSourceTest::StringableTrecSource::openNextFile() throw(
    NoMoreDataException, IOException)
{
  if (reader != nullptr) {
    if (!forever) {
      throw make_shared<NoMoreDataException>();
    }
    ++iteration;
  }

  reader = make_shared<BufferedReader>(make_shared<StringReader>(docs));
}

void TrecContentSourceTest::StringableTrecSource::setConfig(
    shared_ptr<Config> config)
{
  htmlParser = make_shared<DemoHTMLParser>();
}

void TrecContentSourceTest::assertDocData(shared_ptr<DocData> dd,
                                          const wstring &expName,
                                          const wstring &expTitle,
                                          const wstring &expBody,
                                          Date expDate) 
{
  assertNotNull(dd);
  TestUtil::assertEquals(expName, dd->getName());
  TestUtil::assertEquals(expTitle, dd->getTitle());
  assertTrue(dd->getBody().find(expBody) != wstring::npos);
  Date date =
      dd->getDate() != L"" ? DateTools::stringToDate(dd->getDate()) : nullptr;
  TestUtil::assertEquals(expDate, date);
}

void TrecContentSourceTest::assertNoMoreDataException(
    shared_ptr<StringableTrecSource> stdm) 
{
  expectThrows(NoMoreDataException::typeid,
               [&]() { stdm->getNextDocData(nullptr); });
}

void TrecContentSourceTest::testOneDocument() 
{
  wstring docs =
      wstring(L"<DOC>\r\n") + L"<DOCNO>TEST-000</DOCNO>\r\n" + L"<DOCHDR>\r\n" +
      L"http://lucene.apache.org.trecdocmaker.test\r\n" +
      L"HTTP/1.1 200 OK\r\n" + L"Date: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Server: Apache/1.3.27 (Unix)\r\n" +
      L"Last-Modified: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Content-Length: 614\r\n" + L"Connection: close\r\n" +
      L"Content-Type: text/html\r\n" + L"</DOCHDR>\r\n" + L"<html>\r\n" +
      L"\r\n" + L"<head>\r\n" + L"<title>\r\n" + L"TEST-000 title\r\n" +
      L"</title>\r\n" + L"</head>\r\n" + L"\r\n" + L"<body>\r\n" +
      L"TEST-000 text\r\n" + L"\r\n" + L"</body>\r\n" + L"\r\n" + L"</DOC>";
  shared_ptr<StringableTrecSource> source =
      make_shared<StringableTrecSource>(docs, false);
  source->setConfig(nullptr);

  shared_ptr<DocData> dd = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd, L"TEST-000_0", L"TEST-000 title", L"TEST-000 text",
                source->parseDate(L"Sun, 11 Jan 2009 08:00:00 GMT"));

  assertNoMoreDataException(source);
}

void TrecContentSourceTest::testTwoDocuments() 
{
  wstring docs =
      wstring(L"<DOC>\r\n") + L"<DOCNO>TEST-000</DOCNO>\r\n" + L"<DOCHDR>\r\n" +
      L"http://lucene.apache.org.trecdocmaker.test\r\n" +
      L"HTTP/1.1 200 OK\r\n" + L"Date: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Server: Apache/1.3.27 (Unix)\r\n" +
      L"Last-Modified: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Content-Length: 614\r\n" + L"Connection: close\r\n" +
      L"Content-Type: text/html\r\n" + L"</DOCHDR>\r\n" + L"<html>\r\n" +
      L"\r\n" + L"<head>\r\n" + L"<title>\r\n" + L"TEST-000 title\r\n" +
      L"</title>\r\n" + L"</head>\r\n" + L"\r\n" + L"<body>\r\n" +
      L"TEST-000 text\r\n" + L"\r\n" + L"</body>\r\n" + L"\r\n" +
      L"</DOC>\r\n" + L"<DOC>\r\n" + L"<DOCNO>TEST-001</DOCNO>\r\n" +
      L"<DOCHDR>\r\n" + L"http://lucene.apache.org.trecdocmaker.test\r\n" +
      L"HTTP/1.1 200 OK\r\n" + L"Date: Sun, 11 Jan 2009 08:01:00 GMT\r\n" +
      L"Server: Apache/1.3.27 (Unix)\r\n" +
      L"Last-Modified: Sun, 11 Jan 2008 08:01:00 GMT\r\n" +
      L"Content-Length: 614\r\n" + L"Connection: close\r\n" +
      L"Content-Type: text/html\r\n" + L"</DOCHDR>\r\n" + L"<html>\r\n" +
      L"\r\n" + L"<head>\r\n" + L"<title>\r\n" + L"TEST-001 title\r\n" +
      L"</title>\r\n" +
      L"<meta name=\"date\" content=\"Tue&#44; 09 Dec 2003 22&#58;39&#58;08 "
      L"GMT\">" +
      L"</head>\r\n" + L"\r\n" + L"<body>\r\n" + L"TEST-001 text\r\n" +
      L"\r\n" + L"</body>\r\n" + L"\r\n" + L"</DOC>";
  shared_ptr<StringableTrecSource> source =
      make_shared<StringableTrecSource>(docs, false);
  source->setConfig(nullptr);

  shared_ptr<DocData> dd = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd, L"TEST-000_0", L"TEST-000 title", L"TEST-000 text",
                source->parseDate(L"Sun, 11 Jan 2009 08:00:00 GMT"));

  dd = source->getNextDocData(dd);
  assertDocData(dd, L"TEST-001_0", L"TEST-001 title", L"TEST-001 text",
                source->parseDate(L"Tue, 09 Dec 2003 22:39:08 GMT"));

  assertNoMoreDataException(source);
}

void TrecContentSourceTest::testMissingDate() 
{
  wstring docs =
      wstring(L"<DOC>\r\n") + L"<DOCNO>TEST-000</DOCNO>\r\n" + L"<DOCHDR>\r\n" +
      L"http://lucene.apache.org.trecdocmaker.test\r\n" +
      L"HTTP/1.1 200 OK\r\n" + L"Server: Apache/1.3.27 (Unix)\r\n" +
      L"Last-Modified: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Content-Length: 614\r\n" + L"Connection: close\r\n" +
      L"Content-Type: text/html\r\n" + L"</DOCHDR>\r\n" + L"<html>\r\n" +
      L"\r\n" + L"<head>\r\n" + L"<title>\r\n" + L"TEST-000 title\r\n" +
      L"</title>\r\n" + L"</head>\r\n" + L"\r\n" + L"<body>\r\n" +
      L"TEST-000 text\r\n" + L"\r\n" + L"</body>\r\n" + L"\r\n" +
      L"</DOC>\r\n" + L"<DOC>\r\n" + L"<DOCNO>TEST-001</DOCNO>\r\n" +
      L"<DOCHDR>\r\n" + L"http://lucene.apache.org.trecdocmaker.test\r\n" +
      L"HTTP/1.1 200 OK\r\n" + L"Date: Sun, 11 Jan 2009 08:01:00 GMT\r\n" +
      L"Server: Apache/1.3.27 (Unix)\r\n" +
      L"Last-Modified: Sun, 11 Jan 2009 08:01:00 GMT\r\n" +
      L"Content-Length: 614\r\n" + L"Connection: close\r\n" +
      L"Content-Type: text/html\r\n" + L"</DOCHDR>\r\n" + L"<html>\r\n" +
      L"\r\n" + L"<head>\r\n" + L"<title>\r\n" + L"TEST-001 title\r\n" +
      L"</title>\r\n" + L"</head>\r\n" + L"\r\n" + L"<body>\r\n" +
      L"TEST-001 text\r\n" + L"\r\n" + L"</body>\r\n" + L"\r\n" + L"</DOC>";
  shared_ptr<StringableTrecSource> source =
      make_shared<StringableTrecSource>(docs, false);
  source->setConfig(nullptr);

  shared_ptr<DocData> dd = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd, L"TEST-000_0", L"TEST-000 title", L"TEST-000 text",
                nullptr);

  dd = source->getNextDocData(dd);
  assertDocData(dd, L"TEST-001_0", L"TEST-001 title", L"TEST-001 text",
                source->parseDate(L"Sun, 11 Jan 2009 08:01:00 GMT"));

  assertNoMoreDataException(source);
}

void TrecContentSourceTest::testBadDate() 
{
  wstring docs =
      wstring(L"<DOC>\r\n") + L"<DOCNO>TEST-000</DOCNO>\r\n" + L"<DOCHDR>\r\n" +
      L"http://lucene.apache.org.trecdocmaker.test\r\n" +
      L"HTTP/1.1 200 OK\r\n" + L"Date: Bad Date\r\n" +
      L"Server: Apache/1.3.27 (Unix)\r\n" +
      L"Last-Modified: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Content-Length: 614\r\n" + L"Connection: close\r\n" +
      L"Content-Type: text/html\r\n" + L"</DOCHDR>\r\n" + L"<html>\r\n" +
      L"\r\n" + L"<head>\r\n" + L"<title>\r\n" + L"TEST-000 title\r\n" +
      L"</title>\r\n" + L"</head>\r\n" + L"\r\n" + L"<body>\r\n" +
      L"TEST-000 text\r\n" + L"\r\n" + L"</body>\r\n" + L"\r\n" + L"</DOC>";
  shared_ptr<StringableTrecSource> source =
      make_shared<StringableTrecSource>(docs, false);
  source->setConfig(nullptr);

  shared_ptr<DocData> dd = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd, L"TEST-000_0", L"TEST-000 title", L"TEST-000 text",
                nullptr);

  assertNoMoreDataException(source);
}

void TrecContentSourceTest::testForever() 
{
  wstring docs =
      wstring(L"<DOC>\r\n") + L"<DOCNO>TEST-000</DOCNO>\r\n" + L"<DOCHDR>\r\n" +
      L"http://lucene.apache.org.trecdocmaker.test\r\n" +
      L"HTTP/1.1 200 OK\r\n" + L"Date: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Server: Apache/1.3.27 (Unix)\r\n" +
      L"Last-Modified: Sun, 11 Jan 2009 08:00:00 GMT\r\n" +
      L"Content-Length: 614\r\n" + L"Connection: close\r\n" +
      L"Content-Type: text/html\r\n" + L"</DOCHDR>\r\n" + L"<html>\r\n" +
      L"\r\n" + L"<head>\r\n" + L"<title>\r\n" + L"TEST-000 title\r\n" +
      L"</title>\r\n" + L"</head>\r\n" + L"\r\n" + L"<body>\r\n" +
      L"TEST-000 text\r\n" + L"\r\n" + L"</body>\r\n" + L"\r\n" + L"</DOC>";
  shared_ptr<StringableTrecSource> source =
      make_shared<StringableTrecSource>(docs, true);
  source->setConfig(nullptr);

  shared_ptr<DocData> dd = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd, L"TEST-000_0", L"TEST-000 title", L"TEST-000 text",
                source->parseDate(L"Sun, 11 Jan 2009 08:00:00 GMT"));

  // same document, but the second iteration changes the name.
  dd = source->getNextDocData(dd);
  assertDocData(dd, L"TEST-000_1", L"TEST-000 title", L"TEST-000 text",
                source->parseDate(L"Sun, 11 Jan 2009 08:00:00 GMT"));
  delete source;

  // Don't test that NoMoreDataException is thrown, since the forever flag is
  // turned on.
}

void TrecContentSourceTest::testTrecFeedDirAllTypes() 
{
  shared_ptr<Path> dataDir = createTempDir(L"trecFeedAllTypes");
  TestUtil::unzip(getDataInputStream(L"trecdocs.zip"), dataDir);
  shared_ptr<TrecContentSource> tcs = make_shared<TrecContentSource>();
  shared_ptr<Properties> props = make_shared<Properties>();
  props->setProperty(L"print.props", L"false");
  props->setProperty(L"content.source.verbose", L"false");
  props->setProperty(L"content.source.excludeIteration", L"true");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"docs.dir",
                     dataDir->toRealPath()->toString()->replace(L'\\', L'/'));
  props->setProperty(L"trec.doc.parser", TrecParserByPath::typeid->getName());
  props->setProperty(L"content.source.forever", L"false");
  tcs->setConfig(make_shared<Config>(props));
  tcs->resetInputs();
  shared_ptr<DocData> dd = make_shared<DocData>();
  int n = 0;
  bool gotExpectedException = false;
  unordered_set<ParsePathType> unseenTypes =
      unordered_set<ParsePathType>(Arrays::asList(ParsePathType::values()));
  try {
    while (n < 100) { // arbiterary limit to prevent looping forever in case of
                      // test failure
      dd = tcs->getNextDocData(dd);
      ++n;
      assertNotNull(L"doc data " + to_wstring(n) + L" should not be null!", dd);
      unseenTypes.remove(tcs->currPathType);
      switch (tcs->currPathType) {
      case ParsePathType::GOV2:
        assertDocData(dd, L"TEST-000", L"TEST-000 title", L"TEST-000 text",
                      tcs->parseDate(L"Sun, 11 Jan 2009 08:00:00 GMT"));
        break;
      case ParsePathType::FBIS:
        assertDocData(dd, L"TEST-001", L"TEST-001 Title", L"TEST-001 text",
                      tcs->parseDate(L"1 January 1991"));
        break;
      case ParsePathType::FR94:
        // no title extraction in this source for now
        assertDocData(dd, L"TEST-002", L"", L"DEPARTMENT OF SOMETHING",
                      tcs->parseDate(L"February 3, 1994"));
        break;
      case ParsePathType::FT:
        assertDocData(dd, L"TEST-003", L"Test-003 title", L"Some pub text",
                      tcs->parseDate(L"980424"));
        break;
      case ParsePathType::LATIMES:
        assertDocData(dd, L"TEST-004", L"Test-004 Title", L"Some paragraph",
                      tcs->parseDate(L"January 17, 1997, Sunday"));
        break;
      default:
        assertTrue(L"Should never get here!", false);
      }
    }
  } catch (const NoMoreDataException &e) {
    gotExpectedException = true;
  }
  assertTrue(L"Should have gotten NoMoreDataException!", gotExpectedException);
  assertEquals(L"Wrong number of documents created by source!", 5, n);
  assertTrue(L"Did not see all types!", unseenTypes.empty());
}
} // namespace org::apache::lucene::benchmark::byTask::feeds