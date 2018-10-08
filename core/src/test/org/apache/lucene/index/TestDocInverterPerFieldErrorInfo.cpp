using namespace std;

#include "TestDocInverterPerFieldErrorInfo.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using org::junit::Test;
const shared_ptr<org::apache::lucene::document::FieldType>
    TestDocInverterPerFieldErrorInfo::storedTextType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_NOT_STORED);

TestDocInverterPerFieldErrorInfo::BadNews::BadNews(const wstring &message)
    : RuntimeException(message)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDocInverterPerFieldErrorInfo::ThrowingAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  if (fieldName == L"distinctiveFieldName") {
    shared_ptr<TokenFilter> tosser =
        make_shared<TokenFilterAnonymousInnerClass>(shared_from_this(),
                                                    tokenizer);
    return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tosser);
  } else {
    return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
  }
}

TestDocInverterPerFieldErrorInfo::ThrowingAnalyzer::
    TokenFilterAnonymousInnerClass::TokenFilterAnonymousInnerClass(
        shared_ptr<ThrowingAnalyzer> outerInstance,
        shared_ptr<Tokenizer> tokenizer)
    : org::apache::lucene::analysis::TokenFilter(tokenizer)
{
  this->outerInstance = outerInstance;
}

bool TestDocInverterPerFieldErrorInfo::ThrowingAnalyzer::
    TokenFilterAnonymousInnerClass::incrementToken() 
{
  throw make_shared<BadNews>(L"Something is icky.");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testInfoStreamGetsFieldName() throws
// Exception
void TestDocInverterPerFieldErrorInfo::testInfoStreamGetsFieldName() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer;
  shared_ptr<IndexWriterConfig> c =
      make_shared<IndexWriterConfig>(make_shared<ThrowingAnalyzer>());
  shared_ptr<ByteArrayOutputStream> *const infoBytes =
      make_shared<ByteArrayOutputStream>();
  shared_ptr<PrintStream> infoPrintStream =
      make_shared<PrintStream>(infoBytes, true, IOUtils::UTF_8);
  shared_ptr<PrintStreamInfoStream> printStreamInfoStream =
      make_shared<PrintStreamInfoStream>(infoPrintStream);
  c->setInfoStream(printStreamInfoStream);
  writer = make_shared<IndexWriter>(dir, c);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"distinctiveFieldName", L"aaa ", storedTextType));
  expectThrows(BadNews::typeid, [&]() { writer->addDocument(doc); });
  infoPrintStream->flush();
  wstring infoStream = wstring(infoBytes->toByteArray(), IOUtils::UTF_8);
  assertTrue(infoStream.find(L"distinctiveFieldName") != wstring::npos);

  delete writer;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoExtraNoise() throws Exception
void TestDocInverterPerFieldErrorInfo::testNoExtraNoise() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer;
  shared_ptr<IndexWriterConfig> c =
      make_shared<IndexWriterConfig>(make_shared<ThrowingAnalyzer>());
  shared_ptr<ByteArrayOutputStream> *const infoBytes =
      make_shared<ByteArrayOutputStream>();
  shared_ptr<PrintStream> infoPrintStream =
      make_shared<PrintStream>(infoBytes, true, IOUtils::UTF_8);
  shared_ptr<PrintStreamInfoStream> printStreamInfoStream =
      make_shared<PrintStreamInfoStream>(infoPrintStream);
  c->setInfoStream(printStreamInfoStream);
  writer = make_shared<IndexWriter>(dir, c);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"boringFieldName", L"aaa ", storedTextType));
  try {
    writer->addDocument(doc);
  } catch (const BadNews &badNews) {
    fail(L"Unwanted exception");
  }
  infoPrintStream->flush();
  wstring infoStream = wstring(infoBytes->toByteArray(), IOUtils::UTF_8);
  assertFalse(infoStream.find(L"boringFieldName") != wstring::npos);

  delete writer;
  delete dir;
}
} // namespace org::apache::lucene::index