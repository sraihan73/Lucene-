using namespace std;

#include "TestIDVersionPostingsFormat.h"

namespace org::apache::lucene::codecs::idversion
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using SingleTokenWithPayloadTokenStream = org::apache::lucene::codecs::
    idversion::StringAndPayloadField::SingleTokenWithPayloadTokenStream;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using ConcurrentMergeScheduler =
    org::apache::lucene::index::ConcurrentMergeScheduler;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MergeScheduler = org::apache::lucene::index::MergeScheduler;
using PerThreadPKLookup = org::apache::lucene::index::PerThreadPKLookup;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using TieredMergePolicy = org::apache::lucene::index::TieredMergePolicy;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LiveFieldValues = org::apache::lucene::search::LiveFieldValues;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIDVersionPostingsFormat::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id0", 100));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id1", 110));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IDVersionSegmentTermsEnum> termsEnum =
      std::static_pointer_cast<IDVersionSegmentTermsEnum>(
          r->leaves()[0]->reader().terms(L"id").begin());
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"id0"), 50));
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"id0"), 100));
  assertFalse(termsEnum->seekExact(make_shared<BytesRef>(L"id0"), 101));
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"id1"), 50));
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"id1"), 110));
  assertFalse(termsEnum->seekExact(make_shared<BytesRef>(L"id1"), 111));
  delete r;

  delete w;
  delete dir;
}

shared_ptr<IDSource> TestIDVersionPostingsFormat::getRandomIDs()
{
  shared_ptr<IDSource> ids;
  switch (random()->nextInt(6)) {
  case 0:
    // random simple
    if (VERBOSE) {
      wcout << L"TEST: use random simple ids" << endl;
    }
    ids = make_shared<IDSourceAnonymousInnerClass>(shared_from_this());
    break;
  case 1:
    // random realistic unicode
    if (VERBOSE) {
      wcout << L"TEST: use random realistic unicode ids" << endl;
    }
    ids = make_shared<IDSourceAnonymousInnerClass2>(shared_from_this());
    break;
  case 2:
    // sequential
    if (VERBOSE) {
      wcout << L"TEST: use seuquential ids" << endl;
    }
    ids = make_shared<IDSourceAnonymousInnerClass3>(shared_from_this());
    break;
  case 3:
    // zero-pad sequential
    if (VERBOSE) {
      wcout << L"TEST: use zero-pad seuquential ids" << endl;
    }
    ids = make_shared<IDSourceAnonymousInnerClass4>(shared_from_this());
    break;
  case 4:
    // random long
    if (VERBOSE) {
      wcout << L"TEST: use random long ids" << endl;
    }
    ids = make_shared<IDSourceAnonymousInnerClass5>(shared_from_this());
    break;
  case 5:
    // zero-pad random long
    if (VERBOSE) {
      wcout << L"TEST: use zero-pad random long ids" << endl;
    }
    ids = make_shared<IDSourceAnonymousInnerClass6>(shared_from_this());
    break;
  default:
    throw make_shared<AssertionError>();
  }

  return ids;
}

TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass::
    IDSourceAnonymousInnerClass(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

wstring TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass::next()
{
  return TestUtil::randomSimpleString(LuceneTestCase::random());
}

TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass2::
    IDSourceAnonymousInnerClass2(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

wstring TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass2::next()
{
  return TestUtil::randomRealisticUnicodeString(LuceneTestCase::random());
}

TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass3::
    IDSourceAnonymousInnerClass3(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

wstring TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass3::next()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Integer::toString(upto++);
}

TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass4::
    IDSourceAnonymousInnerClass4(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
  radix = TestUtil::nextInt(LuceneTestCase::random(), Character::MIN_RADIX,
                            Character::MAX_RADIX);
  zeroPad = wstring::format(
      Locale::ROOT,
      L"%0" + to_wstring(TestUtil::nextInt(LuceneTestCase::random(), 5, 20)) +
          L"d",
      0);
}

wstring TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass4::next()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = Integer::toString(upto++);
  return zeroPad->substr(zeroPad->length() - s.length()) + s;
}

TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass5::
    IDSourceAnonymousInnerClass5(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
  radix = TestUtil::nextInt(LuceneTestCase::random(), Character::MIN_RADIX,
                            Character::MAX_RADIX);
}

wstring TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass5::next()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Long::toString(
      LuceneTestCase::random()->nextLong() & 0x3ffffffffffffffLL, radix);
}

TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass6::
    IDSourceAnonymousInnerClass6(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
  radix = TestUtil::nextInt(LuceneTestCase::random(), Character::MIN_RADIX,
                            Character::MAX_RADIX);
  zeroPad = wstring::format(Locale::ROOT, L"%015d", 0);
}

wstring TestIDVersionPostingsFormat::IDSourceAnonymousInnerClass6::next()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Long::toString(
      LuceneTestCase::random()->nextLong() & 0x3ffffffffffffffLL, radix);
}

void TestIDVersionPostingsFormat::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  int minItemsInBlock = TestUtil::nextInt(random(), 2, 50);
  int maxItemsInBlock = 2 * (minItemsInBlock - 1) + random()->nextInt(50);
  iwc->setCodec(TestUtil::alwaysPostingsFormat(
      make_shared<IDVersionPostingsFormat>(minItemsInBlock, maxItemsInBlock)));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  // IndexWriter w = new IndexWriter(dir, iwc);
  int numDocs = atLeast(1000);
  unordered_map<wstring, int64_t> idValues =
      unordered_map<wstring, int64_t>();
  int docUpto = 0;
  if (VERBOSE) {
    wcout << L"TEST: numDocs=" << numDocs << endl;
  }

  shared_ptr<IDSource> ids = getRandomIDs();
  wstring idPrefix;
  if (random()->nextBoolean()) {
    idPrefix = L"";
  } else {
    idPrefix = TestUtil::randomSimpleString(random());
    if (VERBOSE) {
      wcout << L"TEST: use id prefix: " << idPrefix << endl;
    }
  }

  bool useMonotonicVersion = random()->nextBoolean();
  if (VERBOSE) {
    wcout << L"TEST: useMonotonicVersion=" << useMonotonicVersion << endl;
  }

  deque<wstring> idsList = deque<wstring>();

  int64_t version = 0;
  while (docUpto < numDocs) {
    wstring idValue = idPrefix + ids->next();
    if (idValues.find(idValue) != idValues.end()) {
      continue;
    }
    if (useMonotonicVersion) {
      version += TestUtil::nextInt(random(), 1, 10);
    } else {
      version = random()->nextLong() & 0x3fffffffffffffffLL;
    }
    idValues.emplace(idValue, version);
    if (VERBOSE) {
      wcout << L"  " << idValue << L" -> " << version << endl;
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(makeIDField(idValue, version));
    w->addDocument(doc);
    idsList.push_back(idValue);

    if (idsList.size() > 0 && random()->nextInt(7) == 5) {
      // Randomly delete or update a previous ID
      idValue = idsList[random()->nextInt(idsList.size())];
      if (random()->nextBoolean()) {
        if (useMonotonicVersion) {
          version += TestUtil::nextInt(random(), 1, 10);
        } else {
          version = random()->nextLong() & 0x3fffffffffffffffLL;
        }
        doc = make_shared<Document>();
        doc->push_back(makeIDField(idValue, version));
        if (VERBOSE) {
          wcout << L"  update " << idValue << L" -> " << version << endl;
        }
        w->updateDocument(make_shared<Term>(L"id", idValue), doc);
        idValues.emplace(idValue, version);
      } else {
        if (VERBOSE) {
          wcout << L"  delete " << idValue << endl;
        }
        w->deleteDocuments(make_shared<Term>(L"id", idValue));
        idValues.erase(idValue);
      }
    }

    docUpto++;
  }

  shared_ptr<IndexReader> r = w->getReader();
  // IndexReader r = DirectoryReader.open(w);
  shared_ptr<PerThreadVersionPKLookup> lookup =
      make_shared<PerThreadVersionPKLookup>(r, L"id");

  deque<unordered_map::Entry<wstring, int64_t>> idValuesList =
      deque<unordered_map::Entry<wstring, int64_t>>(idValues.entrySet());
  int iters = numDocs * 5;
  for (int iter = 0; iter < iters; iter++) {
    wstring idValue;

    if (random()->nextBoolean()) {
      idValue = idValuesList[random()->nextInt(idValuesList.size())].getKey();
    } else if (random()->nextBoolean()) {
      idValue = ids->next();
    } else {
      idValue = idPrefix + TestUtil::randomSimpleString(random());
    }

    shared_ptr<BytesRef> idValueBytes = make_shared<BytesRef>(idValue);

    optional<int64_t> expectedVersion = idValues[idValue];

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" id=" << idValue
            << L" expectedVersion=" << expectedVersion << endl;
    }

    if (!expectedVersion) {
      assertEquals(L"term should not have been found (doesn't exist)", -1,
                   lookup->lookup(idValueBytes));
    } else {
      if (random()->nextBoolean()) {
        if (VERBOSE) {
          wcout << L"  lookup exact version (should be found)" << endl;
        }
        assertTrue(L"term should have been found (version too old)",
                   lookup->lookup(idValueBytes, expectedVersion.value()) != -1);
        TestUtil::assertEquals(expectedVersion.value(), lookup->getVersion());
      } else {
        if (VERBOSE) {
          wcout << L"  lookup version+1 (should not be found)" << endl;
        }
        assertEquals(L"term should not have been found (version newer)", -1,
                     lookup->lookup(idValueBytes, expectedVersion.value() + 1));
      }
    }
  }

  delete r;
  delete w;
  delete dir;
}

TestIDVersionPostingsFormat::PerThreadVersionPKLookup::PerThreadVersionPKLookup(
    shared_ptr<IndexReader> r, const wstring &field) 
    : org::apache::lucene::index::PerThreadPKLookup(r, field)
{
}

int TestIDVersionPostingsFormat::PerThreadVersionPKLookup::lookup(
    shared_ptr<BytesRef> id, int64_t version) 
{
  for (int seg = 0; seg < numSegs; seg++) {
    if ((std::static_pointer_cast<IDVersionSegmentTermsEnum>(termsEnums[seg]))
            ->seekExact(id, version)) {
      if (VERBOSE) {
        wcout << L"  found in seg=" << termsEnums[seg] << endl;
      }
      postingsEnums[seg] = termsEnums[seg]->postings(postingsEnums[seg], 0);
      int docID = postingsEnums[seg]->nextDoc();
      if (docID != PostingsEnum::NO_MORE_DOCS &&
          (liveDocs[seg] == nullptr || liveDocs[seg]->get(docID))) {
        lastVersion = (std::static_pointer_cast<IDVersionSegmentTermsEnum>(
                           termsEnums[seg]))
                          ->getVersion();
        return docBases[seg] + docID;
      }
      assert(hasDeletions);
    }
  }

  return -1;
}

int64_t TestIDVersionPostingsFormat::PerThreadVersionPKLookup::getVersion()
{
  return lastVersion;
}

shared_ptr<Field> TestIDVersionPostingsFormat::makeIDField(const wstring &id,
                                                           int64_t version)
{
  shared_ptr<BytesRef> payload = make_shared<BytesRef>(8);
  payload->length = 8;
  IDVersionPostingsFormat::longToBytes(version, payload);
  return make_shared<StringAndPayloadField>(L"id", id, payload);
}

void TestIDVersionPostingsFormat::testMoreThanOneDocPerIDOneSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  w->addDocument(doc);

  shared_ptr<Document> duplicate = make_shared<Document>();
  duplicate->push_back(makeIDField(L"id", 17));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(duplicate);
    w->commit();
  });

  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::testMoreThanOneDocPerIDTwoSegments() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  iwc->setMergePolicy(make_shared<TieredMergePolicy>());
  shared_ptr<MergeScheduler> ms = iwc->getMergeScheduler();
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
    iwc->setMergeScheduler(
        make_shared<ConcurrentMergeSchedulerAnonymousInnerClass>(
            shared_from_this(), dir));
  }
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  w->addDocument(doc);
  w->commit();
  doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  try {
    w->addDocument(doc);
    w->commit();
    w->forceMerge(1);
    fail(L"didn't hit exception");
  } catch (const invalid_argument &iae) {
    // expected: SMS will hit this
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (IOException | IllegalStateException exc) {
    // expected
    assertTrue(dynamic_cast<invalid_argument>(exc::getCause()) != nullptr);
  }
  w->rollback();
  delete dir;
}

TestIDVersionPostingsFormat::ConcurrentMergeSchedulerAnonymousInnerClass::
    ConcurrentMergeSchedulerAnonymousInnerClass(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance,
        shared_ptr<Directory> dir)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

void TestIDVersionPostingsFormat::ConcurrentMergeSchedulerAnonymousInnerClass::
    handleMergeException(shared_ptr<Directory> dir, runtime_error exc)
{
  assertTrue(dynamic_cast<invalid_argument>(exc) != nullptr);
}

void TestIDVersionPostingsFormat::testMoreThanOneDocPerIDWithUpdates() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  // Replaces the doc we just indexed:
  w->updateDocument(make_shared<Term>(L"id", L"id"), doc);
  w->commit();
  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::testMoreThanOneDocPerIDWithDeletes() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  w->addDocument(doc);
  w->deleteDocuments(make_shared<Term>(L"id", L"id"));
  doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  w->addDocument(doc);
  w->commit();
  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::testMissingPayload() 
{
  shared_ptr<Directory> dir = newDirectory();

  // MockAnalyzer minus maybePayload else it sometimes stuffs in an 8-byte
  // payload!
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(a);
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"id", L"id", Field::Store::NO));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    w->commit();
  });

  delete w;
  delete dir;
}

TestIDVersionPostingsFormat::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIDVersionPostingsFormat::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true, 100);
  tokenizer->setEnableChecks(true);
  shared_ptr<MockTokenFilter> filt =
      make_shared<MockTokenFilter>(tokenizer, MockTokenFilter::EMPTY_STOPSET);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filt);
}

void TestIDVersionPostingsFormat::testMissingPositions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"id", Field::Store::NO));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    w->commit();
  });

  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::testInvalidPayload() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringAndPayloadField>(
      L"id", L"id", make_shared<BytesRef>(L"foo")));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    w->commit();
  });

  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::
    testMoreThanOneDocPerIDWithDeletesAcrossSegments() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  w->addDocument(doc);
  w->commit();
  doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  // Replaces the doc we just indexed:
  w->updateDocument(make_shared<Term>(L"id", L"id"), doc);
  w->forceMerge(1);
  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::testCannotIndexTermVectors() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> ft =
      make_shared<FieldType>(StringAndPayloadField::TYPE);
  ft->setStoreTermVectors(true);
  shared_ptr<SingleTokenWithPayloadTokenStream> ts =
      make_shared<SingleTokenWithPayloadTokenStream>();
  shared_ptr<BytesRef> payload = make_shared<BytesRef>(8);
  payload->length = 8;
  IDVersionPostingsFormat::longToBytes(17, payload);
  ts->setValue(L"foo", payload);
  shared_ptr<Field> field = make_shared<Field>(L"id", ts, ft);
  doc->push_back(make_shared<Field>(L"id", ts, ft));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    w->commit();
    fail(L"didn't hit expected exception");
  });

  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::testMoreThanOnceInSingleDoc() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(makeIDField(L"id", 17));
  doc->push_back(makeIDField(L"id", 17));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    w->commit();
  });

  delete w;
  delete dir;
}

void TestIDVersionPostingsFormat::testInvalidVersions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  // -1
  doc->push_back(make_shared<StringAndPayloadField>(
      L"id", L"id",
      make_shared<BytesRef>(std::deque<char>{
          static_cast<char>(0xff), static_cast<char>(0xff),
          static_cast<char>(0xff), static_cast<char>(0xff),
          static_cast<char>(0xff), static_cast<char>(0xff),
          static_cast<char>(0xff), static_cast<char>(0xff)})));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    w->commit();
  });
  expectThrows(AlreadyClosedException::typeid, [&]() { w->addDocument(doc); });
  delete dir;
}

void TestIDVersionPostingsFormat::testInvalidVersions2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);
  shared_ptr<Document> doc = make_shared<Document>();
  // Long.MAX_VALUE:
  doc->push_back(make_shared<StringAndPayloadField>(
      L"id", L"id",
      make_shared<BytesRef>(std::deque<char>{
          static_cast<char>(0x7f), static_cast<char>(0xff),
          static_cast<char>(0xff), static_cast<char>(0xff),
          static_cast<char>(0xff), static_cast<char>(0xff),
          static_cast<char>(0xff), static_cast<char>(0xff)})));
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    w->commit();
  });
  expectThrows(AlreadyClosedException::typeid, [&]() { w->addDocument(doc); });

  delete dir;
}

void TestIDVersionPostingsFormat::testGlobalVersions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<IDVersionPostingsFormat>()));
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir, iwc, false);

  shared_ptr<IDSource> idsSource = getRandomIDs();
  int numIDs = atLeast(100);
  if (VERBOSE) {
    wcout << L"TEST: " << numIDs << L" ids" << endl;
  }
  shared_ptr<Set<wstring>> idsSeen = unordered_set<wstring>();
  while (idsSeen->size() < numIDs) {
    idsSeen->add(idsSource->next());
  }
  const std::deque<wstring> ids =
      idsSeen->toArray(std::deque<wstring>(numIDs));

  const std::deque<any> locks = std::deque<any>(ids.size());
  for (int i = 0; i < locks.size(); i++) {
    locks[i] = any();
  }

  shared_ptr<AtomicLong> *const nextVersion = make_shared<AtomicLong>();

  shared_ptr<SearcherManager> *const mgr =
      make_shared<SearcherManager>(w->w, make_shared<SearcherFactory>());

  const optional<int64_t> missingValue = -1LL;

  shared_ptr<LiveFieldValues<std::shared_ptr<IndexSearcher>, int64_t>>
      *const versionValues = make_shared<LiveFieldValuesAnonymousInnerClass>(
          shared_from_this(), missingValue);

  // Maps to the version the id was lasted indexed with:
  const unordered_map<wstring, int64_t> truth =
      make_shared<ConcurrentHashMap<wstring, int64_t>>();

  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);

  std::deque<std::shared_ptr<Thread>> threads(
      TestUtil::nextInt(random(), 2, 7));

  constexpr int versionType = random()->nextInt(3);

  if (VERBOSE) {
    if (versionType == 0) {
      wcout << L"TEST: use random versions" << endl;
    } else if (versionType == 1) {
      wcout << L"TEST: use monotonic versions" << endl;
    } else {
      wcout << L"TEST: use nanotime versions" << endl;
    }
  }

  // Run for 3 sec in normal tests, else 60 seconds for nightly:
  constexpr int64_t stopTime =
      System::currentTimeMillis() + (TEST_NIGHTLY ? 60000 : 3000);

  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), w, ids, locks, nextVersion, mgr, missingValue,
        versionValues, truth, startingGun, versionType, stopTime);
    threads[i]->start();
  }

  startingGun->countDown();

  // Keep reopening the NRT reader until all indexing threads are done:
  while (true) {
    delay(TestUtil::nextInt(random(), 1, 10));
    mgr->maybeRefresh();
    for (auto thread : threads) {
      if (thread->isAlive()) {
        goto refreshLoopContinue;
      }
    }

    break;
  refreshLoopContinue:;
  }
refreshLoopBreak:

  // Verify final index against truth:
  for (int i = 0; i < 2; i++) {
    mgr->maybeRefresh();
    shared_ptr<IndexSearcher> s = mgr->acquire();
    try {
      shared_ptr<IndexReader> r = s->getIndexReader();
      // cannot assert this: maybe not all IDs were indexed
      /*
      assertEquals(numIDs, r.numDocs());
      if (i == 1) {
        // After forceMerge no deleted docs:
        assertEquals(numIDs, r.maxDoc());
      }
      */
      shared_ptr<PerThreadVersionPKLookup> lookup =
          make_shared<PerThreadVersionPKLookup>(r, L"id");
      for (auto ent : truth) {
        assertTrue(lookup->lookup(make_shared<BytesRef>(ent.first), -1LL) !=
                   -1);
        TestUtil::assertEquals(ent.second::longValue(), lookup->getVersion());
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      mgr->release(s);
    }

    if (i == 1) {
      break;
    }

    // forceMerge and verify again
    w->forceMerge(1);
  }

  delete mgr;
  delete w;
  delete dir;
}

TestIDVersionPostingsFormat::LiveFieldValuesAnonymousInnerClass::
    LiveFieldValuesAnonymousInnerClass(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance,
        optional<int64_t> &missingValue)
    : org::apache::lucene::search::LiveFieldValues<
          org::apache::lucene::search::IndexSearcher, long>(mgr, missingValue)
{
  this->outerInstance = outerInstance;
  this->missingValue = missingValue;
}

optional<int64_t>
TestIDVersionPostingsFormat::LiveFieldValuesAnonymousInnerClass::
    lookupFromSearcher(shared_ptr<IndexSearcher> s, const wstring &id)
{
  // TODO: would be cleaner if we could do our PerThreadLookup here instead of
  // "up above": We always return missing: the caller then does a lookup against
  // the current reader
  return missingValue;
}

TestIDVersionPostingsFormat::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestIDVersionPostingsFormat> outerInstance,
        shared_ptr<RandomIndexWriter> w, deque<wstring> &ids,
        deque<any> &locks, shared_ptr<AtomicLong> nextVersion,
        shared_ptr<SearcherManager> mgr, optional<int64_t> &missingValue,
        shared_ptr<LiveFieldValues<std::shared_ptr<IndexSearcher>, int64_t>>
            versionValues,
        unordered_map<wstring, int64_t> &truth,
        shared_ptr<CountDownLatch> startingGun, int versionType,
        int64_t stopTime)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->ids = ids;
  this->locks = locks;
  this->nextVersion = nextVersion;
  this->mgr = mgr;
  this->missingValue = missingValue;
  this->versionValues = versionValues;
  this->truth = truth;
  this->startingGun = startingGun;
  this->versionType = versionType;
  this->stopTime = stopTime;
}

void TestIDVersionPostingsFormat::ThreadAnonymousInnerClass::run()
{
  try {
    runForReal();
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestIDVersionPostingsFormat::ThreadAnonymousInnerClass::runForReal() throw(
    IOException, InterruptedException)
{
  startingGun->await();
  shared_ptr<PerThreadVersionPKLookup> lookup = nullptr;
  shared_ptr<IndexReader> lookupReader = nullptr;
  while (System::currentTimeMillis() < stopTime) {

    // Intentionally pull version first, and then sleep/yield, to provoke
    // version conflicts:
    int64_t newVersion;
    if (versionType == 0) {
      // Random:
      newVersion = LuceneTestCase::random()->nextLong() & 0x3fffffffffffffffLL;
    } else if (versionType == 1) {
      // Monotonic
      newVersion = nextVersion->getAndIncrement();
    } else {
      newVersion = System::nanoTime();
    }

    if (versionType != 0) {
      if (LuceneTestCase::random()->nextBoolean()) {
        Thread::yield();
      } else {
        delay(TestUtil::nextInt(LuceneTestCase::random(), 1, 4));
      }
    }

    int x = LuceneTestCase::random()->nextInt(ids.size());

    // TODO: we could relax this, if e.g. we assign indexer thread based on ID.
    // This would ensure a given ID cannot be indexed at the same time in
    // multiple threads:

    {
      // Only one thread can update an ID at once:
      lock_guard<mutex> lock(locks[x]);

      wstring id = ids[x];

      // We will attempt to index id with newVersion, but only do so if id
      // wasn't yet indexed, or it was indexed with an older version (<
      // newVersion):

      // Must lookup the RT value before pulling from the index, in case a
      // reopen happens just after we lookup:
      optional<int64_t> currentVersion = versionValues->get(id);

      shared_ptr<IndexSearcher> s = mgr->acquire();
      try {
        if (VERBOSE) {
          wcout << L"\n"
                << Thread::currentThread().getName() << L": update id=" << id
                << L" newVersion=" << newVersion << endl;
        }

        if (lookup == nullptr || lookupReader != s->getIndexReader()) {
          // TODO: sort of messy; we could add reopen to
          // PerThreadVersionPKLookup?
          // TODO: this is thin ice .... that we don't incRef/decRef this reader
          // we are implicitly holding onto:
          lookupReader = s->getIndexReader();
          if (VERBOSE) {
            wcout << Thread::currentThread().getName()
                  << L": open new PK lookup reader=" << lookupReader << endl;
          }
          lookup = make_shared<PerThreadVersionPKLookup>(lookupReader, L"id");
        }

        optional<int64_t> truthVersion = truth[id];
        if (VERBOSE) {
          wcout << Thread::currentThread().getName() << L":   truthVersion="
                << truthVersion << endl;
        }

        bool doIndex;
        if (currentVersion == missingValue) {
          if (VERBOSE) {
            wcout << Thread::currentThread().getName()
                  << L":   id not in RT cache" << endl;
          }
          int otherDocID =
              lookup->lookup(make_shared<BytesRef>(id), newVersion + 1);
          if (otherDocID == -1) {
            if (VERBOSE) {
              wcout << Thread::currentThread().getName()
                    << L":   id not in index, or version is <= newVersion; "
                       L"will index"
                    << endl;
            }
            doIndex = true;
          } else {
            if (VERBOSE) {
              wcout << Thread::currentThread().getName()
                    << L":   id is in index with version="
                    << lookup->getVersion() << L"; will not index" << endl;
            }
            doIndex = false;
            if (truthVersion.value() != lookup->getVersion()) {
              wcout << Thread::currentThread() << L": now fail0!" << endl;
            }
            TestUtil::assertEquals(truthVersion.value(), lookup->getVersion());
          }
        } else {
          if (VERBOSE) {
            wcout << Thread::currentThread().getName()
                  << L":   id is in RT cache: currentVersion=" << currentVersion
                  << endl;
          }
          doIndex = newVersion > currentVersion;
        }

        if (doIndex) {
          if (VERBOSE) {
            wcout << Thread::currentThread().getName() << L":   now index"
                  << endl;
          }
          bool passes = !truthVersion || truthVersion.value() <= newVersion;
          if (passes == false) {
            wcout << Thread::currentThread() << L": now fail!" << endl;
          }
          assertTrue(passes);
          shared_ptr<Document> doc = make_shared<Document>();
          doc->push_back(makeIDField(id, newVersion));
          w->updateDocument(make_shared<Term>(L"id", id), doc);
          truth.emplace(id, newVersion);
          versionValues->add(id, newVersion);
        } else {
          if (VERBOSE) {
            wcout << Thread::currentThread().getName() << L":   skip index"
                  << endl;
          }
          assertNotNull(truthVersion);
          assertTrue(truthVersion.value() >= newVersion);
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(s);
      }
    }
  }
}
} // namespace org::apache::lucene::codecs::idversion