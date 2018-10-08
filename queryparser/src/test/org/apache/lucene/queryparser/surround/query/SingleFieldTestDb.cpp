using namespace std;

#include "SingleFieldTestDb.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using TextField = org::apache::lucene::document::TextField;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;

SingleFieldTestDb::SingleFieldTestDb(shared_ptr<Random> random,
                                     std::deque<wstring> &documents,
                                     const wstring &fName)
{
  try {
    db = make_shared<MockDirectoryWrapper>(random, make_shared<RAMDirectory>());
    docs = documents;
    fieldName = fName;
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        db, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random)));
    for (int j = 0; j < docs.size(); j++) {
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(
          make_shared<TextField>(fieldName, docs[j], Field::Store::NO));
      writer->addDocument(d);
    }
    delete writer;
  } catch (const IOException &ioe) {
    throw make_shared<Error>(ioe);
  }
}

shared_ptr<Directory> SingleFieldTestDb::getDb() { return db; }

std::deque<wstring> SingleFieldTestDb::getDocs() { return docs; }

wstring SingleFieldTestDb::getFieldname() { return fieldName; }
} // namespace org::apache::lucene::queryparser::surround::query