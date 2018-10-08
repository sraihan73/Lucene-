using namespace std;

#include "BaseDocValuesFormatTestCase.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DocValuesStatus =
    org::apache::lucene::index::CheckIndex::Status::DocValuesStatus;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BitSet = org::apache::lucene::util::BitSet;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

void BaseDocValuesFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  if (usually()) {
    doc->push_back(
        make_shared<NumericDocValuesField>(L"ndv", random()->nextInt(1 << 12)));
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"bdv", make_shared<BytesRef>(TestUtil::randomSimpleString(random()))));
    doc->push_back(make_shared<SortedDocValuesField>(
        L"sdv",
        make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 2))));
  }
  int numValues = random()->nextInt(5);
  for (int i = 0; i < numValues; ++i) {
    doc->push_back(make_shared<SortedSetDocValuesField>(
        L"ssdv",
        make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 2))));
  }
  numValues = random()->nextInt(5);
  for (int i = 0; i < numValues; ++i) {
    doc->push_back(make_shared<SortedNumericDocValuesField>(
        L"sndv", TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                                    numeric_limits<int64_t>::max())));
  }
}

void BaseDocValuesFormatTestCase::testOneNumber() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 5));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    shared_ptr<Document> hitDoc = isearcher->doc(hits->scoreDocs[i]->doc);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<NumericDocValues> dv =
        ireader->leaves()[0]->reader().getNumericDocValues(L"dv");
    int docID = hits->scoreDocs[i]->doc;
    TestUtil::assertEquals(docID, dv->advance(docID));
    TestUtil::assertEquals(5, dv->longValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testOneFloat() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<FloatDocValuesField>(L"dv", 5.7f));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int docID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(docID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);

    shared_ptr<NumericDocValues> dv =
        ireader->leaves()[0]->reader().getNumericDocValues(L"dv");
    TestUtil::assertEquals(docID, dv->advance(docID));
    TestUtil::assertEquals(Float::floatToRawIntBits(5.7f), dv->longValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoNumbers() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 5));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv2", 17));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int docID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(docID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<NumericDocValues> dv =
        ireader->leaves()[0]->reader().getNumericDocValues(L"dv1");
    TestUtil::assertEquals(docID, dv->advance(docID));
    TestUtil::assertEquals(5, dv->longValue());
    dv = ireader->leaves()[0]->reader().getNumericDocValues(L"dv2");
    TestUtil::assertEquals(docID, dv->advance(docID));
    TestUtil::assertEquals(17, dv->longValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoBinaryValues() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv1", make_shared<BytesRef>(longTerm)));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv2", make_shared<BytesRef>(text)));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int hitDocID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(hitDocID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<BinaryDocValues> dv =
        ireader->leaves()[0]->reader().getBinaryDocValues(L"dv1");
    TestUtil::assertEquals(hitDocID, dv->advance(hitDocID));
    shared_ptr<BytesRef> scratch = dv->binaryValue();
    TestUtil::assertEquals(make_shared<BytesRef>(longTerm), scratch);
    dv = ireader->leaves()[0]->reader().getBinaryDocValues(L"dv2");
    TestUtil::assertEquals(hitDocID, dv->advance(hitDocID));
    scratch = dv->binaryValue();
    TestUtil::assertEquals(make_shared<BytesRef>(text), scratch);
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoFieldsMixed() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 5));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv2", make_shared<BytesRef>(L"hello world")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int docID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(docID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<NumericDocValues> dv =
        ireader->leaves()[0]->reader().getNumericDocValues(L"dv1");
    TestUtil::assertEquals(docID, dv->advance(docID));
    TestUtil::assertEquals(5, dv->longValue());
    shared_ptr<BinaryDocValues> dv2 =
        ireader->leaves()[0]->reader().getBinaryDocValues(L"dv2");
    TestUtil::assertEquals(docID, dv2->advance(docID));
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello world"),
                           dv2->binaryValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testThreeFieldsMixed() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv1", make_shared<BytesRef>(L"hello hello")));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv2", 5));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv3", make_shared<BytesRef>(L"hello world")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int docID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(docID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<SortedDocValues> dv =
        ireader->leaves()[0]->reader().getSortedDocValues(L"dv1");
    TestUtil::assertEquals(docID, dv->advance(docID));
    int ord = dv->ordValue();
    shared_ptr<BytesRef> scratch = dv->lookupOrd(ord);
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello hello"), scratch);
    shared_ptr<NumericDocValues> dv2 =
        ireader->leaves()[0]->reader().getNumericDocValues(L"dv2");
    TestUtil::assertEquals(docID, dv2->advance(docID));
    TestUtil::assertEquals(5, dv2->longValue());
    shared_ptr<BinaryDocValues> dv3 =
        ireader->leaves()[0]->reader().getBinaryDocValues(L"dv3");
    TestUtil::assertEquals(docID, dv3->advance(docID));
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello world"),
                           dv3->binaryValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testThreeFieldsMixed2() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv1", make_shared<BytesRef>(L"hello world")));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv2", make_shared<BytesRef>(L"hello hello")));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv3", 5));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int docID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(docID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<SortedDocValues> dv =
        ireader->leaves()[0]->reader().getSortedDocValues(L"dv2");
    TestUtil::assertEquals(docID, dv->advance(docID));
    int ord = dv->ordValue();
    scratch = dv->lookupOrd(ord);
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello hello"), scratch);
    shared_ptr<NumericDocValues> dv2 =
        ireader->leaves()[0]->reader().getNumericDocValues(L"dv3");
    TestUtil::assertEquals(docID, dv2->advance(docID));
    TestUtil::assertEquals(5, dv2->longValue());
    shared_ptr<BinaryDocValues> dv3 =
        ireader->leaves()[0]->reader().getBinaryDocValues(L"dv1");
    TestUtil::assertEquals(docID, dv3->advance(docID));
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello world"),
                           dv3->binaryValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoDocumentsNumeric() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 1));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 2));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<NumericDocValues> dv =
      ireader->leaves()[0]->reader().getNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(1, dv->longValue());
  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(2, dv->longValue());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoDocumentsMerged() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"id", L"0", StringField::TYPE_STORED));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", -10));
  iwriter->addDocument(doc);
  iwriter->commit();
  doc = make_shared<Document>();
  doc->push_back(newField(L"id", L"1", StringField::TYPE_STORED));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 99));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<NumericDocValues> dv =
      ireader->leaves()[0]->reader().getNumericDocValues(L"dv");
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc2 = ireader->leaves()[0]->reader().document(i);
    int64_t expected;
    if (doc2[L"id"].equals(L"0")) {
      expected = -10;
    } else {
      expected = 99;
    }
    TestUtil::assertEquals(i, dv->nextDoc());
    TestUtil::assertEquals(expected, dv->longValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testBigNumericRange() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(
      L"dv", numeric_limits<int64_t>::min()));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(
      L"dv", numeric_limits<int64_t>::max()));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<NumericDocValues> dv =
      ireader->leaves()[0]->reader().getNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(numeric_limits<int64_t>::min(), dv->longValue());
  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(numeric_limits<int64_t>::max(), dv->longValue());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testBigNumericRange2() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<NumericDocValuesField>(L"dv", -8841491950446638677LL));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<NumericDocValuesField>(L"dv", 9062230939892376225LL));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<NumericDocValues> dv =
      ireader->leaves()[0]->reader().getNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(-8841491950446638677LL, dv->longValue());
  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(9062230939892376225LL, dv->longValue());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int hitDocID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(hitDocID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<BinaryDocValues> dv =
        ireader->leaves()[0]->reader().getBinaryDocValues(L"dv");
    TestUtil::assertEquals(hitDocID, dv->advance(hitDocID));
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello world"),
                           dv->binaryValue());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testBytesTwoDocumentsMerged() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"id", L"0", StringField::TYPE_STORED));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 1")));
  iwriter->addDocument(doc);
  iwriter->commit();
  doc = make_shared<Document>();
  doc->push_back(newField(L"id", L"1", StringField::TYPE_STORED));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello 2")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<BinaryDocValues> dv =
      ireader->leaves()[0]->reader().getBinaryDocValues(L"dv");
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc2 = ireader->leaves()[0]->reader().document(i);
    wstring expected;
    if (doc2[L"id"].equals(L"0")) {
      expected = L"hello world 1";
    } else {
      expected = L"hello 2";
    }
    TestUtil::assertEquals(i, dv->nextDoc());
    TestUtil::assertEquals(expected, dv->binaryValue()->utf8ToString());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testBytesMergeAwayAllValues() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"field", make_shared<BytesRef>(L"hi")));
  iwriter->addDocument(doc);
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<BinaryDocValues> dv =
      getOnlyLeafReader(ireader)->getBinaryDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = make_shared<IndexSearcher>(ireader);

  TestUtil::assertEquals(
      1, isearcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"fieldname", longTerm)),
                      1)
             ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  TestUtil::assertEquals(1, hits->totalHits);
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    int docID = hits->scoreDocs[i]->doc;
    shared_ptr<Document> hitDoc = isearcher->doc(docID);
    TestUtil::assertEquals(text, hitDoc[L"fieldname"]);
    assert(ireader->leaves().size() == 1);
    shared_ptr<SortedDocValues> dv =
        ireader->leaves()[0]->reader().getSortedDocValues(L"dv");
    TestUtil::assertEquals(docID, dv->advance(docID));
    scratch = dv->lookupOrd(dv->ordValue());
    TestUtil::assertEquals(make_shared<BytesRef>(L"hello world"), scratch);
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedBytesTwoDocuments() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 1")));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 2")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<SortedDocValues> dv =
      ireader->leaves()[0]->reader().getSortedDocValues(L"dv");
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  TestUtil::assertEquals(0, dv->nextDoc());
  scratch = dv->lookupOrd(dv->ordValue());
  TestUtil::assertEquals(L"hello world 1", scratch->utf8ToString());
  TestUtil::assertEquals(1, dv->nextDoc());
  scratch = dv->lookupOrd(dv->ordValue());
  TestUtil::assertEquals(L"hello world 2", scratch->utf8ToString());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedBytesThreeDocuments() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 1")));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 2")));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 1")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<SortedDocValues> dv =
      ireader->leaves()[0]->reader().getSortedDocValues(L"dv");
  TestUtil::assertEquals(2, dv->getValueCount());
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->ordValue());
  shared_ptr<BytesRef> scratch = dv->lookupOrd(0);
  TestUtil::assertEquals(L"hello world 1", scratch->utf8ToString());
  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(1, dv->ordValue());
  scratch = dv->lookupOrd(1);
  TestUtil::assertEquals(L"hello world 2", scratch->utf8ToString());
  TestUtil::assertEquals(2, dv->nextDoc());
  TestUtil::assertEquals(0, dv->ordValue());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedBytesTwoDocumentsMerged() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"id", L"0", StringField::TYPE_STORED));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 1")));
  iwriter->addDocument(doc);
  iwriter->commit();
  doc = make_shared<Document>();
  doc->push_back(newField(L"id", L"1", StringField::TYPE_STORED));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 2")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<SortedDocValues> dv =
      ireader->leaves()[0]->reader().getSortedDocValues(L"dv");
  TestUtil::assertEquals(2, dv->getValueCount()); // 2 ords
  TestUtil::assertEquals(0, dv->nextDoc());
  shared_ptr<BytesRef> scratch = dv->binaryValue();
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello world 1"), scratch);
  scratch = dv->lookupOrd(1);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello world 2"), scratch);
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc2 = ireader->leaves()[0]->reader().document(i);
    wstring expected;
    if (doc2[L"id"].equals(L"0")) {
      expected = L"hello world 1";
    } else {
      expected = L"hello world 2";
    }
    if (dv->docID() < i) {
      TestUtil::assertEquals(i, dv->nextDoc());
    }
    scratch = dv->lookupOrd(dv->ordValue());
    TestUtil::assertEquals(expected, scratch->utf8ToString());
  }

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedMergeAwayAllValues() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testBytesWithNewline() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello\nworld\r1")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<BinaryDocValues> dv =
      ireader->leaves()[0]->reader().getBinaryDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello\nworld\r1"),
                         dv->binaryValue());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testMissingSortedBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"hello world 2")));
  iwriter->addDocument(doc);
  // 2nd doc missing the DV field
  iwriter->addDocument(make_shared<Document>());
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<SortedDocValues> dv =
      ireader->leaves()[0]->reader().getSortedDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  shared_ptr<BytesRef> scratch = dv->lookupOrd(dv->ordValue());
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello world 2"), scratch);
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());
  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedTermsEnum() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"field", make_shared<BytesRef>(L"world")));
  iwriter->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"field", make_shared<BytesRef>(L"beer")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedDocValues(L"field");
  TestUtil::assertEquals(3, dv->getValueCount());

  shared_ptr<TermsEnum> termsEnum = dv->termsEnum();

  // next()
  TestUtil::assertEquals(L"beer", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  TestUtil::assertEquals(L"hello", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  TestUtil::assertEquals(L"world", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());

  // seekCeil()
  TestUtil::assertEquals(SeekStatus::NOT_FOUND,
                         termsEnum->seekCeil(make_shared<BytesRef>(L"ha!")));
  TestUtil::assertEquals(L"hello", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  TestUtil::assertEquals(SeekStatus::FOUND,
                         termsEnum->seekCeil(make_shared<BytesRef>(L"beer")));
  TestUtil::assertEquals(L"beer", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  TestUtil::assertEquals(SeekStatus::END,
                         termsEnum->seekCeil(make_shared<BytesRef>(L"zzz")));
  TestUtil::assertEquals(SeekStatus::NOT_FOUND,
                         termsEnum->seekCeil(make_shared<BytesRef>(L"aba")));
  TestUtil::assertEquals(0, termsEnum->ord());

  // seekExact()
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"beer")));
  TestUtil::assertEquals(L"beer", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"hello")));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(Codec::getDefault()->toString(), L"hello",
               termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"world")));
  TestUtil::assertEquals(L"world", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());
  assertFalse(termsEnum->seekExact(make_shared<BytesRef>(L"bogus")));

  // seek(ord)
  termsEnum->seekExact(0);
  TestUtil::assertEquals(L"beer", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  termsEnum->seekExact(1);
  TestUtil::assertEquals(L"hello", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  termsEnum->seekExact(2);
  TestUtil::assertEquals(L"world", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());

  // NORMAL automaton
  termsEnum = dv->intersect(make_shared<CompiledAutomaton>(
      (make_shared<RegExp>(L".*l.*"))->toAutomaton()));
  TestUtil::assertEquals(L"hello", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  TestUtil::assertEquals(L"world", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());
  assertNull(termsEnum->next());

  // SINGLE automaton
  termsEnum = dv->intersect(make_shared<CompiledAutomaton>(
      (make_shared<RegExp>(L"hello"))->toAutomaton()));
  TestUtil::assertEquals(L"hello", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  assertNull(termsEnum->next());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testEmptySortedBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"")));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<SortedDocValues> dv =
      ireader->leaves()[0]->reader().getSortedDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->ordValue());
  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(0, dv->ordValue());
  shared_ptr<BytesRef> scratch = dv->lookupOrd(0);
  TestUtil::assertEquals(L"", scratch->utf8ToString());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testEmptyBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv", make_shared<BytesRef>(L"")));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv", make_shared<BytesRef>(L"")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<BinaryDocValues> dv =
      ireader->leaves()[0]->reader().getBinaryDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(L"", dv->binaryValue()->utf8ToString());
  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(L"", dv->binaryValue()->utf8ToString());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testVeryLargeButLegalBytes() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<char> bytes(32766);
  shared_ptr<BytesRef> b = make_shared<BytesRef>(bytes);
  random()->nextBytes(bytes);
  doc->push_back(make_shared<BinaryDocValuesField>(L"dv", b));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<BinaryDocValues> dv =
      ireader->leaves()[0]->reader().getBinaryDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(make_shared<BytesRef>(bytes), dv->binaryValue());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testVeryLargeButLegalSortedBytes() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<char> bytes(32766);
  shared_ptr<BytesRef> b = make_shared<BytesRef>(bytes);
  random()->nextBytes(bytes);
  doc->push_back(make_shared<SortedDocValuesField>(L"dv", b));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<BinaryDocValues> dv =
      DocValues::getBinary(ireader->leaves()[0]->reader(), L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(make_shared<BytesRef>(bytes), dv->binaryValue());
  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testCodecUsesOwnBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv", make_shared<BytesRef>(L"boo!")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<BinaryDocValues> dv =
      ireader->leaves()[0]->reader().getBinaryDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(L"boo!", dv->binaryValue()->utf8ToString());

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testCodecUsesOwnSortedBytes() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"boo!")));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  assert(ireader->leaves().size() == 1);
  shared_ptr<BinaryDocValues> dv =
      DocValues::getBinary(ireader->leaves()[0]->reader(), L"dv");
  std::deque<char> mybytes(20);
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(L"boo!", dv->binaryValue()->utf8ToString());
  assertFalse(dv->binaryValue()->bytes == mybytes);

  delete ireader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testDocValuesSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  for (int i = 0; i < 5; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"docId", i));
    doc->push_back(make_shared<TextField>(L"docId", L"" + to_wstring(i),
                                          Field::Store::NO));
    writer->addDocument(doc);
  }
  writer->commit();
  writer->forceMerge(1, true);

  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, reader->leaves()->size());

  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(L"docId", L"0")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"docId", L"1")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"docId", L"2")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"docId", L"3")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"docId", L"4")),
             BooleanClause::Occur::SHOULD);

  shared_ptr<TopDocs> search = searcher->search(query->build(), 10);
  TestUtil::assertEquals(5, search->totalHits);
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs = search->scoreDocs;
  shared_ptr<NumericDocValues> docValues =
      getOnlyLeafReader(reader)->getNumericDocValues(L"docId");
  for (int i = 0; i < scoreDocs.size(); i++) {
    TestUtil::assertEquals(i, scoreDocs[i]->doc);
    TestUtil::assertEquals(i, docValues->advance(i));
    TestUtil::assertEquals(i, docValues->longValue());
  }
  reader->close();
  delete dir;
}

void BaseDocValuesFormatTestCase::testRandomSortedBytes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> cfg =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, cfg);
  int numDocs = atLeast(100);
  shared_ptr<BytesRefHash> hash = make_shared<BytesRefHash>();
  unordered_map<wstring, wstring> docToString =
      unordered_map<wstring, wstring>();
  int maxLength = TestUtil::nextInt(random(), 1, 50);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"id", L"" + to_wstring(i), Field::Store::YES));
    wstring string =
        TestUtil::randomRealisticUnicodeString(random(), 1, maxLength);
    shared_ptr<BytesRef> br = make_shared<BytesRef>(string);
    doc->push_back(make_shared<SortedDocValuesField>(L"field", br));
    hash->add(br);
    docToString.emplace(L"" + to_wstring(i), string);
    w->addDocument(doc);
  }
  if (rarely()) {
    w->commit();
  }
  int numDocsNoValue = atLeast(10);
  for (int i = 0; i < numDocsNoValue; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"id", L"noValue", Field::Store::YES));
    w->addDocument(doc);
  }
  if (rarely()) {
    w->commit();
  }
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring id = L"" + to_wstring(i) + to_wstring(numDocs);
    doc->push_back(newTextField(L"id", id, Field::Store::YES));
    wstring string =
        TestUtil::randomRealisticUnicodeString(random(), 1, maxLength);
    shared_ptr<BytesRef> br = make_shared<BytesRef>(string);
    hash->add(br);
    docToString.emplace(id, string);
    doc->push_back(make_shared<SortedDocValuesField>(L"field", br));
    w->addDocument(doc);
  }
  w->commit();
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<SortedDocValues> docValues =
      MultiDocValues::getSortedValues(reader, L"field");
  std::deque<int> sort = hash->sort();
  shared_ptr<BytesRef> expected = make_shared<BytesRef>();
  TestUtil::assertEquals(hash->size(), docValues->getValueCount());
  for (int i = 0; i < hash->size(); i++) {
    hash->get(sort[i], expected);
    shared_ptr<BytesRef> *const actual = docValues->lookupOrd(i);
    TestUtil::assertEquals(expected->utf8ToString(), actual->utf8ToString());
    int ord = docValues->lookupTerm(expected);
    TestUtil::assertEquals(i, ord);
  }
  shared_ptr<Set<unordered_map::Entry<wstring, wstring>>> entrySet =
      docToString.entrySet();

  for (auto entry : entrySet) {
    // pk lookup
    shared_ptr<PostingsEnum> termPostingsEnum =
        TestUtil::docs(random(), reader, L"id",
                       make_shared<BytesRef>(entry.first), nullptr, 0);
    int docId = termPostingsEnum->nextDoc();
    expected = make_shared<BytesRef>(entry.second);
    docValues = MultiDocValues::getSortedValues(reader, L"field");
    TestUtil::assertEquals(docId, docValues->advance(docId));
    shared_ptr<BytesRef> *const actual = docValues->binaryValue();
    TestUtil::assertEquals(expected, actual);
  }

  delete reader;
  delete w;
  delete dir;
}

void BaseDocValuesFormatTestCase::doTestNumericsVsStoredFields(
    double density, function<int64_t()> &longs) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField =
      make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> storedField =
      newStringField(L"stored", L"", Field::Store::YES);
  shared_ptr<Field> dvField = make_shared<NumericDocValuesField>(L"dv", 0);
  doc->push_back(idField);
  doc->push_back(storedField);
  doc->push_back(dvField);

  // index some docs
  int numDocs = atLeast(300);
  // numDocs should be always > 256 so that in case of a codec that optimizes
  // for numbers of values <= 256, all storage layouts are tested
  assert(numDocs > 256);
  for (int i = 0; i < numDocs; i++) {
    if (random()->nextDouble() > density) {
      writer->addDocument(make_shared<Document>());
      continue;
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    int64_t value = longs();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    storedField->setStringValue(Long::toString(value));
    dvField->setLongValue(value);
    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }

  // merge some segments and ensure that at least one of them has more than
  // 256 values
  writer->forceMerge(numDocs / 256);

  delete writer;

  // compare
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<NumericDocValues> docValues = DocValues::getNumeric(r, L"dv");
    docValues->nextDoc();
    for (int i = 0; i < r->maxDoc(); i++) {
      wstring storedValue = r->document(i)[L"stored"];
      if (storedValue == L"") {
        assertTrue(docValues->docID() > i);
      } else {
        TestUtil::assertEquals(i, docValues->docID());
        TestUtil::assertEquals(StringHelper::fromString<int64_t>(storedValue),
                               docValues->longValue());
        docValues->nextDoc();
      }
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docValues->docID());
  }
  ir->close();
  delete dir;
}

void BaseDocValuesFormatTestCase::doTestSortedNumericsVsStoredFields(
    function<int64_t()> &counts,
    function<int64_t()> &values) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);

  // index some docs
  int numDocs = atLeast(300);
  // numDocs should be always > 256 so that in case of a codec that optimizes
  // for numbers of values <= 256, all storage layouts are tested
  assert(numDocs > 256);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));

    int valueCount = static_cast<int>(counts());
    std::deque<int64_t> valueArray(valueCount);
    for (int j = 0; j < valueCount; j++) {
      int64_t value = values();
      valueArray[j] = value;
      doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", value));
    }
    Arrays::sort(valueArray);
    for (int j = 0; j < valueCount; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(
          make_shared<StoredField>(L"stored", Long::toString(valueArray[j])));
    }
    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }

  // merge some segments and ensure that at least one of them has more than
  // 256 values
  writer->forceMerge(numDocs / 256);

  delete writer;

  // compare
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<SortedNumericDocValues> docValues =
        DocValues::getSortedNumeric(r, L"dv");
    for (int i = 0; i < r->maxDoc(); i++) {
      if (i > docValues->docID()) {
        docValues->nextDoc();
      }
      std::deque<wstring> expected = r->document(i)->getValues(L"stored");
      if (i < docValues->docID()) {
        TestUtil::assertEquals(0, expected.size());
      } else {
        std::deque<wstring> actual(docValues->docValueCount());
        for (int j = 0; j < actual.size(); j++) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          actual[j] = Long::toString(docValues->nextValue());
        }
        assertArrayEquals(expected, actual);
      }
    }
  }
  ir->close();
  delete dir;
}

void BaseDocValuesFormatTestCase::testBooleanNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(1, [&]() { random()->nextInt(2); });
  }
}

void BaseDocValuesFormatTestCase::
    testSparseBooleanNumericsVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(random()->nextDouble(),
                                 [&]() { random()->nextInt(2); });
  }
}

void BaseDocValuesFormatTestCase::testByteNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(1, [&]() {
      org::apache::lucene::util::TestUtil::nextInt(
          random(), numeric_limits<char>::min(), numeric_limits<char>::max());
    });
  }
}

void BaseDocValuesFormatTestCase::testSparseByteNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(random()->nextDouble(), [&]() {
      org::apache::lucene::util::TestUtil::nextInt(
          random(), numeric_limits<char>::min(), numeric_limits<char>::max());
    });
  }
}

void BaseDocValuesFormatTestCase::testShortNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(1, [&]() {
      org::apache::lucene::util::TestUtil::nextInt(
          random(), numeric_limits<short>::min(), numeric_limits<short>::max());
    });
  }
}

void BaseDocValuesFormatTestCase::testSparseShortNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(random()->nextDouble(), [&]() {
      org::apache::lucene::util::TestUtil::nextInt(
          random(), numeric_limits<short>::min(), numeric_limits<short>::max());
    });
  }
}

void BaseDocValuesFormatTestCase::testIntNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(1, random()::nextInt);
  }
}

void BaseDocValuesFormatTestCase::testSparseIntNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(random()->nextDouble(), random()::nextInt);
  }
}

void BaseDocValuesFormatTestCase::testLongNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(1, random()::nextLong);
  }
}

void BaseDocValuesFormatTestCase::testSparseLongNumericsVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestNumericsVsStoredFields(random()->nextDouble(), random()::nextLong);
  }
}

void BaseDocValuesFormatTestCase::doTestBinaryVsStoredFields(
    double density, function<char[] * ()> &bytes) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField =
      make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> storedField =
      make_shared<StoredField>(L"stored", std::deque<char>(0));
  shared_ptr<Field> dvField =
      make_shared<BinaryDocValuesField>(L"dv", make_shared<BytesRef>());
  doc->push_back(idField);
  doc->push_back(storedField);
  doc->push_back(dvField);

  // index some docs
  int numDocs = atLeast(300);
  for (int i = 0; i < numDocs; i++) {
    if (random()->nextDouble() > density) {
      writer->addDocument(make_shared<Document>());
      continue;
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    std::deque<char> buffer = bytes();
    storedField->setBytesValue(buffer);
    dvField->setBytesValue(buffer);
    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }

  // compare
  shared_ptr<DirectoryReader> ir = writer->getReader();
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> docValues = DocValues::getBinary(r, L"dv");
    docValues->nextDoc();
    for (int i = 0; i < r->maxDoc(); i++) {
      shared_ptr<BytesRef> binaryValue =
          r->document(i)->getBinaryValue(L"stored");
      if (binaryValue == nullptr) {
        assertTrue(docValues->docID() > i);
      } else {
        TestUtil::assertEquals(i, docValues->docID());
        TestUtil::assertEquals(binaryValue, docValues->binaryValue());
        docValues->nextDoc();
      }
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docValues->docID());
  }
  ir->close();

  // compare again
  writer->forceMerge(1);
  ir = writer->getReader();
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> docValues = DocValues::getBinary(r, L"dv");
    docValues->nextDoc();
    for (int i = 0; i < r->maxDoc(); i++) {
      shared_ptr<BytesRef> binaryValue =
          r->document(i)->getBinaryValue(L"stored");
      if (binaryValue == nullptr) {
        assertTrue(docValues->docID() > i);
      } else {
        TestUtil::assertEquals(i, docValues->docID());
        TestUtil::assertEquals(binaryValue, docValues->binaryValue());
        docValues->nextDoc();
      }
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docValues->docID());
  }
  ir->close();
  delete writer;
  delete dir;
}

void BaseDocValuesFormatTestCase::testBinaryFixedLengthVsStoredFields() throw(
    runtime_error)
{
  doTestBinaryFixedLengthVsStoredFields(1);
}

void BaseDocValuesFormatTestCase::
    testSparseBinaryFixedLengthVsStoredFields() 
{
  doTestBinaryFixedLengthVsStoredFields(random()->nextDouble());
}

void BaseDocValuesFormatTestCase::doTestBinaryFixedLengthVsStoredFields(
    double density) 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    int fixedLength = TestUtil::nextInt(random(), 0, 10);
    doTestBinaryVsStoredFields(density, [&]() {
      std::deque<char> buffer(fixedLength);
      random()->nextBytes(buffer);
      return buffer;
    });
  }
}

void BaseDocValuesFormatTestCase::
    testBinaryVariableLengthVsStoredFields() 
{
  doTestBinaryVariableLengthVsStoredFields(1);
}

void BaseDocValuesFormatTestCase::
    testSparseBinaryVariableLengthVsStoredFields() 
{
  doTestBinaryVariableLengthVsStoredFields(random()->nextDouble());
}

void BaseDocValuesFormatTestCase::doTestBinaryVariableLengthVsStoredFields(
    double density) 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestBinaryVsStoredFields(density, [&]() {
      constexpr int length = random()->nextInt(10);
      std::deque<char> buffer(length);
      random()->nextBytes(buffer);
      return buffer;
    });
  }
}

void BaseDocValuesFormatTestCase::doTestSortedVsStoredFields(
    int numDocs, double density,
    function<char[] * ()> &bytes) 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir(L"dvduel"));
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField =
      make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> storedField =
      make_shared<StoredField>(L"stored", std::deque<char>(0));
  shared_ptr<Field> dvField =
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>());
  doc->push_back(idField);
  doc->push_back(storedField);
  doc->push_back(dvField);

  // index some docs
  for (int i = 0; i < numDocs; i++) {
    if (random()->nextDouble() > density) {
      writer->addDocument(make_shared<Document>());
      continue;
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    std::deque<char> buffer = bytes();
    storedField->setBytesValue(buffer);
    dvField->setBytesValue(buffer);
    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }

  // compare
  shared_ptr<DirectoryReader> ir = writer->getReader();
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> docValues = DocValues::getBinary(r, L"dv");
    docValues->nextDoc();
    for (int i = 0; i < r->maxDoc(); i++) {
      shared_ptr<BytesRef> binaryValue =
          r->document(i)->getBinaryValue(L"stored");
      if (binaryValue == nullptr) {
        assertTrue(docValues->docID() > i);
      } else {
        TestUtil::assertEquals(i, docValues->docID());
        TestUtil::assertEquals(binaryValue, docValues->binaryValue());
        docValues->nextDoc();
      }
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docValues->docID());
  }
  ir->close();
  writer->forceMerge(1);

  // compare again
  ir = writer->getReader();
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> docValues = DocValues::getBinary(r, L"dv");
    docValues->nextDoc();
    for (int i = 0; i < r->maxDoc(); i++) {
      shared_ptr<BytesRef> binaryValue =
          r->document(i)->getBinaryValue(L"stored");
      if (binaryValue == nullptr) {
        assertTrue(docValues->docID() > i);
      } else {
        TestUtil::assertEquals(i, docValues->docID());
        TestUtil::assertEquals(binaryValue, docValues->binaryValue());
        docValues->nextDoc();
      }
    }
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docValues->docID());
  }
  ir->close();
  delete writer;
  delete dir;
}

void BaseDocValuesFormatTestCase::testSortedFixedLengthVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    int fixedLength = TestUtil::nextInt(random(), 1, 10);
    doTestSortedVsStoredFields(atLeast(300), 1, fixedLength, fixedLength);
  }
}

void BaseDocValuesFormatTestCase::
    testSparseSortedFixedLengthVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    int fixedLength = TestUtil::nextInt(random(), 1, 10);
    doTestSortedVsStoredFields(atLeast(300), random()->nextDouble(),
                               fixedLength, fixedLength);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedVariableLengthVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedVsStoredFields(atLeast(300), 1, 1, 10);
  }
}

void BaseDocValuesFormatTestCase::
    testSparseSortedVariableLengthVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedVsStoredFields(atLeast(300), random()->nextDouble(), 1, 10);
  }
}

void BaseDocValuesFormatTestCase::doTestSortedVsStoredFields(
    int numDocs, double density, int minLength,
    int maxLength) 
{
  doTestSortedVsStoredFields(numDocs, density, [&]() {
    int length = TestUtil::nextInt(random(), minLength, maxLength);
    std::deque<char> buffer(length);
    random()->nextBytes(buffer);
    return buffer;
  });
}

void BaseDocValuesFormatTestCase::testSortedSetOneValue() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetTwoFields() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field2", make_shared<BytesRef>(L"world")));
  iwriter->addDocument(doc);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(0, dv->nextDoc());

  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  dv = getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field2");

  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"world"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetTwoDocumentsMerged() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  iwriter->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"world")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(2, dv->getValueCount());

  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(1, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  bytes = dv->lookupOrd(1);
  TestUtil::assertEquals(make_shared<BytesRef>(L"world"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetTwoValues() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"world")));
  iwriter->addDocument(doc);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(0, dv->nextDoc());

  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(1, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  bytes = dv->lookupOrd(1);
  TestUtil::assertEquals(make_shared<BytesRef>(L"world"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetTwoValuesUnordered() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"world")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(0, dv->nextDoc());

  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(1, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  bytes = dv->lookupOrd(1);
  TestUtil::assertEquals(make_shared<BytesRef>(L"world"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetThreeValuesTwoDocs() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"world")));
  iwriter->addDocument(doc);
  iwriter->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"beer")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(3, dv->getValueCount());

  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(1, dv->nextOrd());
  TestUtil::assertEquals(2, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(1, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"beer"), bytes);

  bytes = dv->lookupOrd(1);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  bytes = dv->lookupOrd(2);
  TestUtil::assertEquals(make_shared<BytesRef>(L"world"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetTwoDocumentsLastMissing() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);

  doc = make_shared<Document>();
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(1, dv->getValueCount());

  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::
    testSortedSetTwoDocumentsLastMissingMerge() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  iwriter->commit();

  doc = make_shared<Document>();
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(1, dv->getValueCount());

  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetTwoDocumentsFirstMissing() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  iwriter->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);

  iwriter->forceMerge(1);
  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(1, dv->getValueCount());

  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::
    testSortedSetTwoDocumentsFirstMissingMerge() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  iwriter->addDocument(doc);
  iwriter->commit();

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(1, dv->getValueCount());

  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(0, dv->nextOrd());
  TestUtil::assertEquals(NO_MORE_ORDS, dv->nextOrd());

  shared_ptr<BytesRef> bytes = dv->lookupOrd(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"hello"), bytes);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetMergeAwayAllValues() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(0, dv->getValueCount());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetTermsEnum() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"world")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"beer")));
  iwriter->addDocument(doc);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(3, dv->getValueCount());

  shared_ptr<TermsEnum> termsEnum = dv->termsEnum();

  // next()
  TestUtil::assertEquals(L"beer", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  TestUtil::assertEquals(L"hello", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  TestUtil::assertEquals(L"world", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());

  // seekCeil()
  TestUtil::assertEquals(SeekStatus::NOT_FOUND,
                         termsEnum->seekCeil(make_shared<BytesRef>(L"ha!")));
  TestUtil::assertEquals(L"hello", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  TestUtil::assertEquals(SeekStatus::FOUND,
                         termsEnum->seekCeil(make_shared<BytesRef>(L"beer")));
  TestUtil::assertEquals(L"beer", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  TestUtil::assertEquals(SeekStatus::END,
                         termsEnum->seekCeil(make_shared<BytesRef>(L"zzz")));

  // seekExact()
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"beer")));
  TestUtil::assertEquals(L"beer", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"hello")));
  TestUtil::assertEquals(L"hello", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"world")));
  TestUtil::assertEquals(L"world", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());
  assertFalse(termsEnum->seekExact(make_shared<BytesRef>(L"bogus")));

  // seek(ord)
  termsEnum->seekExact(0);
  TestUtil::assertEquals(L"beer", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(0, termsEnum->ord());
  termsEnum->seekExact(1);
  TestUtil::assertEquals(L"hello", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  termsEnum->seekExact(2);
  TestUtil::assertEquals(L"world", termsEnum->term()->utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());

  // NORMAL automaton
  termsEnum = dv->intersect(make_shared<CompiledAutomaton>(
      (make_shared<RegExp>(L".*l.*"))->toAutomaton()));
  TestUtil::assertEquals(L"hello", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  TestUtil::assertEquals(L"world", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(2, termsEnum->ord());
  assertNull(termsEnum->next());

  // SINGLE automaton
  termsEnum = dv->intersect(make_shared<CompiledAutomaton>(
      (make_shared<RegExp>(L"hello"))->toAutomaton()));
  TestUtil::assertEquals(L"hello", termsEnum->next().utf8ToString());
  TestUtil::assertEquals(1, termsEnum->ord());
  assertNull(termsEnum->next());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::doTestSortedSetVsStoredFields(
    int numDocs, int minLength, int maxLength, int maxValuesPerDoc,
    int maxUniqueValues) 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir(L"dvduel"));
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);

  shared_ptr<Set<wstring>> valueSet = unordered_set<wstring>();
  for (int i = 0; i < 10000 && valueSet->size() < maxUniqueValues; ++i) {
    constexpr int length = TestUtil::nextInt(random(), minLength, maxLength);
    valueSet->add(TestUtil::randomSimpleString(random(), length));
  }
  std::deque<wstring> uniqueValues =
      valueSet->toArray(std::deque<wstring>(0));

  // index some docs
  if (VERBOSE) {
    wcout << L"\nTEST: now add numDocs=" << numDocs << endl;
  }
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Field> idField =
        make_shared<StringField>(L"id", Integer::toString(i), Field::Store::NO);
    doc->push_back(idField);
    int numValues = TestUtil::nextInt(random(), 0, maxValuesPerDoc);
    // create a random set of strings
    shared_ptr<Set<wstring>> values = set<wstring>();
    for (int v = 0; v < numValues; v++) {
      values->add(RandomPicks::randomFrom(random(), uniqueValues));
    }

    // add ordered to the stored field
    for (auto v : values) {
      doc->push_back(make_shared<StoredField>(L"stored", v));
    }

    // add in any order to the dv field
    deque<wstring> unordered = deque<wstring>(values);
    Collections::shuffle(unordered, random());
    for (auto v : unordered) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"dv", make_shared<BytesRef>(v)));
    }

    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  if (VERBOSE) {
    wcout << L"\nTEST: now delete " << numDeletions << L" docs" << endl;
  }
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }

  // compare
  if (VERBOSE) {
    wcout << L"\nTEST: now get reader" << endl;
  }
  shared_ptr<DirectoryReader> ir = writer->getReader();
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<SortedSetDocValues> docValues = r->getSortedSetDocValues(L"dv");
    for (int i = 0; i < r->maxDoc(); i++) {
      std::deque<wstring> stringValues = r->document(i)->getValues(L"stored");
      if (docValues != nullptr) {
        if (docValues->docID() < i) {
          docValues->nextDoc();
        }
      }
      if (docValues != nullptr && stringValues.size() > 0) {
        TestUtil::assertEquals(i, docValues->docID());
        for (int j = 0; j < stringValues.size(); j++) {
          assert(docValues != nullptr);
          int64_t ord = docValues->nextOrd();
          assert(ord != NO_MORE_ORDS);
          shared_ptr<BytesRef> scratch = docValues->lookupOrd(ord);
          TestUtil::assertEquals(stringValues[j], scratch->utf8ToString());
        }
        TestUtil::assertEquals(NO_MORE_ORDS, docValues->nextOrd());
      }
    }
  }
  if (VERBOSE) {
    wcout << L"\nTEST: now close reader" << endl;
  }
  ir->close();
  if (VERBOSE) {
    wcout << L"TEST: force merge" << endl;
  }
  writer->forceMerge(1);

  // compare again
  ir = writer->getReader();
  TestUtil::checkReader(ir);
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<SortedSetDocValues> docValues = r->getSortedSetDocValues(L"dv");
    for (int i = 0; i < r->maxDoc(); i++) {
      std::deque<wstring> stringValues = r->document(i)->getValues(L"stored");
      if (docValues->docID() < i) {
        docValues->nextDoc();
      }
      if (docValues != nullptr && stringValues.size() > 0) {
        TestUtil::assertEquals(i, docValues->docID());
        for (int j = 0; j < stringValues.size(); j++) {
          assert(docValues != nullptr);
          int64_t ord = docValues->nextOrd();
          assert(ord != NO_MORE_ORDS);
          shared_ptr<BytesRef> scratch = docValues->lookupOrd(ord);
          TestUtil::assertEquals(stringValues[j], scratch->utf8ToString());
        }
        TestUtil::assertEquals(NO_MORE_ORDS, docValues->nextOrd());
      }
    }
  }
  if (VERBOSE) {
    wcout << L"TEST: close reader" << endl;
  }
  ir->close();
  if (VERBOSE) {
    wcout << L"TEST: close writer" << endl;
  }
  delete writer;
  if (VERBOSE) {
    wcout << L"TEST: close dir" << endl;
  }
  delete dir;
}

void BaseDocValuesFormatTestCase::
    testSortedSetFixedLengthVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    int fixedLength = TestUtil::nextInt(random(), 1, 10);
    doTestSortedSetVsStoredFields(atLeast(300), fixedLength, fixedLength, 16,
                                  100);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedNumericsSingleValuedVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedNumericsVsStoredFields([&]() { 1; }, random()::nextLong);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedNumericsSingleValuedMissingVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedNumericsVsStoredFields(
        [&]() { random()->nextBoolean() ? 0 : 1; }, random()::nextLong);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedNumericsMultipleValuesVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedNumericsVsStoredFields(
        [&]() {
          org::apache::lucene::util::TestUtil::nextLong(random(), 0, 50);
        },
        random()::nextLong);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedNumericsFewUniqueSetsVsStoredFields() 
{
  const std::deque<int64_t> values =
      std::deque<int64_t>(TestUtil::nextInt(random(), 2, 6));
  for (int i = 0; i < values.size(); ++i) {
    values[i] = random()->nextLong();
  }
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedNumericsVsStoredFields(
        [&]() {
          org::apache::lucene::util::TestUtil::nextLong(random(), 0, 6);
        },
        [&]() { values[random()->nextInt(values.size())]; });
  }
}

void BaseDocValuesFormatTestCase::
    testSortedSetVariableLengthVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(atLeast(300), 1, 10, 16, 100);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedSetFixedLengthSingleValuedVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    int fixedLength = TestUtil::nextInt(random(), 1, 10);
    doTestSortedSetVsStoredFields(atLeast(300), fixedLength, fixedLength, 1,
                                  100);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedSetVariableLengthSingleValuedVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(atLeast(300), 1, 10, 1, 100);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedSetFixedLengthFewUniqueSetsVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(atLeast(300), 10, 10, 6, 6);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedSetVariableLengthFewUniqueSetsVsStoredFields() throw(
        runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(atLeast(300), 1, 10, 6, 6);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedSetVariableLengthManyValuesPerDocVsStoredFields() throw(
        runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(atLeast(20), 1, 10, 500, 1000);
  }
}

void BaseDocValuesFormatTestCase::
    testSortedSetFixedLengthManyValuesPerDocVsStoredFields() throw(
        runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(atLeast(20), 10, 10, 500, 1000);
  }
}

void BaseDocValuesFormatTestCase::testGCDCompression() 
{
  doTestGCDCompression(1);
}

void BaseDocValuesFormatTestCase::testSparseGCDCompression() throw(
    runtime_error)
{
  doTestGCDCompression(random()->nextDouble());
}

void BaseDocValuesFormatTestCase::doTestGCDCompression(double density) throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    constexpr int64_t min =
        -((static_cast<int64_t>(random()->nextInt(1 << 30))) << 32);
    constexpr int64_t mul = random()->nextInt() & 0xFFFFFFFFLL;
    const function<int64_t()> longs = [&]() {
      return min + mul * random()->nextInt(1 << 20);
    };
    doTestNumericsVsStoredFields(density, longs);
  }
}

void BaseDocValuesFormatTestCase::testZeros() 
{
  doTestNumericsVsStoredFields(1, [&]() { 0; });
}

void BaseDocValuesFormatTestCase::testSparseZeros() 
{
  doTestNumericsVsStoredFields(random()->nextDouble(), [&]() { 0; });
}

void BaseDocValuesFormatTestCase::testZeroOrMin() 
{
  // try to make GCD compression fail if the format did not anticipate that
  // the GCD of 0 and MIN_VALUE is negative
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    const function<int64_t()> longs = [&]() {
      return random()->nextBoolean() ? 0 : numeric_limits<int64_t>::min();
    };
    doTestNumericsVsStoredFields(1, longs);
  }
}

void BaseDocValuesFormatTestCase::testTwoNumbersOneMissing() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 0));
  iw->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  iw->addDocument(doc);
  iw->forceMerge(1);
  delete iw;

  shared_ptr<IndexReader> ir = DirectoryReader::open(directory);
  TestUtil::assertEquals(1, ir->leaves().size());
  shared_ptr<LeafReader> ar = ir->leaves()[0]->reader();
  shared_ptr<NumericDocValues> dv = ar->getNumericDocValues(L"dv1");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->longValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());
  delete ir;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoNumbersOneMissingWithMerging() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 0));
  iw->addDocument(doc);
  iw->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  iw->addDocument(doc);
  iw->forceMerge(1);
  delete iw;

  shared_ptr<IndexReader> ir = DirectoryReader::open(directory);
  TestUtil::assertEquals(1, ir->leaves().size());
  shared_ptr<LeafReader> ar = ir->leaves()[0]->reader();
  shared_ptr<NumericDocValues> dv = ar->getNumericDocValues(L"dv1");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->longValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());
  delete ir;
  delete directory;
}

void BaseDocValuesFormatTestCase::testThreeNumbersOneMissingWithMerging() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 0));
  iw->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  iw->addDocument(doc);
  iw->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 5));
  iw->addDocument(doc);
  iw->forceMerge(1);
  delete iw;

  shared_ptr<IndexReader> ir = DirectoryReader::open(directory);
  TestUtil::assertEquals(1, ir->leaves().size());
  shared_ptr<LeafReader> ar = ir->leaves()[0]->reader();
  shared_ptr<NumericDocValues> dv = ar->getNumericDocValues(L"dv1");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(0, dv->longValue());
  TestUtil::assertEquals(2, dv->nextDoc());
  TestUtil::assertEquals(5, dv->longValue());
  delete ir;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoBytesOneMissing() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv1", make_shared<BytesRef>()));
  iw->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  iw->addDocument(doc);
  iw->forceMerge(1);
  delete iw;

  shared_ptr<IndexReader> ir = DirectoryReader::open(directory);
  TestUtil::assertEquals(1, ir->leaves().size());
  shared_ptr<LeafReader> ar = ir->leaves()[0]->reader();
  shared_ptr<BinaryDocValues> dv = ar->getBinaryDocValues(L"dv1");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(make_shared<BytesRef>(), dv->binaryValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());
  delete ir;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoBytesOneMissingWithMerging() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv1", make_shared<BytesRef>()));
  iw->addDocument(doc);
  iw->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  iw->addDocument(doc);
  iw->forceMerge(1);
  delete iw;

  shared_ptr<IndexReader> ir = DirectoryReader::open(directory);
  TestUtil::assertEquals(1, ir->leaves().size());
  shared_ptr<LeafReader> ar = ir->leaves()[0]->reader();
  shared_ptr<BinaryDocValues> dv = ar->getBinaryDocValues(L"dv1");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(make_shared<BytesRef>(), dv->binaryValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());
  delete ir;
  delete directory;
}

void BaseDocValuesFormatTestCase::testThreeBytesOneMissingWithMerging() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv1", make_shared<BytesRef>()));
  iw->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  iw->addDocument(doc);
  iw->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv1", make_shared<BytesRef>(L"boo")));
  iw->addDocument(doc);
  iw->forceMerge(1);
  delete iw;

  shared_ptr<IndexReader> ir = DirectoryReader::open(directory);
  TestUtil::assertEquals(1, ir->leaves().size());
  shared_ptr<LeafReader> ar = ir->leaves()[0]->reader();
  shared_ptr<BinaryDocValues> dv = ar->getBinaryDocValues(L"dv1");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(make_shared<BytesRef>(), dv->binaryValue());
  TestUtil::assertEquals(2, dv->nextDoc());
  TestUtil::assertEquals(make_shared<BytesRef>(L"boo"), dv->binaryValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());
  delete ir;
  delete directory;
}

void BaseDocValuesFormatTestCase::testThreads() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField =
      make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> storedBinField =
      make_shared<StoredField>(L"storedBin", std::deque<char>(0));
  shared_ptr<Field> dvBinField =
      make_shared<BinaryDocValuesField>(L"dvBin", make_shared<BytesRef>());
  shared_ptr<Field> dvSortedField =
      make_shared<SortedDocValuesField>(L"dvSorted", make_shared<BytesRef>());
  shared_ptr<Field> storedNumericField =
      make_shared<StoredField>(L"storedNum", L"");
  shared_ptr<Field> dvNumericField =
      make_shared<NumericDocValuesField>(L"dvNum", 0);
  doc->push_back(idField);
  doc->push_back(storedBinField);
  doc->push_back(dvBinField);
  doc->push_back(dvSortedField);
  doc->push_back(storedNumericField);
  doc->push_back(dvNumericField);

  // index some docs
  int numDocs = atLeast(300);
  for (int i = 0; i < numDocs; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    int length = TestUtil::nextInt(random(), 0, 8);
    std::deque<char> buffer(length);
    random()->nextBytes(buffer);
    storedBinField->setBytesValue(buffer);
    dvBinField->setBytesValue(buffer);
    dvSortedField->setBytesValue(buffer);
    int64_t numericValue = random()->nextLong();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    storedNumericField->setStringValue(Long::toString(numericValue));
    dvNumericField->setLongValue(numericValue);
    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }
  delete writer;

  // compare
  shared_ptr<DirectoryReader> *const ir = DirectoryReader::open(dir);
  int numThreads = TestUtil::nextInt(random(), 2, 7);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);

  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(shared_from_this(), ir,
                                                        startingGun);
    threads[i]->start();
  }
  startingGun->countDown();
  for (auto t : threads) {
    t->join();
  }
  ir->close();
  delete dir;
}

BaseDocValuesFormatTestCase::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        shared_ptr<org::apache::lucene::index::DirectoryReader> ir,
        shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->ir = ir;
  this->startingGun = startingGun;
}

void BaseDocValuesFormatTestCase::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<BinaryDocValues> binaries = r->getBinaryDocValues(L"dvBin");
      shared_ptr<SortedDocValues> sorted = r->getSortedDocValues(L"dvSorted");
      shared_ptr<NumericDocValues> numerics = r->getNumericDocValues(L"dvNum");
      for (int j = 0; j < r->maxDoc(); j++) {
        shared_ptr<BytesRef> binaryValue =
            r->document(j)->getBinaryValue(L"storedBin");
        TestUtil::assertEquals(j, binaries->nextDoc());
        shared_ptr<BytesRef> scratch = binaries->binaryValue();
        TestUtil::assertEquals(binaryValue, scratch);
        TestUtil::assertEquals(j, sorted->nextDoc());
        scratch = sorted->binaryValue();
        TestUtil::assertEquals(binaryValue, scratch);
        wstring expected = r->document(j)[L"storedNum"];
        TestUtil::assertEquals(j, numerics->nextDoc());
        TestUtil::assertEquals(StringHelper::fromString<int64_t>(expected),
                               numerics->longValue());
      }
    }
    TestUtil::checkReader(ir);
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testThreads2() throws Exception
void BaseDocValuesFormatTestCase::testThreads2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  shared_ptr<Field> idField =
      make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> storedBinField =
      make_shared<StoredField>(L"storedBin", std::deque<char>(0));
  shared_ptr<Field> dvBinField =
      make_shared<BinaryDocValuesField>(L"dvBin", make_shared<BytesRef>());
  shared_ptr<Field> dvSortedField =
      make_shared<SortedDocValuesField>(L"dvSorted", make_shared<BytesRef>());
  shared_ptr<Field> storedNumericField =
      make_shared<StoredField>(L"storedNum", L"");
  shared_ptr<Field> dvNumericField =
      make_shared<NumericDocValuesField>(L"dvNum", 0);

  // index some docs
  int numDocs = TestUtil::nextInt(random(), 1025, 2047);
  for (int i = 0; i < numDocs; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    int length = TestUtil::nextInt(random(), 0, 8);
    std::deque<char> buffer(length);
    random()->nextBytes(buffer);
    storedBinField->setBytesValue(buffer);
    dvBinField->setBytesValue(buffer);
    dvSortedField->setBytesValue(buffer);
    int64_t numericValue = random()->nextLong();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    storedNumericField->setStringValue(Long::toString(numericValue));
    dvNumericField->setLongValue(numericValue);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(idField);
    if (random()->nextInt(4) > 0) {
      doc->push_back(storedBinField);
      doc->push_back(dvBinField);
      doc->push_back(dvSortedField);
    }
    if (random()->nextInt(4) > 0) {
      doc->push_back(storedNumericField);
      doc->push_back(dvNumericField);
    }
    int numSortedSetFields = random()->nextInt(3);
    shared_ptr<Set<wstring>> values = set<wstring>();
    for (int j = 0; j < numSortedSetFields; j++) {
      values->add(TestUtil::randomSimpleString(random()));
    }
    for (auto v : values) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"dvSortedSet", make_shared<BytesRef>(v)));
      doc->push_back(make_shared<StoredField>(L"storedSortedSet", v));
    }
    int numSortedNumericFields = random()->nextInt(3);
    shared_ptr<Set<int64_t>> numValues = set<int64_t>();
    for (int j = 0; j < numSortedNumericFields; j++) {
      numValues->add(TestUtil::nextLong(random(),
                                        numeric_limits<int64_t>::min(),
                                        numeric_limits<int64_t>::max()));
    }
    for (shared_ptr<> : : optional<int64_t> l : numValues) {
      doc->push_back(
          make_shared<SortedNumericDocValuesField>(L"dvSortedNumeric", l));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(
          make_shared<StoredField>(L"storedSortedNumeric", Long::toString(l)));
    }
    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }
  delete writer;

  // compare
  shared_ptr<DirectoryReader> *const ir = DirectoryReader::open(dir);
  int numThreads = TestUtil::nextInt(random(), 2, 7);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);

  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass2>(shared_from_this(), ir,
                                                         startingGun);
    threads[i]->start();
  }
  startingGun->countDown();
  for (auto t : threads) {
    t->join();
  }
  ir->close();
  delete dir;
}

BaseDocValuesFormatTestCase::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        shared_ptr<org::apache::lucene::index::DirectoryReader> ir,
        shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->ir = ir;
  this->startingGun = startingGun;
}

void BaseDocValuesFormatTestCase::ThreadAnonymousInnerClass2::run()
{
  try {
    startingGun->await();
    for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<BinaryDocValues> binaries = r->getBinaryDocValues(L"dvBin");
      shared_ptr<SortedDocValues> sorted = r->getSortedDocValues(L"dvSorted");
      shared_ptr<NumericDocValues> numerics = r->getNumericDocValues(L"dvNum");
      shared_ptr<SortedSetDocValues> sortedSet =
          r->getSortedSetDocValues(L"dvSortedSet");
      shared_ptr<SortedNumericDocValues> sortedNumeric =
          r->getSortedNumericDocValues(L"dvSortedNumeric");
      for (int j = 0; j < r->maxDoc(); j++) {
        shared_ptr<BytesRef> binaryValue =
            r->document(j)->getBinaryValue(L"storedBin");
        if (binaryValue != nullptr) {
          if (binaries != nullptr) {
            TestUtil::assertEquals(j, binaries->nextDoc());
            shared_ptr<BytesRef> scratch = binaries->binaryValue();
            TestUtil::assertEquals(binaryValue, scratch);
            TestUtil::assertEquals(j, sorted->nextDoc());
            scratch = sorted->binaryValue();
            TestUtil::assertEquals(binaryValue, scratch);
          }
        }

        wstring number = r->document(j)[L"storedNum"];
        if (number != L"") {
          if (numerics != nullptr) {
            TestUtil::assertEquals(j, numerics->advance(j));
            TestUtil::assertEquals(StringHelper::fromString<int64_t>(number),
                                   numerics->longValue());
          }
        }

        std::deque<wstring> values =
            r->document(j)->getValues(L"storedSortedSet");
        if (values.size() > 0) {
          assertNotNull(sortedSet);
          TestUtil::assertEquals(j, sortedSet->nextDoc());
          for (int k = 0; k < values.size(); k++) {
            int64_t ord = sortedSet->nextOrd();
            assertTrue(ord != SortedSetDocValues::NO_MORE_ORDS);
            shared_ptr<BytesRef> value = sortedSet->lookupOrd(ord);
            TestUtil::assertEquals(values[k], value->utf8ToString());
          }
          TestUtil::assertEquals(SortedSetDocValues::NO_MORE_ORDS,
                                 sortedSet->nextOrd());
        }

        std::deque<wstring> numValues =
            r->document(j)->getValues(L"storedSortedNumeric");
        if (numValues.size() > 0) {
          assertNotNull(sortedNumeric);
          TestUtil::assertEquals(j, sortedNumeric->nextDoc());
          TestUtil::assertEquals(numValues.size(),
                                 sortedNumeric->docValueCount());
          for (int k = 0; k < numValues.size(); k++) {
            int64_t v = sortedNumeric->nextValue();
            // C++ TODO: There is no native C++ equivalent to 'toString':
            TestUtil::assertEquals(numValues[k], Long::toString(v));
          }
        }
      }
    }
    TestUtil::checkReader(ir);
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testThreads3() throws Exception
void BaseDocValuesFormatTestCase::testThreads3() 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);

  int numSortedSets = random()->nextInt(21);
  int numBinaries = random()->nextInt(21);
  int numSortedNums = random()->nextInt(21);

  int numDocs = TestUtil::nextInt(random(), 2025, 2047);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();

    for (int j = 0; j < numSortedSets; j++) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"ss" + to_wstring(j),
          make_shared<BytesRef>(TestUtil::randomSimpleString(random()))));
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"ss" + to_wstring(j),
          make_shared<BytesRef>(TestUtil::randomSimpleString(random()))));
    }

    for (int j = 0; j < numBinaries; j++) {
      doc->push_back(make_shared<BinaryDocValuesField>(
          L"b" + to_wstring(j),
          make_shared<BytesRef>(TestUtil::randomSimpleString(random()))));
    }

    for (int j = 0; j < numSortedNums; j++) {
      doc->push_back(make_shared<SortedNumericDocValuesField>(
          L"sn" + to_wstring(j),
          TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                             numeric_limits<int64_t>::max())));
      doc->push_back(make_shared<SortedNumericDocValuesField>(
          L"sn" + to_wstring(j),
          TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                             numeric_limits<int64_t>::max())));
    }
    writer->addDocument(doc);
  }
  delete writer;

  // now check with threads
  for (int i = 0; i < 10; i++) {
    shared_ptr<DirectoryReader> *const r = DirectoryReader::open(dir);
    shared_ptr<CountDownLatch> *const startingGun =
        make_shared<CountDownLatch>(1);
    std::deque<std::shared_ptr<Thread>> threads(
        TestUtil::nextInt(random(), 4, 10));
    for (int tid = 0; tid < threads.size(); tid++) {
      threads[tid] = make_shared<ThreadAnonymousInnerClass3>(shared_from_this(),
                                                             r, startingGun);
    }
    for (int tid = 0; tid < threads.size(); tid++) {
      threads[tid]->start();
    }
    startingGun->countDown();
    for (int tid = 0; tid < threads.size(); tid++) {
      threads[tid]->join();
    }
    r->close();
  }

  delete dir;
}

BaseDocValuesFormatTestCase::ThreadAnonymousInnerClass3::
    ThreadAnonymousInnerClass3(
        shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        shared_ptr<org::apache::lucene::index::DirectoryReader> r,
        shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->r = r;
  this->startingGun = startingGun;
}

void BaseDocValuesFormatTestCase::ThreadAnonymousInnerClass3::run()
{
  try {
    shared_ptr<ByteArrayOutputStream> bos =
        make_shared<ByteArrayOutputStream>(1024);
    shared_ptr<PrintStream> infoStream =
        make_shared<PrintStream>(bos, false, IOUtils::UTF_8);
    startingGun->await();
    for (shared_ptr<LeafReaderContext> leaf : r->leaves()) {
      shared_ptr<DocValuesStatus> status = CheckIndex::testDocValues(
          std::static_pointer_cast<SegmentReader>(leaf->reader()), infoStream,
          true);
      if (status->error != nullptr) {
        throw status->error;
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void BaseDocValuesFormatTestCase::testEmptyBinaryValueOnPageSizes() throw(
    runtime_error)
{
  // Test larger and larger power-of-two sized values,
  // followed by empty string value:
  for (int i = 0; i < 20; i++) {
    if (i > 14 && codecAcceptsHugeBinaryValues(L"field") == false) {
      break;
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> w =
        make_shared<RandomIndexWriter>(random(), dir);
    shared_ptr<BytesRef> bytes = make_shared<BytesRef>();
    bytes->bytes = std::deque<char>(1 << i);
    bytes->length = 1 << i;
    for (int j = 0; j < 4; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<BinaryDocValuesField>(L"field", bytes));
      w->addDocument(doc);
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StoredField>(L"id", L"5"));
    doc->push_back(
        make_shared<BinaryDocValuesField>(L"field", make_shared<BytesRef>()));
    w->addDocument(doc);
    shared_ptr<IndexReader> r = w->getReader();
    delete w;

    shared_ptr<BinaryDocValues> values =
        MultiDocValues::getBinaryValues(r, L"field");
    for (int j = 0; j < 5; j++) {
      TestUtil::assertEquals(j, values->nextDoc());
      shared_ptr<BytesRef> result = values->binaryValue();
      assertTrue(result->length == 0 || result->length == 1 << i);
    }
    delete r;
    delete dir;
  }
}

void BaseDocValuesFormatTestCase::testOneSortedNumber() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", 5));
  writer->addDocument(doc);
  delete writer;

  // Now search the index:
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  assert(reader->leaves().size() == 1);
  shared_ptr<SortedNumericDocValues> dv =
      reader->leaves()[0]->reader().getSortedNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(1, dv->docValueCount());
  TestUtil::assertEquals(5, dv->nextValue());

  delete reader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testOneSortedNumberOneMissing() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, make_shared<IndexWriterConfig>(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", 5));
  writer->addDocument(doc);
  writer->addDocument(make_shared<Document>());
  delete writer;

  // Now search the index:
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  assert(reader->leaves().size() == 1);
  shared_ptr<SortedNumericDocValues> dv =
      reader->leaves()[0]->reader().getSortedNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(1, dv->docValueCount());
  TestUtil::assertEquals(5, dv->nextValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  delete reader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testNumberMergeAwayAllValues() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"field", 5));
  iwriter->addDocument(doc);
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<NumericDocValues> dv =
      getOnlyLeafReader(ireader)->getNumericDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoSortedNumber() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", 11));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", -5));
  writer->addDocument(doc);
  delete writer;

  // Now search the index:
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  assert(reader->leaves().size() == 1);
  shared_ptr<SortedNumericDocValues> dv =
      reader->leaves()[0]->reader().getSortedNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(2, dv->docValueCount());
  TestUtil::assertEquals(-5, dv->nextValue());
  TestUtil::assertEquals(11, dv->nextValue());

  delete reader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoSortedNumberSameValue() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", 11));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", 11));
  writer->addDocument(doc);
  delete writer;

  // Now search the index:
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  assert(reader->leaves().size() == 1);
  shared_ptr<SortedNumericDocValues> dv =
      reader->leaves()[0]->reader().getSortedNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(2, dv->docValueCount());
  TestUtil::assertEquals(11, dv->nextValue());
  TestUtil::assertEquals(11, dv->nextValue());

  delete reader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testTwoSortedNumberOneMissing() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, make_shared<IndexWriterConfig>(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", 11));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", -5));
  writer->addDocument(doc);
  writer->addDocument(make_shared<Document>());
  delete writer;

  // Now search the index:
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  assert(reader->leaves().size() == 1);
  shared_ptr<SortedNumericDocValues> dv =
      reader->leaves()[0]->reader().getSortedNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(2, dv->docValueCount());
  TestUtil::assertEquals(-5, dv->nextValue());
  TestUtil::assertEquals(11, dv->nextValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  delete reader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedNumberMerge() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", 11));
  writer->addDocument(doc);
  writer->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv", -5));
  writer->addDocument(doc);
  writer->forceMerge(1);
  delete writer;

  // Now search the index:
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  assert(reader->leaves().size() == 1);
  shared_ptr<SortedNumericDocValues> dv =
      reader->leaves()[0]->reader().getSortedNumericDocValues(L"dv");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(1, dv->docValueCount());
  TestUtil::assertEquals(11, dv->nextValue());
  TestUtil::assertEquals(1, dv->nextDoc());
  TestUtil::assertEquals(1, dv->docValueCount());
  TestUtil::assertEquals(-5, dv->nextValue());

  delete reader;
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedNumberMergeAwayAllValues() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  iwriter->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"field", 5));
  iwriter->addDocument(doc);
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedNumericDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedNumericDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedEnumAdvanceIndependently() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<SortedDocValuesField> field =
      make_shared<SortedDocValuesField>(L"field", make_shared<BytesRef>(L"2"));
  doc->push_back(field);
  iwriter->addDocument(doc);
  field->setBytesValue(make_shared<BytesRef>(L"1"));
  iwriter->addDocument(doc);
  field->setBytesValue(make_shared<BytesRef>(L"3"));
  iwriter->addDocument(doc);

  iwriter->commit();
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedDocValues(L"field");
  doTestSortedSetEnumAdvanceIndependently(DocValues::singleton(dv));

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testSortedSetEnumAdvanceIndependently() throw(
    IOException)
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<SortedSetDocValuesField> field1 =
      make_shared<SortedSetDocValuesField>(L"field",
                                           make_shared<BytesRef>(L"2"));
  shared_ptr<SortedSetDocValuesField> field2 =
      make_shared<SortedSetDocValuesField>(L"field",
                                           make_shared<BytesRef>(L"3"));
  doc->push_back(field1);
  doc->push_back(field2);
  iwriter->addDocument(doc);
  field1->setBytesValue(make_shared<BytesRef>(L"1"));
  iwriter->addDocument(doc);
  field2->setBytesValue(make_shared<BytesRef>(L"2"));
  iwriter->addDocument(doc);

  iwriter->commit();
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  doTestSortedSetEnumAdvanceIndependently(dv);

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::doTestSortedSetEnumAdvanceIndependently(
    shared_ptr<SortedSetDocValues> dv) 
{
  if (dv->getValueCount() < 2) {
    return;
  }
  deque<std::shared_ptr<BytesRef>> terms = deque<std::shared_ptr<BytesRef>>();
  shared_ptr<TermsEnum> te = dv->termsEnum();
  terms.push_back(BytesRef::deepCopyOf(te->next()));
  terms.push_back(BytesRef::deepCopyOf(te->next()));

  // Make sure that calls to next() does not modify the term of the other enum
  shared_ptr<TermsEnum> enum1 = dv->termsEnum();
  shared_ptr<TermsEnum> enum2 = dv->termsEnum();
  shared_ptr<BytesRefBuilder> term1 = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRefBuilder> term2 = make_shared<BytesRefBuilder>();

  term1->copyBytes(enum1->next());
  term2->copyBytes(enum2->next());
  term1->copyBytes(enum1->next());

  TestUtil::assertEquals(term1->get(), enum1->term());
  TestUtil::assertEquals(term2->get(), enum2->term());

  // Same for seekCeil
  enum1 = dv->termsEnum();
  enum2 = dv->termsEnum();
  term1 = make_shared<BytesRefBuilder>();
  term2 = make_shared<BytesRefBuilder>();

  term2->copyBytes(enum2->next());
  shared_ptr<BytesRefBuilder> seekTerm = make_shared<BytesRefBuilder>();
  seekTerm->append(terms[0]);
  seekTerm->append(static_cast<char>(0));
  enum1->seekCeil(seekTerm->get());
  term1->copyBytes(enum1->term());

  TestUtil::assertEquals(term1->get(), enum1->term());
  TestUtil::assertEquals(term2->get(), enum2->term());

  // Same for seekCeil on an exact value
  enum1 = dv->termsEnum();
  enum2 = dv->termsEnum();
  term1 = make_shared<BytesRefBuilder>();
  term2 = make_shared<BytesRefBuilder>();

  term2->copyBytes(enum2->next());
  enum1->seekCeil(terms[1]);
  term1->copyBytes(enum1->term());

  TestUtil::assertEquals(term1->get(), enum1->term());
  TestUtil::assertEquals(term2->get(), enum2->term());

  // Same for seekExact
  enum1 = dv->termsEnum();
  enum2 = dv->termsEnum();
  term1 = make_shared<BytesRefBuilder>();
  term2 = make_shared<BytesRefBuilder>();

  term2->copyBytes(enum2->next());
  constexpr bool found = enum1->seekExact(terms[1]);
  assertTrue(found);
  term1->copyBytes(enum1->term());

  // Same for seek by ord
  enum1 = dv->termsEnum();
  enum2 = dv->termsEnum();
  term1 = make_shared<BytesRefBuilder>();
  term2 = make_shared<BytesRefBuilder>();

  term2->copyBytes(enum2->next());
  enum1->seekExact(1);
  term1->copyBytes(enum1->term());

  TestUtil::assertEquals(term1->get(), enum1->term());
  TestUtil::assertEquals(term2->get(), enum2->term());
}

void BaseDocValuesFormatTestCase::
    testSortedMergeAwayAllValuesLargeSegment() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  constexpr int numEmptyDocs = atLeast(1024);
  for (int i = 0; i < numEmptyDocs; ++i) {
    iwriter->addDocument(make_shared<Document>());
  }
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::
    testSortedSetMergeAwayAllValuesLargeSegment() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  constexpr int numEmptyDocs = atLeast(1024);
  for (int i = 0; i < numEmptyDocs; ++i) {
    iwriter->addDocument(make_shared<Document>());
  }
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedSetDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedSetDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::
    testNumericMergeAwayAllValuesLargeSegment() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"field", 42LL));
  iwriter->addDocument(doc);
  constexpr int numEmptyDocs = atLeast(1024);
  for (int i = 0; i < numEmptyDocs; ++i) {
    iwriter->addDocument(make_shared<Document>());
  }
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<NumericDocValues> dv =
      getOnlyLeafReader(ireader)->getNumericDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::
    testSortedNumericMergeAwayAllValuesLargeSegment() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"field", 42LL));
  iwriter->addDocument(doc);
  constexpr int numEmptyDocs = atLeast(1024);
  for (int i = 0; i < numEmptyDocs; ++i) {
    iwriter->addDocument(make_shared<Document>());
  }
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<SortedNumericDocValues> dv =
      getOnlyLeafReader(ireader)->getSortedNumericDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::
    testBinaryMergeAwayAllValuesLargeSegment() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwconfig = newIndexWriterConfig(analyzer);
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, iwconfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  iwriter->addDocument(doc);
  constexpr int numEmptyDocs = atLeast(1024);
  for (int i = 0; i < numEmptyDocs; ++i) {
    iwriter->addDocument(make_shared<Document>());
  }
  iwriter->commit();
  iwriter->deleteDocuments(make_shared<Term>(L"id", L"1"));
  iwriter->forceMerge(1);

  shared_ptr<DirectoryReader> ireader = iwriter->getReader();
  delete iwriter;

  shared_ptr<BinaryDocValues> dv =
      getOnlyLeafReader(ireader)->getBinaryDocValues(L"field");
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dv->nextDoc());

  ireader->close();
  delete directory;
}

void BaseDocValuesFormatTestCase::testRandomAdvanceNumeric() 
{
  constexpr int64_t longRange;
  if (random()->nextBoolean()) {
    longRange = TestUtil::nextInt(random(), 1, 1024);
  } else {
    longRange =
        TestUtil::nextLong(random(), 1, numeric_limits<int64_t>::max());
  }
  doTestRandomAdvance(make_shared<FieldCreatorAnonymousInnerClass>(
      shared_from_this(), longRange));
}

BaseDocValuesFormatTestCase::FieldCreatorAnonymousInnerClass::
    FieldCreatorAnonymousInnerClass(
        shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        int64_t longRange)
{
  this->outerInstance = outerInstance;
  this->longRange = longRange;
}

shared_ptr<Field>
BaseDocValuesFormatTestCase::FieldCreatorAnonymousInnerClass::next()
{
  return make_shared<NumericDocValuesField>(
      L"field", TestUtil::nextLong(random(), 0, longRange));
}

shared_ptr<DocIdSetIterator>
BaseDocValuesFormatTestCase::FieldCreatorAnonymousInnerClass::iterator(
    shared_ptr<IndexReader> r) 
{
  return MultiDocValues::getNumericValues(r, L"field");
}

void BaseDocValuesFormatTestCase::testRandomAdvanceBinary() 
{
  doTestRandomAdvance(
      make_shared<FieldCreatorAnonymousInnerClass2>(shared_from_this()));
}

BaseDocValuesFormatTestCase::FieldCreatorAnonymousInnerClass2::
    FieldCreatorAnonymousInnerClass2(
        shared_ptr<BaseDocValuesFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Field>
BaseDocValuesFormatTestCase::FieldCreatorAnonymousInnerClass2::next()
{
  std::deque<char> bytes(random()->nextInt(10));
  random()->nextBytes(bytes);
  return make_shared<BinaryDocValuesField>(L"field",
                                           make_shared<BytesRef>(bytes));
}

shared_ptr<DocIdSetIterator>
BaseDocValuesFormatTestCase::FieldCreatorAnonymousInnerClass2::iterator(
    shared_ptr<IndexReader> r) 
{
  return MultiDocValues::getBinaryValues(r, L"field");
}

void BaseDocValuesFormatTestCase::doTestRandomAdvance(
    shared_ptr<FieldCreator> fieldCreator) 
{

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), directory, conf);
  int numChunks = atLeast(10);
  int id = 0;
  shared_ptr<Set<int>> missingSet = unordered_set<int>();
  for (int i = 0; i < numChunks; i++) {
    // change sparseness for each chunk
    double sparseChance = random()->nextDouble();
    int docCount = atLeast(1000);
    for (int j = 0; j < docCount; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<StoredField>(L"id", id));
      if (random()->nextDouble() > sparseChance) {
        doc->push_back(fieldCreator->next());
      } else {
        missingSet->add(id);
      }
      id++;
      w->addDocument(doc);
    }
  }

  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }

  // Now search the index:
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<BitSet> missing = make_shared<FixedBitSet>(r->maxDoc());
  for (int docID = 0; docID < r->maxDoc(); docID++) {
    shared_ptr<Document> doc = r->document(docID);
    if (missingSet->contains(doc->getField(L"id")->numericValue())) {
      missing->set(docID);
    }
  }

  for (int iter = 0; iter < 100; iter++) {
    shared_ptr<DocIdSetIterator> values = fieldCreator->iterator(r);
    TestUtil::assertEquals(-1, values->docID());

    while (true) {
      int docID;
      if (random()->nextBoolean()) {
        docID = values->nextDoc();
      } else {
        int range;
        if (random()->nextInt(10) == 7) {
          // big jump
          range = r->maxDoc() - values->docID();
        } else {
          // small jump
          range = 25;
        }
        int inc = TestUtil::nextInt(random(), 1, range);
        docID = values->advance(values->docID() + inc);
      }
      if (docID == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }
      assertFalse(missing->get(docID));
    }
  }

  IOUtils::close({r, w, directory});
}

bool BaseDocValuesFormatTestCase::codecAcceptsHugeBinaryValues(
    const wstring &field)
{
  return true;
}
} // namespace org::apache::lucene::index