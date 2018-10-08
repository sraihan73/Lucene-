using namespace std;

#include "TestHighFrequencyDictionary.h"

namespace org::apache::lucene::search::suggest
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using HighFrequencyDictionary =
    org::apache::lucene::search::spell::HighFrequencyDictionary;
using Directory = org::apache::lucene::store::Directory;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestHighFrequencyDictionary::testEmpty() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->commit();
  delete writer;
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<HighFrequencyDictionary>(ir, L"bogus", 0.1f);
  shared_ptr<BytesRefIterator> tf = dictionary->getEntryIterator();
  assertNull(tf->next());
  delete dir;
}
} // namespace org::apache::lucene::search::suggest