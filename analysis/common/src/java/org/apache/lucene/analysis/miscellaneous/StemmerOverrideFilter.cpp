using namespace std;

#include "StemmerOverrideFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefHash.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/UnicodeUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/ByteSequenceOutputs.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;

StemmerOverrideFilter::StemmerOverrideFilter(
    shared_ptr<TokenStream> input,
    shared_ptr<StemmerOverrideMap> stemmerOverrideMap)
    : org::apache::lucene::analysis::TokenFilter(input),
      stemmerOverrideMap(stemmerOverrideMap),
      fstReader(stemmerOverrideMap->getBytesReader())
{
}

bool StemmerOverrideFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (fstReader == nullptr) {
      // No overrides
      return true;
    }
    if (!keywordAtt->isKeyword()) { // don't muck with already-keyworded terms
      shared_ptr<BytesRef> *const stem = stemmerOverrideMap->get(
          termAtt->buffer(), termAtt->length(), scratchArc, fstReader);
      if (stem != nullptr) {
        spare = ArrayUtil::grow(termAtt->buffer(), stem->length);
        constexpr int length = UnicodeUtil::UTF8toUTF16(stem, spare);
        if (spare != termAtt->buffer()) {
          termAtt->copyBuffer(spare, 0, length);
        } else {
          termAtt->setLength(length);
        }
        keywordAtt->setKeyword(true);
      }
    }
    return true;
  } else {
    return false;
  }
}

StemmerOverrideFilter::StemmerOverrideMap::StemmerOverrideMap(
    shared_ptr<FST<std::shared_ptr<BytesRef>>> fst, bool ignoreCase)
    : fst(fst), ignoreCase(ignoreCase)
{
}

shared_ptr<FST::BytesReader>
StemmerOverrideFilter::StemmerOverrideMap::getBytesReader()
{
  if (fst == nullptr) {
    return nullptr;
  } else {
    return fst->getBytesReader();
  }
}

shared_ptr<BytesRef> StemmerOverrideFilter::StemmerOverrideMap::get(
    std::deque<wchar_t> &buffer, int bufferLen,
    shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> scratchArc,
    shared_ptr<FST::BytesReader> fstReader) 
{
  shared_ptr<BytesRef> pendingOutput = fst->outputs->getNoOutput();
  shared_ptr<BytesRef> matchOutput = nullptr;
  int bufUpto = 0;
  fst->getFirstArc(scratchArc);
  while (bufUpto < bufferLen) {
    constexpr int codePoint =
        Character::codePointAt(buffer, bufUpto, bufferLen);
    if (fst->findTargetArc(ignoreCase ? towlower(codePoint) : codePoint,
                           scratchArc, scratchArc, fstReader) == nullptr) {
      return nullptr;
    }
    pendingOutput = fst->outputs->add(pendingOutput, scratchArc->output);
    bufUpto += Character::charCount(codePoint);
  }
  if (scratchArc->isFinal()) {
    matchOutput = fst->outputs->add(pendingOutput, scratchArc->nextFinalOutput);
  }
  return matchOutput;
}

StemmerOverrideFilter::Builder::Builder() : Builder(false) {}

StemmerOverrideFilter::Builder::Builder(bool ignoreCase)
    : ignoreCase(ignoreCase)
{
}

bool StemmerOverrideFilter::Builder::add(shared_ptr<std::wstring> input,
                                         shared_ptr<std::wstring> output)
{
  constexpr int length = input->length();
  if (ignoreCase) {
    // convert on the fly to lowercase
    charsSpare->grow(length);
    const std::deque<wchar_t> buffer = charsSpare->chars();
    for (int i = 0; i < length;) {
      i += Character::toChars(towlower(Character::codePointAt(input, i)), buffer,
                              i);
    }
    spare->copyChars(buffer, 0, length);
  } else {
    spare->copyChars(input, 0, length);
  }
  if (hash->add(spare->get()) >= 0) {
    outputValues.push_back(output);
    return true;
  }
  return false;
}

shared_ptr<StemmerOverrideMap>
StemmerOverrideFilter::Builder::build() 
{
  shared_ptr<ByteSequenceOutputs> outputs = ByteSequenceOutputs::getSingleton();
  shared_ptr<org::apache::lucene::util::fst::Builder<std::shared_ptr<BytesRef>>>
      builder = make_shared<
          org::apache::lucene::util::fst::Builder<std::shared_ptr<BytesRef>>>(
          FST::INPUT_TYPE::BYTE4, outputs);
  const std::deque<int> sort = hash->sort();
  shared_ptr<IntsRefBuilder> intsSpare = make_shared<IntsRefBuilder>();
  constexpr int size = hash->size();
  shared_ptr<BytesRef> spare = make_shared<BytesRef>();
  for (int i = 0; i < size; i++) {
    int id = sort[i];
    shared_ptr<BytesRef> bytesRef = hash->get(id, spare);
    intsSpare->copyUTF8Bytes(bytesRef);
    builder->add(intsSpare->get(), make_shared<BytesRef>(outputValues[id]));
  }
  return make_shared<StemmerOverrideMap>(builder->finish(), ignoreCase);
}
} // namespace org::apache::lucene::analysis::miscellaneous