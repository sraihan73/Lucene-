using namespace std;

#include "SimpleTextFieldsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "SimpleTextPostingsFormat.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::END =
        make_shared<org::apache::lucene::util::BytesRef>(L"END");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::FIELD =
        make_shared<org::apache::lucene::util::BytesRef>(L"field ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::TERM =
        make_shared<org::apache::lucene::util::BytesRef>(L"  term ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::DOC =
        make_shared<org::apache::lucene::util::BytesRef>(L"    doc ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::FREQ =
        make_shared<org::apache::lucene::util::BytesRef>(L"      freq ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::POS =
        make_shared<org::apache::lucene::util::BytesRef>(L"      pos ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::START_OFFSET =
        make_shared<org::apache::lucene::util::BytesRef>(L"      startOffset ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::END_OFFSET =
        make_shared<org::apache::lucene::util::BytesRef>(L"      endOffset ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextFieldsWriter::PAYLOAD =
        make_shared<org::apache::lucene::util::BytesRef>(L"        payload ");

SimpleTextFieldsWriter::SimpleTextFieldsWriter(
    shared_ptr<SegmentWriteState> writeState) 
    : writeState(writeState), segment(writeState->segmentInfo->name)
{
  const wstring fileName = SimpleTextPostingsFormat::getPostingsFileName(
      writeState->segmentInfo->name, writeState->segmentSuffix);
  out = writeState->directory->createOutput(fileName, writeState->context);
}

void SimpleTextFieldsWriter::write(shared_ptr<Fields> fields) 
{
  write(writeState->fieldInfos, fields);
}

void SimpleTextFieldsWriter::write(shared_ptr<FieldInfos> fieldInfos,
                                   shared_ptr<Fields> fields) 
{

  // for each field
  for (auto field : fields) {
    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      // Annoyingly, this can happen!
      continue;
    }
    shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(field);

    bool wroteField = false;

    bool hasPositions = terms->hasPositions();
    bool hasFreqs = terms->hasFreqs();
    bool hasPayloads = fieldInfo->hasPayloads();
    bool hasOffsets = terms->hasOffsets();

    int flags = 0;
    if (hasPositions) {
      flags = PostingsEnum::POSITIONS;
      if (hasPayloads) {
        flags = flags | PostingsEnum::PAYLOADS;
      }
      if (hasOffsets) {
        flags = flags | PostingsEnum::OFFSETS;
      }
    } else {
      if (hasFreqs) {
        flags = flags | PostingsEnum::FREQS;
      }
    }

    shared_ptr<TermsEnum> termsEnum = terms->begin();
    shared_ptr<PostingsEnum> postingsEnum = nullptr;

    // for each term in field
    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }

      postingsEnum = termsEnum->postings(postingsEnum, flags);

      assert(
          (postingsEnum != nullptr, L"termsEnum=" + termsEnum + L" hasPos=" +
                                        StringHelper::toString(hasPositions) +
                                        L" flags=" + to_wstring(flags)));

      bool wroteTerm = false;

      // for each doc in field+term
      while (true) {
        int doc = postingsEnum->nextDoc();
        if (doc == PostingsEnum::NO_MORE_DOCS) {
          break;
        }

        if (!wroteTerm) {

          if (!wroteField) {
            // we lazily do this, in case the field had
            // no terms
            write(FIELD);
            write(field);
            newline();
            wroteField = true;
          }

          // we lazily do this, in case the term had
          // zero docs
          write(TERM);
          write(term);
          newline();
          wroteTerm = true;
        }

        write(DOC);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        write(Integer::toString(doc));
        newline();
        if (hasFreqs) {
          int freq = postingsEnum->freq();
          write(FREQ);
          // C++ TODO: There is no native C++ equivalent to 'toString':
          write(Integer::toString(freq));
          newline();

          if (hasPositions) {
            // for assert:
            int lastStartOffset = 0;

            // for each pos in field+term+doc
            for (int i = 0; i < freq; i++) {
              int position = postingsEnum->nextPosition();

              write(POS);
              // C++ TODO: There is no native C++ equivalent to 'toString':
              write(Integer::toString(position));
              newline();

              if (hasOffsets) {
                int startOffset = postingsEnum->startOffset();
                int endOffset = postingsEnum->endOffset();
                assert(endOffset >= startOffset);
                assert((startOffset >= lastStartOffset,
                        L"startOffset=" + to_wstring(startOffset) +
                            L" lastStartOffset=" +
                            to_wstring(lastStartOffset)));
                lastStartOffset = startOffset;
                write(START_OFFSET);
                // C++ TODO: There is no native C++ equivalent to 'toString':
                write(Integer::toString(startOffset));
                newline();
                write(END_OFFSET);
                // C++ TODO: There is no native C++ equivalent to 'toString':
                write(Integer::toString(endOffset));
                newline();
              }

              shared_ptr<BytesRef> payload = postingsEnum->getPayload();

              if (payload != nullptr && payload->length > 0) {
                assert(payload->length != 0);
                write(PAYLOAD);
                write(payload);
                newline();
              }
            }
          }
        }
      }
    }
  }
}

void SimpleTextFieldsWriter::write(const wstring &s) 
{
  SimpleTextUtil::write(out, s, scratch);
}

void SimpleTextFieldsWriter::write(shared_ptr<BytesRef> b) 
{
  SimpleTextUtil::write(out, b);
}

void SimpleTextFieldsWriter::newline() 
{
  SimpleTextUtil::writeNewline(out);
}

SimpleTextFieldsWriter::~SimpleTextFieldsWriter()
{
  if (out != nullptr) {
    try {
      write(END);
      newline();
      SimpleTextUtil::writeChecksum(out, scratch);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete out;
      out.reset();
    }
  }
}
} // namespace org::apache::lucene::codecs::simpletext