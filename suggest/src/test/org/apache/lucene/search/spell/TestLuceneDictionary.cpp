using namespace std;

#include "TestLuceneDictionary.h"

namespace org::apache::lucene::search::spell
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestLuceneDictionary::setUp() 
{
  LuceneTestCase::setUp();
  store = newDirectory();
  analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(store, newIndexWriterConfig(analyzer));

  shared_ptr<Document> doc;

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"aaa", L"foo", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"aaa", L"foo", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"contents", L"Tom", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"contents", L"Jerry", Field::Store::YES));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"zzz", L"bar", Field::Store::YES));
  writer->addDocument(doc);

  writer->forceMerge(1);
  delete writer;
}

void TestLuceneDictionary::tearDown() 
{
  if (indexReader != nullptr) {
    delete indexReader;
  }
  delete store;
  delete analyzer;
  LuceneTestCase::tearDown();
}

void TestLuceneDictionary::testFieldNonExistent() 
{
  try {
    indexReader = DirectoryReader::open(store);

    ld = make_shared<LuceneDictionary>(indexReader, L"nonexistent_field");
    it = ld->getEntryIterator();

    assertNull(L"More elements than expected", spare = it->next());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}

void TestLuceneDictionary::testFieldAaa() 
{
  try {
    indexReader = DirectoryReader::open(store);

    ld = make_shared<LuceneDictionary>(indexReader, L"aaa");
    it = ld->getEntryIterator();
    assertNotNull(L"First element doesn't exist.", spare = it->next());
    assertTrue(L"First element isn't correct", spare->utf8ToString() == L"foo");
    assertNull(L"More elements than expected", it->next());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}

void TestLuceneDictionary::testFieldContents_1() 
{
  try {
    indexReader = DirectoryReader::open(store);

    ld = make_shared<LuceneDictionary>(indexReader, L"contents");
    it = ld->getEntryIterator();

    assertNotNull(L"First element doesn't exist.", spare = it->next());
    assertTrue(L"First element isn't correct",
               spare->utf8ToString() == L"Jerry");
    assertNotNull(L"Second element doesn't exist.", spare = it->next());
    assertTrue(L"Second element isn't correct",
               spare->utf8ToString() == L"Tom");
    assertNull(L"More elements than expected", it->next());

    ld = make_shared<LuceneDictionary>(indexReader, L"contents");
    it = ld->getEntryIterator();

    int counter = 2;
    while (it->next() != nullptr) {
      counter--;
    }

    assertTrue(L"Number of words incorrect", counter == 0);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}

void TestLuceneDictionary::testFieldContents_2() 
{
  try {
    indexReader = DirectoryReader::open(store);

    ld = make_shared<LuceneDictionary>(indexReader, L"contents");
    it = ld->getEntryIterator();

    // just iterate through words
    assertEquals(L"First element isn't correct", L"Jerry",
                 it->next()->utf8ToString());
    assertEquals(L"Second element isn't correct", L"Tom",
                 it->next()->utf8ToString());
    assertNull(L"Nonexistent element is really null", it->next());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}

void TestLuceneDictionary::testFieldZzz() 
{
  try {
    indexReader = DirectoryReader::open(store);

    ld = make_shared<LuceneDictionary>(indexReader, L"zzz");
    it = ld->getEntryIterator();

    assertNotNull(L"First element doesn't exist.", spare = it->next());
    assertEquals(L"First element isn't correct", L"bar", spare->utf8ToString());
    assertNull(L"More elements than expected", it->next());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (indexReader != nullptr) {
      delete indexReader;
    }
  }
}

void TestLuceneDictionary::testSpellchecker() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<SpellChecker> sc = make_shared<SpellChecker>(dir);
  indexReader = DirectoryReader::open(store);
  sc->indexDictionary(make_shared<LuceneDictionary>(indexReader, L"contents"),
                      newIndexWriterConfig(nullptr), false);
  std::deque<wstring> suggestions = sc->suggestSimilar(L"Tam", 1);
  assertEquals(1, suggestions.size());
  assertEquals(L"Tom", suggestions[0]);
  suggestions = sc->suggestSimilar(L"Jarry", 1);
  assertEquals(1, suggestions.size());
  assertEquals(L"Jerry", suggestions[0]);
  delete indexReader;
  delete sc;
  delete dir;
}
} // namespace org::apache::lucene::search::spell