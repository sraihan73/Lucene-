using namespace std;

#include "TestCodecs.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;
using org::junit::BeforeClass;
std::deque<wstring> TestCodecs::fieldNames = {L"one", L"two", L"three",
                                               L"four"};
int TestCodecs::NUM_TEST_ITER = 0;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass()
void TestCodecs::beforeClass() { NUM_TEST_ITER = atLeast(20); }

TestCodecs::FieldData::FieldData(const wstring &name,
                                 shared_ptr<FieldInfos::Builder> fieldInfos,
                                 std::deque<std::shared_ptr<TermData>> &terms,
                                 bool const omitTF, bool const storePayloads)
    : fieldInfo(fieldInfos->getOrAdd(name)), terms(terms), omitTF(omitTF),
      storePayloads(storePayloads)
{
  // TODO: change this test to use all three
  if (omitTF) {
    fieldInfo->setIndexOptions(IndexOptions::DOCS);
  } else {
    fieldInfo->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  }
  if (storePayloads) {
    fieldInfo->setStorePayloads();
  }
  for (int i = 0; i < terms.size(); i++) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    terms[i]->field = shared_from_this();
  }

  Arrays::sort(terms);
}

int TestCodecs::FieldData::compareTo(shared_ptr<FieldData> other)
{
  return fieldInfo->name.compare(other->fieldInfo->name);
}

TestCodecs::PositionData::PositionData(int const pos,
                                       shared_ptr<BytesRef> payload)
{
  this->pos = pos;
  this->payload = payload;
}

TestCodecs::TermData::TermData(
    const wstring &text, std::deque<int> &docs,
    std::deque<std::deque<std::shared_ptr<PositionData>>> &positions)
    : text(make_shared<BytesRef>(text))
{
  this->text2 = text;
  this->docs = docs;
  this->positions = positions;
}

int TestCodecs::TermData::compareTo(shared_ptr<TermData> o)
{
  return text->compareTo(o->text);
}

const wstring TestCodecs::SEGMENT = L"0";

std::deque<std::shared_ptr<TermData>>
TestCodecs::makeRandomTerms(bool const omitTF, bool const storePayloads)
{
  constexpr int numTerms = 1 + random()->nextInt(NUM_TERMS_RAND);
  // final int numTerms = 2;
  std::deque<std::shared_ptr<TermData>> terms(numTerms);

  const unordered_set<wstring> termsSeen = unordered_set<wstring>();

  for (int i = 0; i < numTerms; i++) {

    // Make term text
    wstring text2;
    while (true) {
      text2 = TestUtil::randomUnicodeString(random());
      if (!find(termsSeen.begin(), termsSeen.end(), text2) != termsSeen.end() &&
          !StringHelper::endsWith(text2, L".")) {
        termsSeen.insert(text2);
        break;
      }
    }

    constexpr int docFreq = 1 + random()->nextInt(DOC_FREQ_RAND);
    const std::deque<int> docs = std::deque<int>(docFreq);
    std::deque<std::deque<std::shared_ptr<PositionData>>> positions;

    if (!omitTF) {
      positions =
          std::deque<std::deque<std::shared_ptr<PositionData>>>(docFreq);
    } else {
      positions.clear();
    }

    int docID = 0;
    for (int j = 0; j < docFreq; j++) {
      docID += TestUtil::nextInt(random(), 1, 10);
      docs[j] = docID;

      if (!omitTF) {
        constexpr int termFreq = 1 + random()->nextInt(TERM_DOC_FREQ_RAND);
        positions[j] = std::deque<std::shared_ptr<PositionData>>(termFreq);
        int position = 0;
        for (int k = 0; k < termFreq; k++) {
          position += TestUtil::nextInt(random(), 1, 10);

          shared_ptr<BytesRef> *const payload;
          if (storePayloads && random()->nextInt(4) == 0) {
            const std::deque<char> bytes =
                std::deque<char>(1 + random()->nextInt(5));
            for (int l = 0; l < bytes.size(); l++) {
              bytes[l] = static_cast<char>(random()->nextInt(255));
            }
            payload = make_shared<BytesRef>(bytes);
          } else {
            payload.reset();
          }

          positions[j][k] = make_shared<PositionData>(position, payload);
        }
      }
    }

    terms[i] = make_shared<TermData>(text2, docs, positions);
  }

  return terms;
}

void TestCodecs::testFixedPostings() 
{
  constexpr int NUM_TERMS = 100;
  std::deque<std::shared_ptr<TermData>> terms(NUM_TERMS);
  for (int i = 0; i < NUM_TERMS; i++) {
    const std::deque<int> docs = std::deque<int>{i};
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring text = Integer::toString(i, Character::MAX_RADIX);
    terms[i] = make_shared<TermData>(text, docs, nullptr);
  }

  shared_ptr<FieldInfos::Builder> *const builder =
      make_shared<FieldInfos::Builder>(
          make_shared<FieldInfos::FieldNumbers>(nullptr));

  shared_ptr<FieldData> *const field =
      make_shared<FieldData>(L"field", builder, terms, true, false);
  std::deque<std::shared_ptr<FieldData>> fields = {field};
  shared_ptr<FieldInfos> *const fieldInfos = builder->finish();
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<Codec> codec = Codec::getDefault();
  shared_ptr<SegmentInfo> *const si = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, SEGMENT, 10000, false, codec,
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);

  this->write(si, fieldInfos, dir, fields);
  shared_ptr<FieldsProducer> *const reader =
      codec->postingsFormat()->fieldsProducer(make_shared<SegmentReadState>(
          dir, si, fieldInfos, newIOContext(random())));

  constexpr FieldsProducer::const_iterator fieldsEnum = reader->begin();
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  wstring fieldName = fieldsEnum->next();
  assertNotNull(fieldName);
  shared_ptr<Terms> *const terms2 = reader->terms(fieldName);
  assertNotNull(terms2);

  shared_ptr<TermsEnum> *const termsEnum = terms2->begin();

  shared_ptr<PostingsEnum> postingsEnum = nullptr;
  for (int i = 0; i < NUM_TERMS; i++) {
    shared_ptr<BytesRef> *const term = termsEnum->next();
    assertNotNull(term);
    TestUtil::assertEquals(terms[i]->text2, term->utf8ToString());

    // do this twice to stress test the codec's reuse, ie,
    // make sure it properly fully resets (rewinds) its
    // internal state:
    for (int iter = 0; iter < 2; iter++) {
      postingsEnum =
          TestUtil::docs(random(), termsEnum, postingsEnum, PostingsEnum::NONE);
      TestUtil::assertEquals(terms[i]->docs[0], postingsEnum->nextDoc());
      TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                             postingsEnum->nextDoc());
    }
  }
  assertNull(termsEnum->next());

  for (int i = 0; i < NUM_TERMS; i++) {
    TestUtil::assertEquals(
        termsEnum->seekCeil(make_shared<BytesRef>(terms[i]->text2)),
        TermsEnum::SeekStatus::FOUND);
  }

  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(fieldsEnum->hasNext());
  reader->close();
  delete dir;
}

void TestCodecs::testRandomPostings() 
{
  shared_ptr<FieldInfos::Builder> *const builder =
      make_shared<FieldInfos::Builder>(
          make_shared<FieldInfos::FieldNumbers>(nullptr));

  std::deque<std::shared_ptr<FieldData>> fields(NUM_FIELDS);
  for (int i = 0; i < NUM_FIELDS; i++) {
    constexpr bool omitTF = 0 == (i % 3);
    constexpr bool storePayloads = 1 == (i % 3);
    fields[i] = make_shared<FieldData>(
        fieldNames[i], builder, this->makeRandomTerms(omitTF, storePayloads),
        omitTF, storePayloads);
  }

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<FieldInfos> *const fieldInfos = builder->finish();

  if (VERBOSE) {
    wcout << L"TEST: now write postings" << endl;
  }

  shared_ptr<Codec> codec = Codec::getDefault();
  shared_ptr<SegmentInfo> *const si = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, SEGMENT, 10000, false, codec,
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);
  this->write(si, fieldInfos, dir, fields);

  if (VERBOSE) {
    wcout << L"TEST: now read postings" << endl;
  }
  shared_ptr<FieldsProducer> *const terms =
      codec->postingsFormat()->fieldsProducer(make_shared<SegmentReadState>(
          dir, si, fieldInfos, newIOContext(random())));

  std::deque<std::shared_ptr<Verify>> threads(NUM_TEST_THREADS - 1);
  for (int i = 0; i < NUM_TEST_THREADS - 1; i++) {
    threads[i] = make_shared<Verify>(si, fields, terms);
    threads[i]->setDaemon(true);
    threads[i]->start();
  }

  (make_shared<Verify>(si, fields, terms))->run();

  for (int i = 0; i < NUM_TEST_THREADS - 1; i++) {
    threads[i]->join();
    assert(!threads[i]->failed);
  }

  terms->close();
  delete dir;
}

TestCodecs::Verify::Verify(shared_ptr<SegmentInfo> si,
                           std::deque<std::shared_ptr<FieldData>> &fields,
                           shared_ptr<Fields> termsDict)
    : termsDict(termsDict), fields(fields), si(si)
{
}

void TestCodecs::Verify::run()
{
  try {
    this->_run();
  }
  // C++ WARNING: 'final' catch parameters are not available in C++:
  // ORIGINAL LINE: catch (final Throwable t)
  catch (const runtime_error &t) {
    failed = true;
    throw runtime_error(t);
  }
}

void TestCodecs::Verify::verifyDocs(
    std::deque<int> &docs,
    std::deque<std::deque<std::shared_ptr<PositionData>>> &positions,
    shared_ptr<PostingsEnum> postingsEnum,
    bool const doPos) 
{
  for (int i = 0; i < docs.size(); i++) {
    constexpr int doc = postingsEnum->nextDoc();
    assertTrue(doc != DocIdSetIterator::NO_MORE_DOCS);
    TestUtil::assertEquals(docs[i], doc);
    if (doPos) {
      this->verifyPositions(positions[i], postingsEnum);
    }
  }
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         postingsEnum->nextDoc());
}

void TestCodecs::Verify::verifyPositions(
    std::deque<std::shared_ptr<PositionData>> &positions,
    shared_ptr<PostingsEnum> posEnum) 
{
  for (int i = 0; i < positions.size(); i++) {
    constexpr int pos = posEnum->nextPosition();
    TestUtil::assertEquals(positions[i]->pos, pos);
    if (positions[i]->payload != nullptr) {
      assertNotNull(posEnum->getPayload());
      if (LuceneTestCase::random()->nextInt(3) < 2) {
        // Verify the payload bytes
        shared_ptr<BytesRef> *const otherPayload = posEnum->getPayload();
        // C++ TODO: There is no native C++ equivalent to 'toString':
        assertTrue(L"expected=" + positions[i]->payload->toString() + L" got=" +
                       otherPayload->toString(),
                   positions[i]->payload.equals(otherPayload));
      }
    } else {
      assertNull(posEnum->getPayload());
    }
  }
}

void TestCodecs::Verify::_run() 
{

  for (int iter = 0; iter < NUM_TEST_ITER; iter++) {
    shared_ptr<FieldData> *const field =
        fields[LuceneTestCase::random()->nextInt(fields.size())];
    shared_ptr<TermsEnum> *const termsEnum =
        termsDict->terms(field->fieldInfo->name)->begin();

    int upto = 0;
    // Test straight enum of the terms:
    while (true) {
      shared_ptr<BytesRef> *const term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      shared_ptr<BytesRef> *const expected =
          make_shared<BytesRef>(field->terms[upto++]->text2);
      assertTrue(L"expected=" + expected + L" vs actual " + term,
                 expected->bytesEquals(term));
    }
    TestUtil::assertEquals(upto, field->terms.size());

    // Test random seek:
    shared_ptr<TermData> term =
        field->terms[LuceneTestCase::random()->nextInt(field->terms.size())];
    TermsEnum::SeekStatus status =
        termsEnum->seekCeil(make_shared<BytesRef>(term->text2));
    TestUtil::assertEquals(status, TermsEnum::SeekStatus::FOUND);
    TestUtil::assertEquals(term->docs.size(), termsEnum->docFreq());
    if (field->omitTF) {
      this->verifyDocs(term->docs, term->positions,
                       TestUtil::docs(LuceneTestCase::random(), termsEnum,
                                      nullptr, PostingsEnum::NONE),
                       false);
    } else {
      this->verifyDocs(term->docs, term->positions,
                       termsEnum->postings(nullptr, PostingsEnum::ALL), true);
    }

    // Test random seek by ord:
    constexpr int idx = LuceneTestCase::random()->nextInt(field->terms.size());
    term = field->terms[idx];
    bool success = false;
    try {
      termsEnum->seekExact(idx);
      success = true;
    } catch (const UnsupportedOperationException &uoe) {
      // ok -- skip it
    }
    if (success) {
      TestUtil::assertEquals(status, TermsEnum::SeekStatus::FOUND);
      assertTrue(
          termsEnum->term()->bytesEquals(make_shared<BytesRef>(term->text2)));
      TestUtil::assertEquals(term->docs.size(), termsEnum->docFreq());
      if (field->omitTF) {
        this->verifyDocs(term->docs, term->positions,
                         TestUtil::docs(LuceneTestCase::random(), termsEnum,
                                        nullptr, PostingsEnum::NONE),
                         false);
      } else {
        this->verifyDocs(term->docs, term->positions,
                         termsEnum->postings(nullptr, PostingsEnum::ALL), true);
      }
    }

    // Test seek to non-existent terms:
    if (VERBOSE) {
      wcout << L"TEST: seek non-exist terms" << endl;
    }
    for (int i = 0; i < 100; i++) {
      const wstring text2 =
          TestUtil::randomUnicodeString(LuceneTestCase::random()) + L".";
      status = termsEnum->seekCeil(make_shared<BytesRef>(text2));
      assertTrue(status == TermsEnum::SeekStatus::NOT_FOUND ||
                 status == TermsEnum::SeekStatus::END);
    }

    // Seek to each term, backwards:
    if (VERBOSE) {
      wcout << L"TEST: seek terms backwards" << endl;
    }
    for (int i = field->terms.size() - 1; i >= 0; i--) {
      assertEquals(
          Thread::currentThread().getName() + L": field=" +
              field->fieldInfo->name + L" term=" + field->terms[i]->text2,
          TermsEnum::SeekStatus::FOUND,
          termsEnum->seekCeil(make_shared<BytesRef>(field->terms[i]->text2)));
      TestUtil::assertEquals(field->terms[i]->docs->size(),
                             termsEnum->docFreq());
    }

    // Seek to each term by ord, backwards
    for (int i = field->terms.size() - 1; i >= 0; i--) {
      try {
        termsEnum->seekExact(i);
        TestUtil::assertEquals(field->terms[i]->docs->size(),
                               termsEnum->docFreq());
        assertTrue(termsEnum->term()->bytesEquals(
            make_shared<BytesRef>(field->terms[i]->text2)));
      } catch (const UnsupportedOperationException &uoe) {
      }
    }

    // Seek to non-existent empty-string term
    status = termsEnum->seekCeil(make_shared<BytesRef>(L""));
    assertNotNull(status);
    // assertEquals(TermsEnum.SeekStatus.NOT_FOUND, status);

    // Make sure we're now pointing to first term
    assertTrue(termsEnum->term()->bytesEquals(
        make_shared<BytesRef>(field->terms[0]->text2)));

    // Test docs enum
    termsEnum->seekCeil(make_shared<BytesRef>(L""));
    upto = 0;
    do {
      term = field->terms[upto];
      if (LuceneTestCase::random()->nextInt(3) == 1) {
        shared_ptr<PostingsEnum> *const postings;
        if (!field->omitTF) {
          // TODO: we should randomize which postings features are available,
          // but need to coordinate this with the checks below that rely on such
          // features
          postings = termsEnum->postings(nullptr, PostingsEnum::ALL);
        } else {
          postings = TestUtil::docs(LuceneTestCase::random(), termsEnum,
                                    nullptr, PostingsEnum::FREQS);
        }
        assertNotNull(postings);
        int upto2 = -1;
        bool ended = false;
        while (upto2 < term->docs.size() - 1) {
          // Maybe skip:
          constexpr int left = term->docs.size() - upto2;
          int doc;
          if (LuceneTestCase::random()->nextInt(3) == 1 && left >= 1) {
            constexpr int inc = 1 + LuceneTestCase::random()->nextInt(left - 1);
            upto2 += inc;
            if (LuceneTestCase::random()->nextInt(2) == 1) {
              doc = postings->advance(term->docs[upto2]);
              TestUtil::assertEquals(term->docs[upto2], doc);
            } else {
              doc = postings->advance(1 + term->docs[upto2]);
              if (doc == DocIdSetIterator::NO_MORE_DOCS) {
                // skipped past last doc
                assert(upto2 == term->docs.size() - 1);
                ended = true;
                break;
              } else {
                // skipped to next doc
                assert(upto2 < term->docs.size() - 1);
                if (doc >= term->docs[1 + upto2]) {
                  upto2++;
                }
              }
            }
          } else {
            doc = postings->nextDoc();
            assertTrue(doc != -1);
            upto2++;
          }
          TestUtil::assertEquals(term->docs[upto2], doc);
          if (!field->omitTF) {
            TestUtil::assertEquals(term->positions[upto2].length,
                                   postings->freq());
            if (LuceneTestCase::random()->nextInt(2) == 1) {
              this->verifyPositions(term->positions[upto2], postings);
            }
          }
        }

        if (!ended) {
          TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                                 postings->nextDoc());
        }
      }
      upto++;

    } while (termsEnum->next() != nullptr);

    TestUtil::assertEquals(upto, field->terms.size());
  }
}

TestCodecs::DataFields::DataFields(
    std::deque<std::shared_ptr<FieldData>> &fields)
    : fields(fields)
{
  // already sorted:
}

shared_ptr<Iterator<wstring>> TestCodecs::DataFields::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

TestCodecs::DataFields::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(shared_ptr<DataFields> outerInstance)
{
  this->outerInstance = outerInstance;
  upto = -1;
}

bool TestCodecs::DataFields::IteratorAnonymousInnerClass::hasNext()
{
  return upto + 1 < outerInstance->fields.size();
}

wstring TestCodecs::DataFields::IteratorAnonymousInnerClass::next()
{
  upto++;
  return outerInstance->fields[upto]->fieldInfo.name;
}

void TestCodecs::DataFields::IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Terms> TestCodecs::DataFields::terms(const wstring &field)
{
  // Slow linear search:
  for (auto fieldData : fields) {
    if (fieldData->fieldInfo->name == field) {
      return make_shared<DataTerms>(fieldData);
    }
  }
  return nullptr;
}

int TestCodecs::DataFields::size() { return fields.size(); }

TestCodecs::DataTerms::DataTerms(shared_ptr<FieldData> fieldData)
    : fieldData(fieldData)
{
}

shared_ptr<TermsEnum> TestCodecs::DataTerms::iterator()
{
  return make_shared<DataTermsEnum>(fieldData);
}

int64_t TestCodecs::DataTerms::size()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestCodecs::DataTerms::getSumTotalTermFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestCodecs::DataTerms::getSumDocFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int TestCodecs::DataTerms::getDocCount()
{
  throw make_shared<UnsupportedOperationException>();
}

bool TestCodecs::DataTerms::hasFreqs()
{
  return fieldData->fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS) >= 0;
}

bool TestCodecs::DataTerms::hasOffsets()
{
  return fieldData->fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool TestCodecs::DataTerms::hasPositions()
{
  return fieldData->fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool TestCodecs::DataTerms::hasPayloads()
{
  return fieldData->fieldInfo->hasPayloads();
}

TestCodecs::DataTermsEnum::DataTermsEnum(shared_ptr<FieldData> fieldData)
    : fieldData(fieldData)
{
}

shared_ptr<BytesRef> TestCodecs::DataTermsEnum::next()
{
  upto++;
  if (upto == fieldData->terms.size()) {
    return nullptr;
  }

  return term();
}

shared_ptr<BytesRef> TestCodecs::DataTermsEnum::term()
{
  return fieldData->terms[upto]->text;
}

SeekStatus TestCodecs::DataTermsEnum::seekCeil(shared_ptr<BytesRef> text)
{
  // Stupid linear impl:
  for (int i = 0; i < fieldData->terms.size(); i++) {
    int cmp = fieldData->terms[i]->text->compareTo(text);
    if (cmp == 0) {
      upto = i;
      return SeekStatus::FOUND;
    } else if (cmp > 0) {
      upto = i;
      return SeekStatus::NOT_FOUND;
    }
  }

  return SeekStatus::END;
}

void TestCodecs::DataTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestCodecs::DataTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

int TestCodecs::DataTermsEnum::docFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestCodecs::DataTermsEnum::totalTermFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PostingsEnum>
TestCodecs::DataTermsEnum::postings(shared_ptr<PostingsEnum> reuse, int flags)
{
  return make_shared<DataPostingsEnum>(fieldData->terms[upto]);
}

TestCodecs::DataPostingsEnum::DataPostingsEnum(shared_ptr<TermData> termData)
    : termData(termData)
{
}

int64_t TestCodecs::DataPostingsEnum::cost()
{
  throw make_shared<UnsupportedOperationException>();
}

int TestCodecs::DataPostingsEnum::nextDoc()
{
  docUpto++;
  if (docUpto == termData->docs.size()) {
    return NO_MORE_DOCS;
  }
  posUpto = -1;
  return docID();
}

int TestCodecs::DataPostingsEnum::docID() { return termData->docs[docUpto]; }

int TestCodecs::DataPostingsEnum::advance(int target)
{
  // Slow linear impl:
  nextDoc();
  while (docID() < target) {
    nextDoc();
  }

  return docID();
}

int TestCodecs::DataPostingsEnum::freq()
{
  return termData->positions[docUpto].length;
}

int TestCodecs::DataPostingsEnum::nextPosition()
{
  posUpto++;
  return termData->positions[docUpto][posUpto]->pos;
}

shared_ptr<BytesRef> TestCodecs::DataPostingsEnum::getPayload()
{
  return termData->positions[docUpto][posUpto]->payload;
}

int TestCodecs::DataPostingsEnum::startOffset()
{
  throw make_shared<UnsupportedOperationException>();
}

int TestCodecs::DataPostingsEnum::endOffset()
{
  throw make_shared<UnsupportedOperationException>();
}

void TestCodecs::write(
    shared_ptr<SegmentInfo> si, shared_ptr<FieldInfos> fieldInfos,
    shared_ptr<Directory> dir,
    std::deque<std::shared_ptr<FieldData>> &fields) 
{

  shared_ptr<Codec> *const codec = si->getCodec();
  shared_ptr<SegmentWriteState> *const state = make_shared<SegmentWriteState>(
      InfoStream::getDefault(), dir, si, fieldInfos, nullptr,
      newIOContext(random()));

  Arrays::sort(fields);
  shared_ptr<FieldsConsumer> consumer =
      codec->postingsFormat()->fieldsConsumer(state);
  bool success = false;
  try {
    consumer->write(make_shared<DataFields>(fields));
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({consumer});
    } else {
      IOUtils::closeWhileHandlingException({consumer});
    }
  }
}

void TestCodecs::testDocsOnlyFreq() 
{
  // tests that when fields are indexed with DOCS_ONLY, the Codec
  // returns 1 in docsEnum.freq()
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> random = TestCodecs::random();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random)));
  // we don't need many documents to assert this, but don't use one document
  // either
  int numDocs = atLeast(random, 50);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"f", L"doc", Store::NO));
    writer->addDocument(doc);
  }
  delete writer;

  shared_ptr<Term> term =
      make_shared<Term>(L"f", make_shared<BytesRef>(L"doc"));
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> ctx : reader->leaves()) {
    shared_ptr<PostingsEnum> de = ctx->reader()->postings(term);
    while (de->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      assertEquals(L"wrong freq for doc " + to_wstring(de->docID()), 1,
                   de->freq());
    }
  }
  reader->close();

  delete dir;
}
} // namespace org::apache::lucene::index