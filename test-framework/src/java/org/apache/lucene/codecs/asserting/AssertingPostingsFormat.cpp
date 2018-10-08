using namespace std;

#include "AssertingPostingsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingLeafReader = org::apache::lucene::index::AssertingLeafReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using TestUtil = org::apache::lucene::util::TestUtil;

AssertingPostingsFormat::AssertingPostingsFormat()
    : org::apache::lucene::codecs::PostingsFormat(L"Asserting")
{
}

shared_ptr<FieldsConsumer> AssertingPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<AssertingFieldsConsumer>(state,
                                              in_->fieldsConsumer(state));
}

shared_ptr<FieldsProducer> AssertingPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<AssertingFieldsProducer>(in_->fieldsProducer(state));
}

AssertingPostingsFormat::AssertingFieldsProducer::AssertingFieldsProducer(
    shared_ptr<FieldsProducer> in_)
    : in_(in_)
{
  // do a few simple checks on init
  assert(toString() != L"");
  assert(ramBytesUsed() >= 0);
  assert(getChildResources() != nullptr);
}

AssertingPostingsFormat::AssertingFieldsProducer::~AssertingFieldsProducer()
{
  in_->close();
  in_->close(); // close again
}

shared_ptr<Iterator<wstring>>
AssertingPostingsFormat::AssertingFieldsProducer::iterator()
{
  FieldsProducer::const_iterator iterator = in_->begin();
  assert(iterator != nullptr);
  return iterator;
}

shared_ptr<Terms> AssertingPostingsFormat::AssertingFieldsProducer::terms(
    const wstring &field) 
{
  shared_ptr<Terms> terms = in_->terms(field);
  return terms == nullptr
             ? nullptr
             : make_shared<AssertingLeafReader::AssertingTerms>(terms);
}

int AssertingPostingsFormat::AssertingFieldsProducer::size()
{
  return in_->size();
}

int64_t AssertingPostingsFormat::AssertingFieldsProducer::ramBytesUsed()
{
  int64_t v = in_->ramBytesUsed();
  assert(v >= 0);
  return v;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AssertingPostingsFormat::AssertingFieldsProducer::getChildResources()
{
  shared_ptr<deque<std::shared_ptr<Accountable>>> res =
      in_->getChildResources();
  TestUtil::checkReadOnly(res);
  return res;
}

void AssertingPostingsFormat::AssertingFieldsProducer::checkIntegrity() throw(
    IOException)
{
  in_->checkIntegrity();
}

shared_ptr<FieldsProducer>
AssertingPostingsFormat::AssertingFieldsProducer::getMergeInstance() throw(
    IOException)
{
  return make_shared<AssertingFieldsProducer>(in_->getMergeInstance());
}

wstring AssertingPostingsFormat::AssertingFieldsProducer::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + in_->toString() + L")";
}

AssertingPostingsFormat::AssertingFieldsConsumer::AssertingFieldsConsumer(
    shared_ptr<SegmentWriteState> writeState, shared_ptr<FieldsConsumer> in_)
    : in_(in_), writeState(writeState)
{
}

void AssertingPostingsFormat::AssertingFieldsConsumer::write(
    shared_ptr<Fields> fields) 
{
  in_->write(fields);

  // TODO: more asserts?  can we somehow run a
  // "limited" CheckIndex here???  Or ... can we improve
  // AssertingFieldsProducer and us it also to wrap the
  // incoming Fields here?

  wstring lastField = L"";

  for (auto field : fields) {

    shared_ptr<FieldInfo> fieldInfo = writeState->fieldInfos->fieldInfo(field);
    assert(fieldInfo != nullptr);
    assert(lastField == L"" || lastField.compare(field) < 0);
    lastField = field;

    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }
    assert(terms != nullptr);

    shared_ptr<TermsEnum> termsEnum = terms->begin();
    shared_ptr<BytesRefBuilder> lastTerm = nullptr;
    shared_ptr<PostingsEnum> postingsEnum = nullptr;

    bool hasFreqs = fieldInfo->getIndexOptions().compareTo(
                        IndexOptions::DOCS_AND_FREQS) >= 0;
    bool hasPositions = fieldInfo->getIndexOptions().compareTo(
                            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
    bool hasOffsets =
        fieldInfo->getIndexOptions().compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
    bool hasPayloads = terms->hasPayloads();

    assert(hasPositions == terms->hasPositions());
    assert(hasOffsets == terms->hasOffsets());

    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      assert(lastTerm == nullptr || lastTerm->get()->compareTo(term) < 0);
      if (lastTerm == nullptr) {
        lastTerm = make_shared<BytesRefBuilder>();
        lastTerm->append(term);
      } else {
        lastTerm->copyBytes(term);
      }

      int flags = 0;
      if (hasPositions == false) {
        if (hasFreqs) {
          flags = flags | PostingsEnum::FREQS;
        }
        postingsEnum = termsEnum->postings(postingsEnum, flags);
      } else {
        flags = PostingsEnum::POSITIONS;
        if (hasPayloads) {
          flags |= PostingsEnum::PAYLOADS;
        }
        if (hasOffsets) {
          flags = flags | PostingsEnum::OFFSETS;
        }
        postingsEnum = termsEnum->postings(postingsEnum, flags);
      }

      assert((postingsEnum != nullptr,
              L"termsEnum=" + termsEnum + L" hasPositions=" +
                  StringHelper::toString(hasPositions)));

      int lastDocID = -1;

      while (true) {
        int docID = postingsEnum->nextDoc();
        if (docID == PostingsEnum::NO_MORE_DOCS) {
          break;
        }
        assert(docID > lastDocID);
        lastDocID = docID;
        if (hasFreqs) {
          int freq = postingsEnum->freq();
          assert(freq > 0);

          if (hasPositions) {
            int lastPos = -1;
            int lastStartOffset = -1;
            for (int i = 0; i < freq; i++) {
              int pos = postingsEnum->nextPosition();
              assert((pos >= lastPos,
                      L"pos=" + to_wstring(pos) + L" vs lastPos=" +
                          to_wstring(lastPos) + L" i=" + to_wstring(i) +
                          L" freq=" + to_wstring(freq)));
              assert((pos <= IndexWriter::MAX_POSITION,
                      L"pos=" + to_wstring(pos) +
                          L" is > IndexWriter.MAX_POSITION=" +
                          to_wstring(IndexWriter::MAX_POSITION)));
              lastPos = pos;

              if (hasOffsets) {
                int startOffset = postingsEnum->startOffset();
                int endOffset = postingsEnum->endOffset();
                assert(endOffset >= startOffset);
                assert(startOffset >= lastStartOffset);
                lastStartOffset = startOffset;
              }
            }
          }
        }
      }
    }
  }
}

AssertingPostingsFormat::AssertingFieldsConsumer::~AssertingFieldsConsumer()
{
  delete in_;
  delete in_; // close again
}
} // namespace org::apache::lucene::codecs::asserting