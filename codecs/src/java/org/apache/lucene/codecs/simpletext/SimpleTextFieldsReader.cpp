using namespace std;

#include "SimpleTextFieldsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/BufferedChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PairOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
using Builder = org::apache::lucene::util::fst::Builder;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.DOC; import
//    static org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.END;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.END_OFFSET;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.FIELD; import
//    static org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.FREQ;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.PAYLOAD; import
//    static org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.POS;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.START_OFFSET;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.TERM;

SimpleTextFieldsReader::SimpleTextFieldsReader(
    shared_ptr<SegmentReadState> state) 
    : in_(state->directory->openInput(
          SimpleTextPostingsFormat::getPostingsFileName(
              state->segmentInfo->name, state->segmentSuffix),
          state->context)),
      fieldInfos(state->fieldInfos), maxDoc(state->segmentInfo->maxDoc())
{
  bool success = false;
  try {
    fields = readFields(in_->clone());
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      IOUtils::closeWhileHandlingException({shared_from_this()});
    }
  }
}

map_obj<wstring, int64_t> SimpleTextFieldsReader::readFields(
    shared_ptr<IndexInput> in_) 
{
  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(in_);
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  map_obj<wstring, int64_t> fields = map_obj<wstring, int64_t>();

  while (true) {
    SimpleTextUtil::readLine(input, scratch);
    if (scratch->get().equals(END)) {
      SimpleTextUtil::checkFooter(input);
      return fields;
    } else if (StringHelper::startsWith(scratch->get(), FIELD)) {
      wstring fieldName =
          wstring(scratch->bytes(), FIELD->length,
                  scratch->length() - FIELD->length, StandardCharsets::UTF_8);
      fields.emplace(fieldName, input->getFilePointer());
    }
  }
}

SimpleTextFieldsReader::SimpleTextTermsEnum::SimpleTextTermsEnum(
    shared_ptr<SimpleTextFieldsReader> outerInstance,
    shared_ptr<FST<
        PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
        fst,
    IndexOptions indexOptions)
    : indexOptions(indexOptions), fstEnum(make_shared<BytesRefFSTEnum<>>(fst)),
      outerInstance(outerInstance)
{
}

bool SimpleTextFieldsReader::SimpleTextTermsEnum::seekExact(
    shared_ptr<BytesRef> text) 
{

  shared_ptr<BytesRefFSTEnum::InputOutput<
      PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
      *const result = fstEnum->seekExact(text);
  if (result != nullptr) {
    shared_ptr<
        PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>
        pair1 = result->output;
    shared_ptr<PairOutputs::Pair<int64_t, int64_t>> pair2 = pair1->output2;
    docsStart = pair1->output1;
    docFreq_ = pair2->output1.intValue();
    totalTermFreq_ = pair2->output2;
    return true;
  } else {
    return false;
  }
}

SeekStatus SimpleTextFieldsReader::SimpleTextTermsEnum::seekCeil(
    shared_ptr<BytesRef> text) 
{

  // System.out.println("seek to text=" + text.utf8ToString());
  shared_ptr<BytesRefFSTEnum::InputOutput<
      PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
      *const result = fstEnum->seekCeil(text);
  if (result == nullptr) {
    // System.out.println("  end");
    return SeekStatus::END;
  } else {
    // System.out.println("  got text=" + term.utf8ToString());
    shared_ptr<
        PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>
        pair1 = result->output;
    shared_ptr<PairOutputs::Pair<int64_t, int64_t>> pair2 = pair1->output2;
    docsStart = pair1->output1;
    docFreq_ = pair2->output1.intValue();
    totalTermFreq_ = pair2->output2;

    if (result->input->equals(text)) {
      // System.out.println("  match docsStart=" + docsStart);
      return SeekStatus::FOUND;
    } else {
      // System.out.println("  not match docsStart=" + docsStart);
      return SeekStatus::NOT_FOUND;
    }
  }
}

shared_ptr<BytesRef>
SimpleTextFieldsReader::SimpleTextTermsEnum::next() 
{
  assert(!ended);
  shared_ptr<BytesRefFSTEnum::InputOutput<
      PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
      *const result = fstEnum->next();
  if (result != nullptr) {
    shared_ptr<
        PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>
        pair1 = result->output;
    shared_ptr<PairOutputs::Pair<int64_t, int64_t>> pair2 = pair1->output2;
    docsStart = pair1->output1;
    docFreq_ = pair2->output1.intValue();
    totalTermFreq_ = pair2->output2;
    return result->input;
  } else {
    return nullptr;
  }
}

shared_ptr<BytesRef> SimpleTextFieldsReader::SimpleTextTermsEnum::term()
{
  return fstEnum->current()->input;
}

int64_t SimpleTextFieldsReader::SimpleTextTermsEnum::ord() 
{
  throw make_shared<UnsupportedOperationException>();
}

void SimpleTextFieldsReader::SimpleTextTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int SimpleTextFieldsReader::SimpleTextTermsEnum::docFreq() { return docFreq_; }

int64_t SimpleTextFieldsReader::SimpleTextTermsEnum::totalTermFreq()
{
  return indexOptions == IndexOptions::DOCS ? -1 : totalTermFreq_;
}

shared_ptr<PostingsEnum> SimpleTextFieldsReader::SimpleTextTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{

  bool hasPositions =
      indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  if (hasPositions &&
      PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {

    shared_ptr<SimpleTextPostingsEnum> docsAndPositionsEnum;
    if (reuse != nullptr &&
        std::dynamic_pointer_cast<SimpleTextPostingsEnum>(reuse) != nullptr &&
        (std::static_pointer_cast<SimpleTextPostingsEnum>(reuse))
            ->canReuse(outerInstance->in_)) {
      docsAndPositionsEnum =
          std::static_pointer_cast<SimpleTextPostingsEnum>(reuse);
    } else {
      docsAndPositionsEnum = make_shared<SimpleTextPostingsEnum>(outerInstance);
    }
    return docsAndPositionsEnum->reset(docsStart, indexOptions, docFreq_);
  }

  shared_ptr<SimpleTextDocsEnum> docsEnum;
  if (reuse != nullptr &&
      std::dynamic_pointer_cast<SimpleTextDocsEnum>(reuse) != nullptr &&
      (std::static_pointer_cast<SimpleTextDocsEnum>(reuse))
          ->canReuse(outerInstance->in_)) {
    docsEnum = std::static_pointer_cast<SimpleTextDocsEnum>(reuse);
  } else {
    docsEnum = make_shared<SimpleTextDocsEnum>(outerInstance);
  }
  return docsEnum->reset(docsStart, indexOptions == IndexOptions::DOCS,
                         docFreq_);
}

SimpleTextFieldsReader::SimpleTextDocsEnum::SimpleTextDocsEnum(
    shared_ptr<SimpleTextFieldsReader> outerInstance)
    : inStart(outerInstance->in_), in_(this->inStart->clone()),
      outerInstance(outerInstance)
{
}

bool SimpleTextFieldsReader::SimpleTextDocsEnum::canReuse(
    shared_ptr<IndexInput> in_)
{
  return in_ == inStart;
}

shared_ptr<SimpleTextDocsEnum>
SimpleTextFieldsReader::SimpleTextDocsEnum::reset(
    int64_t fp, bool omitTF, int docFreq) 
{
  in_->seek(fp);
  this->omitTF = omitTF;
  docID_ = -1;
  tf = 1;
  cost_ = docFreq;
  return shared_from_this();
}

int SimpleTextFieldsReader::SimpleTextDocsEnum::docID() { return docID_; }

int SimpleTextFieldsReader::SimpleTextDocsEnum::freq() 
{
  return tf;
}

int SimpleTextFieldsReader::SimpleTextDocsEnum::nextPosition() throw(
    IOException)
{
  return -1;
}

int SimpleTextFieldsReader::SimpleTextDocsEnum::startOffset() 
{
  return -1;
}

int SimpleTextFieldsReader::SimpleTextDocsEnum::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
SimpleTextFieldsReader::SimpleTextDocsEnum::getPayload() 
{
  return nullptr;
}

int SimpleTextFieldsReader::SimpleTextDocsEnum::nextDoc() 
{
  if (docID_ == NO_MORE_DOCS) {
    return docID_;
  }
  bool first = true;
  int termFreq = 0;
  while (true) {
    constexpr int64_t lineStart = in_->getFilePointer();
    SimpleTextUtil::readLine(in_, scratch);
    if (StringHelper::startsWith(scratch->get(), DOC)) {
      if (!first) {
        in_->seek(lineStart);
        if (!omitTF) {
          tf = termFreq;
        }
        return docID_;
      }
      scratchUTF16->copyUTF8Bytes(scratch->bytes(), DOC->length,
                                  scratch->length() - DOC->length);
      docID_ =
          ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
      termFreq = 0;
      first = false;
    } else if (StringHelper::startsWith(scratch->get(), FREQ)) {
      scratchUTF16->copyUTF8Bytes(scratch->bytes(), FREQ->length,
                                  scratch->length() - FREQ->length);
      termFreq =
          ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
    } else if (StringHelper::startsWith(scratch->get(), POS)) {
      // skip termFreq++;
    } else if (StringHelper::startsWith(scratch->get(), START_OFFSET)) {
      // skip
    } else if (StringHelper::startsWith(scratch->get(), END_OFFSET)) {
      // skip
    } else if (StringHelper::startsWith(scratch->get(), PAYLOAD)) {
      // skip
    } else {
      assert((StringHelper::startsWith(scratch->get(), TERM) ||
                  StringHelper::startsWith(scratch->get(), FIELD) ||
                  StringHelper::startsWith(scratch->get(), END),
              L"scratch=" + scratch->get().utf8ToString()));
      if (!first) {
        in_->seek(lineStart);
        if (!omitTF) {
          tf = termFreq;
        }
        return docID_;
      }
      return docID_ = NO_MORE_DOCS;
    }
  }
}

int SimpleTextFieldsReader::SimpleTextDocsEnum::advance(int target) throw(
    IOException)
{
  // Naive -- better to index skip data
  return slowAdvance(target);
}

int64_t SimpleTextFieldsReader::SimpleTextDocsEnum::cost() { return cost_; }

SimpleTextFieldsReader::SimpleTextPostingsEnum::SimpleTextPostingsEnum(
    shared_ptr<SimpleTextFieldsReader> outerInstance)
    : inStart(outerInstance->in_), in_(inStart->clone()),
      outerInstance(outerInstance)
{
}

bool SimpleTextFieldsReader::SimpleTextPostingsEnum::canReuse(
    shared_ptr<IndexInput> in_)
{
  return in_ == inStart;
}

shared_ptr<SimpleTextPostingsEnum>
SimpleTextFieldsReader::SimpleTextPostingsEnum::reset(int64_t fp,
                                                      IndexOptions indexOptions,
                                                      int docFreq)
{
  nextDocStart = fp;
  docID_ = -1;
  readPositions =
      indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  readOffsets =
      indexOptions.compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
  if (!readOffsets) {
    startOffset_ = -1;
    endOffset_ = -1;
  }
  cost_ = docFreq;
  return shared_from_this();
}

int SimpleTextFieldsReader::SimpleTextPostingsEnum::docID() { return docID_; }

int SimpleTextFieldsReader::SimpleTextPostingsEnum::freq() 
{
  return tf;
}

int SimpleTextFieldsReader::SimpleTextPostingsEnum::nextDoc() 
{
  bool first = true;
  in_->seek(nextDocStart);
  int64_t posStart = 0;
  while (true) {
    constexpr int64_t lineStart = in_->getFilePointer();
    SimpleTextUtil::readLine(in_, scratch);
    // System.out.println("NEXT DOC: " + scratch.utf8ToString());
    if (StringHelper::startsWith(scratch->get(), DOC)) {
      if (!first) {
        nextDocStart = lineStart;
        in_->seek(posStart);
        return docID_;
      }
      scratchUTF16->copyUTF8Bytes(scratch->bytes(), DOC->length,
                                  scratch->length() - DOC->length);
      docID_ =
          ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
      tf = 0;
      first = false;
    } else if (StringHelper::startsWith(scratch->get(), FREQ)) {
      scratchUTF16->copyUTF8Bytes(scratch->bytes(), FREQ->length,
                                  scratch->length() - FREQ->length);
      tf =
          ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
      posStart = in_->getFilePointer();
    } else if (StringHelper::startsWith(scratch->get(), POS)) {
      // skip
    } else if (StringHelper::startsWith(scratch->get(), START_OFFSET)) {
      // skip
    } else if (StringHelper::startsWith(scratch->get(), END_OFFSET)) {
      // skip
    } else if (StringHelper::startsWith(scratch->get(), PAYLOAD)) {
      // skip
    } else {
      assert((StringHelper::startsWith(scratch->get(), TERM) ||
              StringHelper::startsWith(scratch->get(), FIELD) ||
              StringHelper::startsWith(scratch->get(), END)));
      if (!first) {
        nextDocStart = lineStart;
        in_->seek(posStart);
        return docID_;
      }
      return docID_ = NO_MORE_DOCS;
    }
  }
}

int SimpleTextFieldsReader::SimpleTextPostingsEnum::advance(int target) throw(
    IOException)
{
  // Naive -- better to index skip data
  return slowAdvance(target);
}

int SimpleTextFieldsReader::SimpleTextPostingsEnum::nextPosition() throw(
    IOException)
{
  if (readPositions) {
    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), POS),
            L"got line=" + scratch->get().utf8ToString()));
    scratchUTF16_2->copyUTF8Bytes(scratch->bytes(), POS->length,
                                  scratch->length() - POS->length);
    pos = ArrayUtil::parseInt(scratchUTF16_2->chars(), 0,
                              scratchUTF16_2->length());
  } else {
    pos = -1;
  }

  if (readOffsets) {
    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), START_OFFSET),
            L"got line=" + scratch->get().utf8ToString()));
    scratchUTF16_2->copyUTF8Bytes(scratch->bytes(), START_OFFSET->length,
                                  scratch->length() - START_OFFSET->length);
    startOffset_ = ArrayUtil::parseInt(scratchUTF16_2->chars(), 0,
                                       scratchUTF16_2->length());
    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), END_OFFSET),
            L"got line=" + scratch->get().utf8ToString()));
    scratchUTF16_2->grow(scratch->length() - END_OFFSET->length);
    scratchUTF16_2->copyUTF8Bytes(scratch->bytes(), END_OFFSET->length,
                                  scratch->length() - END_OFFSET->length);
    endOffset_ = ArrayUtil::parseInt(scratchUTF16_2->chars(), 0,
                                     scratchUTF16_2->length());
  }

  constexpr int64_t fp = in_->getFilePointer();
  SimpleTextUtil::readLine(in_, scratch);
  if (StringHelper::startsWith(scratch->get(), PAYLOAD)) {
    constexpr int len = scratch->length() - PAYLOAD->length;
    scratch2->grow(len);
    System::arraycopy(scratch->bytes(), PAYLOAD->length, scratch2->bytes(), 0,
                      len);
    scratch2->setLength(len);
    payload = scratch2->get();
  } else {
    payload.reset();
    in_->seek(fp);
  }
  return pos;
}

int SimpleTextFieldsReader::SimpleTextPostingsEnum::startOffset() throw(
    IOException)
{
  return startOffset_;
}

int SimpleTextFieldsReader::SimpleTextPostingsEnum::endOffset() throw(
    IOException)
{
  return endOffset_;
}

shared_ptr<BytesRef>
SimpleTextFieldsReader::SimpleTextPostingsEnum::getPayload()
{
  return payload;
}

int64_t SimpleTextFieldsReader::SimpleTextPostingsEnum::cost()
{
  return cost_;
}

SimpleTextFieldsReader::TermData::TermData(int64_t docsStart, int docFreq)
{
  this->docsStart = docsStart;
  this->docFreq = docFreq;
}

SimpleTextFieldsReader::SimpleTextTerms::SimpleTextTerms(
    shared_ptr<SimpleTextFieldsReader> outerInstance, const wstring &field,
    int64_t termsStart, int maxDoc) 
    : termsStart(termsStart),
      fieldInfo(outerInstance->fieldInfos->fieldInfo(field)), maxDoc(maxDoc),
      outerInstance(outerInstance)
{
  loadTerms();
}

void SimpleTextFieldsReader::SimpleTextTerms::loadTerms() 
{
  shared_ptr<PositiveIntOutputs> posIntOutputs =
      PositiveIntOutputs::getSingleton();
  shared_ptr<Builder<
      PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
      *const b;
  shared_ptr<PairOutputs<int64_t, int64_t>> *const outputsInner =
      make_shared<PairOutputs<int64_t, int64_t>>(posIntOutputs,
                                                     posIntOutputs);
  shared_ptr<PairOutputs<int64_t, PairOutputs::Pair<int64_t, int64_t>>>
      *const outputs = make_shared<
          PairOutputs<int64_t, PairOutputs::Pair<int64_t, int64_t>>>(
          posIntOutputs, outputsInner);
  b = make_shared<Builder<
      PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>(
      FST::INPUT_TYPE::BYTE1, outputs);
  shared_ptr<IndexInput> in_ = outerInstance->in_->clone();
  in_->seek(termsStart);
  shared_ptr<BytesRefBuilder> *const lastTerm = make_shared<BytesRefBuilder>();
  int64_t lastDocsStart = -1;
  int docFreq = 0;
  int64_t totalTermFreq = 0;
  shared_ptr<FixedBitSet> visitedDocs = make_shared<FixedBitSet>(maxDoc);
  shared_ptr<IntsRefBuilder> *const scratchIntsRef =
      make_shared<IntsRefBuilder>();
  while (true) {
    SimpleTextUtil::readLine(in_, scratch);
    if (scratch->get().equals(END) ||
        StringHelper::startsWith(scratch->get(), FIELD)) {
      if (lastDocsStart != -1) {
        b->add(Util::toIntsRef(lastTerm->get(), scratchIntsRef),
               outputs->newPair(
                   lastDocsStart,
                   outputsInner->newPair(static_cast<int64_t>(docFreq),
                                         totalTermFreq)));
        sumTotalTermFreq += totalTermFreq;
      }
      break;
    } else if (StringHelper::startsWith(scratch->get(), DOC)) {
      docFreq++;
      sumDocFreq++;
      scratchUTF16->copyUTF8Bytes(scratch->bytes(), DOC->length,
                                  scratch->length() - DOC->length);
      int docID =
          ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
      visitedDocs->set(docID);
    } else if (StringHelper::startsWith(scratch->get(), FREQ)) {
      scratchUTF16->copyUTF8Bytes(scratch->bytes(), FREQ->length,
                                  scratch->length() - FREQ->length);
      totalTermFreq +=
          ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
    } else if (StringHelper::startsWith(scratch->get(), TERM)) {
      if (lastDocsStart != -1) {
        b->add(Util::toIntsRef(lastTerm->get(), scratchIntsRef),
               outputs->newPair(
                   lastDocsStart,
                   outputsInner->newPair(static_cast<int64_t>(docFreq),
                                         totalTermFreq)));
      }
      lastDocsStart = in_->getFilePointer();
      constexpr int len = scratch->length() - TERM->length;
      lastTerm->grow(len);
      System::arraycopy(scratch->bytes(), TERM->length, lastTerm->bytes(), 0,
                        len);
      lastTerm->setLength(len);
      docFreq = 0;
      sumTotalTermFreq += totalTermFreq;
      totalTermFreq = 0;
      termCount++;
    }
  }
  docCount = visitedDocs->cardinality();
  fst = b->finish();
  /*
  PrintStream ps = new PrintStream("out.dot");
  fst.toDot(ps);
  ps.close();
  System.out.println("SAVED out.dot");
  */
  // System.out.println("FST " + fst.sizeInBytes());
}

int64_t SimpleTextFieldsReader::SimpleTextTerms::ramBytesUsed()
{
  return TERMS_BASE_RAM_BYTES_USED +
         (fst != nullptr ? fst->ramBytesUsed() : 0) +
         RamUsageEstimator::sizeOf(scratch->bytes()) +
         RamUsageEstimator::sizeOf(scratchUTF16->chars());
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
SimpleTextFieldsReader::SimpleTextTerms::getChildResources()
{
  if (fst == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"term cache", fst));
  }
}

wstring SimpleTextFieldsReader::SimpleTextTerms::toString()
{
  return getClass().getSimpleName() + L"(terms=" + to_wstring(termCount) +
         L",postings=" + to_wstring(sumDocFreq) + L",positions=" +
         to_wstring(sumTotalTermFreq) + L",docs=" + to_wstring(docCount) + L")";
}

shared_ptr<TermsEnum>
SimpleTextFieldsReader::SimpleTextTerms::iterator() 
{
  if (fst != nullptr) {
    return make_shared<SimpleTextTermsEnum>(outerInstance, fst,
                                            fieldInfo->getIndexOptions());
  } else {
    return TermsEnum::EMPTY;
  }
}

int64_t SimpleTextFieldsReader::SimpleTextTerms::size()
{
  return static_cast<int64_t>(termCount);
}

int64_t SimpleTextFieldsReader::SimpleTextTerms::getSumTotalTermFreq()
{
  return fieldInfo->getIndexOptions() == IndexOptions::DOCS ? -1
                                                            : sumTotalTermFreq;
}

int64_t
SimpleTextFieldsReader::SimpleTextTerms::getSumDocFreq() 
{
  return sumDocFreq;
}

int SimpleTextFieldsReader::SimpleTextTerms::getDocCount() 
{
  return docCount;
}

bool SimpleTextFieldsReader::SimpleTextTerms::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool SimpleTextFieldsReader::SimpleTextTerms::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool SimpleTextFieldsReader::SimpleTextTerms::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool SimpleTextFieldsReader::SimpleTextTerms::hasPayloads()
{
  return fieldInfo->hasPayloads();
}

shared_ptr<Iterator<wstring>> SimpleTextFieldsReader::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Terms>
SimpleTextFieldsReader::terms(const wstring &field) 
{
  shared_ptr<SimpleTextTerms> terms = termsCache[field];
  if (terms == nullptr) {
    optional<int64_t> fp = fields[field];
    if (!fp) {
      return nullptr;
    } else {
      terms =
          make_shared<SimpleTextTerms>(shared_from_this(), field, fp, maxDoc);
      termsCache.emplace(field, terms);
    }
  }
  return terms;
}

int SimpleTextFieldsReader::size() { return -1; }

SimpleTextFieldsReader::~SimpleTextFieldsReader() { delete in_; }

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t SimpleTextFieldsReader::ramBytesUsed()
{
  int64_t sizeInBytes =
      BASE_RAM_BYTES_USED +
      fields.size() * 2 * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  for (auto simpleTextTerms : termsCache) {
    sizeInBytes += (simpleTextTerms->second != nullptr)
                       ? simpleTextTerms->second.ramBytesUsed()
                       : 0;
  }
  return sizeInBytes;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<deque<std::shared_ptr<Accountable>>>
SimpleTextFieldsReader::getChildResources()
{
  return Accountables::namedAccountables(L"field", termsCache);
}

wstring SimpleTextFieldsReader::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() + L")";
}

void SimpleTextFieldsReader::checkIntegrity()  {}
} // namespace org::apache::lucene::codecs::simpletext