using namespace std;

#include "RandomPostingsTester.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using Directory = org::apache::lucene::store::Directory;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using Version = org::apache::lucene::util::Version;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using RandomAcceptedStrings = org::apache::lucene::util::automaton::
    AutomatonTestUtil::RandomAcceptedStrings;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
//    import static org.junit.Assert.assertEquals;
//    import static org.junit.Assert.assertFalse;
//    import static org.junit.Assert.assertNotNull;
//    import static org.junit.Assert.assertNull;
//    import static org.junit.Assert.assertTrue;

RandomPostingsTester::RandomPostingsTester(shared_ptr<Random> random) throw(
    IOException)
    : random(random)
{
  fields = map_obj<>();

  constexpr int numFields = TestUtil::nextInt(random, 1, 5);
  if (LuceneTestCase::VERBOSE) {
    wcout << L"TEST: " << numFields << L" fields" << endl;
  }
  maxDoc = 0;

  std::deque<std::shared_ptr<FieldInfo>> fieldInfoArray(numFields);
  int fieldUpto = 0;
  while (fieldUpto < numFields) {
    wstring field = TestUtil::randomSimpleString(random);
    if (fields.find(field) != fields.end()) {
      continue;
    }

    fieldInfoArray[fieldUpto] = make_shared<FieldInfo>(
        field, fieldUpto, false, false, true,
        IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS,
        DocValuesType::NONE, -1, unordered_map<>(), 0, 0, false);
    fieldUpto++;

    shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
        postings =
            map_obj<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>();
    fields.emplace(field, postings);
    shared_ptr<Set<wstring>> seenTerms = unordered_set<wstring>();

    int numTerms;
    if (random->nextInt(10) == 7) {
      numTerms = LuceneTestCase::atLeast(random, 50);
    } else {
      numTerms = TestUtil::nextInt(random, 2, 20);
    }

    while (postings->size() < numTerms) {
      int termUpto = postings->size();
      // Cannot contain surrogates else default Java string sort order (by UTF16
      // code unit) is different from Lucene:
      wstring term = TestUtil::randomSimpleString(random);
      if (seenTerms->contains(term)) {
        continue;
      }
      seenTerms->add(term);

      if (LuceneTestCase::TEST_NIGHTLY && termUpto == 0 && fieldUpto == 1) {
        // Make 1 big term:
        term = L"big_" + term;
      } else if (termUpto == 1 && fieldUpto == 1) {
        // Make 1 medium term:
        term = L"medium_" + term;
      } else if (random->nextBoolean()) {
        // Low freq term:
        term = L"low_" + term;
      } else {
        // Very low freq term (don't multiply by RANDOM_MULTIPLIER):
        term = L"verylow_" + term;
      }

      int64_t termSeed = random->nextLong();
      postings->put(make_shared<BytesRef>(term),
                    make_shared<SeedAndOrd>(termSeed));

      // NOTE: sort of silly: we enum all the docs just to
      // get the maxDoc
      shared_ptr<PostingsEnum> postingsEnum =
          getSeedPostings(term, termSeed, IndexOptions::DOCS, true);
      int doc;
      int lastDoc = 0;
      while ((doc = postingsEnum->nextDoc()) != PostingsEnum::NO_MORE_DOCS) {
        lastDoc = doc;
      }
      maxDoc = max(lastDoc, maxDoc);
    }

    // assign ords
    int64_t ord = 0;
    for (auto ent : postings) {
      ent->second->ord = ord++;
    }
  }

  fieldInfos = make_shared<FieldInfos>(fieldInfoArray);

  // It's the count, not the last docID:
  maxDoc++;

  allTerms = deque<>();
  for (auto fieldEnt : fields) {
    wstring field = fieldEnt.first;
    int64_t ord = 0;
    for (shared_ptr<unordered_map::Entry<std::shared_ptr<BytesRef>,
                                         std::shared_ptr<SeedAndOrd>>>
             termEnt : fieldEnt.second::entrySet()) {
      allTerms.push_back(
          make_shared<FieldAndTerm>(field, termEnt.first, ord++));
    }
  }

  if (LuceneTestCase::VERBOSE) {
    wcout << L"TEST: done init postings; " << allTerms.size()
          << L" total terms, across " << fieldInfos->size() << L" fields"
          << endl;
  }
}

shared_ptr<SeedPostings>
RandomPostingsTester::getSeedPostings(const wstring &term, int64_t seed,
                                      IndexOptions options, bool allowPayloads)
{
  int minDocFreq, maxDocFreq;
  if (StringHelper::startsWith(term, L"big_")) {
    minDocFreq = LuceneTestCase::RANDOM_MULTIPLIER * 50000;
    maxDocFreq = LuceneTestCase::RANDOM_MULTIPLIER * 70000;
  } else if (StringHelper::startsWith(term, L"medium_")) {
    minDocFreq = LuceneTestCase::RANDOM_MULTIPLIER * 3000;
    maxDocFreq = LuceneTestCase::RANDOM_MULTIPLIER * 6000;
  } else if (StringHelper::startsWith(term, L"low_")) {
    minDocFreq = LuceneTestCase::RANDOM_MULTIPLIER;
    maxDocFreq = LuceneTestCase::RANDOM_MULTIPLIER * 40;
  } else {
    minDocFreq = 1;
    maxDocFreq = 3;
  }

  return make_shared<SeedPostings>(seed, minDocFreq, maxDocFreq, options,
                                   allowPayloads);
}

RandomPostingsTester::SeedPostings::SeedPostings(int64_t seed, int minDocFreq,
                                                 int maxDocFreq,
                                                 IndexOptions options,
                                                 bool allowPayloads)
    : docRandom(make_shared<Random>(random->nextLong())),
      random(make_shared<Random>(seed)),
      maxDocSpacing(TestUtil::nextInt(random, 1, 100)),
      fixedPayloads(random->nextBoolean()),
      payload(make_shared<BytesRef>(payloadBytes)),
      doPositions(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS.compareTo(options) <= 0),
      allowPayloads(allowPayloads)
{
  docFreq = TestUtil::nextInt(random, minDocFreq, maxDocFreq);

  // TODO: more realistic to inversely tie this to numDocs:

  if (random->nextInt(10) == 7) {
    // 10% of the time create big payloads:
    payloadSize = 1 + random->nextInt(3);
  } else {
    payloadSize = 1 + random->nextInt(1);
  }

  std::deque<char> payloadBytes(payloadSize);
}

int RandomPostingsTester::SeedPostings::nextDoc()
{
  while (true) {
    _nextDoc();
    return docID_;
  }
}

int RandomPostingsTester::SeedPostings::_nextDoc()
{
  if (docID_ == -1) {
    docID_ = 0;
  }
  // Must consume random:
  while (posUpto < freq_) {
    nextPosition();
  }

  if (upto < docFreq) {
    if (upto == 0 && docRandom->nextBoolean()) {
      // Sometimes index docID = 0
    } else if (maxDocSpacing == 1) {
      docID_++;
    } else {
      // TODO: sometimes have a biggish gap here!
      docID_ += TestUtil::nextInt(docRandom, 1, maxDocSpacing);
    }

    if (random->nextInt(200) == 17) {
      freq_ = TestUtil::nextInt(random, 1, 1000);
    } else if (random->nextInt(10) == 17) {
      freq_ = TestUtil::nextInt(random, 1, 20);
    } else {
      freq_ = TestUtil::nextInt(random, 1, 4);
    }

    pos = 0;
    offset = 0;
    posUpto = 0;
    posSpacing = TestUtil::nextInt(random, 1, 100);

    upto++;
    return docID_;
  } else {
    return docID_ = NO_MORE_DOCS;
  }
}

int RandomPostingsTester::SeedPostings::docID() { return docID_; }

int RandomPostingsTester::SeedPostings::freq() { return freq_; }

int RandomPostingsTester::SeedPostings::nextPosition()
{
  if (!doPositions) {
    posUpto = freq_;
    return -1;
  }
  assert(posUpto < freq_);

  if (posUpto == 0 && random->nextBoolean()) {
    // Sometimes index pos = 0
  } else if (posSpacing == 1) {
    pos++;
  } else {
    pos += TestUtil::nextInt(random, 1, posSpacing);
  }

  if (payloadSize != 0) {
    if (fixedPayloads) {
      payload->length = payloadSize;
      random->nextBytes(payload->bytes);
    } else {
      int thisPayloadSize = random->nextInt(payloadSize);
      if (thisPayloadSize != 0) {
        payload->length = payloadSize;
        random->nextBytes(payload->bytes);
      } else {
        payload->length = 0;
      }
    }
  } else {
    payload->length = 0;
  }
  if (!allowPayloads) {
    payload->length = 0;
  }

  startOffset_ = offset + random->nextInt(5);
  endOffset_ = startOffset_ + random->nextInt(10);
  offset = endOffset_;

  posUpto++;
  return pos;
}

int RandomPostingsTester::SeedPostings::startOffset() { return startOffset_; }

int RandomPostingsTester::SeedPostings::endOffset() { return endOffset_; }

shared_ptr<BytesRef> RandomPostingsTester::SeedPostings::getPayload()
{
  return payload->length == 0 ? nullptr : payload;
}

int RandomPostingsTester::SeedPostings::advance(int target) 
{
  return slowAdvance(target);
}

int64_t RandomPostingsTester::SeedPostings::cost() { return docFreq; }

RandomPostingsTester::FieldAndTerm::FieldAndTerm(const wstring &field,
                                                 shared_ptr<BytesRef> term,
                                                 int64_t ord)
    : field(field), term(BytesRef::deepCopyOf(term)), ord(ord)
{
}

RandomPostingsTester::SeedAndOrd::SeedAndOrd(int64_t seed) : seed(seed) {}

RandomPostingsTester::SeedFields::SeedFields(
    unordered_map<wstring, SortedMap<std::shared_ptr<BytesRef>,
                                     std::shared_ptr<SeedAndOrd>>> &fields,
    shared_ptr<FieldInfos> fieldInfos, IndexOptions maxAllowed,
    bool allowPayloads)
    : fields(fields), fieldInfos(fieldInfos), maxAllowed(maxAllowed),
      allowPayloads(allowPayloads)
{
}

shared_ptr<Iterator<wstring>> RandomPostingsTester::SeedFields::iterator()
{
  return fields.keySet().begin();
}

shared_ptr<Terms> RandomPostingsTester::SeedFields::terms(const wstring &field)
{
  shared_ptr<SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
      terms = fields[field];
  if (terms == nullptr) {
    return nullptr;
  } else {
    return make_shared<SeedTerms>(terms, fieldInfos->fieldInfo(field),
                                  maxAllowed, allowPayloads);
  }
}

int RandomPostingsTester::SeedFields::size() { return fields.size(); }

RandomPostingsTester::SeedTerms::SeedTerms(
    shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
        terms,
    shared_ptr<FieldInfo> fieldInfo, IndexOptions maxAllowed,
    bool allowPayloads)
    : terms(terms), fieldInfo(fieldInfo), maxAllowed(maxAllowed),
      allowPayloads(allowPayloads)
{
}

shared_ptr<TermsEnum> RandomPostingsTester::SeedTerms::iterator()
{
  shared_ptr<SeedTermsEnum> termsEnum =
      make_shared<SeedTermsEnum>(terms, maxAllowed, allowPayloads);
  termsEnum->reset();

  return termsEnum;
}

int64_t RandomPostingsTester::SeedTerms::size() { return terms->size(); }

int64_t RandomPostingsTester::SeedTerms::getSumTotalTermFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t RandomPostingsTester::SeedTerms::getSumDocFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int RandomPostingsTester::SeedTerms::getDocCount()
{
  throw make_shared<UnsupportedOperationException>();
}

bool RandomPostingsTester::SeedTerms::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool RandomPostingsTester::SeedTerms::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool RandomPostingsTester::SeedTerms::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool RandomPostingsTester::SeedTerms::hasPayloads()
{
  return allowPayloads && fieldInfo->hasPayloads();
}

RandomPostingsTester::SeedTermsEnum::SeedTermsEnum(
    shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
        terms,
    IndexOptions maxAllowed, bool allowPayloads)
    : terms(terms), maxAllowed(maxAllowed), allowPayloads(allowPayloads)
{
}

void RandomPostingsTester::SeedTermsEnum::reset()
{
  iterator = terms->entrySet().begin();
}

SeekStatus
RandomPostingsTester::SeedTermsEnum::seekCeil(shared_ptr<BytesRef> text)
{
  shared_ptr<SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
      tailMap = terms->tailMap(text);
  if (tailMap->isEmpty()) {
    return SeekStatus::END;
  } else {
    iterator = tailMap->entrySet().begin();
    current = iterator->next();
    if (tailMap->firstKey().equals(text)) {
      return SeekStatus::FOUND;
    } else {
      return SeekStatus::NOT_FOUND;
    }
  }
}

shared_ptr<BytesRef> RandomPostingsTester::SeedTermsEnum::next()
{
  if (iterator->hasNext()) {
    current = iterator->next();
    return term();
  } else {
    return nullptr;
  }
}

void RandomPostingsTester::SeedTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef> RandomPostingsTester::SeedTermsEnum::term()
{
  return current.getKey();
}

int64_t RandomPostingsTester::SeedTermsEnum::ord()
{
  return current.getValue().ord;
}

int RandomPostingsTester::SeedTermsEnum::docFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t RandomPostingsTester::SeedTermsEnum::totalTermFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PostingsEnum>
RandomPostingsTester::SeedTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                              int flags) 
{
  if (PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {
    if (maxAllowed.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) < 0) {
      return nullptr;
    }
    if (PostingsEnum::featureRequested(flags, PostingsEnum::OFFSETS) &&
        maxAllowed.compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) < 0) {
      return nullptr;
    }
    if (PostingsEnum::featureRequested(flags, PostingsEnum::PAYLOADS) &&
        allowPayloads == false) {
      return nullptr;
    }
  }
  if (PostingsEnum::featureRequested(flags, PostingsEnum::FREQS) &&
      maxAllowed.compareTo(IndexOptions::DOCS_AND_FREQS) < 0) {
    return nullptr;
  }
  return getSeedPostings(current.getKey().utf8ToString(),
                         current.getValue().seed, maxAllowed, allowPayloads);
}

shared_ptr<FieldsProducer> RandomPostingsTester::buildIndex(
    shared_ptr<Codec> codec, shared_ptr<Directory> dir, IndexOptions maxAllowed,
    bool allowPayloads, bool alwaysTestMax) 
{
  shared_ptr<SegmentInfo> segmentInfo = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, L"_0", maxDoc, false, codec,
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);

  int maxIndexOption = Arrays::asList(IndexOptions::values())->find(maxAllowed);
  if (LuceneTestCase::VERBOSE) {
    wcout << L"\nTEST: now build index" << endl;
  }

  // TODO use allowPayloads

  std::deque<std::shared_ptr<FieldInfo>> newFieldInfoArray(fields.size());
  for (int fieldUpto = 0; fieldUpto < fields.size(); fieldUpto++) {
    shared_ptr<FieldInfo> oldFieldInfo = fieldInfos->fieldInfo(fieldUpto);

    // Randomly picked the IndexOptions to index this
    // field with:
    IndexOptions indexOptions =
        IndexOptions::values()[alwaysTestMax ? maxIndexOption
                                             : TestUtil::nextInt(
                                                   random, 1, maxIndexOption)];
    bool doPayloads = indexOptions.compareTo(
                          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0 &&
                      allowPayloads;

    newFieldInfoArray[fieldUpto] = make_shared<FieldInfo>(
        oldFieldInfo->name, fieldUpto, false, false, doPayloads, indexOptions,
        DocValuesType::NONE, -1, unordered_map<>(), 0, 0, false);
  }

  shared_ptr<FieldInfos> newFieldInfos =
      make_shared<FieldInfos>(newFieldInfoArray);

  // Estimate that flushed segment size will be 25% of
  // what we use in RAM:
  int64_t bytes = totalPostings * 8 + totalPayloadBytes;

  shared_ptr<SegmentWriteState> writeState = make_shared<SegmentWriteState>(
      nullptr, dir, segmentInfo, newFieldInfos, nullptr,
      make_shared<IOContext>(make_shared<FlushInfo>(maxDoc, bytes)));

  shared_ptr<Fields> seedFields =
      make_shared<SeedFields>(fields, newFieldInfos, maxAllowed, allowPayloads);

  shared_ptr<FieldsConsumer> consumer =
      codec->postingsFormat()->fieldsConsumer(writeState);
  bool success = false;
  try {
    consumer->write(seedFields);
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

  if (LuceneTestCase::VERBOSE) {
    wcout << L"TEST: after indexing: files=" << endl;
    for (auto file : dir->listAll()) {
      wcout << L"  " << file << L": " << dir->fileLength(file) << L" bytes"
            << endl;
    }
  }

  currentFieldInfos = newFieldInfos;

  shared_ptr<SegmentReadState> readState = make_shared<SegmentReadState>(
      dir, segmentInfo, newFieldInfos, IOContext::READ);

  return codec->postingsFormat()->fieldsProducer(readState);
}

void RandomPostingsTester::verifyEnum(
    shared_ptr<Random> random, shared_ptr<ThreadState> threadState,
    const wstring &field, shared_ptr<BytesRef> term,
    shared_ptr<TermsEnum> termsEnum, IndexOptions maxTestOptions,
    IndexOptions maxIndexOptions, shared_ptr<EnumSet<Option>> options,
    bool alwaysTestMax) 
{

  if (LuceneTestCase::VERBOSE) {
    wcout << L"  verifyEnum: options=" << options << L" maxTestOptions="
          << maxTestOptions << endl;
  }

  // Make sure TermsEnum really is positioned on the
  // expected term:
  TestUtil::assertEquals(term, termsEnum->term());

  shared_ptr<FieldInfo> fieldInfo = currentFieldInfos->fieldInfo(field);

  // NOTE: can be empty deque if we are using liveDocs:
  // C++ TODO: The following line could not be converted:
  SeedPostings expected =
      getSeedPostings(term.utf8ToString(), fields.get(field).get(term).seed,
                      maxIndexOptions, true);
  TestUtil::assertEquals(expected->docFreq, termsEnum->docFreq());

  bool allowFreqs = fieldInfo->getIndexOptions().compareTo(
                        IndexOptions::DOCS_AND_FREQS) >= 0 &&
                    maxTestOptions.compareTo(IndexOptions::DOCS_AND_FREQS) >= 0;
  bool doCheckFreqs = allowFreqs && (alwaysTestMax || random->nextInt(3) <= 2);

  bool allowPositions =
      fieldInfo->getIndexOptions().compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0 &&
      maxTestOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  bool doCheckPositions =
      allowPositions && (alwaysTestMax || random->nextInt(3) <= 2);

  bool allowOffsets =
      fieldInfo->getIndexOptions().compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0 &&
      maxTestOptions.compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
  bool doCheckOffsets =
      allowOffsets && (alwaysTestMax || random->nextInt(3) <= 2);

  bool doCheckPayloads = options->contains(Option::PAYLOADS) &&
                         allowPositions && fieldInfo->hasPayloads() &&
                         (alwaysTestMax || random->nextInt(3) <= 2);

  shared_ptr<PostingsEnum> prevPostingsEnum = nullptr;

  shared_ptr<PostingsEnum> postingsEnum;

  if (!doCheckPositions) {
    if (allowPositions && random->nextInt(10) == 7) {
      // 10% of the time, even though we will not check positions, pull a
      // DocsAndPositions enum

      if (options->contains(Option::REUSE_ENUMS) && random->nextInt(10) < 9) {
        prevPostingsEnum = threadState->reusePostingsEnum;
      }

      int flags = PostingsEnum::POSITIONS;
      if (alwaysTestMax || random->nextBoolean()) {
        flags |= PostingsEnum::OFFSETS;
      }
      if (alwaysTestMax || random->nextBoolean()) {
        flags |= PostingsEnum::PAYLOADS;
      }

      if (LuceneTestCase::VERBOSE) {
        wcout << L"  get DocsEnum (but we won't check positions) flags="
              << flags << endl;
      }

      threadState->reusePostingsEnum =
          termsEnum->postings(prevPostingsEnum, flags);
      postingsEnum = threadState->reusePostingsEnum;
    } else {
      if (LuceneTestCase::VERBOSE) {
        wcout << L"  get DocsEnum" << endl;
      }
      if (options->contains(Option::REUSE_ENUMS) && random->nextInt(10) < 9) {
        prevPostingsEnum = threadState->reusePostingsEnum;
      }
      threadState->reusePostingsEnum = termsEnum->postings(
          prevPostingsEnum,
          doCheckFreqs ? PostingsEnum::FREQS : PostingsEnum::NONE);
      postingsEnum = threadState->reusePostingsEnum;
    }
  } else {
    if (options->contains(Option::REUSE_ENUMS) && random->nextInt(10) < 9) {
      prevPostingsEnum = threadState->reusePostingsEnum;
    }

    int flags = PostingsEnum::POSITIONS;
    if (alwaysTestMax || doCheckOffsets || random->nextInt(3) == 1) {
      flags |= PostingsEnum::OFFSETS;
    }
    if (alwaysTestMax || doCheckPayloads || random->nextInt(3) == 1) {
      flags |= PostingsEnum::PAYLOADS;
    }

    if (LuceneTestCase::VERBOSE) {
      wcout << L"  get DocsEnum flags=" << flags << endl;
    }

    threadState->reusePostingsEnum =
        termsEnum->postings(prevPostingsEnum, flags);
    postingsEnum = threadState->reusePostingsEnum;
  }

  assertNotNull(L"null DocsEnum", postingsEnum);
  int initialDocID = postingsEnum->docID();
  assertEquals(L"inital docID should be -1" + postingsEnum, -1, initialDocID);

  if (LuceneTestCase::VERBOSE) {
    if (prevPostingsEnum == nullptr) {
      wcout << L"  got enum=" << postingsEnum << endl;
    } else if (prevPostingsEnum == postingsEnum) {
      wcout << L"  got reuse enum=" << postingsEnum << endl;
    } else {
      wcout << L"  got enum=" << postingsEnum << L" (reuse of "
            << prevPostingsEnum << L" failed)" << endl;
    }
  }

  // 10% of the time don't consume all docs:
  int stopAt;
  if (!alwaysTestMax && options->contains(Option::PARTIAL_DOC_CONSUME) &&
      expected->docFreq > 1 && random->nextInt(10) == 7) {
    stopAt = random->nextInt(expected->docFreq - 1);
    if (LuceneTestCase::VERBOSE) {
      wcout << L"  will not consume all docs (" << stopAt << L" vs "
            << expected->docFreq << L")" << endl;
    }
  } else {
    stopAt = expected->docFreq;
    if (LuceneTestCase::VERBOSE) {
      wcout << L"  consume all docs" << endl;
    }
  }

  double skipChance = alwaysTestMax ? 0.5 : random->nextDouble();
  int numSkips =
      expected->docFreq < 3
          ? 1
          : TestUtil::nextInt(random, 1, min(20, expected->docFreq / 3));
  int skipInc = expected->docFreq / numSkips;
  int skipDocInc = maxDoc / numSkips;

  // Sometimes do 100% skipping:
  bool doAllSkipping =
      options->contains(Option::SKIPPING) && random->nextInt(7) == 1;

  double freqAskChance = alwaysTestMax ? 1.0 : random->nextDouble();
  double payloadCheckChance = alwaysTestMax ? 1.0 : random->nextDouble();
  double offsetCheckChance = alwaysTestMax ? 1.0 : random->nextDouble();

  if (LuceneTestCase::VERBOSE) {
    if (options->contains(Option::SKIPPING)) {
      wcout << L"  skipChance=" << skipChance << L" numSkips=" << numSkips
            << endl;
    } else {
      wcout << L"  no skipping" << endl;
    }
    if (doCheckFreqs) {
      wcout << L"  freqAskChance=" << freqAskChance << endl;
    }
    if (doCheckPayloads) {
      wcout << L"  payloadCheckChance=" << payloadCheckChance << endl;
    }
    if (doCheckOffsets) {
      wcout << L"  offsetCheckChance=" << offsetCheckChance << endl;
    }
  }

  while (expected->upto <= stopAt) {
    if (expected->upto == stopAt) {
      if (stopAt == expected->docFreq) {
        assertEquals(L"DocsEnum should have ended but didn't",
                     PostingsEnum::NO_MORE_DOCS, postingsEnum->nextDoc());

        // Common bug is to forget to set this.doc=NO_MORE_DOCS in the enum!:
        assertEquals(L"DocsEnum should have ended but didn't",
                     PostingsEnum::NO_MORE_DOCS, postingsEnum->docID());
      }
      break;
    }

    if (options->contains(Option::SKIPPING) &&
        (doAllSkipping || random->nextDouble() <= skipChance)) {
      int targetDocID = -1;
      if (expected->upto < stopAt && random->nextBoolean()) {
        // Pick target we know exists:
        constexpr int skipCount = TestUtil::nextInt(random, 1, skipInc);
        for (int skip = 0; skip < skipCount; skip++) {
          if (expected->nextDoc() == PostingsEnum::NO_MORE_DOCS) {
            break;
          }
        }
      } else {
        // Pick random target (might not exist):
        constexpr int skipDocIDs = TestUtil::nextInt(random, 1, skipDocInc);
        if (skipDocIDs > 0) {
          targetDocID = expected->docID() + skipDocIDs;
          expected->advance(targetDocID);
        }
      }

      if (expected->upto >= stopAt) {
        int target =
            random->nextBoolean() ? maxDoc : PostingsEnum::NO_MORE_DOCS;
        if (LuceneTestCase::VERBOSE) {
          wcout << L"  now advance to end (target=" << target << L")" << endl;
        }
        assertEquals(L"DocsEnum should have ended but didn't",
                     PostingsEnum::NO_MORE_DOCS, postingsEnum->advance(target));
        break;
      } else {
        if (LuceneTestCase::VERBOSE) {
          if (targetDocID != -1) {
            wcout << L"  now advance to random target=" << targetDocID << L" ("
                  << expected->upto << L" of " << stopAt << L") current="
                  << postingsEnum->docID() << endl;
          } else {
            wcout << L"  now advance to known-exists target="
                  << expected->docID() << L" (" << expected->upto << L" of "
                  << stopAt << L") current=" << postingsEnum->docID() << endl;
          }
        }
        int docID = postingsEnum->advance(
            targetDocID != -1 ? targetDocID : expected->docID());
        assertEquals(L"docID is wrong", expected->docID(), docID);
      }
    } else {
      expected->nextDoc();
      if (LuceneTestCase::VERBOSE) {
        wcout << L"  now nextDoc to " << expected->docID() << L" ("
              << expected->upto << L" of " << stopAt << L")" << endl;
      }
      int docID = postingsEnum->nextDoc();
      assertEquals(L"docID is wrong", expected->docID(), docID);
      if (docID == PostingsEnum::NO_MORE_DOCS) {
        break;
      }
    }

    if (doCheckFreqs && random->nextDouble() <= freqAskChance) {
      if (LuceneTestCase::VERBOSE) {
        wcout << L"    now freq()=" << expected->freq() << endl;
      }
      int freq = postingsEnum->freq();
      assertEquals(L"freq is wrong", expected->freq(), freq);
    }

    if (doCheckPositions) {
      int freq = postingsEnum->freq();
      int numPosToConsume;
      if (!alwaysTestMax && options->contains(Option::PARTIAL_POS_CONSUME) &&
          random->nextInt(5) == 1) {
        numPosToConsume = random->nextInt(freq);
      } else {
        numPosToConsume = freq;
      }

      for (int i = 0; i < numPosToConsume; i++) {
        int pos = expected->nextPosition();
        if (LuceneTestCase::VERBOSE) {
          wcout << L"    now nextPosition to " << pos << endl;
        }
        assertEquals(L"position is wrong", pos, postingsEnum->nextPosition());

        if (doCheckPayloads) {
          shared_ptr<BytesRef> expectedPayload = expected->getPayload();
          if (random->nextDouble() <= payloadCheckChance) {
            if (LuceneTestCase::VERBOSE) {
              wcout << L"      now check expectedPayload length="
                    << (expectedPayload == nullptr ? 0
                                                   : expectedPayload->length)
                    << endl;
            }
            if (expectedPayload == nullptr || expectedPayload->length == 0) {
              assertNull(L"should not have payload",
                         postingsEnum->getPayload());
            } else {
              shared_ptr<BytesRef> payload = postingsEnum->getPayload();
              assertNotNull(L"should have payload but doesn't", payload);

              assertEquals(L"payload length is wrong", expectedPayload->length,
                           payload->length);
              for (int byteUpto = 0; byteUpto < expectedPayload->length;
                   byteUpto++) {
                assertEquals(
                    L"payload bytes are wrong",
                    expectedPayload->bytes[expectedPayload->offset + byteUpto],
                    payload->bytes[payload->offset + byteUpto]);
              }

              // make a deep copy
              payload = BytesRef::deepCopyOf(payload);
              assertEquals(
                  L"2nd call to getPayload returns something different!",
                  payload, postingsEnum->getPayload());
            }
          } else {
            if (LuceneTestCase::VERBOSE) {
              wcout << L"      skip check payload length="
                    << (expectedPayload == nullptr ? 0
                                                   : expectedPayload->length)
                    << endl;
            }
          }
        }

        if (doCheckOffsets) {
          if (random->nextDouble() <= offsetCheckChance) {
            if (LuceneTestCase::VERBOSE) {
              wcout << L"      now check offsets: startOff="
                    << expected->startOffset() << L" endOffset="
                    << expected->endOffset() << endl;
            }
            assertEquals(L"startOffset is wrong", expected->startOffset(),
                         postingsEnum->startOffset());
            assertEquals(L"endOffset is wrong", expected->endOffset(),
                         postingsEnum->endOffset());
          } else {
            if (LuceneTestCase::VERBOSE) {
              wcout << L"      skip check offsets" << endl;
            }
          }
        } else if (fieldInfo->getIndexOptions().compareTo(
                       IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) <
                   0) {
          if (LuceneTestCase::VERBOSE) {
            wcout << L"      now check offsets are -1" << endl;
          }
          assertEquals(L"startOffset isn't -1", -1,
                       postingsEnum->startOffset());
          assertEquals(L"endOffset isn't -1", -1, postingsEnum->endOffset());
        }
      }
    }
  }
}

RandomPostingsTester::TestThread::TestThread(
    shared_ptr<Random> random, shared_ptr<RandomPostingsTester> postingsTester,
    shared_ptr<Fields> fieldsSource, shared_ptr<EnumSet<Option>> options,
    IndexOptions maxTestOptions, IndexOptions maxIndexOptions,
    bool alwaysTestMax)
{
  this->random = random;
  this->fieldsSource = fieldsSource;
  this->options = options;
  this->maxTestOptions = maxTestOptions;
  this->maxIndexOptions = maxIndexOptions;
  this->alwaysTestMax = alwaysTestMax;
  this->postingsTester = postingsTester;
}

void RandomPostingsTester::TestThread::run()
{
  try {
    try {
      postingsTester->testTermsOneThread(random, fieldsSource, options,
                                         maxTestOptions, maxIndexOptions,
                                         alwaysTestMax);
    } catch (const runtime_error &t) {
      throw runtime_error(t);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    fieldsSource.reset();
    postingsTester.reset();
  }
}

void RandomPostingsTester::testTerms(
    shared_ptr<Fields> fieldsSource, shared_ptr<EnumSet<Option>> options,
    IndexOptions const maxTestOptions, IndexOptions const maxIndexOptions,
    bool const alwaysTestMax) 
{

  if (options->contains(Option::THREADS)) {
    int numThreads = TestUtil::nextInt(random, 2, 5);
    std::deque<std::shared_ptr<Thread>> threads(numThreads);
    for (int threadUpto = 0; threadUpto < numThreads; threadUpto++) {
      threads[threadUpto] = make_shared<TestThread>(
          make_shared<Random>(random->nextLong()), shared_from_this(),
          fieldsSource, options, maxTestOptions, maxIndexOptions,
          alwaysTestMax);
      threads[threadUpto]->start();
    }
    for (int threadUpto = 0; threadUpto < numThreads; threadUpto++) {
      threads[threadUpto]->join();
    }
  } else {
    testTermsOneThread(random, fieldsSource, options, maxTestOptions,
                       maxIndexOptions, alwaysTestMax);
  }
}

void RandomPostingsTester::testTermsOneThread(
    shared_ptr<Random> random, shared_ptr<Fields> fieldsSource,
    shared_ptr<EnumSet<Option>> options, IndexOptions maxTestOptions,
    IndexOptions maxIndexOptions, bool alwaysTestMax) 
{

  shared_ptr<ThreadState> threadState = make_shared<ThreadState>();

  // Test random terms/fields:
  deque<std::shared_ptr<TermState>> termStates =
      deque<std::shared_ptr<TermState>>();
  deque<std::shared_ptr<FieldAndTerm>> termStateTerms =
      deque<std::shared_ptr<FieldAndTerm>>();

  bool supportsOrds = true;

  Collections::shuffle(allTerms, random);
  int upto = 0;
  while (upto < allTerms.size()) {

    bool useTermState = termStates.size() != 0 && random->nextInt(5) == 1;
    bool useTermOrd =
        supportsOrds && useTermState == false && random->nextInt(5) == 1;

    shared_ptr<FieldAndTerm> fieldAndTerm;
    shared_ptr<TermsEnum> termsEnum;

    shared_ptr<TermState> termState = nullptr;

    if (!useTermState) {
      // Seek by random field+term:
      fieldAndTerm = allTerms[upto++];
      if (LuceneTestCase::VERBOSE) {
        if (useTermOrd) {
          wcout << L"\nTEST: seek to term=" << fieldAndTerm->field << L":"
                << fieldAndTerm->term->utf8ToString() << L" using ord="
                << fieldAndTerm->ord << endl;
        } else {
          wcout << L"\nTEST: seek to term=" << fieldAndTerm->field << L":"
                << fieldAndTerm->term->utf8ToString() << endl;
        }
      }
    } else {
      // Seek by previous saved TermState
      int idx = random->nextInt(termStates.size());
      fieldAndTerm = termStateTerms[idx];
      if (LuceneTestCase::VERBOSE) {
        wcout << L"\nTEST: seek using TermState to term=" << fieldAndTerm->field
              << L":" << fieldAndTerm->term->utf8ToString() << endl;
      }
      termState = termStates[idx];
    }

    shared_ptr<Terms> terms = fieldsSource->terms(fieldAndTerm->field);
    assertNotNull(terms);
    termsEnum = terms->begin();

    if (!useTermState) {
      if (useTermOrd) {
        // Try seek by ord sometimes:
        try {
          termsEnum->seekExact(fieldAndTerm->ord);
        } catch (const UnsupportedOperationException &uoe) {
          supportsOrds = false;
          assertTrue(termsEnum->seekExact(fieldAndTerm->term));
        }
      } else {
        assertTrue(termsEnum->seekExact(fieldAndTerm->term));
      }
    } else {
      termsEnum->seekExact(fieldAndTerm->term, termState);
    }

    // check we really seeked to the right place
    TestUtil::assertEquals(fieldAndTerm->term, termsEnum->term());

    int64_t termOrd;
    if (supportsOrds) {
      try {
        termOrd = termsEnum->ord();
      } catch (const UnsupportedOperationException &uoe) {
        supportsOrds = false;
        termOrd = -1;
      }
    } else {
      termOrd = -1;
    }

    if (termOrd != -1) {
      // PostingsFormat supports ords
      TestUtil::assertEquals(fieldAndTerm->ord, termsEnum->ord());
    }

    bool savedTermState = false;

    if (options->contains(Option::TERM_STATE) && !useTermState &&
        random->nextInt(5) == 1) {
      // Save away this TermState:
      termStates.push_back(termsEnum->termState());
      termStateTerms.push_back(fieldAndTerm);
      savedTermState = true;
    }

    verifyEnum(random, threadState, fieldAndTerm->field, fieldAndTerm->term,
               termsEnum, maxTestOptions, maxIndexOptions, options,
               alwaysTestMax);

    // Sometimes save term state after pulling the enum:
    if (options->contains(Option::TERM_STATE) && !useTermState &&
        !savedTermState && random->nextInt(5) == 1) {
      // Save away this TermState:
      termStates.push_back(termsEnum->termState());
      termStateTerms.push_back(fieldAndTerm);
      useTermState = true;
    }

    // 10% of the time make sure you can pull another enum
    // from the same term:
    if (alwaysTestMax || random->nextInt(10) == 7) {
      // Try same term again
      if (LuceneTestCase::VERBOSE) {
        wcout << L"TEST: try enum again on same term" << endl;
      }

      verifyEnum(random, threadState, fieldAndTerm->field, fieldAndTerm->term,
                 termsEnum, maxTestOptions, maxIndexOptions, options,
                 alwaysTestMax);
    }
  }

  // Test Terms.intersect:
  for (auto field : fields) {
    while (true) {
      shared_ptr<Automaton> a = AutomatonTestUtil::randomAutomaton(random);
      shared_ptr<CompiledAutomaton> ca = make_shared<CompiledAutomaton>(
          a, nullptr, true, numeric_limits<int>::max(), false);
      if (ca->type != CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
        // Keep retrying until we get an A that will really "use" the PF's
        // intersect code:
        continue;
      }
      // System.out.println("A:\n" + a.toDot());

      shared_ptr<BytesRef> startTerm = nullptr;
      if (random->nextBoolean()) {
        shared_ptr<AutomatonTestUtil::RandomAcceptedStrings> ras =
            make_shared<AutomatonTestUtil::RandomAcceptedStrings>(a);
        for (int iter = 0; iter < 100; iter++) {
          std::deque<int> codePoints = ras->getRandomAcceptedString(random);
          if (codePoints.empty()) {
            continue;
          }
          startTerm = make_shared<BytesRef>(
              UnicodeUtil::newString(codePoints, 0, codePoints.size()));
          break;
        }
        // Don't allow empty string startTerm:
        if (startTerm == nullptr) {
          continue;
        }
      }
      shared_ptr<TermsEnum> intersected =
          fieldsSource->terms(field.first)->intersect(ca, startTerm);

      shared_ptr<Set<std::shared_ptr<BytesRef>>> intersectedTerms =
          unordered_set<std::shared_ptr<BytesRef>>();
      shared_ptr<BytesRef> term;
      while ((term = intersected->next()) != nullptr) {
        if (startTerm != nullptr) {
          // NOTE: not <=
          assertTrue(startTerm->compareTo(term) < 0);
        }
        intersectedTerms->add(BytesRef::deepCopyOf(term));
        verifyEnum(random, threadState, field.first, term, intersected,
                   maxTestOptions, maxIndexOptions, options, alwaysTestMax);
      }

      if (ca->runAutomaton == nullptr) {
        assertTrue(intersectedTerms->isEmpty());
      } else {
        for (auto term2 : fields[field.first]) {
          bool expected;
          if (startTerm != nullptr && startTerm->compareTo(term2->first) >= 0) {
            expected = false;
          } else {
            expected = ca->runAutomaton->run(
                term2->first.bytes, term2->first.offset, term2->first->length);
          }
          assertEquals(L"term=" + term2->first, expected,
                       intersectedTerms->contains(term2->first));
        }
      }

      break;
    }
  }
}

void RandomPostingsTester::testFields(shared_ptr<Fields> fields) throw(
    runtime_error)
{
  org::apache::lucene::index::Fields::const_iterator iterator = fields->begin();
  while (iterator != fields->end()) {
    *iterator;
    try {
      (*iterator)->remove();
      throw make_shared<AssertionError>(
          L"Fields.iterator() allows for removal");
    } catch (const UnsupportedOperationException &expected) {
      // expected;
    }
    iterator++;
  }
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(iterator->hasNext());
  LuceneTestCase::expectThrows(NoSuchElementException::typeid,
                               [&]() { iterator->next(); });
}

void RandomPostingsTester::testFull(shared_ptr<Codec> codec,
                                    shared_ptr<Path> path, IndexOptions options,
                                    bool withPayloads) 
{
  shared_ptr<Directory> dir = LuceneTestCase::newFSDirectory(path);

  // TODO test thread safety of buildIndex too
  shared_ptr<FieldsProducer> fieldsProducer =
      buildIndex(codec, dir, options, withPayloads, true);

  testFields(fieldsProducer);

  std::deque<IndexOptions> allOptions = IndexOptions::values();
  int maxIndexOption = Arrays::asList(allOptions)->find(options);

  for (int i = 0; i <= maxIndexOption; i++) {
    testTerms(fieldsProducer, EnumSet::allOf(Option::typeid), allOptions[i],
              options, true);
    if (withPayloads) {
      // If we indexed w/ payloads, also test enums w/o accessing payloads:
      testTerms(fieldsProducer,
                EnumSet::complementOf(EnumSet::of(Option::PAYLOADS)),
                allOptions[i], options, true);
    }
  }

  fieldsProducer->close();
  delete dir;
}
} // namespace org::apache::lucene::index