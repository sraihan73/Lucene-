using namespace std;

#include "CoreParserTestIndexData.h"

namespace org::apache::lucene::queryparser::xml
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IntPoint = org::apache::lucene::document::IntPoint;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

CoreParserTestIndexData::CoreParserTestIndexData(
    shared_ptr<Analyzer> analyzer) 
    : dir(LuceneTestCase::newDirectory()), reader(DirectoryReader::open(dir)),
      searcher(LuceneTestCase::newSearcher(reader, false))
{
  shared_ptr<BufferedReader> d =
      make_shared<BufferedReader>(make_shared<InputStreamReader>(
          TestCoreParser::typeid->getResourceAsStream(L"reuters21578.txt"),
          StandardCharsets::US_ASCII));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, LuceneTestCase::newIndexWriterConfig(analyzer));
  wstring line = d->readLine();
  while (line != L"") {
    int endOfDate = (int)line.find(L'\t');
    wstring date = line.substr(0, endOfDate)->trim();
    wstring content = line.substr(endOfDate)->trim();
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        LuceneTestCase::newTextField(L"date", date, Field::Store::YES));
    doc->push_back(
        LuceneTestCase::newTextField(L"contents", content, Field::Store::YES));
    doc->push_back(make_shared<IntPoint>(L"date3", stoi(date)));
    writer->addDocument(doc);
    line = d->readLine();
  }
  d->close();
  delete writer;
}

CoreParserTestIndexData::~CoreParserTestIndexData()
{
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::queryparser::xml