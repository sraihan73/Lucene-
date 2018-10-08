using namespace std;

#include "Test2BTerms.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;
const shared_ptr<org::apache::lucene::util::BytesRef> Test2BTerms::bytes =
    make_shared<org::apache::lucene::util::BytesRef>(TOKEN_LEN);

Test2BTerms::MyTokenStream::MyTokenStream(shared_ptr<Random> random,
                                          int tokensPerDoc)
    : org::apache::lucene::analysis::TokenStream(
          new MyAttributeFactory(AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY)),
      tokensPerDoc(tokensPerDoc), random(random)
{
  addAttribute(TermToBytesRefAttribute::typeid);
  bytes->length = TOKEN_LEN;
  nextSave = TestUtil::nextInt(random, 500000, 1000000);
}

bool Test2BTerms::MyTokenStream::incrementToken()
{
  clearAttributes();
  if (tokenCount >= tokensPerDoc) {
    return false;
  }
  int shift = 32;
  for (int i = 0; i < 5; i++) {
    bytes->bytes[i] = static_cast<char>((termCounter >> shift) & 0xFF);
    shift -= 8;
  }
  termCounter++;
  tokenCount++;
  if (--nextSave == 0) {
    savedTerms.push_back(BytesRef::deepCopyOf(bytes));
    wcout << L"TEST: save term=" << bytes << endl;
    nextSave = TestUtil::nextInt(random, 500000, 1000000);
  }
  return true;
}

void Test2BTerms::MyTokenStream::reset() { tokenCount = 0; }

shared_ptr<BytesRef>
Test2BTerms::MyTokenStream::MyTermAttributeImpl::getBytesRef()
{
  return bytes;
}

void Test2BTerms::MyTokenStream::MyTermAttributeImpl::clear() {}

void Test2BTerms::MyTokenStream::MyTermAttributeImpl::copyTo(
    shared_ptr<AttributeImpl> target)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<MyTermAttributeImpl>
Test2BTerms::MyTokenStream::MyTermAttributeImpl::clone()
{
  throw make_shared<UnsupportedOperationException>();
}

void Test2BTerms::MyTokenStream::MyTermAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
  reflector(TermToBytesRefAttribute::typeid, L"bytes", getBytesRef());
}

Test2BTerms::MyTokenStream::MyAttributeFactory::MyAttributeFactory(
    shared_ptr<AttributeFactory> delegate_)
    : delegate_(delegate_)
{
}

shared_ptr<AttributeImpl>
Test2BTerms::MyTokenStream::MyAttributeFactory::createAttributeInstance(
    type_info attClass)
{
  if (attClass == TermToBytesRefAttribute::typeid) {
    return make_shared<MyTermAttributeImpl>();
  }
  if (CharTermAttribute::typeid->isAssignableFrom(attClass)) {
    throw invalid_argument(L"no");
  }
  return delegate_->createAttributeInstance(attClass);
}

void Test2BTerms::test2BTerms() 
{

  wcout << L"Starting Test2B" << endl;
  constexpr int64_t TERM_COUNT =
      (static_cast<int64_t>(numeric_limits<int>::max())) + 100000000;

  constexpr int TERMS_PER_DOC = TestUtil::nextInt(random(), 100000, 1000000);

  deque<std::shared_ptr<BytesRef>> savedTerms;

  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BTerms"));
  // MockDirectoryWrapper dir = newFSDirectory(new
  // File("/p/lucene/indices/2bindex"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }
  dir->setCheckIndexOnClose(false); // don't double-checkindex

  if (true) {

    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
                 .setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH)
                 .setRAMBufferSizeMB(256.0)
                 .setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
                 .setMergePolicy(newLogMergePolicy(false, 10))
                 .setOpenMode(IndexWriterConfig::OpenMode::CREATE)
                 .setCodec(TestUtil::getDefaultCodec()));

    shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
    if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) != nullptr) {
      // 1 petabyte:
      (std::static_pointer_cast<LogByteSizeMergePolicy>(mp))
          ->setMaxMergeMB(1024 * 1024 * 1024);
    }

    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<MyTokenStream> *const ts =
        make_shared<MyTokenStream>(random(), TERMS_PER_DOC);

    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setIndexOptions(IndexOptions::DOCS);
    customType->setOmitNorms(true);
    shared_ptr<Field> field = make_shared<Field>(L"field", ts, customType);
    doc->push_back(field);
    // w.setInfoStream(System.out);
    constexpr int numDocs = static_cast<int>(TERM_COUNT / TERMS_PER_DOC);

    wcout << L"TERMS_PER_DOC=" << TERMS_PER_DOC << endl;
    wcout << L"numDocs=" << numDocs << endl;

    for (int i = 0; i < numDocs; i++) {
      constexpr int64_t t0 = System::currentTimeMillis();
      w->addDocument(doc);
      wcout << i << L" of " << numDocs << L" "
            << (System::currentTimeMillis() - t0) << L" msec" << endl;
    }
    savedTerms = ts->savedTerms;

    wcout << L"TEST: full merge" << endl;
    w->forceMerge(1);
    wcout << L"TEST: close writer" << endl;
    delete w;
  }

  wcout << L"TEST: open reader" << endl;
  shared_ptr<IndexReader> *const r = DirectoryReader::open(dir);
  if (savedTerms.empty()) {
    savedTerms = findTerms(r);
  }
  constexpr int numSavedTerms = savedTerms.size();
  const deque<std::shared_ptr<BytesRef>> bigOrdTerms =
      deque<std::shared_ptr<BytesRef>>(
          savedTerms.subList(numSavedTerms - 10, numSavedTerms));
  wcout << L"TEST: test big ord terms..." << endl;
  testSavedTerms(r, bigOrdTerms);
  wcout << L"TEST: test all saved terms..." << endl;
  testSavedTerms(r, savedTerms);
  delete r;

  wcout << L"TEST: now CheckIndex..." << endl;
  shared_ptr<CheckIndex::Status> status = TestUtil::checkIndex(dir);
  constexpr int64_t tc = status->segmentInfos[0]->termIndexStatus.termCount;
  assertTrue(L"count " + to_wstring(tc) + L" is not > " +
                 numeric_limits<int>::max(),
             tc > numeric_limits<int>::max());

  delete dir;
  wcout << L"TEST: done!" << endl;
}

deque<std::shared_ptr<BytesRef>>
Test2BTerms::findTerms(shared_ptr<IndexReader> r) 
{
  wcout << L"TEST: findTerms" << endl;
  shared_ptr<TermsEnum> *const termsEnum =
      MultiFields::getTerms(r, L"field")->begin();
  const deque<std::shared_ptr<BytesRef>> savedTerms =
      deque<std::shared_ptr<BytesRef>>();
  int nextSave = TestUtil::nextInt(random(), 500000, 1000000);
  shared_ptr<BytesRef> term;
  while ((term = termsEnum->next()) != nullptr) {
    if (--nextSave == 0) {
      savedTerms.push_back(BytesRef::deepCopyOf(term));
      wcout << L"TEST: add " << term << endl;
      nextSave = TestUtil::nextInt(random(), 500000, 1000000);
    }
  }
  return savedTerms;
}

void Test2BTerms::testSavedTerms(
    shared_ptr<IndexReader> r,
    deque<std::shared_ptr<BytesRef>> &terms) 
{
  wcout << L"TEST: run " << terms.size() << L" terms on reader=" << r << endl;
  shared_ptr<IndexSearcher> s = newSearcher(r);
  Collections::shuffle(terms, random());
  shared_ptr<TermsEnum> termsEnum = MultiFields::getTerms(r, L"field")->begin();
  bool failed = false;
  for (int iter = 0; iter < 10 * terms.size(); iter++) {
    shared_ptr<BytesRef> *const term = terms[random()->nextInt(terms.size())];
    wcout << L"TEST: search " << term << endl;
    constexpr int64_t t0 = System::currentTimeMillis();
    constexpr int64_t count =
        s->search(make_shared<TermQuery>(make_shared<Term>(L"field", term)), 1)
            ->totalHits;
    if (count <= 0) {
      wcout << L"  FAILED: count=" << count << endl;
      failed = true;
    }
    constexpr int64_t t1 = System::currentTimeMillis();
    wcout << L"  took " << (t1 - t0) << L" millis" << endl;

    TermsEnum::SeekStatus result = termsEnum->seekCeil(term);
    if (result != TermsEnum::SeekStatus::FOUND) {
      if (result == TermsEnum::SeekStatus::END) {
        wcout << L"  FAILED: got END" << endl;
      } else {
        wcout << L"  FAILED: wrong term: got " << termsEnum->term() << endl;
      }
      failed = true;
    }
  }
  assertFalse(failed);
}
} // namespace org::apache::lucene::index