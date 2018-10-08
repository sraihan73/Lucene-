using namespace std;

#include "TestStressIndexing2.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
int TestStressIndexing2::maxFields = 4;
int TestStressIndexing2::bigFieldSize = 10;
bool TestStressIndexing2::sameFieldOrder = false;
int TestStressIndexing2::mergeFactor = 3;
int TestStressIndexing2::maxBufferedDocs = 3;
int TestStressIndexing2::seed = 0;

void TestStressIndexing2::testRandomIWReader() 
{
  shared_ptr<Directory> dir = newMaybeVirusCheckingDirectory();

  // TODO: verify equals using IW.getReader
  shared_ptr<DocsAndWriter> dw = indexRandomIWReader(5, 3, 100, dir);
  shared_ptr<DirectoryReader> reader = dw->writer->getReader();
  dw->writer->commit();
  verifyEquals(random(), reader, dir, L"id");
  reader->close();
  delete dw->writer;
  delete dir;
}

void TestStressIndexing2::testRandom() 
{
  shared_ptr<Directory> dir1 = newMaybeVirusCheckingDirectory();
  shared_ptr<Directory> dir2 = newMaybeVirusCheckingDirectory();
  // mergeFactor=2; maxBufferedDocs=2; Map docs = indexRandom(1, 3, 2, dir1);
  bool doReaderPooling = random()->nextBoolean();
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      indexRandom(5, 3, 100, dir1, doReaderPooling);
  indexSerial(random(), docs, dir2);

  // verifying verify
  // verifyEquals(dir1, dir1, "id");
  // verifyEquals(dir2, dir2, "id");

  verifyEquals(dir1, dir2, L"id");
  delete dir1;
  delete dir2;
}

void TestStressIndexing2::testMultiConfig() 
{
  // test lots of smaller different params together

  int num = atLeast(3);
  for (int i = 0; i < num; i++) { // increase iterations for better testing
    if (VERBOSE) {
      wcout << L"\n\nTEST: top iter=" << i << endl;
    }
    sameFieldOrder = random()->nextBoolean();
    mergeFactor = random()->nextInt(3) + 2;
    maxBufferedDocs = random()->nextInt(3) + 2;
    bool doReaderPooling = random()->nextBoolean();
    seed++;

    int nThreads = random()->nextInt(5) + 1;
    int iter = random()->nextInt(5) + 1;
    int range = random()->nextInt(20) + 1;
    shared_ptr<Directory> dir1 = newDirectory();
    shared_ptr<Directory> dir2 = newDirectory();
    if (VERBOSE) {
      wcout << L"  nThreads=" << nThreads << L" iter=" << iter << L" range="
            << range << L" doPooling=" << doReaderPooling << L" sameFieldOrder="
            << sameFieldOrder << L" mergeFactor=" << mergeFactor
            << L" maxBufferedDocs=" << maxBufferedDocs << endl;
    }
    unordered_map<wstring, std::shared_ptr<Document>> docs =
        indexRandom(nThreads, iter, range, dir1, doReaderPooling);
    if (VERBOSE) {
      wcout << L"TEST: index serial" << endl;
    }
    indexSerial(random(), docs, dir2);
    if (VERBOSE) {
      wcout << L"TEST: verify" << endl;
    }
    verifyEquals(dir1, dir2, L"id");
    delete dir1;
    delete dir2;
  }
}

shared_ptr<Term> TestStressIndexing2::idTerm = make_shared<Term>(L"id", L"");
shared_ptr<java::util::Comparator<std::shared_ptr<IndexableField>>>
    TestStressIndexing2::fieldNameComparator =
        make_shared<ComparatorAnonymousInnerClass>();

TestStressIndexing2::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int TestStressIndexing2::ComparatorAnonymousInnerClass::compare(
    shared_ptr<IndexableField> o1, shared_ptr<IndexableField> o2)
{
  return o1->name().compare(o2->name());
}

shared_ptr<DocsAndWriter> TestStressIndexing2::indexRandomIWReader(
    int nThreads, int iterations, int range,
    shared_ptr<Directory> dir) 
{
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      unordered_map<wstring, std::shared_ptr<Document>>();
  shared_ptr<IndexWriter> w = RandomIndexWriter::mockIndexWriter(
      dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(OpenMode::CREATE)
          ->setRAMBufferSizeMB(0.1)
          ->setMaxBufferedDocs(maxBufferedDocs)
          ->setMergePolicy(newLogMergePolicy()),
      random());
  w->commit();
  shared_ptr<LogMergePolicy> lmp = std::static_pointer_cast<LogMergePolicy>(
      w->getConfig()->getMergePolicy());
  lmp->setNoCFSRatio(0.0);
  lmp->setMergeFactor(mergeFactor);
  /***
      w.setMaxMergeDocs(Integer.MAX_VALUE);
      w.setMaxFieldLength(10000);
      w.setRAMBufferSizeMB(1);
      w.setMergeFactor(10);
  ***/

  threads = std::deque<std::shared_ptr<IndexingThread>>(nThreads);
  for (int i = 0; i < threads.size(); i++) {
    shared_ptr<IndexingThread> th = make_shared<IndexingThread>();
    th->w = w;
    th->base = 1000000 * i;
    th->range = range;
    th->iterations = iterations;
    threads[i] = th;
  }

  for (int i = 0; i < threads.size(); i++) {
    threads[i]->start();
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
  }

  // w.forceMerge(1);
  // w.close();

  for (int i = 0; i < threads.size(); i++) {
    shared_ptr<IndexingThread> th = threads[i];
    {
      lock_guard<mutex> lock(th);
      docs.putAll(th->docs);
    }
  }

  TestUtil::checkIndex(dir);
  shared_ptr<DocsAndWriter> dw = make_shared<DocsAndWriter>();
  dw->docs = docs;
  dw->writer = w;
  return dw;
}

unordered_map<wstring, std::shared_ptr<Document>>
TestStressIndexing2::indexRandom(
    int nThreads, int iterations, int range, shared_ptr<Directory> dir,
    bool doReaderPooling) 
{
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      unordered_map<wstring, std::shared_ptr<Document>>();
  shared_ptr<IndexWriter> w = RandomIndexWriter::mockIndexWriter(
      dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(OpenMode::CREATE)
          ->setRAMBufferSizeMB(0.1)
          ->setMaxBufferedDocs(maxBufferedDocs)
          ->setReaderPooling(doReaderPooling)
          ->setMergePolicy(newLogMergePolicy()),
      random());
  shared_ptr<LogMergePolicy> lmp = std::static_pointer_cast<LogMergePolicy>(
      w->getConfig()->getMergePolicy());
  lmp->setNoCFSRatio(0.0);
  lmp->setMergeFactor(mergeFactor);

  threads = std::deque<std::shared_ptr<IndexingThread>>(nThreads);
  for (int i = 0; i < threads.size(); i++) {
    shared_ptr<IndexingThread> th = make_shared<IndexingThread>();
    th->w = w;
    th->base = 1000000 * i;
    th->range = range;
    th->iterations = iterations;
    threads[i] = th;
  }

  for (int i = 0; i < threads.size(); i++) {
    threads[i]->start();
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
  }

  // w.forceMerge(1);
  delete w;

  for (int i = 0; i < threads.size(); i++) {
    shared_ptr<IndexingThread> th = threads[i];
    {
      lock_guard<mutex> lock(th);
      docs.putAll(th->docs);
    }
  }

  // System.out.println("TEST: checkindex");
  TestUtil::checkIndex(dir);

  return docs;
}

void TestStressIndexing2::indexSerial(
    shared_ptr<Random> random,
    unordered_map<wstring, std::shared_ptr<Document>> &docs,
    shared_ptr<Directory> dir) 
{
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, LuceneTestCase::newIndexWriterConfig(
               random, make_shared<MockAnalyzer>(random))
               ->setMergePolicy(newLogMergePolicy()));

  // index all docs in a single thread
  Iterator<std::shared_ptr<Document>> iter = docs.values().begin();
  while (iter->hasNext()) {
    shared_ptr<Document> d = iter->next();
    deque<std::shared_ptr<IndexableField>> fields =
        deque<std::shared_ptr<IndexableField>>();
    fields.addAll(d->getFields());
    // put fields in same order each time
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(fields, fieldNameComparator);
    sort(fields.begin(), fields.end(), fieldNameComparator);

    shared_ptr<Document> d1 = make_shared<Document>();
    for (int i = 0; i < fields.size(); i++) {
      d1->push_back(fields[i]);
    }
    w->addDocument(d1);
    // System.out.println("indexing "+d1);
    iter++;
  }

  delete w;
}

void TestStressIndexing2::verifyEquals(
    shared_ptr<Random> r, shared_ptr<DirectoryReader> r1,
    shared_ptr<Directory> dir2, const wstring &idField) 
{
  shared_ptr<DirectoryReader> r2 = DirectoryReader::open(dir2);
  verifyEquals(r1, r2, idField);
  r2->close();
}

void TestStressIndexing2::verifyEquals(
    shared_ptr<Directory> dir1, shared_ptr<Directory> dir2,
    const wstring &idField) 
{
  shared_ptr<DirectoryReader> r1 = DirectoryReader::open(dir1);
  shared_ptr<DirectoryReader> r2 = DirectoryReader::open(dir2);
  verifyEquals(r1, r2, idField);
  r1->close();
  r2->close();
}

void TestStressIndexing2::printDocs(shared_ptr<DirectoryReader> r) throw(
    runtime_error)
{
  for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
    // TODO: improve this
    shared_ptr<LeafReader> sub = ctx->reader();
    shared_ptr<Bits> liveDocs = sub->getLiveDocs();
    wcout << L"  "
          << (std::static_pointer_cast<SegmentReader>(sub))->getSegmentInfo()
          << endl;
    for (int docID = 0; docID < sub->maxDoc(); docID++) {
      shared_ptr<Document> doc = sub->document(docID);
      if (liveDocs == nullptr || liveDocs->get(docID)) {
        wcout << L"    docID=" << docID << L" id:" << doc[L"id"] << endl;
      } else {
        wcout << L"    DEL docID=" << docID << L" id:" << doc[L"id"] << endl;
      }
    }
  }
}

int TestStressIndexing2::nextNonDeletedDoc(
    shared_ptr<PostingsEnum> it, shared_ptr<Bits> liveDocs) 
{
  int doc = it->nextDoc();
  while (doc != DocIdSetIterator::NO_MORE_DOCS && liveDocs != nullptr &&
         liveDocs->get(doc) == false) {
    doc = it->nextDoc();
  }
  return doc;
}

void TestStressIndexing2::verifyEquals(
    shared_ptr<DirectoryReader> r1, shared_ptr<DirectoryReader> r2,
    const wstring &idField) 
{
  if (VERBOSE) {
    wcout << L"\nr1 docs:" << endl;
    printDocs(r1);
    wcout << L"\nr2 docs:" << endl;
    printDocs(r2);
  }
  if (r1->numDocs() != r2->numDocs()) {
    assert((false, L"r1.numDocs()=" + to_wstring(r1->numDocs()) +
                       L" vs r2.numDocs()=" + to_wstring(r2->numDocs())));
  }
  bool hasDeletes =
      !(r1->maxDoc() == r2->maxDoc() && r1->numDocs() == r1->maxDoc());

  std::deque<int> r2r1(r2->maxDoc()); // r2 id to r1 id mapping

  // create mapping from id2 space to id2 based on idField
  shared_ptr<Fields> *const f1 = MultiFields::getFields(r1);
  if (f1->empty()) {
    // make sure r2 is empty
    assertNull(MultiFields::getFields(r2));
    return;
  }
  shared_ptr<Terms> *const terms1 = f1->terms(idField);
  if (terms1 == nullptr) {
    assertTrue(MultiFields::getFields(r2)->empty() ||
               MultiFields::getFields(r2)->terms(idField) == nullptr);
    return;
  }
  shared_ptr<TermsEnum> *const termsEnum = terms1->begin();

  shared_ptr<Bits> *const liveDocs1 = MultiFields::getLiveDocs(r1);
  shared_ptr<Bits> *const liveDocs2 = MultiFields::getLiveDocs(r2);

  shared_ptr<Fields> fields = MultiFields::getFields(r2);
  shared_ptr<Terms> terms2 = fields->terms(idField);
  if (fields->empty() || terms2 == nullptr) {
    // make sure r1 is in fact empty (eg has only all
    // deleted docs):
    shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(r1);
    shared_ptr<PostingsEnum> docs = nullptr;
    while (termsEnum->next() != nullptr) {
      docs = TestUtil::docs(random(), termsEnum, docs, PostingsEnum::NONE);
      while (nextNonDeletedDoc(docs, liveDocs) !=
             DocIdSetIterator::NO_MORE_DOCS) {
        fail(L"r1 is not empty but r2 is");
      }
    }
    return;
  }
  shared_ptr<TermsEnum> termsEnum2 = terms2->begin();

  shared_ptr<PostingsEnum> termDocs1 = nullptr;
  shared_ptr<PostingsEnum> termDocs2 = nullptr;

  while (true) {
    shared_ptr<BytesRef> term = termsEnum->next();
    // System.out.println("TEST: match id term=" + term);
    if (term == nullptr) {
      break;
    }

    termDocs1 =
        TestUtil::docs(random(), termsEnum, termDocs1, PostingsEnum::NONE);
    if (termsEnum2->seekExact(term)) {
      termDocs2 =
          TestUtil::docs(random(), termsEnum2, termDocs2, PostingsEnum::NONE);
    } else {
      termDocs2.reset();
    }

    if (nextNonDeletedDoc(termDocs1, liveDocs1) ==
        DocIdSetIterator::NO_MORE_DOCS) {
      // This doc is deleted and wasn't replaced
      assertTrue(termDocs2 == nullptr ||
                 nextNonDeletedDoc(termDocs2, liveDocs2) ==
                     DocIdSetIterator::NO_MORE_DOCS);
      continue;
    }

    int id1 = termDocs1->docID();
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                           nextNonDeletedDoc(termDocs1, liveDocs1));

    assertTrue(nextNonDeletedDoc(termDocs2, liveDocs2) !=
               DocIdSetIterator::NO_MORE_DOCS);
    int id2 = termDocs2->docID();
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                           nextNonDeletedDoc(termDocs2, liveDocs2));

    r2r1[id2] = id1;

    // verify stored fields are equivalent
    try {
      verifyEquals(r1->document(id1), r2->document(id2));
    } catch (const runtime_error &t) {
      wcout << L"FAILED id=" << term << L" id1=" << id1 << L" id2=" << id2
            << L" term=" << term << endl;
      wcout << L"  d1=" << r1->document(id1) << endl;
      wcout << L"  d2=" << r2->document(id2) << endl;
      throw t;
    }

    try {
      // verify term vectors are equivalent
      verifyEquals(r1->getTermVectors(id1), r2->getTermVectors(id2));
    } catch (const runtime_error &e) {
      wcout << L"FAILED id=" << term << L" id1=" << id1 << L" id2=" << id2
            << endl;
      shared_ptr<Fields> tv1 = r1->getTermVectors(id1);
      wcout << L"  d1=" << tv1 << endl;
      if (tv1->size() > 0) {
        shared_ptr<PostingsEnum> dpEnum = nullptr;
        shared_ptr<PostingsEnum> dEnum = nullptr;
        for (auto field : tv1) {
          wcout << L"    " << field << L":" << endl;
          shared_ptr<Terms> terms3 = tv1->terms(field);
          assertNotNull(terms3);
          shared_ptr<TermsEnum> termsEnum3 = terms3->begin();
          shared_ptr<BytesRef> term2;
          while ((term2 = termsEnum3->next()) != nullptr) {
            wcout << L"      " << term2->utf8ToString() << L": freq="
                  << termsEnum3->totalTermFreq() << endl;
            dpEnum = termsEnum3->postings(dpEnum, PostingsEnum::ALL);
            if (terms3->hasPositions()) {
              assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
              constexpr int freq = dpEnum->freq();
              wcout << L"        doc=" << dpEnum->docID() << L" freq=" << freq
                    << endl;
              for (int posUpto = 0; posUpto < freq; posUpto++) {
                wcout << L"          pos=" << dpEnum->nextPosition() << endl;
              }
            } else {
              dEnum = TestUtil::docs(random(), termsEnum3, dEnum,
                                     PostingsEnum::FREQS);
              assertNotNull(dEnum);
              assertTrue(dEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
              constexpr int freq = dEnum->freq();
              wcout << L"        doc=" << dEnum->docID() << L" freq=" << freq
                    << endl;
            }
          }
        }
      }

      shared_ptr<Fields> tv2 = r2->getTermVectors(id2);
      wcout << L"  d2=" << tv2 << endl;
      if (tv2->size() > 0) {
        shared_ptr<PostingsEnum> dpEnum = nullptr;
        shared_ptr<PostingsEnum> dEnum = nullptr;
        for (auto field : tv2) {
          wcout << L"    " << field << L":" << endl;
          shared_ptr<Terms> terms3 = tv2->terms(field);
          assertNotNull(terms3);
          shared_ptr<TermsEnum> termsEnum3 = terms3->begin();
          shared_ptr<BytesRef> term2;
          while ((term2 = termsEnum3->next()) != nullptr) {
            wcout << L"      " << term2->utf8ToString() << L": freq="
                  << termsEnum3->totalTermFreq() << endl;
            dpEnum = termsEnum3->postings(dpEnum, PostingsEnum::ALL);
            if (dpEnum != nullptr) {
              assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
              constexpr int freq = dpEnum->freq();
              wcout << L"        doc=" << dpEnum->docID() << L" freq=" << freq
                    << endl;
              for (int posUpto = 0; posUpto < freq; posUpto++) {
                wcout << L"          pos=" << dpEnum->nextPosition() << endl;
              }
            } else {
              dEnum = TestUtil::docs(random(), termsEnum3, dEnum,
                                     PostingsEnum::FREQS);
              assertNotNull(dEnum);
              assertTrue(dEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
              constexpr int freq = dEnum->freq();
              wcout << L"        doc=" << dEnum->docID() << L" freq=" << freq
                    << endl;
            }
          }
        }
      }

      throw e;
    }
  }

  // System.out.println("TEST: done match id");

  // Verify postings
  // System.out.println("TEST: create te1");
  shared_ptr<Fields> *const fields1 = MultiFields::getFields(r1);
  constexpr org::apache::lucene::index::Fields::const_iterator fields1Enum =
      fields1->begin();
  shared_ptr<Fields> *const fields2 = MultiFields::getFields(r2);
  constexpr org::apache::lucene::index::Fields::const_iterator fields2Enum =
      fields2->begin();

  wstring field1 = L"", field2 = L"";
  shared_ptr<TermsEnum> termsEnum1 = nullptr;
  termsEnum2.reset();
  shared_ptr<PostingsEnum> docs1 = nullptr, docs2 = nullptr;

  // pack both doc and freq into single element for easy sorting
  std::deque<int64_t> info1(r1->numDocs());
  std::deque<int64_t> info2(r2->numDocs());

  for (;;) {
    shared_ptr<BytesRef> term1 = nullptr, term2 = nullptr;

    // iterate until we get some docs
    int len1;
    for (;;) {
      len1 = 0;
      if (termsEnum1 == nullptr) {
        // C++ TODO: Java iterators are only converted within the context of
        // 'while' and 'for' loops:
        if (!fields1Enum->hasNext()) {
          break;
        }
        // C++ TODO: Java iterators are only converted within the context of
        // 'while' and 'for' loops:
        field1 = fields1Enum->next();
        shared_ptr<Terms> terms = fields1->terms(field1);
        if (terms == nullptr) {
          continue;
        }
        termsEnum1 = terms->begin();
      }
      term1 = termsEnum1->next();
      if (term1 == nullptr) {
        // no more terms in this field
        termsEnum1.reset();
        continue;
      }

      // System.out.println("TEST: term1=" + term1);
      docs1 = TestUtil::docs(random(), termsEnum1, docs1, PostingsEnum::FREQS);
      while (docs1->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        int d = docs1->docID();
        if (liveDocs1 != nullptr && liveDocs1->get(d) == false) {
          continue;
        }
        int f = docs1->freq();
        info1[len1] = ((static_cast<int64_t>(d)) << 32) | f;
        len1++;
      }
      if (len1 > 0) {
        break;
      }
    }

    // iterate until we get some docs
    int len2;
    for (;;) {
      len2 = 0;
      if (termsEnum2 == nullptr) {
        // C++ TODO: Java iterators are only converted within the context of
        // 'while' and 'for' loops:
        if (!fields2Enum->hasNext()) {
          break;
        }
        // C++ TODO: Java iterators are only converted within the context of
        // 'while' and 'for' loops:
        field2 = fields2Enum->next();
        shared_ptr<Terms> terms = fields2->terms(field2);
        if (terms == nullptr) {
          continue;
        }
        termsEnum2 = terms->begin();
      }
      term2 = termsEnum2->next();
      if (term2 == nullptr) {
        // no more terms in this field
        termsEnum2.reset();
        continue;
      }

      // System.out.println("TEST: term1=" + term1);
      docs2 = TestUtil::docs(random(), termsEnum2, docs2, PostingsEnum::FREQS);
      while (docs2->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        if (liveDocs2 != nullptr && liveDocs2->get(docs2->docID()) == false) {
          continue;
        }
        int d = r2r1[docs2->docID()];
        int f = docs2->freq();
        info2[len2] = ((static_cast<int64_t>(d)) << 32) | f;
        len2++;
      }
      if (len2 > 0) {
        break;
      }
    }

    TestUtil::assertEquals(len1, len2);
    if (len1 == 0) {
      break; // no more terms
    }

    TestUtil::assertEquals(field1, field2);
    TestUtil::assertEquals(term1, term2);

    if (!hasDeletes) {
      TestUtil::assertEquals(termsEnum1->docFreq(), termsEnum2->docFreq());
    }

    assertEquals(L"len1=" + to_wstring(len1) + L" len2=" + to_wstring(len2) +
                     L" deletes?=" + StringHelper::toString(hasDeletes),
                 term1, term2);

    // sort info2 to get it into ascending docid
    Arrays::sort(info2, 0, len2);

    // now compare
    for (int i = 0; i < len1; i++) {
      assertEquals(
          L"i=" + to_wstring(i) + L" len=" + to_wstring(len1) + L" d1=" +
              to_wstring(static_cast<int64_t>(
                  static_cast<uint64_t>(info1[i]) >> 32)) +
              L" f1=" + to_wstring(info1[i] & numeric_limits<int>::max()) +
              L" d2=" +
              to_wstring(static_cast<int64_t>(
                  static_cast<uint64_t>(info2[i]) >> 32)) +
              L" f2=" + to_wstring(info2[i] & numeric_limits<int>::max()) +
              L" field=" + field1 + L" term=" + term1->utf8ToString(),
          info1[i], info2[i]);
    }
  }
}

void TestStressIndexing2::verifyEquals(shared_ptr<Document> d1,
                                       shared_ptr<Document> d2)
{
  deque<std::shared_ptr<IndexableField>> ff1 =
      deque<std::shared_ptr<IndexableField>>(d1->getFields());
  deque<std::shared_ptr<IndexableField>> ff2 =
      deque<std::shared_ptr<IndexableField>>(d2->getFields());

  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(ff1, fieldNameComparator);
  sort(ff1.begin(), ff1.end(), fieldNameComparator);
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(ff2, fieldNameComparator);
  sort(ff2.begin(), ff2.end(), fieldNameComparator);

  assertEquals(ff1 + L" : " + ff2, ff1.size(), ff2.size());

  for (int i = 0; i < ff1.size(); i++) {
    shared_ptr<IndexableField> f1 = ff1[i];
    shared_ptr<IndexableField> f2 = ff2[i];
    if (f1->binaryValue() != nullptr) {
      assert(f2->binaryValue() != nullptr);
    } else {
      wstring s1 = f1->stringValue();
      wstring s2 = f2->stringValue();
      assertEquals(ff1 + L" : " + ff2, s1, s2);
    }
  }
}

void TestStressIndexing2::verifyEquals(shared_ptr<Fields> d1,
                                       shared_ptr<Fields> d2) 
{
  if (d1->empty()) {
    assertTrue(d2->empty() || d2->empty());
    return;
  }
  assertTrue(d2->size() > 0);

  org::apache::lucene::index::Fields::const_iterator fieldsEnum2 = d2->begin();

  for (auto field1 : d1) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    wstring field2 = fieldsEnum2->next();
    TestUtil::assertEquals(field1, field2);

    shared_ptr<Terms> terms1 = d1->terms(field1);
    assertNotNull(terms1);
    shared_ptr<TermsEnum> termsEnum1 = terms1->begin();

    shared_ptr<Terms> terms2 = d2->terms(field2);
    assertNotNull(terms2);
    shared_ptr<TermsEnum> termsEnum2 = terms2->begin();

    shared_ptr<PostingsEnum> dpEnum1 = nullptr;
    shared_ptr<PostingsEnum> dpEnum2 = nullptr;
    shared_ptr<PostingsEnum> dEnum1 = nullptr;
    shared_ptr<PostingsEnum> dEnum2 = nullptr;

    shared_ptr<BytesRef> term1;
    while ((term1 = termsEnum1->next()) != nullptr) {
      shared_ptr<BytesRef> term2 = termsEnum2->next();
      TestUtil::assertEquals(term1, term2);
      TestUtil::assertEquals(termsEnum1->totalTermFreq(),
                             termsEnum2->totalTermFreq());

      dpEnum1 = termsEnum1->postings(dpEnum1, PostingsEnum::ALL);
      dpEnum2 = termsEnum2->postings(dpEnum2, PostingsEnum::ALL);

      if (terms1->hasPositions()) {
        assertTrue(terms2->hasPositions());
        int docID1 = dpEnum1->nextDoc();
        dpEnum2->nextDoc();
        // docIDs are not supposed to be equal
        // int docID2 = dpEnum2.nextDoc();
        // assertEquals(docID1, docID2);
        assertTrue(docID1 != DocIdSetIterator::NO_MORE_DOCS);

        int freq1 = dpEnum1->freq();
        int freq2 = dpEnum2->freq();
        TestUtil::assertEquals(freq1, freq2);

        for (int posUpto = 0; posUpto < freq1; posUpto++) {
          int pos1 = dpEnum1->nextPosition();
          int pos2 = dpEnum2->nextPosition();
          TestUtil::assertEquals(pos1, pos2);
          if (terms1->hasOffsets()) {
            assertTrue(terms2->hasOffsets());
            TestUtil::assertEquals(dpEnum1->startOffset(),
                                   dpEnum2->startOffset());
            TestUtil::assertEquals(dpEnum1->endOffset(), dpEnum2->endOffset());
          }
        }
        TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                               dpEnum1->nextDoc());
        TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                               dpEnum2->nextDoc());
      } else {
        dEnum1 =
            TestUtil::docs(random(), termsEnum1, dEnum1, PostingsEnum::FREQS);
        dEnum2 =
            TestUtil::docs(random(), termsEnum2, dEnum2, PostingsEnum::FREQS);
        assertNotNull(dEnum1);
        assertNotNull(dEnum2);
        int docID1 = dEnum1->nextDoc();
        dEnum2->nextDoc();
        // docIDs are not supposed to be equal
        // int docID2 = dEnum2.nextDoc();
        // assertEquals(docID1, docID2);
        assertTrue(docID1 != DocIdSetIterator::NO_MORE_DOCS);
        int freq1 = dEnum1->freq();
        int freq2 = dEnum2->freq();
        TestUtil::assertEquals(freq1, freq2);
        TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                               dEnum1->nextDoc());
        TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                               dEnum2->nextDoc());
      }
    }

    assertNull(termsEnum2->next());
  }
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(fieldsEnum2->hasNext());
}

int TestStressIndexing2::IndexingThread::nextInt(int lim)
{
  return r->nextInt(lim);
}

int TestStressIndexing2::IndexingThread::nextInt(int start, int end)
{
  return start + r->nextInt(end - start);
}

int TestStressIndexing2::IndexingThread::addUTF8Token(int start)
{
  constexpr int end = start + nextInt(20);
  if (buffer.size() < 1 + end) {
    std::deque<wchar_t> newBuffer(static_cast<int>((1 + end) * 1.25));
    System::arraycopy(buffer, 0, newBuffer, 0, buffer.size());
    buffer = newBuffer;
  }

  for (int i = start; i < end; i++) {
    int t = nextInt(5);
    if (0 == t && i < end - 1) {
      // Make a surrogate pair
      // High surrogate
      buffer[i++] = static_cast<wchar_t>(nextInt(0xd800, 0xdc00));
      // Low surrogate
      buffer[i] = static_cast<wchar_t>(nextInt(0xdc00, 0xe000));
    } else if (t <= 1) {
      buffer[i] = static_cast<wchar_t>(nextInt(0x80));
    } else if (2 == t) {
      buffer[i] = static_cast<wchar_t>(nextInt(0x80, 0x800));
    } else if (3 == t) {
      buffer[i] = static_cast<wchar_t>(nextInt(0x800, 0xd800));
    } else if (4 == t) {
      buffer[i] = static_cast<wchar_t>(nextInt(0xe000, 0xffff));
    }
  }
  buffer[end] = L' ';
  return 1 + end;
}

wstring TestStressIndexing2::IndexingThread::getString(int nTokens)
{
  nTokens = nTokens != 0 ? nTokens : r->nextInt(4) + 1;

  // Half the time make a random UTF8 string
  if (r->nextBoolean()) {
    return getUTF8String(nTokens);
  }

  // avoid StringBuffer because it adds extra synchronization.
  std::deque<wchar_t> arr(nTokens * 2);
  for (int i = 0; i < nTokens; i++) {
    arr[i * 2] = static_cast<wchar_t>(L'A' + r->nextInt(10));
    arr[i * 2 + 1] = L' ';
  }
  return wstring(arr);
}

wstring TestStressIndexing2::IndexingThread::getUTF8String(int nTokens)
{
  int upto = 0;
  Arrays::fill(buffer, static_cast<wchar_t>(0));
  for (int i = 0; i < nTokens; i++) {
    upto = addUTF8Token(upto);
  }
  return wstring(buffer, 0, upto);
}

wstring TestStressIndexing2::IndexingThread::getIdString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Integer::toString(base + nextInt(range));
}

void TestStressIndexing2::IndexingThread::indexDoc() 
{
  shared_ptr<Document> d = make_shared<Document>();

  shared_ptr<FieldType> customType1 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType1->setTokenized(false);
  customType1->setOmitNorms(true);

  deque<std::shared_ptr<Field>> fields = deque<std::shared_ptr<Field>>();
  wstring idString = getIdString();
  shared_ptr<Field> idField =
      LuceneTestCase::newField(L"id", idString, customType1);
  fields.push_back(idField);

  unordered_map<wstring, std::shared_ptr<FieldType>> tvTypes =
      unordered_map<wstring, std::shared_ptr<FieldType>>();

  int nFields = nextInt(maxFields);
  for (int i = 0; i < nFields; i++) {

    wstring fieldName = L"f" + to_wstring(nextInt(100));
    shared_ptr<FieldType> customType;

    // Use the same term deque settings if we already
    // added this field to the doc:
    shared_ptr<FieldType> oldTVType = tvTypes[fieldName];
    if (oldTVType != nullptr) {
      customType = make_shared<FieldType>(oldTVType);
    } else {
      customType = make_shared<FieldType>();
      switch (nextInt(4)) {
      case 0:
        break;
      case 1:
        customType->setStoreTermVectors(true);
        break;
      case 2:
        customType->setStoreTermVectors(true);
        customType->setStoreTermVectorPositions(true);
        break;
      case 3:
        customType->setStoreTermVectors(true);
        customType->setStoreTermVectorOffsets(true);
        break;
      }
      shared_ptr<FieldType> newType = make_shared<FieldType>(customType);
      newType->freeze();
      tvTypes.emplace(fieldName, newType);
    }

    switch (nextInt(4)) {
    case 0:
      customType->setStored(true);
      customType->setOmitNorms(true);
      customType->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
      customType->freeze();
      fields.push_back(
          LuceneTestCase::newField(fieldName, getString(1), customType));
      break;
    case 1:
      customType->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
      customType->setTokenized(true);
      customType->freeze();
      fields.push_back(
          LuceneTestCase::newField(fieldName, getString(0), customType));
      break;
    case 2:
      customType->setStored(true);
      customType->setStoreTermVectors(false);
      customType->setStoreTermVectorOffsets(false);
      customType->setStoreTermVectorPositions(false);
      customType->freeze();
      fields.push_back(
          LuceneTestCase::newField(fieldName, getString(0), customType));
      break;
    case 3:
      customType->setStored(true);
      customType->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
      customType->setTokenized(true);
      customType->freeze();
      fields.push_back(LuceneTestCase::newField(
          fieldName, getString(bigFieldSize), customType));
      break;
    }
  }

  if (sameFieldOrder) {
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(fields, fieldNameComparator);
    sort(fields.begin(), fields.end(), fieldNameComparator);
  } else {
    // random placement of id field also
    Collections::swap(fields, nextInt(fields.size()), 0);
  }

  for (int i = 0; i < fields.size(); i++) {
    d->push_back(fields[i]);
  }
  if (VERBOSE) {
    wcout << Thread::currentThread().getName() << L": indexing id:" << idString
          << endl;
  }
  w->updateDocument(make_shared<Term>(L"id", idString), d);
  // System.out.println(Thread.currentThread().getName() + ": indexing "+d);
  docs.emplace(idString, d);
}

void TestStressIndexing2::IndexingThread::deleteDoc() 
{
  wstring idString = getIdString();
  if (VERBOSE) {
    wcout << Thread::currentThread().getName() << L": del id:" << idString
          << endl;
  }
  w->deleteDocuments({make_shared<Term>(L"id", idString)});
  docs.erase(idString);
}

void TestStressIndexing2::IndexingThread::deleteByQuery() 
{
  wstring idString = getIdString();
  if (VERBOSE) {
    wcout << Thread::currentThread().getName() << L": del query id:" << idString
          << endl;
  }
  w->deleteDocuments(
      {make_shared<TermQuery>(make_shared<Term>(L"id", idString))});
  docs.erase(idString);
}

void TestStressIndexing2::IndexingThread::run()
{
  try {
    r = make_shared<Random>(base + range + seed);
    for (int i = 0; i < iterations; i++) {
      int what = nextInt(100);
      if (what < 5) {
        deleteDoc();
      } else if (what < 10) {
        deleteByQuery();
      } else {
        indexDoc();
      }
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this()) { docs.size(); }
}
} // namespace org::apache::lucene::index