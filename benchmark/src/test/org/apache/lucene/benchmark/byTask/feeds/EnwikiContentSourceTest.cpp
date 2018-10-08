using namespace std;

#include "EnwikiContentSourceTest.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

EnwikiContentSourceTest::StringableEnwikiSource::StringableEnwikiSource(
    const wstring &docs)
    : docs(docs)
{
}

shared_ptr<InputStream>
EnwikiContentSourceTest::StringableEnwikiSource::openInputStream() throw(
    IOException)
{
  return make_shared<ByteArrayInputStream>(
      docs.getBytes(StandardCharsets::UTF_8));
}

void EnwikiContentSourceTest::assertDocData(
    shared_ptr<DocData> dd, const wstring &expName, const wstring &expTitle,
    const wstring &expBody, const wstring &expDate) 
{
  assertNotNull(dd);
  assertEquals(expName, dd->getName());
  assertEquals(expTitle, dd->getTitle());
  assertEquals(expBody, dd->getBody());
  assertEquals(expDate, dd->getDate());
}

void EnwikiContentSourceTest::assertNoMoreDataException(
    shared_ptr<EnwikiContentSource> stdm) 
{
  expectThrows(NoMoreDataException::typeid,
               [&]() { stdm->getNextDocData(nullptr); });
}

const wstring EnwikiContentSourceTest::PAGE1 =
    wstring(L"  <page>\r\n") + L"    <title>Title1</title>\r\n" +
    L"    <ns>0</ns>\r\n" + L"    <id>1</id>\r\n" + L"    <revision>\r\n" +
    L"      <id>11</id>\r\n" + L"      <parentid>111</parentid>\r\n" +
    L"      <timestamp>2011-09-14T11:35:09Z</timestamp>\r\n" +
    L"      <contributor>\r\n" + L"      <username>Mister1111</username>\r\n" +
    L"        <id>1111</id>\r\n" + L"      </contributor>\r\n" +
    L"      <minor />\r\n" + L"      <comment>/* Never mind */</comment>\r\n" +
    L"      <text>Some text 1 here</text>\r\n" + L"    </revision>\r\n" +
    L"  </page>\r\n";
const wstring EnwikiContentSourceTest::PAGE2 =
    wstring(L"  <page>\r\n") + L"    <title>Title2</title>\r\n" +
    L"    <ns>0</ns>\r\n" + L"    <id>2</id>\r\n" + L"    <revision>\r\n" +
    L"      <id>22</id>\r\n" + L"      <parentid>222</parentid>\r\n" +
    L"      <timestamp>2022-09-14T22:35:09Z</timestamp>\r\n" +
    L"      <contributor>\r\n" + L"      <username>Mister2222</username>\r\n" +
    L"        <id>2222</id>\r\n" + L"      </contributor>\r\n" +
    L"      <minor />\r\n" + L"      <comment>/* Never mind */</comment>\r\n" +
    L"      <text>Some text 2 here</text>\r\n" + L"    </revision>\r\n" +
    L"  </page>\r\n";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOneDocument() throws Exception
void EnwikiContentSourceTest::testOneDocument() 
{
  wstring docs = L"<mediawiki>\r\n" + PAGE1 + L"</mediawiki>";

  shared_ptr<EnwikiContentSource> source = createContentSource(docs, false);

  shared_ptr<DocData> dd = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd, L"1", L"Title1", L"Some text 1 here",
                L"14-SEP-2011 11:35:09.000");

  assertNoMoreDataException(source);
}

shared_ptr<EnwikiContentSource>
EnwikiContentSourceTest::createContentSource(const wstring &docs,
                                             bool forever) 
{

  shared_ptr<Properties> props = make_shared<Properties>();
  props->setProperty(L"print.props", L"false");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  props->setProperty(L"content.source.forever", Boolean::toString(forever));
  shared_ptr<Config> config = make_shared<Config>(props);

  shared_ptr<EnwikiContentSource> source =
      make_shared<StringableEnwikiSource>(docs);
  source->setConfig(config);

  // doc-maker just for initiating content source inputs
  shared_ptr<DocMaker> docMaker = make_shared<DocMaker>();
  docMaker->setConfig(config, source);
  docMaker->resetInputs();
  return source;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTwoDocuments() throws Exception
void EnwikiContentSourceTest::testTwoDocuments() 
{
  wstring docs = L"<mediawiki>\r\n" + PAGE1 + PAGE2 + L"</mediawiki>";

  shared_ptr<EnwikiContentSource> source = createContentSource(docs, false);

  shared_ptr<DocData> dd1 = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd1, L"1", L"Title1", L"Some text 1 here",
                L"14-SEP-2011 11:35:09.000");

  shared_ptr<DocData> dd2 = source->getNextDocData(make_shared<DocData>());
  assertDocData(dd2, L"2", L"Title2", L"Some text 2 here",
                L"14-SEP-2022 22:35:09.000");

  assertNoMoreDataException(source);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testForever() throws Exception
void EnwikiContentSourceTest::testForever() 
{
  wstring docs = L"<mediawiki>\r\n" + PAGE1 + PAGE2 + L"</mediawiki>";

  shared_ptr<EnwikiContentSource> source = createContentSource(docs, true);

  // same documents several times
  for (int i = 0; i < 3; i++) {
    shared_ptr<DocData> dd1 = source->getNextDocData(make_shared<DocData>());
    assertDocData(dd1, L"1", L"Title1", L"Some text 1 here",
                  L"14-SEP-2011 11:35:09.000");

    shared_ptr<DocData> dd2 = source->getNextDocData(make_shared<DocData>());
    assertDocData(dd2, L"2", L"Title2", L"Some text 2 here",
                  L"14-SEP-2022 22:35:09.000");
    // Don't test that NoMoreDataException is thrown, since the forever flag is
    // turned on.
  }

  delete source;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds