using namespace std;

#include "BaseTermVectorsFormatTestCase.h"

namespace org::apache::lucene::index
{
//    import static org.apache.lucene.index.PostingsEnum.ALL;
//    import static org.apache.lucene.index.PostingsEnum.FREQS;
//    import static org.apache.lucene.index.PostingsEnum.NONE;
//    import static org.apache.lucene.index.PostingsEnum.OFFSETS;
//    import static org.apache.lucene.index.PostingsEnum.PAYLOADS;
//    import static org.apache.lucene.index.PostingsEnum.POSITIONS;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Codec = org::apache::lucene::codecs::Codec;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

Options Options::NONE(L"NONE", InnerEnum::NONE, false, false, false);
Options Options::POSITIONS(L"POSITIONS", InnerEnum::POSITIONS, true, false,
                           false);
Options Options::OFFSETS(L"OFFSETS", InnerEnum::OFFSETS, false, true, false);
Options Options::POSITIONS_AND_OFFSETS(L"POSITIONS_AND_OFFSETS",
                                       InnerEnum::POSITIONS_AND_OFFSETS, true,
                                       true, false);
Options Options::POSITIONS_AND_PAYLOADS(L"POSITIONS_AND_PAYLOADS",
                                        InnerEnum::POSITIONS_AND_PAYLOADS, true,
                                        false, true);
Options Options::POSITIONS_AND_OFFSETS_AND_PAYLOADS(
    L"POSITIONS_AND_OFFSETS_AND_PAYLOADS",
    InnerEnum::POSITIONS_AND_OFFSETS_AND_PAYLOADS, true, true, true);

deque<Options> Options::valueList;

Options::StaticConstructor::StaticConstructor()
{
  valueList.push_back(NONE);
  valueList.push_back(POSITIONS);
  valueList.push_back(OFFSETS);
  valueList.push_back(POSITIONS_AND_OFFSETS);
  valueList.push_back(POSITIONS_AND_PAYLOADS);
  valueList.push_back(POSITIONS_AND_OFFSETS_AND_PAYLOADS);
}

Options::StaticConstructor Options::staticConstructor;
int Options::nextOrdinal = 0;

BaseTermVectorsFormatTestCase::Options::Options(
    const wstring &name, InnerEnum innerEnum,
    shared_ptr<BaseTermVectorsFormatTestCase> outerInstance, bool positions,
    bool offsets, bool payloads)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
  this->outerInstance = outerInstance;
  this->positions = positions;
  this->offsets = offsets;
  this->payloads = payloads;
}

bool Options::operator==(const Options &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Options::operator!=(const Options &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Options> Options::values() { return valueList; }

int Options::ordinal() { return ordinalValue; }

wstring Options::toString() { return nameValue; }

Options Options::valueOf(const wstring &name)
{
  for (auto enumInstance : Options::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

shared_ptr<Set<Options>> BaseTermVectorsFormatTestCase::validOptions()
{
  return EnumSet::allOf(Options::typeid);
}

BaseTermVectorsFormatTestCase::Options
BaseTermVectorsFormatTestCase::randomOptions()
{
  return RandomPicks::randomFrom(random(), deque<>(validOptions()));
}

shared_ptr<FieldType> BaseTermVectorsFormatTestCase::fieldType(Options options)
{
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(options.positions);
  ft->setStoreTermVectorOffsets(options.offsets);
  ft->setStoreTermVectorPayloads(options.payloads);
  ft->freeze();
  return ft;
}

void BaseTermVectorsFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  for (auto opts : validOptions()) {
    shared_ptr<FieldType> ft = fieldType(opts);
    constexpr int numFields = random()->nextInt(5);
    for (int j = 0; j < numFields; ++j) {
      doc->push_back(make_shared<Field>(
          L"f_" + opts, TestUtil::randomSimpleString(random(), 2), ft));
    }
  }
}

int BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::startOffset()
{
  return start;
}

int BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::endOffset()
{
  return end;
}

void BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::setOffset(
    int startOffset, int endOffset)
{
  // no check!
  start = startOffset;
  end = endOffset;
}

void BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::clear()
{
  start = end = 0;
}

bool BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::equals(
    any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<PermissiveOffsetAttributeImpl>(other) !=
      nullptr) {
    shared_ptr<PermissiveOffsetAttributeImpl> o =
        any_cast<std::shared_ptr<PermissiveOffsetAttributeImpl>>(other);
    return o->start == start && o->end == end;
  }

  return false;
}

int BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::hashCode()
{
  return start + 31 * end;
}

void BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::copyTo(
    shared_ptr<AttributeImpl> target)
{
  shared_ptr<OffsetAttribute> t =
      std::static_pointer_cast<OffsetAttribute>(target);
  t->setOffset(start, end);
}

void BaseTermVectorsFormatTestCase::PermissiveOffsetAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
  reflector(OffsetAttribute::typeid, L"startOffset", start);
  reflector(OffsetAttribute::typeid, L"endOffset", end);
}

BaseTermVectorsFormatTestCase::RandomTokenStream::RandomTokenStream(
    int len, std::deque<wstring> &sampleTerms,
    std::deque<std::shared_ptr<BytesRef>> &sampleTermBytes)
    : terms(std::deque<wstring>(len)),
      termBytes(std::deque<std::shared_ptr<BytesRef>>(len)),
      positionsIncrements(std::deque<int>(len)),
      positions(std::deque<int>(len)), startOffsets(std::deque<int>(len)),
      endOffsets(std::deque<int>(len)),
      payloads(std::deque<std::shared_ptr<BytesRef>>(len)),
      freqs(unordered_map<>()), positionToTerms(unordered_map<>(len)),
      startOffsetToTerms(unordered_map<>(len)),
      termAtt(addAttribute(CharTermAttribute::typeid)),
      piAtt(addAttribute(PositionIncrementAttribute::typeid)),
      oAtt(addAttribute(OffsetAttribute::typeid)),
      pAtt(addAttribute(PayloadAttribute::typeid))
{
  for (int i = 0; i < len; ++i) {
    constexpr int o = random()->nextInt(sampleTerms.size());
    terms[i] = sampleTerms[o];
    termBytes[i] = sampleTermBytes[o];
    positionsIncrements[i] = TestUtil::nextInt(random(), i == 0 ? 1 : 0, 10);
    if (i == 0) {
      startOffsets[i] = TestUtil::nextInt(random(), 0, 1 << 16);
    } else {
      startOffsets[i] = startOffsets[i - 1] +
                        TestUtil::nextInt(random(), 0, rarely() ? 1 << 16 : 20);
    }
    endOffsets[i] = startOffsets[i] +
                    TestUtil::nextInt(random(), 0, rarely() ? 1 << 10 : 20);
  }

  for (int i = 0; i < len; ++i) {
    if (i == 0) {
      positions[i] = positionsIncrements[i] - 1;
    } else {
      positions[i] = positions[i - 1] + positionsIncrements[i];
    }
  }
  if (rarely()) {
    Arrays::fill(payloads, randomPayload());
  } else {
    for (int i = 0; i < len; ++i) {
      payloads[i] = randomPayload();
    }
  }

  for (int i = 0; i < len; ++i) {
    if (positionToTerms.find(positions[i]) == positionToTerms.end()) {
      positionToTerms.emplace(positions[i], unordered_set<int>(1));
    }
    positionToTerms[positions[i]]->add(i);
    if (startOffsetToTerms.find(startOffsets[i]) == startOffsetToTerms.end()) {
      startOffsetToTerms.emplace(startOffsets[i], unordered_set<int>(1));
    }
    startOffsetToTerms[startOffsets[i]]->add(i);
  }

  for (auto term : terms) {
    if (freqs.find(term) != freqs.end()) {
      freqs.emplace(term, freqs[term] + 1);
    } else {
      freqs.emplace(term, 1);
    }
  }

  addAttributeImpl(make_shared<PermissiveOffsetAttributeImpl>());
}

shared_ptr<BytesRef>
BaseTermVectorsFormatTestCase::RandomTokenStream::randomPayload()
{
  constexpr int len = random()->nextInt(5);
  if (len == 0) {
    return nullptr;
  }
  shared_ptr<BytesRef> *const payload = make_shared<BytesRef>(len);
  random()->nextBytes(payload->bytes);
  payload->length = len;
  return payload;
}

bool BaseTermVectorsFormatTestCase::RandomTokenStream::hasPayloads()
{
  for (auto payload : payloads) {
    if (payload != nullptr && payload->length > 0) {
      return true;
    }
  }
  return false;
}

std::deque<wstring>
BaseTermVectorsFormatTestCase::RandomTokenStream::getTerms()
{
  return terms;
}

std::deque<std::shared_ptr<BytesRef>>
BaseTermVectorsFormatTestCase::RandomTokenStream::getTermBytes()
{
  return termBytes;
}

std::deque<int>
BaseTermVectorsFormatTestCase::RandomTokenStream::getPositionsIncrements()
{
  return positionsIncrements;
}

std::deque<int>
BaseTermVectorsFormatTestCase::RandomTokenStream::getStartOffsets()
{
  return startOffsets;
}

std::deque<int>
BaseTermVectorsFormatTestCase::RandomTokenStream::getEndOffsets()
{
  return endOffsets;
}

std::deque<std::shared_ptr<BytesRef>>
BaseTermVectorsFormatTestCase::RandomTokenStream::getPayloads()
{
  return payloads;
}

void BaseTermVectorsFormatTestCase::RandomTokenStream::reset() throw(
    IOException)
{
  i = 0;
  TokenStream::reset();
}

bool BaseTermVectorsFormatTestCase::RandomTokenStream::incrementToken() throw(
    IOException)
{
  if (i < terms.size()) {
    clearAttributes();
    termAtt->setLength(0)->append(terms[i]);
    piAtt->setPositionIncrement(positionsIncrements[i]);
    oAtt->setOffset(startOffsets[i], endOffsets[i]);
    pAtt->setPayload(payloads[i]);
    ++i;
    return true;
  } else {
    return false;
  }
}

BaseTermVectorsFormatTestCase::RandomDocument::RandomDocument(
    shared_ptr<BaseTermVectorsFormatTestCase> outerInstance, int fieldCount,
    int maxTermCount, Options options, std::deque<wstring> &fieldNames,
    std::deque<wstring> &sampleTerms,
    std::deque<std::shared_ptr<BytesRef>> &sampleTermBytes)
    : fieldNames(std::deque<wstring>(fieldCount)),
      fieldTypes(std::deque<std::shared_ptr<FieldType>>(fieldCount)),
      tokenStreams(std::deque<std::shared_ptr<RandomTokenStream>>(fieldCount)),
      outerInstance(outerInstance)
{
  if (fieldCount > fieldNames.size()) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }
  Arrays::fill(fieldTypes, outerInstance->fieldType(options));
  shared_ptr<Set<wstring>> *const usedFileNames = unordered_set<wstring>();
  for (int i = 0; i < fieldCount; ++i) {
    do {
      this->fieldNames[i] = RandomPicks::randomFrom(random(), fieldNames);
    } while (usedFileNames->contains(this->fieldNames[i]));
    usedFileNames->add(this->fieldNames[i]);
    tokenStreams[i] = make_shared<RandomTokenStream>(
        TestUtil::nextInt(random(), 1, maxTermCount), sampleTerms,
        sampleTermBytes);
  }
}

shared_ptr<Document> BaseTermVectorsFormatTestCase::RandomDocument::toDocument()
{
  shared_ptr<Document> *const doc = make_shared<Document>();
  for (int i = 0; i < fieldNames.size(); ++i) {
    doc->push_back(
        make_shared<Field>(fieldNames[i], tokenStreams[i], fieldTypes[i]));
  }
  return doc;
}

BaseTermVectorsFormatTestCase::RandomDocumentFactory::RandomDocumentFactory(
    shared_ptr<BaseTermVectorsFormatTestCase> outerInstance,
    int distinctFieldNames, int disctinctTerms)
    : fieldNames(fieldNames.toArray(std::deque<wstring>(0))),
      terms(std::deque<wstring>(disctinctTerms)),
      termBytes(std::deque<std::shared_ptr<BytesRef>>(disctinctTerms)),
      outerInstance(outerInstance)
{
  shared_ptr<Set<wstring>> *const fieldNames = unordered_set<wstring>();
  while (fieldNames->size() < distinctFieldNames) {
    fieldNames->add(TestUtil::randomSimpleString(random()));
    fieldNames->remove(L"id");
  }
  for (int i = 0; i < disctinctTerms; ++i) {
    terms[i] = TestUtil::randomRealisticUnicodeString(random());
    termBytes[i] = make_shared<BytesRef>(terms[i]);
  }
}

shared_ptr<RandomDocument>
BaseTermVectorsFormatTestCase::RandomDocumentFactory::newDocument(
    int fieldCount, int maxTermCount, Options options)
{
  return make_shared<RandomDocument>(outerInstance, fieldCount, maxTermCount,
                                     options, fieldNames, terms, termBytes);
}

void BaseTermVectorsFormatTestCase::assertEquals(
    shared_ptr<RandomDocument> doc,
    shared_ptr<Fields> fields) 
{
  // compare field names
  assertEquals(doc == nullptr, fields->empty());
  assertEquals(doc->fieldNames->length, fields->size());
  shared_ptr<Set<wstring>> *const fields1 = unordered_set<wstring>();
  shared_ptr<Set<wstring>> *const fields2 = unordered_set<wstring>();
  for (int i = 0; i < doc->fieldNames->length; ++i) {
    fields1->add(doc->fieldNames[i]);
  }
  for (auto field : fields) {
    fields2->add(field);
  }
  assertEquals(fields1, fields2);

  for (int i = 0; i < doc->fieldNames->length; ++i) {
    assertEquals(doc->tokenStreams[i], doc->fieldTypes[i],
                 fields->terms(doc->fieldNames[i]));
  }
}

bool BaseTermVectorsFormatTestCase::equals(any o1, any o2)
{
  if (o1 == nullptr) {
    return o2 == nullptr;
  } else {
    return o1.equals(o2);
  }
}

void BaseTermVectorsFormatTestCase::assertEquals(
    shared_ptr<RandomTokenStream> tk, shared_ptr<FieldType> ft,
    shared_ptr<Terms> terms) 
{
  assertEquals(1, terms->getDocCount());
  constexpr int termCount =
      (unordered_set<>(Arrays::asList(tk->terms)))->size();
  assertEquals(termCount, terms->size());
  assertEquals(termCount, terms->getSumDocFreq());
  assertEquals(ft->storeTermVectorPositions(), terms->hasPositions());
  assertEquals(ft->storeTermVectorOffsets(), terms->hasOffsets());
  assertEquals(ft->storeTermVectorPayloads() && tk->hasPayloads(),
               terms->hasPayloads());
  shared_ptr<Set<std::shared_ptr<BytesRef>>> *const uniqueTerms =
      unordered_set<std::shared_ptr<BytesRef>>();
  for (auto term : tk->freqs) {
    uniqueTerms->add(make_shared<BytesRef>(term.first));
  }
  std::deque<std::shared_ptr<BytesRef>> sortedTerms =
      uniqueTerms->toArray(std::deque<std::shared_ptr<BytesRef>>(0));
  Arrays::sort(sortedTerms);
  shared_ptr<TermsEnum> *const termsEnum = terms->begin();
  for (int i = 0; i < sortedTerms.size(); ++i) {
    shared_ptr<BytesRef> *const nextTerm = termsEnum->next();
    assertEquals(sortedTerms[i], nextTerm);
    assertEquals(sortedTerms[i], termsEnum->term());
    assertEquals(1, termsEnum->docFreq());

    shared_ptr<PostingsEnum> postingsEnum = termsEnum->postings(nullptr);
    postingsEnum =
        termsEnum->postings(random()->nextBoolean() ? nullptr : postingsEnum);
    assertNotNull(postingsEnum);
    assertEquals(0, postingsEnum->nextDoc());
    assertEquals(0, postingsEnum->docID());
    assertEquals(tk->freqs[termsEnum->term()->utf8ToString()],
                 static_cast<optional<int>>(postingsEnum->freq()));
    assertEquals(PostingsEnum::NO_MORE_DOCS, postingsEnum->nextDoc());
    this->docsEnum->set(postingsEnum);

    shared_ptr<PostingsEnum> docsAndPositionsEnum =
        termsEnum->postings(nullptr);
    docsAndPositionsEnum = termsEnum->postings(
        random()->nextBoolean() ? nullptr : docsAndPositionsEnum,
        PostingsEnum::POSITIONS);
    if (terms->hasPositions() || terms->hasOffsets()) {
      assertEquals(0, docsAndPositionsEnum->nextDoc());
      constexpr int freq = docsAndPositionsEnum->freq();
      assertEquals(tk->freqs[termsEnum->term()->utf8ToString()],
                   static_cast<optional<int>>(freq));
      if (docsAndPositionsEnum != nullptr) {
        for (int k = 0; k < freq; ++k) {
          constexpr int position = docsAndPositionsEnum->nextPosition();
          shared_ptr<Set<int>> *const indexes;
          if (terms->hasPositions()) {
            indexes = tk->positionToTerms[position];
            assertNotNull(indexes);
          } else {
            indexes =
                tk->startOffsetToTerms[docsAndPositionsEnum->startOffset()];
            assertNotNull(indexes);
          }
          if (terms->hasPositions()) {
            bool foundPosition = false;
            for (auto index : indexes) {
              if (tk->termBytes[index]->equals(termsEnum->term()) &&
                  tk->positions[index] == position) {
                foundPosition = true;
                break;
              }
            }
            assertTrue(foundPosition);
          }
          if (terms->hasOffsets()) {
            bool foundOffset = false;
            for (auto index : indexes) {
              if (tk->termBytes[index]->equals(termsEnum->term()) &&
                  tk->startOffsets[index] ==
                      docsAndPositionsEnum->startOffset() &&
                  tk->endOffsets[index] == docsAndPositionsEnum->endOffset()) {
                foundOffset = true;
                break;
              }
            }
            assertTrue(foundOffset);
          }
          if (terms->hasPayloads()) {
            bool foundPayload = false;
            for (auto index : indexes) {
              if (tk->termBytes[index]->equals(termsEnum->term()) &&
                  equals(tk->payloads[index],
                         docsAndPositionsEnum->getPayload())) {
                foundPayload = true;
                break;
              }
            }
            assertTrue(foundPayload);
          }
        }
        try {
          docsAndPositionsEnum->nextPosition();
          fail();
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (Exception | AssertionError e) {
          // ok
        }
      }
      assertEquals(PostingsEnum::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
    }
    this->docsEnum->set(docsAndPositionsEnum);
  }
  assertNull(termsEnum->next());
  for (int i = 0; i < 5; ++i) {
    if (random()->nextBoolean()) {
      assertTrue(termsEnum->seekExact(
          RandomPicks::randomFrom(random(), tk->termBytes)));
    } else {
      assertEquals(SeekStatus::FOUND,
                   termsEnum->seekCeil(
                       RandomPicks::randomFrom(random(), tk->termBytes)));
    }
  }
}

shared_ptr<Document>
BaseTermVectorsFormatTestCase::addId(shared_ptr<Document> doc,
                                     const wstring &id)
{
  doc->push_back(make_shared<StringField>(L"id", id, Field::Store::NO));
  return doc;
}

int BaseTermVectorsFormatTestCase::docID(shared_ptr<IndexReader> reader,
                                         const wstring &id) 
{
  return (make_shared<IndexSearcher>(reader))
      ->search(make_shared<TermQuery>(make_shared<Term>(L"id", id)), 1)
      ->scoreDocs[0]
      ->doc;
}

void BaseTermVectorsFormatTestCase::testRareVectors() 
{
  shared_ptr<RandomDocumentFactory> *const docFactory =
      make_shared<RandomDocumentFactory>(shared_from_this(), 10, 20);
  for (auto options : validOptions()) {
    constexpr int numDocs = atLeast(200);
    constexpr int docWithVectors = random()->nextInt(numDocs);
    shared_ptr<Document> *const emptyDoc = make_shared<Document>();
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<RandomIndexWriter> *const writer =
        make_shared<RandomIndexWriter>(random(), dir);
    shared_ptr<RandomDocument> *const doc =
        docFactory->newDocument(TestUtil::nextInt(random(), 1, 3), 20, options);
    for (int i = 0; i < numDocs; ++i) {
      if (i == docWithVectors) {
        writer->addDocument(addId(doc->toDocument(), L"42"));
      } else {
        writer->addDocument(emptyDoc);
      }
    }
    shared_ptr<IndexReader> *const reader = writer->getReader();
    constexpr int docWithVectorsID = docID(reader, L"42");
    for (int i = 0; i < 10; ++i) {
      constexpr int docID = random()->nextInt(numDocs);
      shared_ptr<Fields> *const fields = reader->getTermVectors(docID);
      if (docID == docWithVectorsID) {
        assertEquals(doc, fields);
      } else {
        assertNull(fields);
      }
    }
    shared_ptr<Fields> *const fields = reader->getTermVectors(docWithVectorsID);
    assertEquals(doc, fields);
    delete reader;
    delete writer;
    delete dir;
  }
}

void BaseTermVectorsFormatTestCase::testHighFreqs() 
{
  shared_ptr<RandomDocumentFactory> *const docFactory =
      make_shared<RandomDocumentFactory>(shared_from_this(), 3, 5);
  for (auto options : validOptions()) {
    if (options == Options::NONE) {
      continue;
    }
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<RandomIndexWriter> *const writer =
        make_shared<RandomIndexWriter>(random(), dir);
    shared_ptr<RandomDocument> *const doc = docFactory->newDocument(
        TestUtil::nextInt(random(), 1, 2), atLeast(20000), options);
    writer->addDocument(doc->toDocument());
    shared_ptr<IndexReader> *const reader = writer->getReader();
    assertEquals(doc, reader->getTermVectors(0));
    delete reader;
    delete writer;
    delete dir;
  }
}

void BaseTermVectorsFormatTestCase::testLotsOfFields() 
{
  shared_ptr<RandomDocumentFactory> *const docFactory =
      make_shared<RandomDocumentFactory>(shared_from_this(), 5000, 10);
  for (auto options : validOptions()) {
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<RandomIndexWriter> *const writer =
        make_shared<RandomIndexWriter>(random(), dir);
    shared_ptr<RandomDocument> *const doc =
        docFactory->newDocument(atLeast(100), 5, options);
    writer->addDocument(doc->toDocument());
    shared_ptr<IndexReader> *const reader = writer->getReader();
    assertEquals(doc, reader->getTermVectors(0));
    delete reader;
    delete writer;
    delete dir;
  }
}

void BaseTermVectorsFormatTestCase::testMixedOptions() 
{
  constexpr int numFields = TestUtil::nextInt(random(), 1, 3);
  shared_ptr<RandomDocumentFactory> *const docFactory =
      make_shared<RandomDocumentFactory>(shared_from_this(), numFields, 10);
  for (auto options1 : validOptions()) {
    for (auto options2 : validOptions()) {
      if (options1 == options2) {
        continue;
      }
      shared_ptr<Directory> *const dir = newDirectory();
      shared_ptr<RandomIndexWriter> *const writer =
          make_shared<RandomIndexWriter>(random(), dir);
      shared_ptr<RandomDocument> *const doc1 =
          docFactory->newDocument(numFields, 20, options1);
      shared_ptr<RandomDocument> *const doc2 =
          docFactory->newDocument(numFields, 20, options2);
      writer->addDocument(addId(doc1->toDocument(), L"1"));
      writer->addDocument(addId(doc2->toDocument(), L"2"));
      shared_ptr<IndexReader> *const reader = writer->getReader();
      constexpr int doc1ID = docID(reader, L"1");
      assertEquals(doc1, reader->getTermVectors(doc1ID));
      constexpr int doc2ID = docID(reader, L"2");
      assertEquals(doc2, reader->getTermVectors(doc2ID));
      delete reader;
      delete writer;
      delete dir;
    }
  }
}

void BaseTermVectorsFormatTestCase::testRandom() 
{
  shared_ptr<RandomDocumentFactory> *const docFactory =
      make_shared<RandomDocumentFactory>(shared_from_this(), 5, 20);
  constexpr int numDocs = atLeast(100);
  std::deque<std::shared_ptr<RandomDocument>> docs(numDocs);
  for (int i = 0; i < numDocs; ++i) {
    docs[i] = docFactory->newDocument(TestUtil::nextInt(random(), 1, 3),
                                      TestUtil::nextInt(random(), 10, 50),
                                      randomOptions());
  }
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const writer =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < numDocs; ++i) {
    writer->addDocument(addId(docs[i]->toDocument(), L"" + to_wstring(i)));
  }
  shared_ptr<IndexReader> *const reader = writer->getReader();
  for (int i = 0; i < numDocs; ++i) {
    constexpr int docID = this->docID(reader, L"" + to_wstring(i));
    assertEquals(docs[i], reader->getTermVectors(docID));
  }
  delete reader;
  delete writer;
  delete dir;
}

void BaseTermVectorsFormatTestCase::testMerge() 
{
  shared_ptr<RandomDocumentFactory> *const docFactory =
      make_shared<RandomDocumentFactory>(shared_from_this(), 5, 20);
  constexpr int numDocs = atLeast(100);
  constexpr int numDeletes = random()->nextInt(numDocs);
  shared_ptr<Set<int>> *const deletes = unordered_set<int>();
  while (deletes->size() < numDeletes) {
    deletes->add(random()->nextInt(numDocs));
  }
  for (auto options : validOptions()) {
    std::deque<std::shared_ptr<RandomDocument>> docs(numDocs);
    for (int i = 0; i < numDocs; ++i) {
      docs[i] = docFactory->newDocument(TestUtil::nextInt(random(), 1, 3),
                                        atLeast(10), options);
    }
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<RandomIndexWriter> *const writer =
        make_shared<RandomIndexWriter>(random(), dir);
    for (int i = 0; i < numDocs; ++i) {
      writer->addDocument(addId(docs[i]->toDocument(), L"" + to_wstring(i)));
      if (rarely()) {
        writer->commit();
      }
    }
    for (auto delete : deletes) {
      writer->deleteDocuments(
          make_shared<Term>(L"id", L"" + to_wstring(delete)));
    }
    // merge with deletes
    writer->forceMerge(1);
    shared_ptr<IndexReader> *const reader = writer->getReader();
    for (int i = 0; i < numDocs; ++i) {
      if (!deletes->contains(i)) {
        constexpr int docID = this->docID(reader, L"" + to_wstring(i));
        assertEquals(docs[i], reader->getTermVectors(docID));
      }
    }
    delete reader;
    delete writer;
    delete dir;
  }
}

void BaseTermVectorsFormatTestCase::testClone() throw(IOException,
                                                      InterruptedException)
{
  shared_ptr<RandomDocumentFactory> *const docFactory =
      make_shared<RandomDocumentFactory>(shared_from_this(), 5, 20);
  constexpr int numDocs = atLeast(100);
  for (auto options : validOptions()) {
    std::deque<std::shared_ptr<RandomDocument>> docs(numDocs);
    for (int i = 0; i < numDocs; ++i) {
      docs[i] = docFactory->newDocument(TestUtil::nextInt(random(), 1, 3),
                                        atLeast(10), options);
    }
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<RandomIndexWriter> *const writer =
        make_shared<RandomIndexWriter>(random(), dir);
    for (int i = 0; i < numDocs; ++i) {
      writer->addDocument(addId(docs[i]->toDocument(), L"" + to_wstring(i)));
    }
    shared_ptr<IndexReader> *const reader = writer->getReader();
    for (int i = 0; i < numDocs; ++i) {
      constexpr int docID = this->docID(reader, L"" + to_wstring(i));
      assertEquals(docs[i], reader->getTermVectors(docID));
    }

    shared_ptr<AtomicReference<runtime_error>> *const exception =
        make_shared<AtomicReference<runtime_error>>();
    std::deque<std::shared_ptr<Thread>> threads(2);
    for (int i = 0; i < threads.size(); ++i) {
      threads[i] = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), numDocs, docs, reader, exception, i);
    }
    for (auto thread : threads) {
      thread->start();
    }
    for (auto thread : threads) {
      thread->join();
    }
    delete reader;
    delete writer;
    delete dir;
    assertNull(L"One thread threw an exception", exception->get());
  }
}

BaseTermVectorsFormatTestCase::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<BaseTermVectorsFormatTestCase> outerInstance, int numDocs,
        deque<std::shared_ptr<
            org::apache::lucene::index::BaseTermVectorsFormatTestCase::
                RandomDocument>> &docs,
        shared_ptr<org::apache::lucene::index::IndexReader> reader,
        shared_ptr<AtomicReference<runtime_error>> exception, int i)
{
  this->outerInstance = outerInstance;
  this->numDocs = numDocs;
  this->docs = docs;
  this->reader = reader;
  this->exception = exception;
  this->i = i;
}

void BaseTermVectorsFormatTestCase::ThreadAnonymousInnerClass::run()
{
  try {
    for (int i = 0; i < atLeast(100); ++i) {
      constexpr int idx = random()->nextInt(numDocs);
      constexpr int docID =
          outerInstance->this->docID(reader, L"" + to_wstring(idx));
      TestUtil::assertEquals(docs[idx], reader->getTermVectors(docID));
    }
  } catch (const runtime_error &t) {
    exception->set(t);
  }
}

void BaseTermVectorsFormatTestCase::testPostingsEnumFreqs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  doc->push_back(make_shared<Field>(L"foo", L"bar bar", ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  shared_ptr<Terms> terms = getOnlyLeafReader(reader)->getTermVector(0, L"foo");
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum);
  assertEquals(make_shared<BytesRef>(L"bar"), termsEnum->next());

  // simple use (FREQS)
  shared_ptr<PostingsEnum> postings = termsEnum->postings(nullptr);
  assertEquals(-1, postings->docID());
  assertEquals(0, postings->nextDoc());
  assertEquals(2, postings->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  // and it had better work
  assertEquals(-1, postings2->docID());
  assertEquals(0, postings2->nextDoc());
  assertEquals(2, postings2->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  assertEquals(-1, docsOnly->docID());
  assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  // and it had better work
  assertEquals(-1, docsOnly2->docID());
  assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for any flags: ok
  for (auto flag :
       std::deque<int>{NONE, FREQS, POSITIONS, PAYLOADS, OFFSETS, ALL}) {
    postings = termsEnum->postings(nullptr, flag);
    assertEquals(-1, postings->docID());
    assertEquals(0, postings->nextDoc());
    if (flag != NONE) {
      assertEquals(2, postings->freq());
    }
    assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());
    // reuse that too
    postings2 = termsEnum->postings(postings, flag);
    assertNotNull(postings2);
    // and it had better work
    assertEquals(-1, postings2->docID());
    assertEquals(0, postings2->nextDoc());
    if (flag != NONE) {
      assertEquals(2, postings2->freq());
    }
    assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());
  }

  delete iw;
  reader->close();
  delete dir;
}

BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<BaseTermVectorsFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

void BaseTermVectorsFormatTestCase::testPostingsEnumPositions() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(true);
  doc->push_back(make_shared<Field>(L"foo", L"bar bar", ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  shared_ptr<Terms> terms = getOnlyLeafReader(reader)->getTermVector(0, L"foo");
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum);
  assertEquals(make_shared<BytesRef>(L"bar"), termsEnum->next());

  // simple use (FREQS)
  shared_ptr<PostingsEnum> postings = termsEnum->postings(nullptr);
  assertEquals(-1, postings->docID());
  assertEquals(0, postings->nextDoc());
  assertEquals(2, postings->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  // and it had better work
  assertEquals(-1, postings2->docID());
  assertEquals(0, postings2->nextDoc());
  assertEquals(2, postings2->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  assertEquals(-1, docsOnly->docID());
  assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  // and it had better work
  assertEquals(-1, docsOnly2->docID());
  assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  // payloads, offsets, etc don't cause an error if they aren't there
  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  // but make sure they work
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<BaseTermVectorsFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

void BaseTermVectorsFormatTestCase::testPostingsEnumOffsets() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(true);
  ft->setStoreTermVectorOffsets(true);
  doc->push_back(make_shared<Field>(L"foo", L"bar bar", ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  shared_ptr<Terms> terms = getOnlyLeafReader(reader)->getTermVector(0, L"foo");
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum);
  assertEquals(make_shared<BytesRef>(L"bar"), termsEnum->next());

  // simple usage (FREQS)
  shared_ptr<PostingsEnum> postings = termsEnum->postings(nullptr);
  assertEquals(-1, postings->docID());
  assertEquals(0, postings->nextDoc());
  assertEquals(2, postings->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  // and it had better work
  assertEquals(-1, postings2->docID());
  assertEquals(0, postings2->nextDoc());
  assertEquals(2, postings2->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  assertEquals(-1, docsOnly->docID());
  assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  // and it had better work
  assertEquals(-1, docsOnly2->docID());
  assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  // payloads don't cause an error if they aren't there
  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  // but make sure they work
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(0, docsAndPositionsEnum->startOffset());
  assertEquals(3, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(4, docsAndPositionsEnum->startOffset());
  assertEquals(7, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(0, docsAndPositionsEnum2->startOffset());
  assertEquals(3, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(4, docsAndPositionsEnum2->startOffset());
  assertEquals(7, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(0, docsAndPositionsEnum->startOffset());
  assertEquals(3, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(4, docsAndPositionsEnum->startOffset());
  assertEquals(7, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(0, docsAndPositionsEnum2->startOffset());
  assertEquals(3, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(4, docsAndPositionsEnum2->startOffset());
  assertEquals(7, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<BaseTermVectorsFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

void BaseTermVectorsFormatTestCase::
    testPostingsEnumOffsetsWithoutPositions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this()));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  doc->push_back(make_shared<Field>(L"foo", L"bar bar", ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  shared_ptr<Terms> terms = getOnlyLeafReader(reader)->getTermVector(0, L"foo");
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum);
  assertEquals(make_shared<BytesRef>(L"bar"), termsEnum->next());

  // simple usage (FREQS)
  shared_ptr<PostingsEnum> postings = termsEnum->postings(nullptr);
  assertEquals(-1, postings->docID());
  assertEquals(0, postings->nextDoc());
  assertEquals(2, postings->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  // and it had better work
  assertEquals(-1, postings2->docID());
  assertEquals(0, postings2->nextDoc());
  assertEquals(2, postings2->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  assertEquals(-1, docsOnly->docID());
  assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  // and it had better work
  assertEquals(-1, docsOnly2->docID());
  assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  // payloads don't cause an error if they aren't there
  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  // but make sure they work
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  assertEquals(0, docsAndPositionsEnum->startOffset());
  assertEquals(3, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  assertEquals(4, docsAndPositionsEnum->startOffset());
  assertEquals(7, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  assertEquals(0, docsAndPositionsEnum2->startOffset());
  assertEquals(3, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  assertEquals(4, docsAndPositionsEnum2->startOffset());
  assertEquals(7, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  assertEquals(0, docsAndPositionsEnum->startOffset());
  assertEquals(3, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(-1, docsAndPositionsEnum->nextPosition());
  assertEquals(4, docsAndPositionsEnum->startOffset());
  assertEquals(7, docsAndPositionsEnum->endOffset());
  assertNull(docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  assertEquals(0, docsAndPositionsEnum2->startOffset());
  assertEquals(3, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(-1, docsAndPositionsEnum2->nextPosition());
  assertEquals(4, docsAndPositionsEnum2->startOffset());
  assertEquals(7, docsAndPositionsEnum2->endOffset());
  assertNull(docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<BaseTermVectorsFormatTestCase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
BaseTermVectorsFormatTestCase::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

void BaseTermVectorsFormatTestCase::testPostingsEnumPayloads() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Token> token1 = make_shared<Token>(L"bar", 0, 3);
  token1->setPayload(make_shared<BytesRef>(L"pay1"));
  shared_ptr<Token> token2 = make_shared<Token>(L"bar", 4, 7);
  token2->setPayload(make_shared<BytesRef>(L"pay2"));
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(true);
  ft->setStoreTermVectorPayloads(true);
  doc->push_back(make_shared<Field>(
      L"foo", make_shared<CannedTokenStream>(token1, token2), ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  shared_ptr<Terms> terms = getOnlyLeafReader(reader)->getTermVector(0, L"foo");
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum);
  assertEquals(make_shared<BytesRef>(L"bar"), termsEnum->next());

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings = termsEnum->postings(nullptr);
  assertEquals(-1, postings->docID());
  assertEquals(0, postings->nextDoc());
  assertEquals(2, postings->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  // and it had better work
  assertEquals(-1, postings2->docID());
  assertEquals(0, postings2->nextDoc());
  assertEquals(2, postings2->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  assertEquals(-1, docsOnly->docID());
  assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  // and it had better work
  assertEquals(-1, docsOnly2->docID());
  assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  // payloads
  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(-1, docsAndPositionsEnum->startOffset());
  assertEquals(-1, docsAndPositionsEnum->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(-1, docsAndPositionsEnum2->startOffset());
  assertEquals(-1, docsAndPositionsEnum2->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}

void BaseTermVectorsFormatTestCase::testPostingsEnumAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Token> token1 = make_shared<Token>(L"bar", 0, 3);
  token1->setPayload(make_shared<BytesRef>(L"pay1"));
  shared_ptr<Token> token2 = make_shared<Token>(L"bar", 4, 7);
  token2->setPayload(make_shared<BytesRef>(L"pay2"));
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(true);
  ft->setStoreTermVectorPayloads(true);
  ft->setStoreTermVectorOffsets(true);
  doc->push_back(make_shared<Field>(
      L"foo", make_shared<CannedTokenStream>(token1, token2), ft));
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);

  shared_ptr<Terms> terms = getOnlyLeafReader(reader)->getTermVector(0, L"foo");
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertNotNull(termsEnum);
  assertEquals(make_shared<BytesRef>(L"bar"), termsEnum->next());

  // sugar method (FREQS)
  shared_ptr<PostingsEnum> postings = termsEnum->postings(nullptr);
  assertEquals(-1, postings->docID());
  assertEquals(0, postings->nextDoc());
  assertEquals(2, postings->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings->nextDoc());

  // termsenum reuse (FREQS)
  shared_ptr<PostingsEnum> postings2 = termsEnum->postings(postings);
  assertNotNull(postings2);
  // and it had better work
  assertEquals(-1, postings2->docID());
  assertEquals(0, postings2->nextDoc());
  assertEquals(2, postings2->freq());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, postings2->nextDoc());

  // asking for docs only: ok
  shared_ptr<PostingsEnum> docsOnly =
      termsEnum->postings(nullptr, PostingsEnum::NONE);
  assertEquals(-1, docsOnly->docID());
  assertEquals(0, docsOnly->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly->freq() == 1 || docsOnly->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly->nextDoc());
  // reuse that too
  shared_ptr<PostingsEnum> docsOnly2 =
      termsEnum->postings(docsOnly, PostingsEnum::NONE);
  assertNotNull(docsOnly2);
  // and it had better work
  assertEquals(-1, docsOnly2->docID());
  assertEquals(0, docsOnly2->nextDoc());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsOnly2->freq() == 1 || docsOnly2->freq() == 2);
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsOnly2->nextDoc());

  // asking for positions, ok
  shared_ptr<PostingsEnum> docsAndPositionsEnum =
      termsEnum->postings(nullptr, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());

  // now reuse the positions
  shared_ptr<PostingsEnum> docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::POSITIONS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  // payloads
  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::PAYLOADS);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 0);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 3);
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->startOffset() == -1 ||
             docsAndPositionsEnum->startOffset() == 4);
  assertTrue(docsAndPositionsEnum->endOffset() == -1 ||
             docsAndPositionsEnum->endOffset() == 7);
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::PAYLOADS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 0);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 3);
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->startOffset() == -1 ||
             docsAndPositionsEnum2->startOffset() == 4);
  assertTrue(docsAndPositionsEnum2->endOffset() == -1 ||
             docsAndPositionsEnum2->endOffset() == 7);
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::OFFSETS);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(0, docsAndPositionsEnum->startOffset());
  assertEquals(3, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(4, docsAndPositionsEnum->startOffset());
  assertEquals(7, docsAndPositionsEnum->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  // reuse
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::OFFSETS);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(0, docsAndPositionsEnum2->startOffset());
  assertEquals(3, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay1"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(4, docsAndPositionsEnum2->startOffset());
  assertEquals(7, docsAndPositionsEnum2->endOffset());
  // we don't define what it is, but if its something else, we should look into
  // it?
  assertTrue(docsAndPositionsEnum2->getPayload() == nullptr ||
             (make_shared<BytesRef>(L"pay2"))
                 ->equals(docsAndPositionsEnum2->getPayload()));
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  docsAndPositionsEnum = termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertNotNull(docsAndPositionsEnum);
  assertEquals(-1, docsAndPositionsEnum->docID());
  assertEquals(0, docsAndPositionsEnum->nextDoc());
  assertEquals(2, docsAndPositionsEnum->freq());
  assertEquals(0, docsAndPositionsEnum->nextPosition());
  assertEquals(0, docsAndPositionsEnum->startOffset());
  assertEquals(3, docsAndPositionsEnum->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum->getPayload());
  assertEquals(1, docsAndPositionsEnum->nextPosition());
  assertEquals(4, docsAndPositionsEnum->startOffset());
  assertEquals(7, docsAndPositionsEnum->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, docsAndPositionsEnum->nextDoc());
  docsAndPositionsEnum2 =
      termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
  assertEquals(-1, docsAndPositionsEnum2->docID());
  assertEquals(0, docsAndPositionsEnum2->nextDoc());
  assertEquals(2, docsAndPositionsEnum2->freq());
  assertEquals(0, docsAndPositionsEnum2->nextPosition());
  assertEquals(0, docsAndPositionsEnum2->startOffset());
  assertEquals(3, docsAndPositionsEnum2->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay1"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(1, docsAndPositionsEnum2->nextPosition());
  assertEquals(4, docsAndPositionsEnum2->startOffset());
  assertEquals(7, docsAndPositionsEnum2->endOffset());
  assertEquals(make_shared<BytesRef>(L"pay2"),
               docsAndPositionsEnum2->getPayload());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS,
               docsAndPositionsEnum2->nextDoc());

  delete iw;
  reader->close();
  delete dir;
}
} // namespace org::apache::lucene::index