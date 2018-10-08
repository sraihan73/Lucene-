using namespace std;

#include "TestLongPostings.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

wstring TestLongPostings::getRandomTerm(const wstring &other) 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  while (true) {
    wstring s = TestUtil::randomRealisticUnicodeString(random());
    if (other != L"" && s == other) {
      continue;
    }
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts
    // = a.tokenStream("foo", s))
    {
      org::apache::lucene::analysis::TokenStream ts = a->tokenStream(L"foo", s);
      shared_ptr<TermToBytesRefAttribute> *const termAtt =
          ts->getAttribute(TermToBytesRefAttribute::typeid);
      ts->reset();

      int count = 0;
      bool changed = false;

      while (ts->incrementToken()) {
        shared_ptr<BytesRef> *const termBytes = termAtt->getBytesRef();
        if (count == 0 && termBytes->utf8ToString() != s) {
          // The value was changed during analysis.  Keep iterating so the
          // tokenStream is exhausted.
          changed = true;
        }
        count++;
      }

      ts->end();
      // Did we iterate just once and the value was unchanged?
      if (!changed && count == 1) {
        return s;
      }
    }
  }
}

void TestLongPostings::testLongPostings() 
{
  // Don't use _TestUtil.getTempDir so that we own the
  // randomness (ie same seed will point to same dir):
  shared_ptr<Directory> dir = newFSDirectory(
      createTempDir(wstring(L"longpostings") + L"." + random()->nextLong()));

  constexpr int NUM_DOCS = atLeast(2000);

  if (VERBOSE) {
    wcout << L"TEST: NUM_DOCS=" << NUM_DOCS << endl;
  }

  const wstring s1 = getRandomTerm(L"");
  const wstring s2 = getRandomTerm(s1);

  if (VERBOSE) {
    wcout << L"\nTEST: s1=" << s1 << L" s2=" << s2 << endl;
    /*
    for(int idx=0;idx<s1.length();idx++) {
      System.out.println("  s1 ch=0x" + Integer.toHexString(s1.charAt(idx)));
    }
    for(int idx=0;idx<s2.length();idx++) {
      System.out.println("  s2 ch=0x" + Integer.toHexString(s2.charAt(idx)));
    }
    */
  }

  shared_ptr<FixedBitSet> *const isS1 = make_shared<FixedBitSet>(NUM_DOCS);
  for (int idx = 0; idx < NUM_DOCS; idx++) {
    if (random()->nextBoolean()) {
      isS1->set(idx);
    }
  }

  shared_ptr<IndexReader> *const r;
  shared_ptr<IndexWriterConfig> *const iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE)
          ->setMergePolicy(newLogMergePolicy());
  iwc->setRAMBufferSizeMB(16.0 + 16.0 * random()->nextDouble());
  iwc->setMaxBufferedDocs(-1);
  shared_ptr<RandomIndexWriter> *const riw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  for (int idx = 0; idx < NUM_DOCS; idx++) {
    shared_ptr<Document> *const doc = make_shared<Document>();
    wstring s = isS1->get(idx) ? s1 : s2;
    shared_ptr<Field> *const f = newTextField(L"field", s, Field::Store::NO);
    constexpr int count = TestUtil::nextInt(random(), 1, 4);
    for (int ct = 0; ct < count; ct++) {
      doc->push_back(f);
    }
    riw->addDocument(doc);
  }

  r = riw->getReader();
  delete riw;

  /*
  if (VERBOSE) {
    System.out.println("TEST: terms");
    TermEnum termEnum = r.terms();
    while(termEnum.next()) {
      System.out.println("  term=" + termEnum.term() + " len=" +
  termEnum.term().text().length()); assertTrue(termEnum.docFreq() > 0);
      System.out.println("    s1?=" + (termEnum.term().text().equals(s1)) + "
  s1len=" + s1.length()); System.out.println("    s2?=" +
  (termEnum.term().text().equals(s2)) + " s2len=" + s2.length()); final std::wstring s
  = termEnum.term().text(); for(int idx=0;idx<s.length();idx++) {
        System.out.println("      ch=0x" + Integer.toHexString(s.charAt(idx)));
      }
    }
  }
  */

  TestUtil::assertEquals(NUM_DOCS, r->numDocs());
  assertTrue(r->docFreq(make_shared<Term>(L"field", s1)) > 0);
  assertTrue(r->docFreq(make_shared<Term>(L"field", s2)) > 0);

  int num = atLeast(1000);
  for (int iter = 0; iter < num; iter++) {

    const wstring term;
    constexpr bool doS1;
    if (random()->nextBoolean()) {
      term = s1;
      doS1 = true;
    } else {
      term = s2;
      doS1 = false;
    }

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" doS1=" << doS1 << endl;
    }

    shared_ptr<PostingsEnum> *const postings =
        MultiFields::getTermPositionsEnum(r, L"field",
                                          make_shared<BytesRef>(term));

    int docID = -1;
    while (docID < DocIdSetIterator::NO_MORE_DOCS) {
      constexpr int what = random()->nextInt(3);
      if (what == 0) {
        if (VERBOSE) {
          wcout << L"TEST: docID=" << docID << L"; do next()" << endl;
        }
        // nextDoc
        int expected = docID + 1;
        while (true) {
          if (expected == NUM_DOCS) {
            expected = numeric_limits<int>::max();
            break;
          } else if (isS1->get(expected) == doS1) {
            break;
          } else {
            expected++;
          }
        }
        docID = postings->nextDoc();
        if (VERBOSE) {
          wcout << L"  got docID=" << docID << endl;
        }
        TestUtil::assertEquals(expected, docID);
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }

        if (random()->nextInt(6) == 3) {
          if (VERBOSE) {
            wcout << L"    check positions" << endl;
          }
          constexpr int freq = postings->freq();
          assertTrue(freq >= 1 && freq <= 4);
          for (int pos = 0; pos < freq; pos++) {
            TestUtil::assertEquals(pos, postings->nextPosition());
            if (random()->nextBoolean()) {
              postings->getPayload();
              if (random()->nextBoolean()) {
                postings->getPayload(); // get it again
              }
            }
          }
        }
      } else {
        // advance
        constexpr int targetDocID;
        if (docID == -1) {
          targetDocID = random()->nextInt(NUM_DOCS + 1);
        } else {
          targetDocID =
              docID + TestUtil::nextInt(random(), 1, NUM_DOCS - docID);
        }
        if (VERBOSE) {
          wcout << L"TEST: docID=" << docID << L"; do advance(" << targetDocID
                << L")" << endl;
        }
        int expected = targetDocID;
        while (true) {
          if (expected == NUM_DOCS) {
            expected = numeric_limits<int>::max();
            break;
          } else if (isS1->get(expected) == doS1) {
            break;
          } else {
            expected++;
          }
        }

        docID = postings->advance(targetDocID);
        if (VERBOSE) {
          wcout << L"  got docID=" << docID << endl;
        }
        TestUtil::assertEquals(expected, docID);
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }

        if (random()->nextInt(6) == 3) {
          constexpr int freq = postings->freq();
          assertTrue(freq >= 1 && freq <= 4);
          for (int pos = 0; pos < freq; pos++) {
            TestUtil::assertEquals(pos, postings->nextPosition());
            if (random()->nextBoolean()) {
              postings->getPayload();
              if (random()->nextBoolean()) {
                postings->getPayload(); // get it again
              }
            }
          }
        }
      }
    }
  }
  delete r;
  delete dir;
}

void TestLongPostings::testLongPostingsNoPositions() 
{
  doTestLongPostingsNoPositions(IndexOptions::DOCS);
  doTestLongPostingsNoPositions(IndexOptions::DOCS_AND_FREQS);
}

void TestLongPostings::doTestLongPostingsNoPositions(
    IndexOptions options) 
{
  // Don't use _TestUtil.getTempDir so that we own the
  // randomness (ie same seed will point to same dir):
  shared_ptr<Directory> dir = newFSDirectory(
      createTempDir(wstring(L"longpostings") + L"." + random()->nextLong()));

  constexpr int NUM_DOCS = atLeast(2000);

  if (VERBOSE) {
    wcout << L"TEST: NUM_DOCS=" << NUM_DOCS << endl;
  }

  const wstring s1 = getRandomTerm(L"");
  const wstring s2 = getRandomTerm(s1);

  if (VERBOSE) {
    wcout << L"\nTEST: s1=" << s1 << L" s2=" << s2 << endl;
    /*
    for(int idx=0;idx<s1.length();idx++) {
      System.out.println("  s1 ch=0x" + Integer.toHexString(s1.charAt(idx)));
    }
    for(int idx=0;idx<s2.length();idx++) {
      System.out.println("  s2 ch=0x" + Integer.toHexString(s2.charAt(idx)));
    }
    */
  }

  shared_ptr<FixedBitSet> *const isS1 = make_shared<FixedBitSet>(NUM_DOCS);
  for (int idx = 0; idx < NUM_DOCS; idx++) {
    if (random()->nextBoolean()) {
      isS1->set(idx);
    }
  }

  shared_ptr<IndexReader> *const r;
  if (true) {
    shared_ptr<IndexWriterConfig> *const iwc =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setOpenMode(IndexWriterConfig::OpenMode::CREATE)
            ->setMergePolicy(newLogMergePolicy());
    iwc->setRAMBufferSizeMB(16.0 + 16.0 * random()->nextDouble());
    iwc->setMaxBufferedDocs(-1);
    shared_ptr<RandomIndexWriter> *const riw =
        make_shared<RandomIndexWriter>(random(), dir, iwc);

    shared_ptr<FieldType> ft =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    ft->setIndexOptions(options);
    for (int idx = 0; idx < NUM_DOCS; idx++) {
      shared_ptr<Document> *const doc = make_shared<Document>();
      wstring s = isS1->get(idx) ? s1 : s2;
      shared_ptr<Field> *const f = newField(L"field", s, ft);
      constexpr int count = TestUtil::nextInt(random(), 1, 4);
      for (int ct = 0; ct < count; ct++) {
        doc->push_back(f);
      }
      riw->addDocument(doc);
    }

    r = riw->getReader();
    delete riw;
  } else {
    r = DirectoryReader::open(dir);
  }

  /*
  if (VERBOSE) {
    System.out.println("TEST: terms");
    TermEnum termEnum = r.terms();
    while(termEnum.next()) {
      System.out.println("  term=" + termEnum.term() + " len=" +
  termEnum.term().text().length()); assertTrue(termEnum.docFreq() > 0);
      System.out.println("    s1?=" + (termEnum.term().text().equals(s1)) + "
  s1len=" + s1.length()); System.out.println("    s2?=" +
  (termEnum.term().text().equals(s2)) + " s2len=" + s2.length()); final std::wstring s
  = termEnum.term().text(); for(int idx=0;idx<s.length();idx++) {
        System.out.println("      ch=0x" + Integer.toHexString(s.charAt(idx)));
      }
    }
  }
  */

  TestUtil::assertEquals(NUM_DOCS, r->numDocs());
  assertTrue(r->docFreq(make_shared<Term>(L"field", s1)) > 0);
  assertTrue(r->docFreq(make_shared<Term>(L"field", s2)) > 0);

  int num = atLeast(1000);
  for (int iter = 0; iter < num; iter++) {

    const wstring term;
    constexpr bool doS1;
    if (random()->nextBoolean()) {
      term = s1;
      doS1 = true;
    } else {
      term = s2;
      doS1 = false;
    }

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" doS1=" << doS1 << L" term="
            << term << endl;
    }

    shared_ptr<PostingsEnum> *const docs;
    shared_ptr<PostingsEnum> *const postings;

    if (options == IndexOptions::DOCS) {
      docs = TestUtil::docs(random(), r, L"field", make_shared<BytesRef>(term),
                            nullptr, PostingsEnum::NONE);
      postings.reset();
    } else {
      docs = postings =
          TestUtil::docs(random(), r, L"field", make_shared<BytesRef>(term),
                         nullptr, PostingsEnum::FREQS);
      assert(postings != nullptr);
    }
    assert(docs != nullptr);

    int docID = -1;
    while (docID < DocIdSetIterator::NO_MORE_DOCS) {
      constexpr int what = random()->nextInt(3);
      if (what == 0) {
        if (VERBOSE) {
          wcout << L"TEST: docID=" << docID << L"; do next()" << endl;
        }
        // nextDoc
        int expected = docID + 1;
        while (true) {
          if (expected == NUM_DOCS) {
            expected = numeric_limits<int>::max();
            break;
          } else if (isS1->get(expected) == doS1) {
            break;
          } else {
            expected++;
          }
        }
        docID = docs->nextDoc();
        if (VERBOSE) {
          wcout << L"  got docID=" << docID << endl;
        }
        TestUtil::assertEquals(expected, docID);
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }

        if (random()->nextInt(6) == 3 && postings != nullptr) {
          constexpr int freq = postings->freq();
          assertTrue(freq >= 1 && freq <= 4);
        }
      } else {
        // advance
        constexpr int targetDocID;
        if (docID == -1) {
          targetDocID = random()->nextInt(NUM_DOCS + 1);
        } else {
          targetDocID =
              docID + TestUtil::nextInt(random(), 1, NUM_DOCS - docID);
        }
        if (VERBOSE) {
          wcout << L"TEST: docID=" << docID << L"; do advance(" << targetDocID
                << L")" << endl;
        }
        int expected = targetDocID;
        while (true) {
          if (expected == NUM_DOCS) {
            expected = numeric_limits<int>::max();
            break;
          } else if (isS1->get(expected) == doS1) {
            break;
          } else {
            expected++;
          }
        }

        docID = docs->advance(targetDocID);
        if (VERBOSE) {
          wcout << L"  got docID=" << docID << endl;
        }
        TestUtil::assertEquals(expected, docID);
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }

        if (random()->nextInt(6) == 3 && postings != nullptr) {
          constexpr int freq = postings->freq();
          assertTrue(L"got invalid freq=" + to_wstring(freq),
                     freq >= 1 && freq <= 4);
        }
      }
    }
  }
  delete r;
  delete dir;
}
} // namespace org::apache::lucene::index