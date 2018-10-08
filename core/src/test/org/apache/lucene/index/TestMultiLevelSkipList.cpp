using namespace std;

#include "TestMultiLevelSkipList.h"

namespace org::apache::lucene::index
{
using namespace org::apache::lucene::analysis;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Before;

TestMultiLevelSkipList::CountingRAMDirectory::CountingRAMDirectory(
    shared_ptr<TestMultiLevelSkipList> outerInstance,
    shared_ptr<Directory> delegate_)
    : org::apache::lucene::store::MockDirectoryWrapper(random(), delegate_),
      outerInstance(outerInstance)
{
}

shared_ptr<IndexInput> TestMultiLevelSkipList::CountingRAMDirectory::openInput(
    const wstring &fileName, shared_ptr<IOContext> context) 
{
  shared_ptr<IndexInput> in_ =
      MockDirectoryWrapper::openInput(fileName, context);
  if (StringHelper::endsWith(fileName, L".frq")) {
    in_ = make_shared<CountingStream>(outerInstance, in_);
  }
  return in_;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void TestMultiLevelSkipList::setUp() 
{
  LuceneTestCase::setUp();
  counter = 0;
}

void TestMultiLevelSkipList::testSimpleSkip() 
{
  shared_ptr<Directory> dir = make_shared<CountingRAMDirectory>(
      shared_from_this(), make_shared<RAMDirectory>());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<PayloadAnalyzer>())
               ->setCodec(TestUtil::alwaysPostingsFormat(
                   TestUtil::getDefaultPostingsFormat()))
               ->setMergePolicy(newLogMergePolicy()));
  shared_ptr<Term> term = make_shared<Term>(L"test", L"a");
  for (int i = 0; i < 5000; i++) {
    shared_ptr<Document> d1 = make_shared<Document>();
    d1->push_back(newTextField(term->field(), term->text(), Field::Store::NO));
    writer->addDocument(d1);
  }
  writer->commit();
  writer->forceMerge(1);
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(dir));

  for (int i = 0; i < 2; i++) {
    counter = 0;
    shared_ptr<PostingsEnum> tp = reader->postings(term, PostingsEnum::ALL);
    checkSkipTo(tp, 14, 185);  // no skips
    checkSkipTo(tp, 17, 190);  // one skip on level 0
    checkSkipTo(tp, 287, 200); // one skip on level 1, two on level 0

    // this test would fail if we had only one skip level,
    // because than more bytes would be read from the freqStream
    checkSkipTo(tp, 4800, 250); // one skip on level 2
  }
}

void TestMultiLevelSkipList::checkSkipTo(shared_ptr<PostingsEnum> tp,
                                         int target,
                                         int maxCounter) 
{
  tp->advance(target);
  if (maxCounter < counter) {
    fail(L"Too many bytes read: " + to_wstring(counter) + L" vs " +
         to_wstring(maxCounter));
  }

  assertEquals(L"Wrong document " + to_wstring(tp->docID()) +
                   L" after skipTo target " + to_wstring(target),
               target, tp->docID());
  assertEquals(L"Frequency is not 1: " + to_wstring(tp->freq()), 1, tp->freq());
  tp->nextPosition();
  shared_ptr<BytesRef> b = tp->getPayload();
  TestUtil::assertEquals(1, b->length);
  assertEquals(L"Wrong payload for the target " + to_wstring(target) + L": " +
                   to_wstring(b->bytes[b->offset]),
               static_cast<char>(target), b->bytes[b->offset]);
}

shared_ptr<TokenStreamComponents>
TestMultiLevelSkipList::PayloadAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<PayloadFilter>(payloadCount, tokenizer));
}

TestMultiLevelSkipList::PayloadFilter::PayloadFilter(
    shared_ptr<AtomicInteger> payloadCount, shared_ptr<TokenStream> input)
    : TokenFilter(input)
{
  this->payloadCount = payloadCount;
  payloadAtt = addAttribute(PayloadAttribute::typeid);
}

bool TestMultiLevelSkipList::PayloadFilter::incrementToken() 
{
  bool hasNext = input->incrementToken();
  if (hasNext) {
    payloadAtt->setPayload(make_shared<BytesRef>(
        std::deque<char>{static_cast<char>(payloadCount->incrementAndGet())}));
  }
  return hasNext;
}

TestMultiLevelSkipList::CountingStream::CountingStream(
    shared_ptr<TestMultiLevelSkipList> outerInstance,
    shared_ptr<IndexInput> input)
    : org::apache::lucene::store::IndexInput(L"CountingStream(" + input + L")"),
      outerInstance(outerInstance)
{
  this->input = input;
}

char TestMultiLevelSkipList::CountingStream::readByte() 
{
  outerInstance->counter++;
  return this->input->readByte();
}

void TestMultiLevelSkipList::CountingStream::readBytes(
    std::deque<char> &b, int offset, int len) 
{
  outerInstance->counter += len;
  this->input->readBytes(b, offset, len);
}

TestMultiLevelSkipList::CountingStream::~CountingStream()
{
  delete this->input;
}

int64_t TestMultiLevelSkipList::CountingStream::getFilePointer()
{
  return this->input->getFilePointer();
}

void TestMultiLevelSkipList::CountingStream::seek(int64_t pos) throw(
    IOException)
{
  this->input->seek(pos);
}

int64_t TestMultiLevelSkipList::CountingStream::length()
{
  return this->input->length();
}

shared_ptr<CountingStream> TestMultiLevelSkipList::CountingStream::clone()
{
  return make_shared<CountingStream>(outerInstance, this->input->clone());
}

shared_ptr<IndexInput> TestMultiLevelSkipList::CountingStream::slice(
    const wstring &sliceDescription, int64_t offset,
    int64_t length) 
{
  return make_shared<CountingStream>(
      outerInstance, this->input->slice(sliceDescription, offset, length));
}
} // namespace org::apache::lucene::index