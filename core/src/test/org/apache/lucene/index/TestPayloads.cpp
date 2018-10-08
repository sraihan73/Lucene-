using namespace std;

#include "TestPayloads.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPayloads::testPayload() 
{
  shared_ptr<BytesRef> payload = make_shared<BytesRef>(L"This is a test!");
  assertEquals(L"Wrong payload length.", (wstring(L"This is a test!")).length(),
               payload->length);

  shared_ptr<BytesRef> clone = payload->clone();
  TestUtil::assertEquals(payload->length, clone->length);
  for (int i = 0; i < payload->length; i++) {
    TestUtil::assertEquals(payload->bytes[i + payload->offset],
                           clone->bytes[i + clone->offset]);
  }
}

void TestPayloads::testPayloadFieldBit() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<PayloadAnalyzer> analyzer = make_shared<PayloadAnalyzer>();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(ram, newIndexWriterConfig(analyzer));
  shared_ptr<Document> d = make_shared<Document>();
  // this field won't have any payloads
  d->push_back(
      newTextField(L"f1", L"This field has no payloads", Field::Store::NO));
  // this field will have payloads in all docs, however not for all term
  // positions, so this field is used to check if the DocumentWriter correctly
  // enables the payloads bit even if only some term positions have payloads
  d->push_back(newTextField(L"f2", L"This field has payloads in all docs",
                            Field::Store::NO));
  d->push_back(newTextField(L"f2",
                            L"This field has payloads in all docs NO PAYLOAD",
                            Field::Store::NO));
  // this field is used to verify if the SegmentMerger enables payloads for a
  // field if it has payloads enabled in only some documents
  d->push_back(newTextField(L"f3", L"This field has payloads in some docs",
                            Field::Store::NO));
  // only add payload data for field f2
  analyzer->setPayloadData(
      L"f2", (wstring(L"somedata")).getBytes(StandardCharsets::UTF_8), 0, 1);
  writer->addDocument(d);
  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  assertFalse(L"Payload field bit should not be set.",
              fi->fieldInfo(L"f1")->hasPayloads());
  assertTrue(L"Payload field bit should be set.",
             fi->fieldInfo(L"f2")->hasPayloads());
  assertFalse(L"Payload field bit should not be set.",
              fi->fieldInfo(L"f3")->hasPayloads());
  delete reader;

  // now we add another document which has payloads for field f3 and verify if
  // the SegmentMerger enabled payloads for that field
  analyzer =
      make_shared<PayloadAnalyzer>(); // Clear payload state for each field
  writer = make_shared<IndexWriter>(
      ram, newIndexWriterConfig(analyzer)->setOpenMode(OpenMode::CREATE));
  d = make_shared<Document>();
  d->push_back(
      newTextField(L"f1", L"This field has no payloads", Field::Store::NO));
  d->push_back(newTextField(L"f2", L"This field has payloads in all docs",
                            Field::Store::NO));
  d->push_back(newTextField(L"f2", L"This field has payloads in all docs",
                            Field::Store::NO));
  d->push_back(newTextField(L"f3", L"This field has payloads in some docs",
                            Field::Store::NO));
  // add payload data for field f2 and f3
  analyzer->setPayloadData(
      L"f2", (wstring(L"somedata")).getBytes(StandardCharsets::UTF_8), 0, 1);
  analyzer->setPayloadData(
      L"f3", (wstring(L"somedata")).getBytes(StandardCharsets::UTF_8), 0, 3);
  writer->addDocument(d);

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  reader = getOnlyLeafReader(DirectoryReader::open(ram));
  fi = reader->getFieldInfos();
  assertFalse(L"Payload field bit should not be set.",
              fi->fieldInfo(L"f1")->hasPayloads());
  assertTrue(L"Payload field bit should be set.",
             fi->fieldInfo(L"f2")->hasPayloads());
  assertTrue(L"Payload field bit should be set.",
             fi->fieldInfo(L"f3")->hasPayloads());
  delete reader;
  delete ram;
}

void TestPayloads::testPayloadsEncoding() 
{
  shared_ptr<Directory> dir = newDirectory();
  performTest(dir);
  delete dir;
}

void TestPayloads::performTest(shared_ptr<Directory> dir) 
{
  shared_ptr<PayloadAnalyzer> analyzer = make_shared<PayloadAnalyzer>();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer)
                                        ->setOpenMode(OpenMode::CREATE)
                                        ->setMergePolicy(newLogMergePolicy()));

  // should be in sync with value in TermInfosWriter
  constexpr int skipInterval = 16;

  constexpr int numTerms = 5;
  const wstring fieldName = L"f1";

  int numDocs = skipInterval + 1;
  // create content for the test documents with just a few terms
  std::deque<std::shared_ptr<Term>> terms = generateTerms(fieldName, numTerms);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < terms.size(); i++) {
    sb->append(terms[i]->text());
    sb->append(L" ");
  }
  wstring content = sb->toString();

  int payloadDataLength =
      numTerms * numDocs * 2 + numTerms * numDocs * (numDocs - 1) / 2;
  std::deque<char> payloadData = generateRandomData(payloadDataLength);

  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(fieldName, content, Field::Store::NO));
  // add the same document multiple times to have the same payload lengths for
  // all occurrences within two consecutive skip intervals
  int offset = 0;
  for (int i = 0; i < 2 * numDocs; i++) {
    analyzer->setPayloadData(fieldName, payloadData, offset, 1);
    offset += numTerms;
    writer->addDocument(d);
  }

  // make sure we create more than one segment to test merging
  writer->commit();

  // now we make sure to have different payload lengths next at the next skip
  // point
  for (int i = 0; i < numDocs; i++) {
    analyzer->setPayloadData(fieldName, payloadData, offset, i);
    offset += i * numTerms;
    writer->addDocument(d);
  }

  writer->forceMerge(1);
  // flush
  delete writer;

  /*
   * Verify the index
   * first we test if all payloads are stored correctly
   */
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);

  std::deque<char> verifyPayloadData(payloadDataLength);
  offset = 0;
  std::deque<std::shared_ptr<PostingsEnum>> tps(numTerms);
  for (int i = 0; i < numTerms; i++) {
    tps[i] = MultiFields::getTermPositionsEnum(
        reader, terms[i]->field(), make_shared<BytesRef>(terms[i]->text()));
  }

  while (tps[0]->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    for (int i = 1; i < numTerms; i++) {
      tps[i]->nextDoc();
    }
    int freq = tps[0]->freq();

    for (int i = 0; i < freq; i++) {
      for (int j = 0; j < numTerms; j++) {
        tps[j]->nextPosition();
        shared_ptr<BytesRef> br = tps[j]->getPayload();
        if (br != nullptr) {
          System::arraycopy(br->bytes, br->offset, verifyPayloadData, offset,
                            br->length);
          offset += br->length;
        }
      }
    }
  }

  assertByteArrayEquals(payloadData, verifyPayloadData);

  /*
   *  test lazy skipping
   */
  shared_ptr<PostingsEnum> tp = MultiFields::getTermPositionsEnum(
      reader, terms[0]->field(), make_shared<BytesRef>(terms[0]->text()));
  tp->nextDoc();
  tp->nextPosition();
  // NOTE: prior rev of this test was failing to first
  // call next here:
  tp->nextDoc();
  // now we don't read this payload
  tp->nextPosition();
  shared_ptr<BytesRef> payload = tp->getPayload();
  assertEquals(L"Wrong payload length.", 1, payload->length);
  TestUtil::assertEquals(payload->bytes[payload->offset],
                         payloadData[numTerms]);
  tp->nextDoc();
  tp->nextPosition();

  // we don't read this payload and skip to a different document
  tp->advance(5);
  tp->nextPosition();
  payload = tp->getPayload();
  assertEquals(L"Wrong payload length.", 1, payload->length);
  TestUtil::assertEquals(payload->bytes[payload->offset],
                         payloadData[5 * numTerms]);

  /*
   * Test different lengths at skip points
   */
  tp = MultiFields::getTermPositionsEnum(
      reader, terms[1]->field(), make_shared<BytesRef>(terms[1]->text()));
  tp->nextDoc();
  tp->nextPosition();
  assertEquals(L"Wrong payload length.", 1, tp->getPayload()->length);
  tp->advance(skipInterval - 1);
  tp->nextPosition();
  assertEquals(L"Wrong payload length.", 1, tp->getPayload()->length);
  tp->advance(2 * skipInterval - 1);
  tp->nextPosition();
  assertEquals(L"Wrong payload length.", 1, tp->getPayload()->length);
  tp->advance(3 * skipInterval - 1);
  tp->nextPosition();
  assertEquals(L"Wrong payload length.", 3 * skipInterval - 2 * numDocs - 1,
               tp->getPayload()->length);

  delete reader;

  // test long payload
  analyzer = make_shared<PayloadAnalyzer>();
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(analyzer)->setOpenMode(OpenMode::CREATE));
  wstring singleTerm = L"lucene";

  d = make_shared<Document>();
  d->push_back(newTextField(fieldName, singleTerm, Field::Store::NO));
  // add a payload whose length is greater than the buffer size of
  // BufferedIndexOutput
  payloadData = generateRandomData(2000);
  analyzer->setPayloadData(fieldName, payloadData, 100, 1500);
  writer->addDocument(d);

  writer->forceMerge(1);
  // flush
  delete writer;

  reader = DirectoryReader::open(dir);
  tp = MultiFields::getTermPositionsEnum(reader, fieldName,
                                         make_shared<BytesRef>(singleTerm));
  tp->nextDoc();
  tp->nextPosition();

  shared_ptr<BytesRef> br = tp->getPayload();
  verifyPayloadData = std::deque<char>(br->length);
  std::deque<char> portion(1500);
  System::arraycopy(payloadData, 100, portion, 0, 1500);

  assertByteArrayEquals(portion, br->bytes, br->offset, br->length);
  delete reader;
}

const shared_ptr<java::nio::charset::Charset> TestPayloads::utf8 =
    java::nio::charset::StandardCharsets::UTF_8;

void TestPayloads::generateRandomData(std::deque<char> &data)
{
  // this test needs the random data to be valid unicode
  wstring s =
      TestUtil::randomFixedByteLengthUnicodeString(random(), data.size());
  std::deque<char> b = s.getBytes(utf8);
  assert(b.size() == data.size());
  System::arraycopy(b, 0, data, 0, b.size());
}

std::deque<char> TestPayloads::generateRandomData(int n)
{
  std::deque<char> data(n);
  generateRandomData(data);
  return data;
}

std::deque<std::shared_ptr<Term>>
TestPayloads::generateTerms(const wstring &fieldName, int n)
{
  int maxDigits = static_cast<int>(log(n) / log(10));
  std::deque<std::shared_ptr<Term>> terms(n);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < n; i++) {
    sb->setLength(0);
    sb->append(L"t");
    int zeros = maxDigits - static_cast<int>(log(i) / log(10));
    for (int j = 0; j < zeros; j++) {
      sb->append(L"0");
    }
    sb->append(i);
    terms[i] = make_shared<Term>(fieldName, sb->toString());
  }
  return terms;
}

void TestPayloads::assertByteArrayEquals(std::deque<char> &b1,
                                         std::deque<char> &b2)
{
  if (b1.size() != b2.size()) {
    fail(L"Byte arrays have different lengths: " + b1.size() + L", " +
         b2.size());
  }

  for (int i = 0; i < b1.size(); i++) {
    if (b1[i] != b2[i]) {
      fail(L"Byte arrays different at index " + to_wstring(i) + L": " +
           to_wstring(b1[i]) + L", " + to_wstring(b2[i]));
    }
  }
}

void TestPayloads::assertByteArrayEquals(std::deque<char> &b1,
                                         std::deque<char> &b2, int b2offset,
                                         int b2length)
{
  if (b1.size() != b2length) {
    fail(L"Byte arrays have different lengths: " + b1.size() + L", " +
         to_wstring(b2length));
  }

  for (int i = 0; i < b1.size(); i++) {
    if (b1[i] != b2[b2offset + i]) {
      fail(L"Byte arrays different at index " + to_wstring(i) + L": " +
           to_wstring(b1[i]) + L", " + to_wstring(b2[b2offset + i]));
    }
  }
}

TestPayloads::PayloadData::PayloadData(std::deque<char> &data, int offset,
                                       int length)
{
  this->data = data;
  this->offset = offset;
  this->length = length;
}

TestPayloads::PayloadAnalyzer::PayloadAnalyzer()
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
}

TestPayloads::PayloadAnalyzer::PayloadAnalyzer(const wstring &field,
                                               std::deque<char> &data,
                                               int offset, int length)
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
  setPayloadData(field, data, offset, length);
}

void TestPayloads::PayloadAnalyzer::setPayloadData(const wstring &field,
                                                   std::deque<char> &data,
                                                   int offset, int length)
{
  fieldToData.emplace(field, make_shared<PayloadData>(data, offset, length));
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPayloads::PayloadAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<PayloadData> payload = fieldToData[fieldName];
  shared_ptr<Tokenizer> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> tokenStream =
      (payload != nullptr)
          ? make_shared<PayloadFilter>(ts, fieldName, fieldToData)
          : ts;
  return make_shared<Analyzer::TokenStreamComponents>(ts, tokenStream);
}

TestPayloads::PayloadFilter::PayloadFilter(
    shared_ptr<TokenStream> in_, const wstring &fieldName,
    unordered_map<wstring, std::shared_ptr<PayloadData>> &fieldToData)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
  this->fieldToData = fieldToData;
  this->fieldName = fieldName;
  payloadAtt = addAttribute(PayloadAttribute::typeid);
  termAttribute = addAttribute(CharTermAttribute::typeid);
}

bool TestPayloads::PayloadFilter::incrementToken() 
{
  bool hasNext = input->incrementToken();
  if (!hasNext) {
    return false;
  }

  // Some values of the same field are to have payloads and others not
  // C++ TODO: There is no native C++ equivalent to 'toString':
  if (offset + payloadData->length <= payloadData->data.size() &&
      !termAttribute->toString()->endsWith(L"NO PAYLOAD")) {
    shared_ptr<BytesRef> p =
        make_shared<BytesRef>(payloadData->data, offset, payloadData->length);
    payloadAtt->setPayload(p);
    offset += payloadData->length;
  } else {
    payloadAtt->setPayload(nullptr);
  }

  return true;
}

void TestPayloads::PayloadFilter::reset() 
{
  TokenFilter::reset();
  this->payloadData = fieldToData[fieldName];
  this->offset = payloadData->offset;
}

void TestPayloads::testThreadSafety() 
{
  constexpr int numThreads = 5;
  constexpr int numDocs = atLeast(50);
  shared_ptr<ByteArrayPool> *const pool =
      make_shared<ByteArrayPool>(numThreads, 5);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  const wstring field = L"test";

  std::deque<std::shared_ptr<Thread>> ingesters(numThreads);
  for (int i = 0; i < numThreads; i++) {
    ingesters[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), numDocs, pool, writer, field);
    ingesters[i]->start();
  }

  for (int i = 0; i < numThreads; i++) {
    ingesters[i]->join();
  }
  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<TermsEnum> terms = MultiFields::getTerms(reader, field)->begin();
  shared_ptr<PostingsEnum> tp = nullptr;
  while (terms->next() != nullptr) {
    wstring termText = terms->term()->utf8ToString();
    tp = terms->postings(tp, PostingsEnum::PAYLOADS);
    while (tp->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      int freq = tp->freq();
      for (int i = 0; i < freq; i++) {
        tp->nextPosition();
        shared_ptr<BytesRef> *const payload = tp->getPayload();
        TestUtil::assertEquals(termText, payload->utf8ToString());
      }
    }
  }
  delete reader;
  delete dir;
  TestUtil::assertEquals(pool->size(), numThreads);
}

TestPayloads::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestPayloads> outerInstance, int numDocs,
    shared_ptr<org::apache::lucene::index::TestPayloads::ByteArrayPool> pool,
    shared_ptr<org::apache::lucene::index::IndexWriter> writer,
    const wstring &field)
{
  this->outerInstance = outerInstance;
  this->numDocs = numDocs;
  this->pool = pool;
  this->writer = writer;
  this->field = field;
}

void TestPayloads::ThreadAnonymousInnerClass::run()
{
  try {
    for (int j = 0; j < numDocs; j++) {
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(make_shared<TextField>(
          field, make_shared<PoolingPayloadTokenStream>(outerInstance, pool)));
      writer->addDocument(d);
    }
  } catch (const runtime_error &e) {
    e.printStackTrace();
    fail(e.what());
  }
}

TestPayloads::PoolingPayloadTokenStream::PoolingPayloadTokenStream(
    shared_ptr<TestPayloads> outerInstance, shared_ptr<ByteArrayPool> pool)
    : outerInstance(outerInstance)
{
  this->pool = pool;
  payload = pool->get();
  outerInstance->generateRandomData(payload);
  term = wstring(payload, 0, payload.size(), utf8);
  first = true;
  payloadAtt = addAttribute(PayloadAttribute::typeid);
  termAtt = addAttribute(CharTermAttribute::typeid);
}

bool TestPayloads::PoolingPayloadTokenStream::incrementToken() throw(
    IOException)
{
  if (!first) {
    return false;
  }
  first = false;
  clearAttributes();
  termAtt->append(term);
  payloadAtt->setPayload(make_shared<BytesRef>(payload));
  return true;
}

TestPayloads::PoolingPayloadTokenStream::~PoolingPayloadTokenStream()
{
  pool->release(payload);
}

TestPayloads::ByteArrayPool::ByteArrayPool(int capacity, int size)
{
  pool = deque<>();
  for (int i = 0; i < capacity; i++) {
    pool.push_back(std::deque<char>(size));
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<char> TestPayloads::ByteArrayPool::get()
{
  return pool.erase(pool.begin());
}

// C++ WARNING: The following method was originally marked 'synchronized':
void TestPayloads::ByteArrayPool::release(std::deque<char> &b)
{
  pool.push_back(b);
}

// C++ WARNING: The following method was originally marked 'synchronized':
int TestPayloads::ByteArrayPool::size() { return pool.size(); }

void TestPayloads::testAcrossFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"hasMaybepayload", L"here we go",
                                        Field::Store::YES));
  writer->addDocument(doc);
  delete writer;

  writer = make_shared<RandomIndexWriter>(
      random(), dir,
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true));
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"hasMaybepayload2", L"here we go",
                                        Field::Store::YES));
  writer->addDocument(doc);
  writer->addDocument(doc);
  writer->forceMerge(1);
  delete writer;

  delete dir;
}

void TestPayloads::testMixupDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field =
      make_shared<TextField>(L"field", L"", Field::Store::NO);
  shared_ptr<TokenStream> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"here we go"));
  field->setTokenStream(ts);
  doc->push_back(field);
  writer->addDocument(doc);
  shared_ptr<Token> withPayload = make_shared<Token>(L"withPayload", 0, 11);
  withPayload->setPayload(make_shared<BytesRef>(L"test"));
  ts = make_shared<CannedTokenStream>(withPayload);
  assertTrue(ts->hasAttribute(PayloadAttribute::typeid));
  field->setTokenStream(ts);
  writer->addDocument(doc);
  ts = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"another"));
  field->setTokenStream(ts);
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  shared_ptr<TermsEnum> te = MultiFields::getTerms(reader, L"field")->begin();
  assertTrue(te->seekExact(make_shared<BytesRef>(L"withPayload")));
  shared_ptr<PostingsEnum> de = te->postings(nullptr, PostingsEnum::PAYLOADS);
  de->nextDoc();
  de->nextPosition();
  TestUtil::assertEquals(make_shared<BytesRef>(L"test"), de->getPayload());
  delete writer;
  reader->close();
  delete dir;
}

void TestPayloads::testMixupMultiValued() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field =
      make_shared<TextField>(L"field", L"", Field::Store::NO);
  shared_ptr<TokenStream> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"here we go"));
  field->setTokenStream(ts);
  doc->push_back(field);
  shared_ptr<Field> field2 =
      make_shared<TextField>(L"field", L"", Field::Store::NO);
  shared_ptr<Token> withPayload = make_shared<Token>(L"withPayload", 0, 11);
  withPayload->setPayload(make_shared<BytesRef>(L"test"));
  ts = make_shared<CannedTokenStream>(withPayload);
  assertTrue(ts->hasAttribute(PayloadAttribute::typeid));
  field2->setTokenStream(ts);
  doc->push_back(field2);
  shared_ptr<Field> field3 =
      make_shared<TextField>(L"field", L"", Field::Store::NO);
  ts = make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"nopayload"));
  field3->setTokenStream(ts);
  doc->push_back(field3);
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  shared_ptr<LeafReader> sr = getOnlyLeafReader(reader);
  shared_ptr<PostingsEnum> de = sr->postings(
      make_shared<Term>(L"field", L"withPayload"), PostingsEnum::PAYLOADS);
  de->nextDoc();
  de->nextPosition();
  TestUtil::assertEquals(make_shared<BytesRef>(L"test"), de->getPayload());
  delete writer;
  reader->close();
  delete dir;
}
} // namespace org::apache::lucene::index