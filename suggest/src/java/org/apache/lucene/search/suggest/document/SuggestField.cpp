using namespace std;

#include "SuggestField.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<org::apache::lucene::document::FieldType>
    SuggestField::FIELD_TYPE =
        make_shared<org::apache::lucene::document::FieldType>();

SuggestField::StaticConstructor::StaticConstructor()
{
  FIELD_TYPE->setTokenized(true);
  FIELD_TYPE->setStored(false);
  FIELD_TYPE->setStoreTermVectors(false);
  FIELD_TYPE->setOmitNorms(false);
  FIELD_TYPE->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  FIELD_TYPE->freeze();
}

SuggestField::StaticConstructor SuggestField::staticConstructor;

SuggestField::SuggestField(const wstring &name, const wstring &value,
                           int weight)
    : org::apache::lucene::document::Field(name, value, FIELD_TYPE),
      surfaceForm(make_shared<BytesRef>(value)), weight(weight)
{
  if (weight < 0) {
    throw invalid_argument(L"weight must be >= 0");
  }
  if (value.length() == 0) {
    throw invalid_argument(L"value must have a length > 0");
  }
  for (int i = 0; i < value.length(); i++) {
    if (isReserved(value[i])) {
      throw invalid_argument(
          L"Illegal input [" + value + L"] UTF-16 codepoint [0x" +
          Integer::toHexString(static_cast<int>(value[i])) + L"] at position " +
          to_wstring(i) + L" is a reserved character");
    }
  }
}

shared_ptr<TokenStream> SuggestField::tokenStream(shared_ptr<Analyzer> analyzer,
                                                  shared_ptr<TokenStream> reuse)
{
  shared_ptr<CompletionTokenStream> completionStream =
      wrapTokenStream(Field::tokenStream(analyzer, reuse));
  completionStream->setPayload(buildSuggestPayload());
  return completionStream;
}

shared_ptr<CompletionTokenStream>
SuggestField::wrapTokenStream(shared_ptr<TokenStream> stream)
{
  if (std::dynamic_pointer_cast<CompletionTokenStream>(stream) != nullptr) {
    return std::static_pointer_cast<CompletionTokenStream>(stream);
  } else {
    return make_shared<CompletionTokenStream>(stream);
  }
}

char SuggestField::type() { return TYPE; }

shared_ptr<BytesRef> SuggestField::buildSuggestPayload()
{
  shared_ptr<ByteArrayOutputStream> byteArrayOutputStream =
      make_shared<ByteArrayOutputStream>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.store.OutputStreamDataOutput output = new
  // org.apache.lucene.store.OutputStreamDataOutput(byteArrayOutputStream))
  {
    org::apache::lucene::store::OutputStreamDataOutput output =
        org::apache::lucene::store::OutputStreamDataOutput(
            byteArrayOutputStream);
    try {
      output->writeVInt(surfaceForm->length);
      output->writeBytes(surfaceForm->bytes, surfaceForm->offset,
                         surfaceForm->length);
      output->writeVInt(weight + 1);
      output->writeByte(type());
    } catch (const IOException &e) {
      throw runtime_error(e); // not possible, it's a ByteArrayOutputStream!
    }
  }
  return make_shared<BytesRef>(byteArrayOutputStream->toByteArray());
}

bool SuggestField::isReserved(wchar_t c)
{
  switch (c) {
  case ConcatenateGraphFilter::SEP_LABEL:
  case CompletionAnalyzer::HOLE_CHARACTER:
  case NRTSuggesterBuilder::END_BYTE:
    return true;
  default:
    return false;
  }
}
} // namespace org::apache::lucene::search::suggest::document