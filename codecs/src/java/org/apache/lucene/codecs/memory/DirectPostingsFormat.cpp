using namespace std;

#include "DirectPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/OrdTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/RunAutomaton.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Transition.h"

namespace org::apache::lucene::codecs::memory
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Lucene50PostingsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsFormat;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using OrdTermState = org::apache::lucene::index::OrdTermState;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IOContext = org::apache::lucene::store::IOContext;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;
using Transition = org::apache::lucene::util::automaton::Transition;

DirectPostingsFormat::DirectPostingsFormat()
    : DirectPostingsFormat(DEFAULT_MIN_SKIP_COUNT, DEFAULT_LOW_FREQ_CUTOFF)
{
}

DirectPostingsFormat::DirectPostingsFormat(int minSkipCount, int lowFreqCutoff)
    : org::apache::lucene::codecs::PostingsFormat(L"Direct"),
      minSkipCount(minSkipCount), lowFreqCutoff(lowFreqCutoff)
{
}

shared_ptr<FieldsConsumer> DirectPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return PostingsFormat::forName(L"Lucene50")->fieldsConsumer(state);
}

shared_ptr<FieldsProducer> DirectPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  shared_ptr<FieldsProducer> postings =
      PostingsFormat::forName(L"Lucene50")->fieldsProducer(state);
  if (state->context->context != IOContext::Context::MERGE) {
    shared_ptr<FieldsProducer> loadedPostings;
    try {
      postings->checkIntegrity();
      loadedPostings = make_shared<DirectFields>(state, postings, minSkipCount,
                                                 lowFreqCutoff);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      postings->close();
    }
    return loadedPostings;
  } else {
    // Don't load postings for merge:
    return postings;
  }
}

DirectPostingsFormat::DirectFields::DirectFields(
    shared_ptr<SegmentReadState> state, shared_ptr<Fields> fields,
    int minSkipCount, int lowFreqCutoff) 
{
  for (auto field : fields) {
    this->fields.emplace(
        field, make_shared<DirectField>(state, field, fields->terms(field),
                                        minSkipCount, lowFreqCutoff));
  }
}

shared_ptr<Iterator<wstring>> DirectPostingsFormat::DirectFields::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms>
DirectPostingsFormat::DirectFields::terms(const wstring &field)
{
  return fields[field];
}

int DirectPostingsFormat::DirectFields::size() { return fields.size(); }

DirectPostingsFormat::DirectFields::~DirectFields() {}

int64_t DirectPostingsFormat::DirectFields::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  for (auto entry : fields) {
    sizeInBytes += entry.first->length() * Character::BYTES;
    sizeInBytes += entry.second::ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
DirectPostingsFormat::DirectFields::getChildResources()
{
  return Accountables::namedAccountables(L"field", fields);
}

void DirectPostingsFormat::DirectFields::checkIntegrity() 
{
  // if we read entirely into ram, we already validated.
  // otherwise returned the raw postings reader
}

wstring DirectPostingsFormat::DirectFields::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() + L")";
}

DirectPostingsFormat::DirectField::LowFreqTerm::LowFreqTerm(
    std::deque<int> &postings, std::deque<char> &payloads, int docFreq,
    int totalTermFreq)
    : postings(postings), payloads(payloads), docFreq(docFreq),
      totalTermFreq(totalTermFreq)
{
}

int64_t DirectPostingsFormat::DirectField::LowFreqTerm::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED +
         ((postings.size() > 0) ? RamUsageEstimator::sizeOf(postings) : 0) +
         ((payloads.size() > 0) ? RamUsageEstimator::sizeOf(payloads) : 0);
}

DirectPostingsFormat::DirectField::HighFreqTerm::HighFreqTerm(
    std::deque<int> &docIDs, std::deque<int> &freqs,
    std::deque<std::deque<int>> &positions,
    std::deque<std::deque<std::deque<char>>> &payloads,
    int64_t totalTermFreq)
    : totalTermFreq(totalTermFreq), docIDs(docIDs), freqs(freqs),
      positions(positions), payloads(payloads)
{
}

int64_t DirectPostingsFormat::DirectField::HighFreqTerm::ramBytesUsed()
{
  int64_t sizeInBytes = BASE_RAM_BYTES_USED;
  sizeInBytes += (docIDs.size() > 0) ? RamUsageEstimator::sizeOf(docIDs) : 0;
  sizeInBytes += (freqs.size() > 0) ? RamUsageEstimator::sizeOf(freqs) : 0;

  if (positions.size() > 0) {
    sizeInBytes += RamUsageEstimator::shallowSizeOf(positions);
    for (auto position : positions) {
      sizeInBytes +=
          (position.size() > 0) ? RamUsageEstimator::sizeOf(position) : 0;
    }
  }

  if (payloads.size() > 0) {
    sizeInBytes += RamUsageEstimator::shallowSizeOf(payloads);
    for (auto payload : payloads) {
      if (payload.size() > 0) {
        sizeInBytes += RamUsageEstimator::shallowSizeOf(payload);
        for (auto pload : payload) {
          sizeInBytes +=
              (pload.size() > 0) ? RamUsageEstimator::sizeOf(pload) : 0;
        }
      }
    }
  }

  return sizeInBytes;
}

void DirectPostingsFormat::DirectField::IntArrayWriter::add(int value)
{
  if (ints.size() == upto) {
    ints = ArrayUtil::grow(ints);
  }
  ints[upto++] = value;
}

std::deque<int> DirectPostingsFormat::DirectField::IntArrayWriter::get()
{
  const std::deque<int> arr = std::deque<int>(upto);
  System::arraycopy(ints, 0, arr, 0, upto);
  upto = 0;
  return arr;
}

DirectPostingsFormat::DirectField::DirectField(
    shared_ptr<SegmentReadState> state, const wstring &field,
    shared_ptr<Terms> termsIn, int minSkipCount,
    int lowFreqCutoff) 
    : termOffsets(std::deque<int>(1 + numTerms)),
      skips(std::deque<int>(skipCount)),
      skipOffsets(std::deque<int>(1 + numTerms)),
      terms(std::deque<std::shared_ptr<TermAndSkip>>(numTerms)),
      hasFreq(fieldInfo::getIndexOptions()->compareTo(IndexOptions::DOCS) > 0),
      hasPos(fieldInfo::getIndexOptions()->compareTo(
                 IndexOptions::DOCS_AND_FREQS) > 0),
      hasOffsets(fieldInfo::getIndexOptions()->compareTo(
                     IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) > 0),
      hasPayloads(fieldInfo::hasPayloads()),
      sumTotalTermFreq(termsIn->getSumTotalTermFreq()),
      docCount(termsIn->getDocCount()), sumDocFreq(termsIn->getSumDocFreq()),
      minSkipCount(minSkipCount)
{
  shared_ptr<FieldInfo> *const fieldInfo = state->fieldInfos->fieldInfo(field);

  constexpr int numTerms = static_cast<int>(termsIn->size());
  if (numTerms == -1) {
    throw invalid_argument(L"codec does not provide Terms.size()");
  }

  std::deque<char> termBytes(1024);

  shared_ptr<BytesRef> term;
  shared_ptr<PostingsEnum> postingsEnum = nullptr;
  shared_ptr<PostingsEnum> docsAndPositionsEnum = nullptr;
  shared_ptr<TermsEnum> *const termsEnum = termsIn->begin();
  int termOffset = 0;

  shared_ptr<IntArrayWriter> *const scratch = make_shared<IntArrayWriter>();

  // Used for payloads, if any:
  shared_ptr<RAMOutputStream> *const ros = make_shared<RAMOutputStream>();

  // if (DEBUG) {
  //   System.out.println("\nLOAD terms seg=" + state.segmentInfo.name + "
  //   field=" + field + " hasOffsets=" + hasOffsets + " hasFreq=" + hasFreq + "
  //   hasPos=" + hasPos + " hasPayloads=" + hasPayloads);
  // }

  while ((term = termsEnum->next()) != nullptr) {
    constexpr int docFreq = termsEnum->docFreq();
    constexpr int64_t totalTermFreq = termsEnum->totalTermFreq();

    // if (DEBUG) {
    //   System.out.println("  term=" + term.utf8ToString());
    // }

    termOffsets[count] = termOffset;

    if (termBytes.size() < (termOffset + term->length)) {
      termBytes = ArrayUtil::grow(termBytes, termOffset + term->length);
    }
    System::arraycopy(term->bytes, term->offset, termBytes, termOffset,
                      term->length);
    termOffset += term->length;
    termOffsets[count + 1] = termOffset;

    if (hasPos) {
      docsAndPositionsEnum =
          termsEnum->postings(docsAndPositionsEnum, PostingsEnum::ALL);
    } else {
      postingsEnum = termsEnum->postings(postingsEnum);
    }

    shared_ptr<TermAndSkip> *const ent;

    shared_ptr<PostingsEnum> *const postingsEnum2;
    if (hasPos) {
      postingsEnum2 = docsAndPositionsEnum;
    } else {
      postingsEnum2 = postingsEnum;
    }

    int docID;

    if (docFreq <= lowFreqCutoff) {

      ros->reset();

      // Pack postings for low-freq terms into a single int[]:
      while ((docID = postingsEnum2->nextDoc()) != PostingsEnum::NO_MORE_DOCS) {
        scratch->add(docID);
        if (hasFreq) {
          constexpr int freq = postingsEnum2->freq();
          scratch->add(freq);
          if (hasPos) {
            for (int pos = 0; pos < freq; pos++) {
              scratch->add(docsAndPositionsEnum->nextPosition());
              if (hasOffsets_) {
                scratch->add(docsAndPositionsEnum->startOffset());
                scratch->add(docsAndPositionsEnum->endOffset());
              }
              if (hasPayloads_) {
                shared_ptr<BytesRef> *const payload =
                    docsAndPositionsEnum->getPayload();
                if (payload != nullptr) {
                  scratch->add(payload->length);
                  ros->writeBytes(payload->bytes, payload->offset,
                                  payload->length);
                } else {
                  scratch->add(0);
                }
              }
            }
          }
        }
      }

      const std::deque<char> payloads;
      if (hasPayloads_) {
        payloads = std::deque<char>(static_cast<int>(ros->getFilePointer()));
        ros->writeTo(payloads, 0);
      } else {
        payloads.clear();
      }

      const std::deque<int> postings = scratch->get();

      ent = make_shared<LowFreqTerm>(postings, payloads, docFreq,
                                     static_cast<int>(totalTermFreq));
    } else {
      const std::deque<int> docs = std::deque<int>(docFreq);
      const std::deque<int> freqs;
      const std::deque<std::deque<int>> positions;
      const std::deque<std::deque<std::deque<char>>> payloads;
      if (hasFreq) {
        freqs = std::deque<int>(docFreq);
        if (hasPos) {
          positions = std::deque<std::deque<int>>(docFreq);
          if (hasPayloads_) {
            payloads = std::deque<std::deque<std::deque<char>>>(docFreq);
          } else {
            payloads.clear();
          }
        } else {
          positions.clear();
          payloads.clear();
        }
      } else {
        freqs.clear();
        positions.clear();
        payloads.clear();
      }

      // Use separate int[] for the postings for high-freq
      // terms:
      int upto = 0;
      while ((docID = postingsEnum2->nextDoc()) != PostingsEnum::NO_MORE_DOCS) {
        docs[upto] = docID;
        if (hasFreq) {
          constexpr int freq = postingsEnum2->freq();
          freqs[upto] = freq;
          if (hasPos) {
            constexpr int mult;
            if (hasOffsets_) {
              mult = 3;
            } else {
              mult = 1;
            }
            if (hasPayloads_) {
              payloads[upto] = std::deque<std::deque<char>>(freq);
            }
            positions[upto] = std::deque<int>(mult * freq);
            int posUpto = 0;
            for (int pos = 0; pos < freq; pos++) {
              positions[upto][posUpto] = docsAndPositionsEnum->nextPosition();
              if (hasPayloads_) {
                shared_ptr<BytesRef> payload =
                    docsAndPositionsEnum->getPayload();
                if (payload != nullptr) {
                  std::deque<char> payloadBytes(payload->length);
                  System::arraycopy(payload->bytes, payload->offset,
                                    payloadBytes, 0, payload->length);
                  payloads[upto][pos] = payloadBytes;
                }
              }
              posUpto++;
              if (hasOffsets_) {
                positions[upto][posUpto++] =
                    docsAndPositionsEnum->startOffset();
                positions[upto][posUpto++] = docsAndPositionsEnum->endOffset();
              }
            }
          }
        }

        upto++;
      }
      assert(upto == docFreq);
      ent = make_shared<HighFreqTerm>(docs, freqs, positions, payloads,
                                      totalTermFreq);
    }

    terms[count] = ent;
    setSkips(count, termBytes);
    count++;
  }

  // End sentinel:
  termOffsets[count] = termOffset;

  finishSkips();

  // System.out.println(skipCount + " skips: " + field);

  this->termBytes = std::deque<char>(termOffset);
  System::arraycopy(termBytes, 0, this->termBytes, 0, termOffset);

  // Pack skips:

  int skipOffset = 0;
  for (int i = 0; i < numTerms; i++) {
    const std::deque<int> termSkips = terms[i]->skips;
    skipOffsets[i] = skipOffset;
    if (termSkips.size() > 0) {
      System::arraycopy(termSkips, 0, skips, skipOffset, termSkips.size());
      skipOffset += termSkips.size();
      terms[i]->skips->clear();
    }
  }
  this->skipOffsets[numTerms] = skipOffset;
  assert(skipOffset == skipCount);
}

int64_t DirectPostingsFormat::DirectField::ramBytesUsed()
{
  int64_t sizeInBytes = BASE_RAM_BYTES_USED;
  sizeInBytes +=
      ((termBytes.size() > 0) ? RamUsageEstimator::sizeOf(termBytes) : 0);
  sizeInBytes +=
      ((termOffsets.size() > 0) ? RamUsageEstimator::sizeOf(termOffsets) : 0);
  sizeInBytes += ((skips.size() > 0) ? RamUsageEstimator::sizeOf(skips) : 0);
  sizeInBytes +=
      ((skipOffsets.size() > 0) ? RamUsageEstimator::sizeOf(skipOffsets) : 0);
  sizeInBytes +=
      ((sameCounts.size() > 0) ? RamUsageEstimator::sizeOf(sameCounts) : 0);

  if (terms.size() > 0) {
    sizeInBytes += RamUsageEstimator::shallowSizeOf(terms);
    for (auto termAndSkip : terms) {
      sizeInBytes += (termAndSkip != nullptr) ? termAndSkip->ramBytesUsed() : 0;
    }
  }

  return sizeInBytes;
}

wstring DirectPostingsFormat::DirectField::toString()
{
  return L"DirectTerms(terms=" + terms.size() + L",postings=" +
         to_wstring(sumDocFreq) + L",positions=" +
         to_wstring(sumTotalTermFreq) + L",docs=" + to_wstring(docCount) + L")";
}

int DirectPostingsFormat::DirectField::compare(int ord,
                                               shared_ptr<BytesRef> other)
{
  const std::deque<char> otherBytes = other->bytes;

  int upto = termOffsets[ord];
  constexpr int termLen = termOffsets[1 + ord] - upto;
  int otherUpto = other->offset;

  constexpr int stop = upto + min(termLen, other->length);
  while (upto < stop) {
    int diff = (termBytes[upto++] & 0xFF) - (otherBytes[otherUpto++] & 0xFF);
    if (diff != 0) {
      return diff;
    }
  }

  // One is a prefix of the other, or, they are equal:
  return termLen - other->length;
}

void DirectPostingsFormat::DirectField::setSkips(int termOrd,
                                                 std::deque<char> &termBytes)
{

  constexpr int termLength = termOffsets[termOrd + 1] - termOffsets[termOrd];

  if (sameCounts.size() < termLength) {
    sameCounts = ArrayUtil::grow(sameCounts, termLength);
  }

  // Update skip pointers:
  if (termOrd > 0) {
    constexpr int lastTermLength =
        termOffsets[termOrd] - termOffsets[termOrd - 1];
    constexpr int limit = min(termLength, lastTermLength);

    int lastTermOffset = termOffsets[termOrd - 1];
    int termOffset = termOffsets[termOrd];

    int i = 0;
    for (; i < limit; i++) {
      if (termBytes[lastTermOffset++] == termBytes[termOffset++]) {
        sameCounts[i]++;
      } else {
        for (; i < limit; i++) {
          if (sameCounts[i] >= minSkipCount) {
            // Go back and add a skip pointer:
            saveSkip(termOrd, sameCounts[i]);
          }
          sameCounts[i] = 1;
        }
        break;
      }
    }

    for (; i < lastTermLength; i++) {
      if (sameCounts[i] >= minSkipCount) {
        // Go back and add a skip pointer:
        saveSkip(termOrd, sameCounts[i]);
      }
      sameCounts[i] = 0;
    }
    for (int j = limit; j < termLength; j++) {
      sameCounts[j] = 1;
    }
  } else {
    for (int i = 0; i < termLength; i++) {
      sameCounts[i]++;
    }
  }
}

void DirectPostingsFormat::DirectField::finishSkips()
{
  assert(count == terms.size());
  int lastTermOffset = termOffsets[count - 1];
  int lastTermLength = termOffsets[count] - lastTermOffset;

  for (int i = 0; i < lastTermLength; i++) {
    if (sameCounts[i] >= minSkipCount) {
      // Go back and add a skip pointer:
      saveSkip(count, sameCounts[i]);
    }
  }

  // Reverse the skip pointers so they are "nested":
  for (int termID = 0; termID < terms.size(); termID++) {
    shared_ptr<TermAndSkip> term = terms[termID];
    if (term->skips.size() > 0 && term->skips.size() > 1) {
      for (int pos = 0; pos < term->skips.size() / 2; pos++) {
        constexpr int otherPos = term->skips.size() - pos - 1;

        constexpr int temp = term->skips[pos];
        term->skips[pos] = term->skips[otherPos];
        term->skips[otherPos] = temp;
      }
    }
  }
}

void DirectPostingsFormat::DirectField::saveSkip(int ord, int backCount)
{
  shared_ptr<TermAndSkip> *const term = terms[ord - backCount];
  skipCount++;
  if (term->skips.empty()) {
    term->skips = std::deque<int>{ord};
  } else {
    // Normally we'd grow at a slight exponential... but
    // given that the skips themselves are already log(N)
    // we can grow by only 1 and still have amortized
    // linear time:
    const std::deque<int> newSkips = std::deque<int>(term->skips.size() + 1);
    System::arraycopy(term->skips, 0, newSkips, 0, term->skips.size());
    term->skips = newSkips;
    term->skips[term->skips.size() - 1] = ord;
  }
}

shared_ptr<TermsEnum> DirectPostingsFormat::DirectField::iterator()
{
  return make_shared<DirectTermsEnum>(shared_from_this());
}

shared_ptr<TermsEnum> DirectPostingsFormat::DirectField::intersect(
    shared_ptr<CompiledAutomaton> compiled, shared_ptr<BytesRef> startTerm)
{
  if (compiled->type != CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
    throw invalid_argument(
        L"please use CompiledAutomaton.getTermsEnum instead");
  }
  return make_shared<DirectIntersectTermsEnum>(shared_from_this(), compiled,
                                               startTerm);
}

int64_t DirectPostingsFormat::DirectField::size() { return terms.size(); }

int64_t DirectPostingsFormat::DirectField::getSumTotalTermFreq()
{
  return sumTotalTermFreq;
}

int64_t DirectPostingsFormat::DirectField::getSumDocFreq()
{
  return sumDocFreq;
}

int DirectPostingsFormat::DirectField::getDocCount() { return docCount; }

bool DirectPostingsFormat::DirectField::hasFreqs() { return hasFreq; }

bool DirectPostingsFormat::DirectField::hasOffsets() { return hasOffsets_; }

bool DirectPostingsFormat::DirectField::hasPositions() { return hasPos; }

bool DirectPostingsFormat::DirectField::hasPayloads() { return hasPayloads_; }

DirectPostingsFormat::DirectField::DirectTermsEnum::DirectTermsEnum(
    shared_ptr<DirectPostingsFormat::DirectField> outerInstance)
    : outerInstance(outerInstance)
{
  termOrd = -1;
}

shared_ptr<BytesRef>
DirectPostingsFormat::DirectField::DirectTermsEnum::setTerm()
{
  scratch->bytes = outerInstance->termBytes;
  scratch->offset = outerInstance->termOffsets[termOrd];
  scratch->length = outerInstance->termOffsets[termOrd + 1] -
                    outerInstance->termOffsets[termOrd];
  return scratch;
}

shared_ptr<BytesRef> DirectPostingsFormat::DirectField::DirectTermsEnum::next()
{
  termOrd++;
  if (termOrd < outerInstance->terms.size()) {
    return setTerm();
  } else {
    return nullptr;
  }
}

shared_ptr<TermState>
DirectPostingsFormat::DirectField::DirectTermsEnum::termState()
{
  shared_ptr<OrdTermState> state = make_shared<OrdTermState>();
  state->ord = termOrd;
  return state;
}

int DirectPostingsFormat::DirectField::DirectTermsEnum::findTerm(
    shared_ptr<BytesRef> term)
{

  // Just do binary search: should be (constant factor)
  // faster than using the skip deque:
  int low = 0;
  int high = outerInstance->terms.size() - 1;

  while (low <= high) {
    int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
    int cmp = outerInstance->compare(mid, term);
    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      return mid; // key found
    }
  }

  return -(low + 1); // key not found.
}

SeekStatus DirectPostingsFormat::DirectField::DirectTermsEnum::seekCeil(
    shared_ptr<BytesRef> term)
{
  // TODO: we should use the skip pointers; should be
  // faster than bin search; we should also hold
  // & reuse current state so seeking forwards is
  // faster
  constexpr int ord = findTerm(term);
  // if (DEBUG) {
  //   System.out.println("  find term=" + term.utf8ToString() + " ord=" + ord);
  // }
  if (ord >= 0) {
    termOrd = ord;
    setTerm();
    return SeekStatus::FOUND;
  } else if (ord == -outerInstance->terms.size() - 1) {
    return SeekStatus::END;
  } else {
    termOrd = -ord - 1;
    setTerm();
    return SeekStatus::NOT_FOUND;
  }
}

bool DirectPostingsFormat::DirectField::DirectTermsEnum::seekExact(
    shared_ptr<BytesRef> term)
{
  // TODO: we should use the skip pointers; should be
  // faster than bin search; we should also hold
  // & reuse current state so seeking forwards is
  // faster
  constexpr int ord = findTerm(term);
  if (ord >= 0) {
    termOrd = ord;
    setTerm();
    return true;
  } else {
    return false;
  }
}

void DirectPostingsFormat::DirectField::DirectTermsEnum::seekExact(
    int64_t ord)
{
  termOrd = static_cast<int>(ord);
  setTerm();
}

void DirectPostingsFormat::DirectField::DirectTermsEnum::seekExact(
    shared_ptr<BytesRef> term, shared_ptr<TermState> state) 
{
  termOrd =
      static_cast<int>((std::static_pointer_cast<OrdTermState>(state))->ord);
  setTerm();
  assert(term->equals(scratch));
}

shared_ptr<BytesRef> DirectPostingsFormat::DirectField::DirectTermsEnum::term()
{
  return scratch;
}

int64_t DirectPostingsFormat::DirectField::DirectTermsEnum::ord()
{
  return termOrd;
}

int DirectPostingsFormat::DirectField::DirectTermsEnum::docFreq()
{
  if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
      nullptr) {
    return (std::static_pointer_cast<LowFreqTerm>(
                outerInstance->terms[termOrd]))
        ->docFreq;
  } else {
    return (std::static_pointer_cast<HighFreqTerm>(
                outerInstance->terms[termOrd]))
        ->docIDs.size();
  }
}

int64_t DirectPostingsFormat::DirectField::DirectTermsEnum::totalTermFreq()
{
  if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
      nullptr) {
    return (std::static_pointer_cast<LowFreqTerm>(
                outerInstance->terms[termOrd]))
        ->totalTermFreq;
  } else {
    return (std::static_pointer_cast<HighFreqTerm>(
                outerInstance->terms[termOrd]))
        ->totalTermFreq;
  }
}

shared_ptr<PostingsEnum>
DirectPostingsFormat::DirectField::DirectTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  // TODO: implement reuse
  // it's hairy!

  // TODO: the logic of which enum impl to choose should be refactored to be
  // simpler...
  if (PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {

    if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
        nullptr) {
      shared_ptr<LowFreqTerm> *const term =
          (std::static_pointer_cast<LowFreqTerm>(
              outerInstance->terms[termOrd]));
      const std::deque<int> postings = term->postings;
      if (outerInstance->hasFreq == false) {
        shared_ptr<LowFreqDocsEnumNoTF> docsEnum;
        if (std::dynamic_pointer_cast<LowFreqDocsEnumNoTF>(reuse) != nullptr) {
          docsEnum = std::static_pointer_cast<LowFreqDocsEnumNoTF>(reuse);
        } else {
          docsEnum = make_shared<LowFreqDocsEnumNoTF>();
        }

        return docsEnum->reset(postings);

      } else if (outerInstance->hasPos == false) {
        shared_ptr<LowFreqDocsEnumNoPos> docsEnum;
        if (std::dynamic_pointer_cast<LowFreqDocsEnumNoPos>(reuse) != nullptr) {
          docsEnum = std::static_pointer_cast<LowFreqDocsEnumNoPos>(reuse);
        } else {
          docsEnum = make_shared<LowFreqDocsEnumNoPos>();
        }

        return docsEnum->reset(postings);
      }
      const std::deque<char> payloads = term->payloads;
      return (make_shared<LowFreqPostingsEnum>(outerInstance->hasOffsets_,
                                               outerInstance->hasPayloads_))
          ->reset(postings, payloads);
    } else {
      shared_ptr<HighFreqTerm> *const term =
          std::static_pointer_cast<HighFreqTerm>(outerInstance->terms[termOrd]);
      if (outerInstance->hasPos == false) {
        return (make_shared<HighFreqDocsEnum>())
            ->reset(term->docIDs, term->freqs);
      } else {
        return (make_shared<HighFreqPostingsEnum>(outerInstance->hasOffsets_))
            ->reset(term->docIDs, term->freqs, term->positions, term->payloads);
      }
    }
  }

  if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
      nullptr) {
    const std::deque<int> postings =
        (std::static_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]))
            ->postings;
    if (outerInstance->hasFreq) {
      if (outerInstance->hasPos) {
        int posLen;
        if (outerInstance->hasOffsets_) {
          posLen = 3;
        } else {
          posLen = 1;
        }
        if (outerInstance->hasPayloads_) {
          posLen++;
        }
        shared_ptr<LowFreqDocsEnum> docsEnum;
        if (std::dynamic_pointer_cast<LowFreqDocsEnum>(reuse) != nullptr) {
          docsEnum = std::static_pointer_cast<LowFreqDocsEnum>(reuse);
          if (!docsEnum->canReuse(posLen)) {
            docsEnum = make_shared<LowFreqDocsEnum>(posLen);
          }
        } else {
          docsEnum = make_shared<LowFreqDocsEnum>(posLen);
        }

        return docsEnum->reset(postings);
      } else {
        shared_ptr<LowFreqDocsEnumNoPos> docsEnum;
        if (std::dynamic_pointer_cast<LowFreqDocsEnumNoPos>(reuse) != nullptr) {
          docsEnum = std::static_pointer_cast<LowFreqDocsEnumNoPos>(reuse);
        } else {
          docsEnum = make_shared<LowFreqDocsEnumNoPos>();
        }

        return docsEnum->reset(postings);
      }
    } else {
      shared_ptr<LowFreqDocsEnumNoTF> docsEnum;
      if (std::dynamic_pointer_cast<LowFreqDocsEnumNoTF>(reuse) != nullptr) {
        docsEnum = std::static_pointer_cast<LowFreqDocsEnumNoTF>(reuse);
      } else {
        docsEnum = make_shared<LowFreqDocsEnumNoTF>();
      }

      return docsEnum->reset(postings);
    }
  } else {
    shared_ptr<HighFreqTerm> *const term =
        std::static_pointer_cast<HighFreqTerm>(outerInstance->terms[termOrd]);

    shared_ptr<HighFreqDocsEnum> docsEnum;
    if (std::dynamic_pointer_cast<HighFreqDocsEnum>(reuse) != nullptr) {
      docsEnum = std::static_pointer_cast<HighFreqDocsEnum>(reuse);
    } else {
      docsEnum = make_shared<HighFreqDocsEnum>();
    }

    // System.out.println("  DE for term=" + new
    // BytesRef(terms[termOrd].term).utf8ToString() + ": " + term.docIDs.length +
    // " docs");
    return docsEnum->reset(term->docIDs, term->freqs);
  }
}

DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::State::State(
    shared_ptr<DirectPostingsFormat::DirectField::DirectIntersectTermsEnum>
        outerInstance)
    : outerInstance(outerInstance)
{
}

DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::
    DirectIntersectTermsEnum(
        shared_ptr<DirectPostingsFormat::DirectField> outerInstance,
        shared_ptr<CompiledAutomaton> compiled, shared_ptr<BytesRef> startTerm)
    : runAutomaton(compiled->runAutomaton), compiledAutomaton(compiled),
      outerInstance(outerInstance)
{
  termOrd = -1;
  states = std::deque<std::shared_ptr<State>>(1);
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  states[0] = make_shared<State>(shared_from_this());
  states[0]->changeOrd = outerInstance->terms.size();
  states[0]->state = 0;
  states[0]->transitionCount =
      compiledAutomaton->automaton->getNumTransitions(states[0]->state);
  compiledAutomaton->automaton->initTransition(states[0]->state,
                                               states[0]->transition);
  states[0]->transitionUpto = -1;
  states[0]->transitionMax = -1;

  // System.out.println("IE.init startTerm=" + startTerm);

  if (startTerm != nullptr) {
    int skipUpto = 0;
    if (startTerm->length == 0) {
      if (outerInstance->terms.size() > 0 &&
          outerInstance->termOffsets[1] == 0) {
        termOrd = 0;
      }
    } else {
      termOrd++;

      for (int i = 0; i < startTerm->length; i++) {
        constexpr int label = startTerm->bytes[startTerm->offset + i] & 0xFF;

        while (label > states[i]->transitionMax) {
          states[i]->transitionUpto++;
          assert(states[i]->transitionUpto < states[i]->transitionCount);
          compiledAutomaton->automaton->getNextTransition(
              states[i]->transition);
          states[i]->transitionMin = states[i]->transition.min;
          states[i]->transitionMax = states[i]->transition.max;
          assert(states[i]->transitionMin >= 0);
          assert(states[i]->transitionMin <= 255);
          assert(states[i]->transitionMax >= 0);
          assert(states[i]->transitionMax <= 255);
        }

        // Skip forwards until we find a term matching
        // the label at this position:
        while (termOrd < outerInstance->terms.size()) {
          constexpr int skipOffset = outerInstance->skipOffsets[termOrd];
          constexpr int numSkips =
              outerInstance->skipOffsets[termOrd + 1] - skipOffset;
          constexpr int termOffset = outerInstance->termOffsets[termOrd];
          constexpr int termLength =
              outerInstance->termOffsets[1 + termOrd] - termOffset;

          // if (DEBUG) {
          //   System.out.println("  check termOrd=" + termOrd + " term=" + new
          //   BytesRef(termBytes, termOffset, termLength).utf8ToString() + "
          //   skips=" + Arrays.toString(skips) + " i=" + i);
          // }

          if (termOrd == states[stateUpto]->changeOrd) {
            // if (DEBUG) {
            //   System.out.println("  end push return");
            // }
            stateUpto--;
            termOrd--;
            return;
          }

          if (termLength == i) {
            termOrd++;
            skipUpto = 0;
            // if (DEBUG) {
            //   System.out.println("    term too short; next term");
            // }
          } else if (label <
                     (outerInstance->termBytes[termOffset + i] & 0xFF)) {
            termOrd--;
            // if (DEBUG) {
            //   System.out.println("  no match; already beyond; return
            //   termOrd=" + termOrd);
            // }
            stateUpto -= skipUpto;
            assert(stateUpto >= 0);
            return;
          } else if (label ==
                     (outerInstance->termBytes[termOffset + i] & 0xFF)) {
            // if (DEBUG) {
            //   System.out.println("    label[" + i + "] matches");
            // }
            if (skipUpto < numSkips) {
              grow();

              constexpr int nextState =
                  runAutomaton->step(states[stateUpto]->state, label);

              // Automaton is required to accept startTerm:
              assert(nextState != -1);

              stateUpto++;
              states[stateUpto]->changeOrd =
                  outerInstance->skips[skipOffset + skipUpto++];
              states[stateUpto]->state = nextState;
              states[stateUpto]->transitionCount =
                  compiledAutomaton->automaton->getNumTransitions(nextState);
              compiledAutomaton->automaton->initTransition(
                  states[stateUpto]->state, states[stateUpto]->transition);
              states[stateUpto]->transitionUpto = -1;
              states[stateUpto]->transitionMax = -1;
              // System.out.println("  push " +
              // states[stateUpto].transitions.length + " trans");

              // if (DEBUG) {
              //   System.out.println("    push skip; changeOrd=" +
              //   states[stateUpto].changeOrd);
              // }

              // Match next label at this same term:
              goto nextLabelContinue;
            } else {
              // if (DEBUG) {
              //   System.out.println("    linear scan");
              // }
              // Index exhausted: just scan now (the
              // number of scans required will be less
              // than the minSkipCount):
              constexpr int startTermOrd = termOrd;
              while (termOrd < outerInstance->terms.size() &&
                     outerInstance->compare(termOrd, startTerm) <= 0) {
                assert(termOrd == startTermOrd ||
                       outerInstance->skipOffsets[termOrd] ==
                           outerInstance->skipOffsets[termOrd + 1]);
                termOrd++;
              }
              assert(termOrd - startTermOrd < outerInstance->minSkipCount);
              termOrd--;
              stateUpto -= skipUpto;
              // if (DEBUG) {
              //   System.out.println("  end termOrd=" + termOrd);
              // }
              return;
            }
          } else {
            if (skipUpto < numSkips) {
              termOrd = outerInstance->skips[skipOffset + skipUpto];
              // if (DEBUG) {
              //   System.out.println("  no match; skip to termOrd=" + termOrd);
              // }
            } else {
              // if (DEBUG) {
              //   System.out.println("  no match; next term");
              // }
              termOrd++;
            }
            skipUpto = 0;
          }
        }

        // startTerm is >= last term so enum will not
        // return any terms:
        termOrd--;
        // if (DEBUG) {
        //   System.out.println("  beyond end; no terms will match");
        // }
        return;
      nextLabelContinue:;
      }
    nextLabelBreak:;
    }

    constexpr int termOffset = outerInstance->termOffsets[termOrd];
    constexpr int termLen =
        outerInstance->termOffsets[1 + termOrd] - termOffset;

    if (termOrd >= 0 && !startTerm->equals(make_shared<BytesRef>(
                            outerInstance->termBytes, termOffset, termLen))) {
      stateUpto -= skipUpto;
      termOrd--;
    }
    // if (DEBUG) {
    //   System.out.println("  loop end; return termOrd=" + termOrd + "
    //   stateUpto=" + stateUpto);
    // }
  }
}

void DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::grow()
{
  if (states.size() == 1 + stateUpto) {
    std::deque<std::shared_ptr<State>> newStates(states.size() + 1);
    System::arraycopy(states, 0, newStates, 0, states.size());
    newStates[states.size()] = make_shared<State>(shared_from_this());
    states = newStates;
  }
}

shared_ptr<BytesRef>
DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::next()
{
  // if (DEBUG) {
  //   System.out.println("\nIE.next");
  // }

  termOrd++;
  int skipUpto = 0;

  if (termOrd == 0 && outerInstance->termOffsets[1] == 0) {
    // Special-case empty string:
    assert(stateUpto == 0);
    // if (DEBUG) {
    //   System.out.println("  visit empty string");
    // }
    if (runAutomaton->isAccept(states[0]->state)) {
      scratch->bytes = outerInstance->termBytes;
      scratch->offset = 0;
      scratch->length = 0;
      return scratch;
    }
    termOrd++;
  }

  while (true) {
    // if (DEBUG) {
    //   System.out.println("  cycle termOrd=" + termOrd + " stateUpto=" +
    //   stateUpto + " skipUpto=" + skipUpto);
    // }
    if (termOrd == outerInstance->terms.size()) {
      // if (DEBUG) {
      //   System.out.println("  return END");
      // }
      return nullptr;
    }

    shared_ptr<State> *const state = states[stateUpto];
    if (termOrd == state->changeOrd) {
      // Pop:
      // if (DEBUG) {
      //   System.out.println("  pop stateUpto=" + stateUpto);
      // }
      stateUpto--;
      /*
      if (DEBUG) {
        try {
          //System.out.println("    prefix pop " + new
      BytesRef(terms[termOrd].term, 0, Math.min(stateUpto,
      terms[termOrd].term.length)).utf8ToString()); System.out.println(" prefix
      pop " + new BytesRef(terms[termOrd].term, 0, Math.min(stateUpto,
      terms[termOrd].term.length))); } catch (ArrayIndexOutOfBoundsException
      aioobe) { System.out.println("    prefix pop " + new
      BytesRef(terms[termOrd].term, 0, Math.min(stateUpto,
      terms[termOrd].term.length)));
        }
      }
      */

      continue;
    }

    constexpr int termOffset = outerInstance->termOffsets[termOrd];
    constexpr int termLength =
        outerInstance->termOffsets[termOrd + 1] - termOffset;
    constexpr int skipOffset = outerInstance->skipOffsets[termOrd];
    constexpr int numSkips =
        outerInstance->skipOffsets[termOrd + 1] - skipOffset;

    // if (DEBUG) {
    //   System.out.println("  term=" + new BytesRef(termBytes, termOffset,
    //   termLength).utf8ToString() + " skips=" + Arrays.toString(skips));
    // }

    assert(termOrd < state->changeOrd);

    assert((stateUpto <= termLength, L"term.length=" + to_wstring(termLength) +
                                         L"; stateUpto=" +
                                         to_wstring(stateUpto)));
    constexpr int label =
        outerInstance->termBytes[termOffset + stateUpto] & 0xFF;

    while (label > state->transitionMax) {
      // System.out.println("  label=" + label + " vs max=" +
      // state.transitionMax + " transUpto=" + state.transitionUpto + " vs " +
      // state.transitions.length);
      state->transitionUpto++;
      if (state->transitionUpto == state->transitionCount) {
        // We've exhausted transitions leaving this
        // state; force pop+next/skip now:
        // System.out.println("forcepop: stateUpto=" + stateUpto);
        if (stateUpto == 0) {
          termOrd = outerInstance->terms.size();
          return nullptr;
        } else {
          assert(state->changeOrd > termOrd);
          // if (DEBUG) {
          //   System.out.println("  jumpend " + (state.changeOrd - termOrd));
          // }
          // System.out.println("  jump to termOrd=" +
          // states[stateUpto].changeOrd + " vs " + termOrd);
          termOrd = states[stateUpto]->changeOrd;
          skipUpto = 0;
          stateUpto--;
        }
        goto nextTermContinue;
      }
      compiledAutomaton->automaton->getNextTransition(state->transition);
      assert((state->transitionUpto < state->transitionCount,
              L" state.transitionUpto=" + to_wstring(state->transitionUpto) +
                  L" vs " + to_wstring(state->transitionCount)));
      state->transitionMin = state->transition->min;
      state->transitionMax = state->transition->max;
      assert(state->transitionMin >= 0);
      assert(state->transitionMin <= 255);
      assert(state->transitionMax >= 0);
      assert(state->transitionMax <= 255);
    }

    /*
    if (DEBUG) {
      System.out.println("    check ord=" + termOrd + " term[" + stateUpto +
    "]=" + (char) label + "(" + label + ") term=" + new
    BytesRef(terms[termOrd].term).utf8ToString() + " trans " + (char)
    state.transitionMin + "(" + state.transitionMin + ")" + "-" + (char)
    state.transitionMax + "(" + state.transitionMax + ") nextChange=+" +
    (state.changeOrd - termOrd) + " skips=" + (skips == null ? "null" :
    Arrays.toString(skips))); System.out.println("    check ord=" + termOrd + "
    term[" + stateUpto + "]=" + Integer.toHexString(label) + "(" + label + ")
    term=" + new BytesRef(termBytes, termOffset, termLength) + " trans " +
                         Integer.toHexString(state.transitionMin) + "(" +
    state.transitionMin + ")" + "-" + Integer.toHexString(state.transitionMax) +
    "(" + state.transitionMax + ") nextChange=+" + (state.changeOrd - termOrd) +
    " skips=" + (skips == null ? "null" : Arrays.toString(skips)));
    }
    */

    constexpr int targetLabel = state->transitionMin;

    if ((outerInstance->termBytes[termOffset + stateUpto] & 0xFF) <
        targetLabel) {
      // if (DEBUG) {
      //   System.out.println("    do bin search");
      // }
      // int startTermOrd = termOrd;
      int low = termOrd + 1;
      int high = state->changeOrd - 1;
      while (true) {
        if (low > high) {
          // Label not found
          termOrd = low;
          // if (DEBUG) {
          //   System.out.println("      advanced by " + (termOrd -
          //   startTermOrd));
          // }
          // System.out.println("  jump " + (termOrd - startTermOrd));
          skipUpto = 0;
          goto nextTermContinue;
        }
        int mid =
            static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
        int cmp =
            (outerInstance
                 ->termBytes[outerInstance->termOffsets[mid] + stateUpto] &
             0xFF) -
            targetLabel;
        // if (DEBUG) {
        //   System.out.println("      bin: check label=" + (char)
        //   (termBytes[termOffsets[low] + stateUpto] & 0xFF) + " ord=" + mid);
        // }
        if (cmp < 0) {
          low = mid + 1;
        } else if (cmp > 0) {
          high = mid - 1;
        } else {
          // Label found; walk backwards to first
          // occurrence:
          while (mid > termOrd &&
                 (outerInstance->termBytes[outerInstance->termOffsets[mid - 1] +
                                           stateUpto] &
                  0xFF) == targetLabel) {
            mid--;
          }
          termOrd = mid;
          // if (DEBUG) {
          //   System.out.println("      advanced by " + (termOrd -
          //   startTermOrd));
          // }
          // System.out.println("  jump " + (termOrd - startTermOrd));
          skipUpto = 0;
          goto nextTermContinue;
        }
      }
    }

    int nextState = runAutomaton->step(states[stateUpto]->state, label);

    if (nextState == -1) {
      // Skip
      // if (DEBUG) {
      //   System.out.println("  automaton doesn't accept; skip");
      // }
      if (skipUpto < numSkips) {
        // if (DEBUG) {
        //   System.out.println("  jump " + (skips[skipOffset+skipUpto]-1 -
        //   termOrd));
        // }
        termOrd = outerInstance->skips[skipOffset + skipUpto];
      } else {
        termOrd++;
      }
      skipUpto = 0;
    } else if (skipUpto < numSkips) {
      // Push:
      // if (DEBUG) {
      //   System.out.println("  push");
      // }
      /*
      if (DEBUG) {
        try {
          //System.out.println("    prefix push " + new BytesRef(term, 0,
      stateUpto+1).utf8ToString()); System.out.println("    prefix push " + new
      BytesRef(term, 0, stateUpto+1)); } catch (ArrayIndexOutOfBoundsException
      aioobe) { System.out.println("    prefix push " + new BytesRef(term, 0,
      stateUpto+1));
        }
      }
      */

      grow();
      stateUpto++;
      states[stateUpto]->state = nextState;
      states[stateUpto]->changeOrd =
          outerInstance->skips[skipOffset + skipUpto++];
      states[stateUpto]->transitionCount =
          compiledAutomaton->automaton->getNumTransitions(nextState);
      compiledAutomaton->automaton->initTransition(
          nextState, states[stateUpto]->transition);
      states[stateUpto]->transitionUpto = -1;
      states[stateUpto]->transitionMax = -1;

      if (stateUpto == termLength) {
        // if (DEBUG) {
        //   System.out.println("  term ends after push");
        // }
        if (runAutomaton->isAccept(nextState)) {
          // if (DEBUG) {
          //   System.out.println("  automaton accepts: return");
          // }
          scratch->bytes = outerInstance->termBytes;
          scratch->offset = outerInstance->termOffsets[termOrd];
          scratch->length =
              outerInstance->termOffsets[1 + termOrd] - scratch->offset;
          // if (DEBUG) {
          //   System.out.println("  ret " + scratch.utf8ToString());
          // }
          return scratch;
        } else {
          // if (DEBUG) {
          //   System.out.println("  automaton rejects: nextTerm");
          // }
          termOrd++;
          skipUpto = 0;
        }
      }
    } else {
      // Run the non-indexed tail of this term:

      // TODO: add assert that we don't inc too many times

      if (compiledAutomaton->commonSuffixRef != nullptr) {
        // System.out.println("suffix " +
        // compiledAutomaton.commonSuffixRef.utf8ToString());
        assert(compiledAutomaton->commonSuffixRef->offset == 0);
        if (termLength < compiledAutomaton->commonSuffixRef->length) {
          termOrd++;
          skipUpto = 0;
          goto nextTermContinue;
        }
        int offset = termOffset + termLength -
                     compiledAutomaton->commonSuffixRef->length;
        for (int suffix = 0;
             suffix < compiledAutomaton->commonSuffixRef->length; suffix++) {
          if (outerInstance->termBytes[offset + suffix] !=
              compiledAutomaton->commonSuffixRef->bytes[suffix]) {
            termOrd++;
            skipUpto = 0;
            goto nextTermContinue;
          }
        }
      }

      int upto = stateUpto + 1;
      while (upto < termLength) {
        nextState = runAutomaton->step(
            nextState, outerInstance->termBytes[termOffset + upto] & 0xFF);
        if (nextState == -1) {
          termOrd++;
          skipUpto = 0;
          // if (DEBUG) {
          //   System.out.println("  nomatch tail; next term");
          // }
          goto nextTermContinue;
        }
        upto++;
      }

      if (runAutomaton->isAccept(nextState)) {
        scratch->bytes = outerInstance->termBytes;
        scratch->offset = outerInstance->termOffsets[termOrd];
        scratch->length =
            outerInstance->termOffsets[1 + termOrd] - scratch->offset;
        // if (DEBUG) {
        //   System.out.println("  match tail; return " +
        //   scratch.utf8ToString()); System.out.println("  ret2 " +
        //   scratch.utf8ToString());
        // }
        return scratch;
      } else {
        termOrd++;
        skipUpto = 0;
        // if (DEBUG) {
        //   System.out.println("  nomatch tail; next term");
        // }
      }
    }
  nextTermContinue:;
  }
nextTermBreak:;
}

shared_ptr<TermState>
DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::termState()
{
  shared_ptr<OrdTermState> state = make_shared<OrdTermState>();
  state->ord = termOrd;
  return state;
}

shared_ptr<BytesRef>
DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::term()
{
  return scratch;
}

int64_t DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::ord()
{
  return termOrd;
}

int DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::docFreq()
{
  if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
      nullptr) {
    return (std::static_pointer_cast<LowFreqTerm>(
                outerInstance->terms[termOrd]))
        ->docFreq;
  } else {
    return (std::static_pointer_cast<HighFreqTerm>(
                outerInstance->terms[termOrd]))
        ->docIDs.size();
  }
}

int64_t
DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::totalTermFreq()
{
  if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
      nullptr) {
    return (std::static_pointer_cast<LowFreqTerm>(
                outerInstance->terms[termOrd]))
        ->totalTermFreq;
  } else {
    return (std::static_pointer_cast<HighFreqTerm>(
                outerInstance->terms[termOrd]))
        ->totalTermFreq;
  }
}

shared_ptr<PostingsEnum>
DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags)
{
  // TODO: implement reuse
  // it's hairy!

  // TODO: the logic of which enum impl to choose should be refactored to be
  // simpler...
  if (outerInstance->hasPos &&
      PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {
    if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
        nullptr) {
      shared_ptr<LowFreqTerm> *const term =
          (std::static_pointer_cast<LowFreqTerm>(
              outerInstance->terms[termOrd]));
      const std::deque<int> postings = term->postings;
      const std::deque<char> payloads = term->payloads;
      return (make_shared<LowFreqPostingsEnum>(outerInstance->hasOffsets_,
                                               outerInstance->hasPayloads_))
          ->reset(postings, payloads);
    } else {
      shared_ptr<HighFreqTerm> *const term =
          std::static_pointer_cast<HighFreqTerm>(outerInstance->terms[termOrd]);
      return (make_shared<HighFreqPostingsEnum>(outerInstance->hasOffsets_))
          ->reset(term->docIDs, term->freqs, term->positions, term->payloads);
    }
  }

  if (std::dynamic_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]) !=
      nullptr) {
    const std::deque<int> postings =
        (std::static_pointer_cast<LowFreqTerm>(outerInstance->terms[termOrd]))
            ->postings;
    if (outerInstance->hasFreq) {
      if (outerInstance->hasPos) {
        int posLen;
        if (outerInstance->hasOffsets_) {
          posLen = 3;
        } else {
          posLen = 1;
        }
        if (outerInstance->hasPayloads_) {
          posLen++;
        }
        return (make_shared<LowFreqDocsEnum>(posLen))->reset(postings);
      } else {
        return (make_shared<LowFreqDocsEnumNoPos>())->reset(postings);
      }
    } else {
      return (make_shared<LowFreqDocsEnumNoTF>())->reset(postings);
    }
  } else {
    shared_ptr<HighFreqTerm> *const term =
        std::static_pointer_cast<HighFreqTerm>(outerInstance->terms[termOrd]);
    //  System.out.println("DE for term=" + new
    //  BytesRef(terms[termOrd].term).utf8ToString() + ": " + term.docIDs.length
    //  + " docs");
    return (make_shared<HighFreqDocsEnum>())->reset(term->docIDs, term->freqs);
  }
}

SeekStatus
DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::seekCeil(
    shared_ptr<BytesRef> term)
{
  throw make_shared<UnsupportedOperationException>();
}

void DirectPostingsFormat::DirectField::DirectIntersectTermsEnum::seekExact(
    int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PostingsEnum>
DirectPostingsFormat::LowFreqDocsEnumNoTF::reset(std::deque<int> &postings)
{
  this->postings = postings;
  upto = -1;
  return shared_from_this();
}

int DirectPostingsFormat::LowFreqDocsEnumNoTF::nextDoc()
{
  upto++;
  if (upto < postings.size()) {
    return postings[upto];
  }
  return NO_MORE_DOCS;
}

int DirectPostingsFormat::LowFreqDocsEnumNoTF::docID()
{
  if (upto < 0) {
    return -1;
  } else if (upto < postings.size()) {
    return postings[upto];
  } else {
    return NO_MORE_DOCS;
  }
}

int DirectPostingsFormat::LowFreqDocsEnumNoTF::freq() { return 1; }

int DirectPostingsFormat::LowFreqDocsEnumNoTF::nextPosition() 
{
  return -1;
}

int DirectPostingsFormat::LowFreqDocsEnumNoTF::startOffset() 
{
  return -1;
}

int DirectPostingsFormat::LowFreqDocsEnumNoTF::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
DirectPostingsFormat::LowFreqDocsEnumNoTF::getPayload() 
{
  return nullptr;
}

int DirectPostingsFormat::LowFreqDocsEnumNoTF::advance(int target) throw(
    IOException)
{
  // Linear scan, but this is low-freq term so it won't
  // be costly:
  return slowAdvance(target);
}

int64_t DirectPostingsFormat::LowFreqDocsEnumNoTF::cost()
{
  return postings.size();
}

DirectPostingsFormat::LowFreqDocsEnumNoPos::LowFreqDocsEnumNoPos() {}

shared_ptr<PostingsEnum>
DirectPostingsFormat::LowFreqDocsEnumNoPos::reset(std::deque<int> &postings)
{
  this->postings = postings;
  upto = -2;
  return shared_from_this();
}

int DirectPostingsFormat::LowFreqDocsEnumNoPos::nextDoc()
{
  upto += 2;
  if (upto < postings.size()) {
    return postings[upto];
  }
  return NO_MORE_DOCS;
}

int DirectPostingsFormat::LowFreqDocsEnumNoPos::docID()
{
  if (upto < 0) {
    return -1;
  } else if (upto < postings.size()) {
    return postings[upto];
  } else {
    return NO_MORE_DOCS;
  }
}

int DirectPostingsFormat::LowFreqDocsEnumNoPos::freq()
{
  return postings[upto + 1];
}

int DirectPostingsFormat::LowFreqDocsEnumNoPos::nextPosition() throw(
    IOException)
{
  return -1;
}

int DirectPostingsFormat::LowFreqDocsEnumNoPos::startOffset() 
{
  return -1;
}

int DirectPostingsFormat::LowFreqDocsEnumNoPos::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
DirectPostingsFormat::LowFreqDocsEnumNoPos::getPayload() 
{
  return nullptr;
}

int DirectPostingsFormat::LowFreqDocsEnumNoPos::advance(int target) throw(
    IOException)
{
  // Linear scan, but this is low-freq term so it won't
  // be costly:
  return slowAdvance(target);
}

int64_t DirectPostingsFormat::LowFreqDocsEnumNoPos::cost()
{
  return postings.size() / 2;
}

DirectPostingsFormat::LowFreqDocsEnum::LowFreqDocsEnum(int posMult)
    : posMult(posMult)
{
  // if (DEBUG) {
  //   System.out.println("LowFreqDE: posMult=" + posMult);
  // }
}

bool DirectPostingsFormat::LowFreqDocsEnum::canReuse(int posMult)
{
  return this->posMult == posMult;
}

shared_ptr<PostingsEnum>
DirectPostingsFormat::LowFreqDocsEnum::reset(std::deque<int> &postings)
{
  this->postings = postings;
  upto = -2;
  freq_ = 0;
  return shared_from_this();
}

int DirectPostingsFormat::LowFreqDocsEnum::nextDoc()
{
  upto += 2 + freq_ * posMult;
  // if (DEBUG) {
  //   System.out.println("  nextDoc freq=" + freq + " upto=" + upto + " vs " +
  //   postings.length);
  // }
  if (upto < postings.size()) {
    freq_ = postings[upto + 1];
    assert(freq_ > 0);
    return postings[upto];
  }
  return NO_MORE_DOCS;
}

int DirectPostingsFormat::LowFreqDocsEnum::docID()
{
  // TODO: store docID member?
  if (upto < 0) {
    return -1;
  } else if (upto < postings.size()) {
    return postings[upto];
  } else {
    return NO_MORE_DOCS;
  }
}

int DirectPostingsFormat::LowFreqDocsEnum::freq()
{
  // TODO: can I do postings[upto+1]?
  return freq_;
}

int DirectPostingsFormat::LowFreqDocsEnum::nextPosition() 
{
  return -1;
}

int DirectPostingsFormat::LowFreqDocsEnum::startOffset() 
{
  return -1;
}

int DirectPostingsFormat::LowFreqDocsEnum::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
DirectPostingsFormat::LowFreqDocsEnum::getPayload() 
{
  return nullptr;
}

int DirectPostingsFormat::LowFreqDocsEnum::advance(int target) throw(
    IOException)
{
  // Linear scan, but this is low-freq term so it won't
  // be costly:
  return slowAdvance(target);
}

int64_t DirectPostingsFormat::LowFreqDocsEnum::cost()
{
  // TODO: could do a better estimate
  return postings.size() / 2;
}

DirectPostingsFormat::LowFreqPostingsEnum::LowFreqPostingsEnum(bool hasOffsets,
                                                               bool hasPayloads)
    : hasOffsets(hasOffsets), hasPayloads(hasPayloads)
{
  if (hasOffsets) {
    if (hasPayloads) {
      posMult = 4;
    } else {
      posMult = 3;
    }
  } else if (hasPayloads) {
    posMult = 2;
  } else {
    posMult = 1;
  }
}

shared_ptr<PostingsEnum> DirectPostingsFormat::LowFreqPostingsEnum::reset(
    std::deque<int> &postings, std::deque<char> &payloadBytes)
{
  this->postings = postings;
  upto = 0;
  skipPositions = 0;
  pos = -1;
  startOffset_ = -1;
  endOffset_ = -1;
  docID_ = -1;
  payloadLength = 0;
  this->payloadBytes = payloadBytes;
  return shared_from_this();
}

int DirectPostingsFormat::LowFreqPostingsEnum::nextDoc()
{
  pos = -1;
  if (hasPayloads) {
    for (int i = 0; i < skipPositions; i++) {
      upto++;
      if (hasOffsets) {
        upto += 2;
      }
      payloadOffset += postings[upto++];
    }
  } else {
    upto += posMult * skipPositions;
  }

  if (upto < postings.size()) {
    docID_ = postings[upto++];
    freq_ = postings[upto++];
    skipPositions = freq_;
    return docID_;
  }

  return docID_ = NO_MORE_DOCS;
}

int DirectPostingsFormat::LowFreqPostingsEnum::docID() { return docID_; }

int DirectPostingsFormat::LowFreqPostingsEnum::freq() { return freq_; }

int DirectPostingsFormat::LowFreqPostingsEnum::nextPosition()
{
  assert(skipPositions > 0);
  skipPositions--;
  pos = postings[upto++];
  if (hasOffsets) {
    startOffset_ = postings[upto++];
    endOffset_ = postings[upto++];
  }
  if (hasPayloads) {
    payloadLength = postings[upto++];
    lastPayloadOffset = payloadOffset;
    payloadOffset += payloadLength;
  }
  return pos;
}

int DirectPostingsFormat::LowFreqPostingsEnum::startOffset()
{
  return startOffset_;
}

int DirectPostingsFormat::LowFreqPostingsEnum::endOffset()
{
  return endOffset_;
}

int DirectPostingsFormat::LowFreqPostingsEnum::advance(int target) throw(
    IOException)
{
  return slowAdvance(target);
}

shared_ptr<BytesRef> DirectPostingsFormat::LowFreqPostingsEnum::getPayload()
{
  if (payloadLength > 0) {
    payload->bytes = payloadBytes;
    payload->offset = lastPayloadOffset;
    payload->length = payloadLength;
    return payload;
  } else {
    return nullptr;
  }
}

int64_t DirectPostingsFormat::LowFreqPostingsEnum::cost()
{
  // TODO: could do a better estimate
  return postings.size() / 2;
}

DirectPostingsFormat::HighFreqDocsEnum::HighFreqDocsEnum() {}

std::deque<int> DirectPostingsFormat::HighFreqDocsEnum::getDocIDs()
{
  return docIDs;
}

std::deque<int> DirectPostingsFormat::HighFreqDocsEnum::getFreqs()
{
  return freqs;
}

shared_ptr<PostingsEnum>
DirectPostingsFormat::HighFreqDocsEnum::reset(std::deque<int> &docIDs,
                                              std::deque<int> &freqs)
{
  this->docIDs = docIDs;
  this->freqs = freqs;
  docID_ = upto = -1;
  return shared_from_this();
}

int DirectPostingsFormat::HighFreqDocsEnum::nextDoc()
{
  upto++;
  try {
    return docID_ = docIDs[upto];
  } catch (const out_of_range &e) {
  }
  return docID_ = NO_MORE_DOCS;
}

int DirectPostingsFormat::HighFreqDocsEnum::docID() { return docID_; }

int DirectPostingsFormat::HighFreqDocsEnum::freq()
{
  if (freqs.empty()) {
    return 1;
  } else {
    return freqs[upto];
  }
}

int DirectPostingsFormat::HighFreqDocsEnum::advance(int target)
{
  /*
  upto++;
  if (upto == docIDs.length) {
    return docID = NO_MORE_DOCS;
  }
  final int index = Arrays.binarySearch(docIDs, upto, docIDs.length, target);
  if (index < 0) {
    upto = -index - 1;
  } else {
    upto = index;
  }
  if (liveDocs != null) {
    while (upto < docIDs.length) {
      if (liveDocs.get(docIDs[upto])) {
        break;
      }
      upto++;
    }
  }
  if (upto == docIDs.length) {
    return NO_MORE_DOCS;
  } else {
    return docID = docIDs[upto];
  }
  */

  // System.out.println("  advance target=" + target + " cur=" + docID() + "
  // upto=" + upto + " of " + docIDs.length);
  // if (DEBUG) {
  //   System.out.println("advance target=" + target + " len=" + docIDs.length);
  // }
  upto++;
  if (upto == docIDs.size()) {
    return docID_ = NO_MORE_DOCS;
  }

  // First "grow" outwards, since most advances are to
  // nearby docs:
  int inc = 10;
  int nextUpto = upto + 10;
  int low;
  int high;
  while (true) {
    // System.out.println("  grow nextUpto=" + nextUpto + " inc=" + inc);
    if (nextUpto >= docIDs.size()) {
      low = nextUpto - inc;
      high = docIDs.size() - 1;
      break;
    }
    // System.out.println("    docID=" + docIDs[nextUpto]);

    if (target <= docIDs[nextUpto]) {
      low = nextUpto - inc;
      high = nextUpto;
      break;
    }
    inc *= 2;
    nextUpto += inc;
  }

  // Now do normal binary search
  // System.out.println("    after fwd: low=" + low + " high=" + high);

  while (true) {

    if (low > high) {
      // Not exactly found
      // System.out.println("    break: no match");
      upto = low;
      break;
    }

    int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
    int cmp = docIDs[mid] - target;
    // System.out.println("    bsearch low=" + low + " high=" + high+ ":
    // docIDs[" + mid + "]=" + docIDs[mid]);

    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      // Found target
      upto = mid;
      // System.out.println("    break: match");
      break;
    }
  }

  // System.out.println("    end upto=" + upto + " docID=" + (upto >=
  // docIDs.length ? NO_MORE_DOCS : docIDs[upto]));

  if (upto == docIDs.size()) {
    // System.out.println("    return END");
    return docID_ = NO_MORE_DOCS;
  } else {
    // System.out.println("    return docID=" + docIDs[upto] + " upto=" + upto);
    return docID_ = docIDs[upto];
  }
}

int64_t DirectPostingsFormat::HighFreqDocsEnum::cost()
{
  return docIDs.size();
}

int DirectPostingsFormat::HighFreqDocsEnum::nextPosition() 
{
  return -1;
}

int DirectPostingsFormat::HighFreqDocsEnum::startOffset() 
{
  return -1;
}

int DirectPostingsFormat::HighFreqDocsEnum::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
DirectPostingsFormat::HighFreqDocsEnum::getPayload() 
{
  return nullptr;
}

DirectPostingsFormat::HighFreqPostingsEnum::HighFreqPostingsEnum(
    bool hasOffsets)
    : hasOffsets(hasOffsets), posJump(hasOffsets ? 3 : 1)
{
}

std::deque<int> DirectPostingsFormat::HighFreqPostingsEnum::getDocIDs()
{
  return docIDs;
}

std::deque<std::deque<int>>
DirectPostingsFormat::HighFreqPostingsEnum::getPositions()
{
  return positions;
}

int DirectPostingsFormat::HighFreqPostingsEnum::getPosJump() { return posJump; }

shared_ptr<PostingsEnum> DirectPostingsFormat::HighFreqPostingsEnum::reset(
    std::deque<int> &docIDs, std::deque<int> &freqs,
    std::deque<std::deque<int>> &positions,
    std::deque<std::deque<std::deque<char>>> &payloads)
{
  this->docIDs = docIDs;
  this->freqs = freqs;
  this->positions = positions;
  this->payloads = payloads;
  upto = -1;
  return shared_from_this();
}

int DirectPostingsFormat::HighFreqPostingsEnum::nextDoc()
{
  upto++;
  if (upto < docIDs.size()) {
    posUpto = -posJump;
    curPositions = positions[upto];
    return docID_ = docIDs[upto];
  }

  return docID_ = NO_MORE_DOCS;
}

int DirectPostingsFormat::HighFreqPostingsEnum::freq() { return freqs[upto]; }

int DirectPostingsFormat::HighFreqPostingsEnum::docID() { return docID_; }

int DirectPostingsFormat::HighFreqPostingsEnum::nextPosition()
{
  posUpto += posJump;
  assert(posUpto < curPositions.size());
  return curPositions[posUpto];
}

int DirectPostingsFormat::HighFreqPostingsEnum::startOffset()
{
  if (hasOffsets) {
    return curPositions[posUpto + 1];
  } else {
    return -1;
  }
}

int DirectPostingsFormat::HighFreqPostingsEnum::endOffset()
{
  if (hasOffsets) {
    return curPositions[posUpto + 2];
  } else {
    return -1;
  }
}

int DirectPostingsFormat::HighFreqPostingsEnum::advance(int target)
{

  /*
  upto++;
  if (upto == docIDs.length) {
    return NO_MORE_DOCS;
  }
  final int index = Arrays.binarySearch(docIDs, upto, docIDs.length, target);
  if (index < 0) {
    upto = -index - 1;
  } else {
    upto = index;
  }
  if (liveDocs != null) {
    while (upto < docIDs.length) {
      if (liveDocs.get(docIDs[upto])) {
        break;
      }
      upto++;
    }
  }
  posUpto = hasOffsets ? -3 : -1;
  if (upto == docIDs.length) {
    return NO_MORE_DOCS;
  } else {
    return docID();
  }
  */

  // System.out.println("  advance target=" + target + " cur=" + docID() + "
  // upto=" + upto + " of " + docIDs.length);
  // if (DEBUG) {
  //   System.out.println("advance target=" + target + " len=" + docIDs.length);
  // }
  upto++;
  if (upto == docIDs.size()) {
    return docID_ = NO_MORE_DOCS;
  }

  // First "grow" outwards, since most advances are to
  // nearby docs:
  int inc = 10;
  int nextUpto = upto + 10;
  int low;
  int high;
  while (true) {
    // System.out.println("  grow nextUpto=" + nextUpto + " inc=" + inc);
    if (nextUpto >= docIDs.size()) {
      low = nextUpto - inc;
      high = docIDs.size() - 1;
      break;
    }
    // System.out.println("    docID=" + docIDs[nextUpto]);

    if (target <= docIDs[nextUpto]) {
      low = nextUpto - inc;
      high = nextUpto;
      break;
    }
    inc *= 2;
    nextUpto += inc;
  }

  // Now do normal binary search
  // System.out.println("    after fwd: low=" + low + " high=" + high);

  while (true) {

    if (low > high) {
      // Not exactly found
      // System.out.println("    break: no match");
      upto = low;
      break;
    }

    int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
    int cmp = docIDs[mid] - target;
    // System.out.println("    bsearch low=" + low + " high=" + high+ ":
    // docIDs[" + mid + "]=" + docIDs[mid]);

    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      // Found target
      upto = mid;
      // System.out.println("    break: match");
      break;
    }
  }

  // System.out.println("    end upto=" + upto + " docID=" + (upto >=
  // docIDs.length ? NO_MORE_DOCS : docIDs[upto]));

  if (upto == docIDs.size()) {
    // System.out.println("    return END");
    return docID_ = NO_MORE_DOCS;
  } else {
    // System.out.println("    return docID=" + docIDs[upto] + " upto=" + upto);
    posUpto = -posJump;
    curPositions = positions[upto];
    return docID_ = docIDs[upto];
  }
}

shared_ptr<BytesRef> DirectPostingsFormat::HighFreqPostingsEnum::getPayload()
{
  if (payloads.empty()) {
    return nullptr;
  } else {
    const std::deque<char> payloadBytes =
        payloads[upto][posUpto / (hasOffsets ? 3 : 1)];
    if (payloadBytes.empty()) {
      return nullptr;
    }
    payload->bytes = payloadBytes;
    payload->length = payloadBytes.size();
    payload->offset = 0;
    return payload;
  }
}

int64_t DirectPostingsFormat::HighFreqPostingsEnum::cost()
{
  return docIDs.size();
}
} // namespace org::apache::lucene::codecs::memory