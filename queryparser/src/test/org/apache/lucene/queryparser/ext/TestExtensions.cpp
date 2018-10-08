using namespace std;

#include "TestExtensions.h"

namespace org::apache::lucene::queryparser::ext
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestExtensions::setUp() 
{
  LuceneTestCase::setUp();
  this->ext = make_shared<Extensions>();
}

void TestExtensions::testBuildExtensionField()
{
  assertEquals(L"field\\:key", ext->buildExtensionField(L"key", L"field"));
  assertEquals(L"\\:key", ext->buildExtensionField(L"key"));

  ext = make_shared<Extensions>(L'.');
  assertEquals(L"field.key", ext->buildExtensionField(L"key", L"field"));
  assertEquals(L".key", ext->buildExtensionField(L"key"));
}

void TestExtensions::testSplitExtensionField()
{
  assertEquals(L"field\\:key", ext->buildExtensionField(L"key", L"field"));
  assertEquals(L"\\:key", ext->buildExtensionField(L"key"));

  ext = make_shared<Extensions>(L'.');
  assertEquals(L"field.key", ext->buildExtensionField(L"key", L"field"));
  assertEquals(L".key", ext->buildExtensionField(L"key"));
}

void TestExtensions::testAddGetExtension()
{
  shared_ptr<ParserExtension> extension = make_shared<ExtensionStub>();
  assertNull(ext->getExtension(L"foo"));
  ext->add(L"foo", extension);
  assertSame(extension, ext->getExtension(L"foo"));
  ext->add(L"foo", nullptr);
  assertNull(ext->getExtension(L"foo"));
}

void TestExtensions::testGetExtDelimiter()
{
  assertEquals(Extensions::DEFAULT_EXTENSION_FIELD_DELIMITER,
               this->ext->getExtensionFieldDelimiter());
  ext = make_shared<Extensions>(L'?');
  assertEquals(L'?', this->ext->getExtensionFieldDelimiter());
}

void TestExtensions::testEscapeExtension()
{
  assertEquals(L"abc\\:\\?\\{\\}\\[\\]\\\\\\(\\)\\+\\-\\!\\~",
               ext->escapeExtensionField(L"abc:?{}[]\\()+-!~"));
  // should throw NPE - escape string is null
  expectThrows(NullPointerException::typeid,
               [&]() { ext->escapeExtensionField(L""); });
}
} // namespace org::apache::lucene::queryparser::ext