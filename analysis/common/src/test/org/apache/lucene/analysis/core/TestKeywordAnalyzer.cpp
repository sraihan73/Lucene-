using namespace std;

#include "TestKeywordAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/SimpleAnalyzer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestKeywordAnalyzer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  directory = newDirectory();
  analyzer = make_shared<SimpleAnalyzer>();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, make_shared<IndexWriterConfig>(analyzer));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"partnum", L"Q36", Field::Store::YES));
  doc->push_back(make_shared<TextField>(
      L"description", L"Illidium Space Modulator", Field::Store::YES));
  writer->addDocument(doc);

  delete writer;

  reader = DirectoryReader::open(directory);
}

void TestKeywordAnalyzer::tearDown() 
{
  IOUtils::close({analyzer, reader, directory});
  BaseTokenStreamTestCase::tearDown();
}

void TestKeywordAnalyzer::testMutipleDocument() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();
  shared_ptr<Analyzer> analyzer = make_shared<KeywordAnalyzer>();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"partnum", L"Q36", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"partnum", L"Q37", Field::Store::YES));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<PostingsEnum> td = TestUtil::docs(
      random(), reader, L"partnum", make_shared<BytesRef>(L"Q36"), nullptr, 0);
  assertTrue(td->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  td = TestUtil::docs(random(), reader, L"partnum",
                      make_shared<BytesRef>(L"Q37"), nullptr, 0);
  assertTrue(td->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  delete analyzer;
}

void TestKeywordAnalyzer::testOffsets() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new KeywordAnalyzer(); org.apache.lucene.analysis.TokenStream stream =
  // analyzer.tokenStream("field", new java.io.StringReader("abcd")))
  {
    org::apache::lucene::analysis::Analyzer analyzer = KeywordAnalyzer();
    org::apache::lucene::analysis::TokenStream stream = analyzer->tokenStream(
        L"field", make_shared<java::io::StringReader>(L"abcd"));
    shared_ptr<OffsetAttribute> offsetAtt =
        stream->addAttribute(OffsetAttribute::typeid);
    stream->reset();
    assertTrue(stream->incrementToken());
    TestUtil::assertEquals(0, offsetAtt->startOffset());
    TestUtil::assertEquals(4, offsetAtt->endOffset());
    assertFalse(stream->incrementToken());
    stream->end();
  }
}

void TestKeywordAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<KeywordAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::core