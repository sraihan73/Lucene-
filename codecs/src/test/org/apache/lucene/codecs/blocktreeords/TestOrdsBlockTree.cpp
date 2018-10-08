using namespace std;

#include "TestOrdsBlockTree.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestOrdsBlockTree::getCodec() { return codec; }

void TestOrdsBlockTree::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<TermsEnum> te = MultiFields::getTerms(r, L"field")->begin();

  // Test next()
  TestUtil::assertEquals(make_shared<BytesRef>(L"a"), te->next());
  TestUtil::assertEquals(0LL, te->ord());
  TestUtil::assertEquals(make_shared<BytesRef>(L"b"), te->next());
  TestUtil::assertEquals(1LL, te->ord());
  TestUtil::assertEquals(make_shared<BytesRef>(L"c"), te->next());
  TestUtil::assertEquals(2LL, te->ord());
  assertNull(te->next());

  // Test seekExact by term
  assertTrue(te->seekExact(make_shared<BytesRef>(L"b")));
  TestUtil::assertEquals(1, te->ord());
  assertTrue(te->seekExact(make_shared<BytesRef>(L"a")));
  TestUtil::assertEquals(0, te->ord());
  assertTrue(te->seekExact(make_shared<BytesRef>(L"c")));
  TestUtil::assertEquals(2, te->ord());

  // Test seekExact by ord
  te->seekExact(1);
  TestUtil::assertEquals(make_shared<BytesRef>(L"b"), te->term());
  te->seekExact(0);
  TestUtil::assertEquals(make_shared<BytesRef>(L"a"), te->term());
  te->seekExact(2);
  TestUtil::assertEquals(make_shared<BytesRef>(L"c"), te->term());

  delete r;
  delete w;
  delete dir;
}

void TestOrdsBlockTree::testTwoBlocks() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  deque<wstring> terms = deque<wstring>();
  for (int i = 0; i < 36; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    terms.push_back(term);
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << endl;
    }
    doc->push_back(newTextField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  for (int i = 0; i < 36; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"m" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    terms.push_back(term);
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << endl;
    }
    doc->push_back(newTextField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  if (VERBOSE) {
    wcout << L"TEST: now forceMerge" << endl;
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<TermsEnum> te = MultiFields::getTerms(r, L"field")->begin();

  assertTrue(te->seekExact(make_shared<BytesRef>(L"mo")));
  TestUtil::assertEquals(27, te->ord());

  te->seekExact(54);
  TestUtil::assertEquals(make_shared<BytesRef>(L"s"), te->term());

  sort(terms.begin(), terms.end());

  for (int i = terms.size() - 1; i >= 0; i--) {
    te->seekExact(i);
    TestUtil::assertEquals(i, te->ord());
    TestUtil::assertEquals(terms[i], te->term()->utf8ToString());
  }

  int iters = atLeast(1000);
  for (int iter = 0; iter < iters; iter++) {
    int ord = random()->nextInt(terms.size());
    shared_ptr<BytesRef> term = make_shared<BytesRef>(terms[ord]);
    if (random()->nextBoolean()) {
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L" seek to ord=" << ord << L" of "
              << terms.size() << endl;
      }
      te->seekExact(ord);
    } else {
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L" seek to term=" << terms[ord]
              << L" ord=" << ord << L" of " << terms.size() << endl;
      }
      te->seekExact(term);
    }
    TestUtil::assertEquals(ord, te->ord());
    TestUtil::assertEquals(term, te->term());
  }

  delete r;
  delete w;
  delete dir;
}

void TestOrdsBlockTree::testThreeBlocks() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  deque<wstring> terms = deque<wstring>();
  for (int i = 0; i < 36; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    terms.push_back(term);
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << endl;
    }
    doc->push_back(newTextField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  for (int i = 0; i < 36; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"m" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    terms.push_back(term);
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << endl;
    }
    doc->push_back(newTextField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  for (int i = 0; i < 36; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"mo" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    terms.push_back(term);
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << endl;
    }
    doc->push_back(newTextField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<TermsEnum> te = MultiFields::getTerms(r, L"field")->begin();

  if (VERBOSE) {
    while (te->next() != nullptr) {
      wcout << L"TERM: " << te->ord() << L" " << te->term()->utf8ToString()
            << endl;
    }
  }

  assertTrue(te->seekExact(make_shared<BytesRef>(L"mo")));
  TestUtil::assertEquals(27, te->ord());

  te->seekExact(90);
  TestUtil::assertEquals(make_shared<BytesRef>(L"s"), te->term());

  testEnum(te, terms);

  delete r;
  delete w;
  delete dir;
}

void TestOrdsBlockTree::testEnum(shared_ptr<TermsEnum> te,
                                 deque<wstring> &terms) 
{
  sort(terms.begin(), terms.end());
  for (int i = terms.size() - 1; i >= 0; i--) {
    if (VERBOSE) {
      wcout << L"TEST: seek to ord=" << i << endl;
    }
    te->seekExact(i);
    TestUtil::assertEquals(i, te->ord());
    TestUtil::assertEquals(terms[i], te->term()->utf8ToString());
  }

  int iters = atLeast(1000);
  for (int iter = 0; iter < iters; iter++) {
    int ord = random()->nextInt(terms.size());
    if (random()->nextBoolean()) {
      te->seekExact(ord);
      TestUtil::assertEquals(terms[ord], te->term()->utf8ToString());
    } else {
      te->seekExact(make_shared<BytesRef>(terms[ord]));
      TestUtil::assertEquals(ord, te->ord());
    }
  }
}

void TestOrdsBlockTree::testFloorBlocks() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 128; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"" + StringHelper::toString(static_cast<wchar_t>(i));
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << L" bytes="
            << make_shared<BytesRef>(term) << endl;
    }
    doc->push_back(newStringField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<TermsEnum> te = MultiFields::getTerms(r, L"field")->begin();

  if (VERBOSE) {
    shared_ptr<BytesRef> term;
    while ((term = te->next()) != nullptr) {
      wcout << L"  " << te->ord() << L": " << term->utf8ToString() << endl;
    }
  }

  assertTrue(te->seekExact(make_shared<BytesRef>(L"a")));
  TestUtil::assertEquals(97, te->ord());

  te->seekExact(98);
  TestUtil::assertEquals(make_shared<BytesRef>(L"b"), te->term());

  assertTrue(te->seekExact(make_shared<BytesRef>(L"z")));
  TestUtil::assertEquals(122, te->ord());

  delete r;
  delete w;
  delete dir;
}

void TestOrdsBlockTree::testNonRootFloorBlocks() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  deque<wstring> terms = deque<wstring>();
  for (int i = 0; i < 36; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    terms.push_back(term);
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << endl;
    }
    doc->push_back(newTextField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  for (int i = 0; i < 128; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring term = L"m" + StringHelper::toString(static_cast<wchar_t>(i));
    terms.push_back(term);
    if (VERBOSE) {
      wcout << L"i=" << i << L" term=" << term << L" bytes="
            << make_shared<BytesRef>(term) << endl;
    }
    doc->push_back(newStringField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<TermsEnum> te = MultiFields::getTerms(r, L"field")->begin();

  shared_ptr<BytesRef> term;
  int ord = 0;
  while ((term = te->next()) != nullptr) {
    if (VERBOSE) {
      wcout << L"TEST: " << te->ord() << L": " << term->utf8ToString() << endl;
    }
    TestUtil::assertEquals(ord, te->ord());
    ord++;
  }

  testEnum(te, terms);

  delete r;
  delete w;
  delete dir;
}

void TestOrdsBlockTree::testSeveralNonRootBlocks() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  deque<wstring> terms = deque<wstring>();
  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 30; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      wstring term = L"" +
                     StringHelper::toString(static_cast<wchar_t>(97 + i)) +
                     StringHelper::toString(static_cast<wchar_t>(97 + j));
      terms.push_back(term);
      if (VERBOSE) {
        wcout << L"term=" << term << endl;
      }
      doc->push_back(newTextField(L"body", term, Field::Store::NO));
      w->addDocument(doc);
    }
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<TermsEnum> te = MultiFields::getTerms(r, L"body")->begin();

  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 30; j++) {
      wstring term = L"" +
                     StringHelper::toString(static_cast<wchar_t>(97 + i)) +
                     StringHelper::toString(static_cast<wchar_t>(97 + j));
      if (VERBOSE) {
        wcout << L"TEST: check term=" << term << endl;
      }
      TestUtil::assertEquals(term, te->next().utf8ToString());
      TestUtil::assertEquals(30 * i + j, te->ord());
    }
  }

  testEnum(te, terms);

  te->seekExact(0);
  TestUtil::assertEquals(L"aa", te->term()->utf8ToString());

  delete r;
  delete w;
  delete dir;
}

void TestOrdsBlockTree::testSeekCeilNotFound() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  // Get empty string in there!
  doc->push_back(newStringField(L"field", L"", Field::Store::NO));
  w->addDocument(doc);

  for (int i = 0; i < 36; i++) {
    doc = make_shared<Document>();
    wstring term = L"" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    wstring term2 = L"a" + StringHelper::toString(static_cast<wchar_t>(97 + i));
    doc->push_back(
        newTextField(L"field", term + L" " + term2, Field::Store::NO));
    w->addDocument(doc);
  }

  w->forceMerge(1);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<TermsEnum> te = MultiFields::getTerms(r, L"field")->begin();
  TestUtil::assertEquals(
      TermsEnum::SeekStatus::NOT_FOUND,
      te->seekCeil(make_shared<BytesRef>(std::deque<char>{0x22})));
  TestUtil::assertEquals(L"a", te->term()->utf8ToString());
  TestUtil::assertEquals(1LL, te->ord());
  delete r;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::codecs::blocktreeords