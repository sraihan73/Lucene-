using namespace std;

#include "TestDocsAndPositions.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocsAndPositions::setUp() 
{
  LuceneTestCase::setUp();
  fieldName = L"field" + random()->nextInt();
}

void TestDocsAndPositions::testPositionsSimple() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int i = 0; i < 39; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setOmitNorms(true);
    doc->push_back(
        newField(fieldName,
                 wstring(L"1 2 3 4 5 6 7 8 9 10 ") + L"1 2 3 4 5 6 7 8 9 10 " +
                     L"1 2 3 4 5 6 7 8 9 10 " + L"1 2 3 4 5 6 7 8 9 10",
                 customType));
    writer->addDocument(doc);
  }
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  int num = atLeast(13);
  for (int i = 0; i < num; i++) {
    shared_ptr<BytesRef> bytes = make_shared<BytesRef>(L"1");
    shared_ptr<IndexReaderContext> topReaderContext = reader->getContext();
    for (auto leafReaderContext : topReaderContext->leaves()) {
      shared_ptr<PostingsEnum> docsAndPosEnum =
          getDocsAndPositions(leafReaderContext->reader(), bytes);
      assertNotNull(docsAndPosEnum);
      if (leafReaderContext->reader()->maxDoc() == 0) {
        continue;
      }
      constexpr int advance = docsAndPosEnum->advance(
          random()->nextInt(leafReaderContext->reader()->maxDoc()));
      do {
        wstring msg =
            L"Advanced to: " + to_wstring(advance) + L" current doc: " +
            docsAndPosEnum->docID(); // TODO: + " usePayloads: " + usePayload;
        assertEquals(msg, 4, docsAndPosEnum->freq());
        assertEquals(msg, 0, docsAndPosEnum->nextPosition());
        assertEquals(msg, 4, docsAndPosEnum->freq());
        assertEquals(msg, 10, docsAndPosEnum->nextPosition());
        assertEquals(msg, 4, docsAndPosEnum->freq());
        assertEquals(msg, 20, docsAndPosEnum->nextPosition());
        assertEquals(msg, 4, docsAndPosEnum->freq());
        assertEquals(msg, 30, docsAndPosEnum->nextPosition());
      } while (docsAndPosEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    }
  }
  delete reader;
  delete directory;
}

shared_ptr<PostingsEnum> TestDocsAndPositions::getDocsAndPositions(
    shared_ptr<LeafReader> reader,
    shared_ptr<BytesRef> bytes) 
{
  shared_ptr<Terms> terms = reader->terms(fieldName);
  if (terms != nullptr) {
    shared_ptr<TermsEnum> te = terms->begin();
    if (te->seekExact(bytes)) {
      return te->postings(nullptr, PostingsEnum::ALL);
    }
  }
  return nullptr;
}

void TestDocsAndPositions::testRandomPositions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  int numDocs = atLeast(47);
  int max = 1051;
  int term = random()->nextInt(max);
  std::deque<std::deque<optional<int>>> positionsInDoc(numDocs);
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    deque<int> positions = deque<int>();
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    int num = atLeast(131);
    for (int j = 0; j < num; j++) {
      int nextInt = random()->nextInt(max);
      builder->append(nextInt)->append(L" ");
      if (nextInt == term) {
        positions.push_back(static_cast<Integer>(j));
      }
    }
    if (positions.empty()) {
      builder->append(term);
      positions.push_back(num);
    }
    doc->push_back(newField(fieldName, builder->toString(), customType));
    positionsInDoc[i] = positions.toArray(std::deque<optional<int>>(0));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  int num = atLeast(13);
  for (int i = 0; i < num; i++) {
    shared_ptr<BytesRef> bytes = make_shared<BytesRef>(L"" + to_wstring(term));
    shared_ptr<IndexReaderContext> topReaderContext = reader->getContext();
    for (auto leafReaderContext : topReaderContext->leaves()) {
      shared_ptr<PostingsEnum> docsAndPosEnum =
          getDocsAndPositions(leafReaderContext->reader(), bytes);
      assertNotNull(docsAndPosEnum);
      int initDoc = 0;
      int maxDoc = leafReaderContext->reader()->maxDoc();
      // initially advance or do next doc
      if (random()->nextBoolean()) {
        initDoc = docsAndPosEnum->nextDoc();
      } else {
        initDoc = docsAndPosEnum->advance(random()->nextInt(maxDoc));
      }
      // now run through the scorer and check if all positions are there...
      do {
        int docID = docsAndPosEnum->docID();
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        std::deque<optional<int>> pos =
            positionsInDoc[leafReaderContext->docBase + docID];
        TestUtil::assertEquals(pos.size(), docsAndPosEnum->freq());
        // number of positions read should be random - don't read all of them
        // allways
        constexpr int howMany = random()->nextInt(20) == 0
                                    ? pos.size() - random()->nextInt(pos.size())
                                    : pos.size();
        for (int j = 0; j < howMany; j++) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          assertEquals(L"iteration: " + to_wstring(i) + L" initDoc: " +
                           to_wstring(initDoc) + L" doc: " + to_wstring(docID) +
                           L" base: " + to_wstring(leafReaderContext->docBase) +
                           L" positions: " + Arrays->toString(pos),
                       pos[j], docsAndPosEnum->nextPosition());
        }

        if (random()->nextInt(10) == 0) { // once is a while advance
          if (docsAndPosEnum->advance(docID + 1 +
                                      random()->nextInt((maxDoc - docID))) ==
              DocIdSetIterator::NO_MORE_DOCS) {
            break;
          }
        }

      } while (docsAndPosEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    }
  }
  delete reader;
  delete dir;
}

void TestDocsAndPositions::testRandomDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  int numDocs = atLeast(49);
  int max = 15678;
  int term = random()->nextInt(max);
  std::deque<int> freqInDoc(numDocs);
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    for (int j = 0; j < 199; j++) {
      int nextInt = random()->nextInt(max);
      builder->append(nextInt)->append(L' ');
      if (nextInt == term) {
        freqInDoc[i]++;
      }
    }
    doc->push_back(newField(fieldName, builder->toString(), customType));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  int num = atLeast(13);
  for (int i = 0; i < num; i++) {
    shared_ptr<BytesRef> bytes = make_shared<BytesRef>(L"" + to_wstring(term));
    shared_ptr<IndexReaderContext> topReaderContext = reader->getContext();
    for (auto context : topReaderContext->leaves()) {
      int maxDoc = context->reader()->maxDoc();
      shared_ptr<PostingsEnum> postingsEnum =
          TestUtil::docs(random(), context->reader(), fieldName, bytes, nullptr,
                         PostingsEnum::FREQS);
      if (findNext(freqInDoc, context->docBase, context->docBase + maxDoc) ==
          numeric_limits<int>::max()) {
        assertNull(postingsEnum);
        continue;
      }
      assertNotNull(postingsEnum);
      postingsEnum->nextDoc();
      for (int j = 0; j < maxDoc; j++) {
        if (freqInDoc[context->docBase + j] != 0) {
          TestUtil::assertEquals(j, postingsEnum->docID());
          TestUtil::assertEquals(postingsEnum->freq(),
                                 freqInDoc[context->docBase + j]);
          if (i % 2 == 0 && random()->nextInt(10) == 0) {
            int next = findNext(freqInDoc, context->docBase + j + 1,
                                context->docBase + maxDoc) -
                       context->docBase;
            int advancedTo = postingsEnum->advance(next);
            if (next >= maxDoc) {
              TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                                     advancedTo);
            } else {
              assertTrue(L"advanced to: " + to_wstring(advancedTo) +
                             L" but should be <= " + to_wstring(next),
                         next >= advancedTo);
            }
          } else {
            postingsEnum->nextDoc();
          }
        }
      }
      assertEquals(L"docBase: " + to_wstring(context->docBase) + L" maxDoc: " +
                       to_wstring(maxDoc) + L" " + postingsEnum->getClass(),
                   DocIdSetIterator::NO_MORE_DOCS, postingsEnum->docID());
    }
  }

  delete reader;
  delete dir;
}

int TestDocsAndPositions::findNext(std::deque<int> &docs, int pos, int max)
{
  for (int i = pos; i < max; i++) {
    if (docs[i] != 0) {
      return i;
    }
  }
  return numeric_limits<int>::max();
}

void TestDocsAndPositions::testLargeNumberOfPositions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  int howMany = 1000;
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  for (int i = 0; i < 39; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
    for (int j = 0; j < howMany; j++) {
      if (j % 2 == 0) {
        builder->append(L"even ");
      } else {
        builder->append(L"odd ");
      }
    }
    doc->push_back(newField(fieldName, builder->toString(), customType));
    writer->addDocument(doc);
  }

  // now do searches
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  int num = atLeast(13);
  for (int i = 0; i < num; i++) {
    shared_ptr<BytesRef> bytes = make_shared<BytesRef>(L"even");

    shared_ptr<IndexReaderContext> topReaderContext = reader->getContext();
    for (auto leafReaderContext : topReaderContext->leaves()) {
      shared_ptr<PostingsEnum> docsAndPosEnum =
          getDocsAndPositions(leafReaderContext->reader(), bytes);
      assertNotNull(docsAndPosEnum);

      int initDoc = 0;
      int maxDoc = leafReaderContext->reader()->maxDoc();
      // initially advance or do next doc
      if (random()->nextBoolean()) {
        initDoc = docsAndPosEnum->nextDoc();
      } else {
        initDoc = docsAndPosEnum->advance(random()->nextInt(maxDoc));
      }
      wstring msg = L"Iteration: " + to_wstring(i) + L" initDoc: " +
                    to_wstring(initDoc); // TODO: + " payloads: " + usePayload;
      TestUtil::assertEquals(howMany / 2, docsAndPosEnum->freq());
      for (int j = 0; j < howMany; j += 2) {
        assertEquals(L"position missmatch index: " + to_wstring(j) +
                         L" with freq: " + to_wstring(docsAndPosEnum->freq()) +
                         L" -- " + msg,
                     j, docsAndPosEnum->nextPosition());
      }
    }
  }
  delete reader;
  delete dir;
}

void TestDocsAndPositions::testDocsEnumStart() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"foo", L"bar", Field::Store::NO));
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  shared_ptr<LeafReader> r = getOnlyLeafReader(reader);
  shared_ptr<PostingsEnum> disi =
      TestUtil::docs(random(), r, L"foo", make_shared<BytesRef>(L"bar"),
                     nullptr, PostingsEnum::NONE);
  int docid = disi->docID();
  TestUtil::assertEquals(-1, docid);
  assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);

  // now reuse and check again
  shared_ptr<TermsEnum> te = r->terms(L"foo")->begin();
  assertTrue(te->seekExact(make_shared<BytesRef>(L"bar")));
  disi = TestUtil::docs(random(), te, disi, PostingsEnum::NONE);
  docid = disi->docID();
  TestUtil::assertEquals(-1, docid);
  assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  delete writer;
  delete r;
  delete dir;
}

void TestDocsAndPositions::testDocsAndPositionsEnumStart() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"foo", L"bar", Field::Store::NO));
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  shared_ptr<LeafReader> r = getOnlyLeafReader(reader);
  shared_ptr<PostingsEnum> disi =
      r->postings(make_shared<Term>(L"foo", L"bar"), PostingsEnum::ALL);
  int docid = disi->docID();
  TestUtil::assertEquals(-1, docid);
  assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);

  // now reuse and check again
  shared_ptr<TermsEnum> te = r->terms(L"foo")->begin();
  assertTrue(te->seekExact(make_shared<BytesRef>(L"bar")));
  disi = te->postings(disi, PostingsEnum::ALL);
  docid = disi->docID();
  TestUtil::assertEquals(-1, docid);
  assertTrue(disi->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  delete writer;
  delete r;
  delete dir;
}
} // namespace org::apache::lucene::index