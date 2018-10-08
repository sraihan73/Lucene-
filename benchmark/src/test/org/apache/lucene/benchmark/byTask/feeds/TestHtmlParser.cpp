using namespace std;

#include "TestHtmlParser.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DemoHTMLParser.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Parser =
    org::apache::lucene::benchmark::byTask::feeds::DemoHTMLParser::Parser;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestHtmlParser::testUnicode() 
{
  wstring text = L"<html><body>汉语</body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"汉语", parser->body);
}

void TestHtmlParser::testEntities() 
{
  wstring text = L"<html><body>&#x6C49;&#x8BED;&yen;</body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"汉语¥", parser->body);
}

void TestHtmlParser::testComments() 
{
  wstring text = L"<html><body>foo<!-- bar --><! baz --></body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"foo", parser->body);
}

void TestHtmlParser::testScript() 
{
  wstring text = wstring(L"<html><body><script type=\"text/javascript\">") +
                 L"document.write(\"test\")</script>foo</body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"foo", parser->body);
}

void TestHtmlParser::testStyle() 
{
  wstring text = wstring(L"<html><head><style type=\"text/css\">") +
                 L"body{background-color:blue;}</style>" +
                 L"</head><body>foo</body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"foo", parser->body);
}

void TestHtmlParser::testDoctype() 
{
  wstring text = wstring(L"<!DOCTYPE HTML PUBLIC ") +
                 L"\"-//W3C//DTD HTML 4.01 Transitional//EN\"" +
                 L"\"http://www.w3.org/TR/html4/loose.dtd\">" +
                 L"<html><body>foo</body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"foo", parser->body);
}

void TestHtmlParser::testMeta() 
{
  wstring text = wstring(L"<html><head>") +
                 L"<meta name=\"a\" content=\"1\" />" +
                 L"<meta name=\"b\" content=\"2\" />" +
                 L"<meta name=\"keywords\" content=\"this is a test\" />" +
                 L"<meta http-equiv=\"Content-Type\" "
                 L"content=\"text/html;charset=UTF-8\" />" +
                 L"</head><body>foobar</body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  shared_ptr<Properties> tags = parser->metaTags;
  assertEquals(4, tags->size());
  assertEquals(L"1", tags->get(L"a"));
  assertEquals(L"2", tags->get(L"b"));
  assertEquals(L"this is a test", tags->get(L"keywords"));
  assertEquals(L"text/html;charset=UTF-8", tags->get(L"content-type"));
}

void TestHtmlParser::testTitle() 
{
  wstring text = L"<html><head><TITLE>foo</TITLE><head><body>bar</body></html>";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"foo", parser->title);
}

void TestHtmlParser::testTurkish() 
{
  shared_ptr<Locale> *const saved = Locale::getDefault();
  try {
    Locale::setDefault(make_shared<Locale>(L"tr", L"TR"));
    wstring text = wstring(L"<html><HEAD><TITLE>ııı</TITLE></head><body>") +
                   L"<IMG SRC=\"../images/head.jpg\" WIDTH=570 HEIGHT=47 "
                   L"BORDER=0 ALT=\"ş\">" +
                   L"<a title=\"(ııı)\"></body></html>";
    shared_ptr<Parser> parser =
        make_shared<Parser>(make_shared<StringReader>(text));
    assertEquals(L"ııı", parser->title);
    assertEquals(L"[ş]", parser->body);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    Locale::setDefault(saved);
  }
}

void TestHtmlParser::testSampleTRECDoc() 
{
  wstring text = wstring(L"<html>\r\n") + L"\r\n" + L"<head>\r\n" +
                 L"<title>\r\n" + L"TEST-000 title\r\n" + L"</title>\r\n" +
                 L"</head>\r\n" + L"\r\n" + L"<body>\r\n" +
                 L"TEST-000 text\r\n" + L"\r\n" + L"</body>\r\n" + L"\r\n";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"TEST-000 title", parser->title);
  assertEquals(L"TEST-000 text", StringHelper::trim(parser->body));
}

void TestHtmlParser::testNoHTML() 
{
  wstring text = L"hallo";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"", parser->title);
  assertEquals(L"hallo", parser->body);
}

void TestHtmlParser::testivalid() 
{
  wstring text = L"<title>foo</title>bar";
  shared_ptr<Parser> parser =
      make_shared<Parser>(make_shared<StringReader>(text));
  assertEquals(L"foo", parser->title);
  assertEquals(L"bar", parser->body);
}
} // namespace org::apache::lucene::benchmark::byTask::feeds