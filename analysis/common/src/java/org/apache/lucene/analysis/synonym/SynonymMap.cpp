using namespace std;

#include "SynonymMap.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataOutput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefHash.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/ByteSequenceOutputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using Util = org::apache::lucene::util::fst::Util;

SynonymMap::SynonymMap(shared_ptr<FST<std::shared_ptr<BytesRef>>> fst,
                       shared_ptr<BytesRefHash> words, int maxHorizontalContext)
    : fst(fst), words(words), maxHorizontalContext(maxHorizontalContext)
{
}

SynonymMap::Builder::Builder() : Builder(true) {}

SynonymMap::Builder::Builder(bool dedup) : dedup(dedup) {}

shared_ptr<CharsRef>
SynonymMap::Builder::join(std::deque<wstring> &words,
                          shared_ptr<CharsRefBuilder> reuse)
{
  int upto = 0;
  std::deque<wchar_t> buffer = reuse->chars();
  for (auto word : words) {
    constexpr int wordLen = word.length();
    constexpr int needed =
        (0 == upto ? wordLen : 1 + upto + wordLen); // Add 1 for WORD_SEPARATOR
    if (needed > buffer.size()) {
      reuse->grow(needed);
      buffer = reuse->chars();
    }
    if (upto > 0) {
      buffer[upto++] = SynonymMap::WORD_SEPARATOR;
    }

    word.getChars(0, wordLen, buffer, upto);
    upto += wordLen;
  }
  reuse->setLength(upto);
  return reuse->get();
}

bool SynonymMap::Builder::hasHoles(shared_ptr<CharsRef> chars)
{
  constexpr int end = chars->offset + chars->length_;
  for (int idx = chars->offset + 1; idx < end; idx++) {
    if (chars->chars[idx] == SynonymMap::WORD_SEPARATOR &&
        chars->chars[idx - 1] == SynonymMap::WORD_SEPARATOR) {
      return true;
    }
  }
  if (chars->chars[chars->offset] == L'\u0000') {
    return true;
  }
  if (chars->chars[chars->offset + chars->length_ - 1] == L'\u0000') {
    return true;
  }

  return false;
}

void SynonymMap::Builder::add(shared_ptr<CharsRef> input, int numInputWords,
                              shared_ptr<CharsRef> output, int numOutputWords,
                              bool includeOrig)
{
  // first convert to UTF-8
  if (numInputWords <= 0) {
    throw invalid_argument(L"numInputWords must be > 0 (got " +
                           to_wstring(numInputWords) + L")");
  }
  if (input->length_ <= 0) {
    throw invalid_argument(L"input.length must be > 0 (got " + input->length_ +
                           L")");
  }
  if (numOutputWords <= 0) {
    throw invalid_argument(L"numOutputWords must be > 0 (got " +
                           to_wstring(numOutputWords) + L")");
  }
  if (output->length_ <= 0) {
    throw invalid_argument(L"output.length must be > 0 (got " +
                           output->length_ + L")");
  }

  assert((!hasHoles(input), L"input has holes: " + input));
  assert((!hasHoles(output), L"output has holes: " + output));

  // System.out.println("fmap.add input=" + input + " numInputWords=" +
  // numInputWords + " output=" + output + " numOutputWords=" + numOutputWords);
  utf8Scratch->copyChars(output->chars, output->offset, output->length_);
  // lookup in hash
  int ord = words->add(utf8Scratch->get());
  if (ord < 0) {
    // already exists in our hash
    ord = (-ord) - 1;
    // System.out.println("  output=" + output + " old ord=" + ord);
  } else {
    // System.out.println("  output=" + output + " new ord=" + ord);
  }

  shared_ptr<MapEntry> e = workingSet[input];
  if (e == nullptr) {
    e = make_shared<MapEntry>();
    workingSet.emplace(CharsRef::deepCopyOf(input),
                       e); // make a copy, since we will keep around in our map_obj
  }

  e->ords.push_back(ord);
  e->includeOrig |= includeOrig;
  maxHorizontalContext = max(maxHorizontalContext, numInputWords);
  maxHorizontalContext = max(maxHorizontalContext, numOutputWords);
}

int SynonymMap::Builder::countWords(shared_ptr<CharsRef> chars)
{
  int wordCount = 1;
  int upto = chars->offset;
  constexpr int limit = chars->offset + chars->length_;
  while (upto < limit) {
    if (chars->chars[upto++] == SynonymMap::WORD_SEPARATOR) {
      wordCount++;
    }
  }
  return wordCount;
}

void SynonymMap::Builder::add(shared_ptr<CharsRef> input,
                              shared_ptr<CharsRef> output, bool includeOrig)
{
  add(input, countWords(input), output, countWords(output), includeOrig);
}

shared_ptr<SynonymMap> SynonymMap::Builder::build() 
{
  shared_ptr<ByteSequenceOutputs> outputs = ByteSequenceOutputs::getSingleton();
  // TODO: are we using the best sharing options?
  shared_ptr<org::apache::lucene::util::fst::Builder<std::shared_ptr<BytesRef>>>
      builder = make_shared<
          org::apache::lucene::util::fst::Builder<std::shared_ptr<BytesRef>>>(
          FST::INPUT_TYPE::BYTE4, outputs);

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  shared_ptr<ByteArrayDataOutput> scratchOutput =
      make_shared<ByteArrayDataOutput>();

  shared_ptr<Set<int>> *const dedupSet;

  if (dedup) {
    dedupSet = unordered_set<>();
  } else {
    dedupSet.reset();
  }

  const std::deque<char> spare = std::deque<char>(5);

  shared_ptr<Set<std::shared_ptr<CharsRef>>> keys = workingSet.keySet();
  std::deque<std::shared_ptr<CharsRef>> sortedKeys =
      keys->toArray(std::deque<std::shared_ptr<CharsRef>>(keys->size()));
  Arrays::sort(sortedKeys, CharsRef::getUTF16SortedAsUTF8Comparator());

  shared_ptr<IntsRefBuilder> *const scratchIntsRef =
      make_shared<IntsRefBuilder>();

  // System.out.println("fmap.build");
  for (int keyIdx = 0; keyIdx < sortedKeys.size(); keyIdx++) {
    shared_ptr<CharsRef> input = sortedKeys[keyIdx];
    shared_ptr<MapEntry> output = workingSet[input];

    int numEntries = output->ords.size();
    // output size, assume the worst case
    int estimatedSize =
        5 + numEntries * 5; // numEntries + one ord for each entry

    scratch->grow(estimatedSize);
    scratchOutput->reset(scratch->bytes());

    // now write our output data:
    int count = 0;
    for (int i = 0; i < numEntries; i++) {
      if (dedupSet != nullptr) {
        // box once
        const optional<int> ent = output->ords[i];
        if (dedupSet->contains(ent)) {
          continue;
        }
        dedupSet->add(ent);
      }
      scratchOutput->writeVInt(output->ords[i]);
      count++;
    }

    constexpr int pos = scratchOutput->getPosition();
    scratchOutput->writeVInt(count << 1 | (output->includeOrig ? 0 : 1));
    constexpr int pos2 = scratchOutput->getPosition();
    constexpr int vIntLen = pos2 - pos;

    // Move the count + includeOrig to the front of the byte[]:
    System::arraycopy(scratch->bytes(), pos, spare, 0, vIntLen);
    System::arraycopy(scratch->bytes(), 0, scratch->bytes(), vIntLen, pos);
    System::arraycopy(spare, 0, scratch->bytes(), 0, vIntLen);

    if (dedupSet != nullptr) {
      dedupSet->clear();
    }

    scratch->setLength(scratchOutput->getPosition());
    // System.out.println("  add input=" + input + " output=" + scratch + "
    // offset=" + scratch.offset + " length=" + scratch.length + " count=" +
    // count);
    builder->add(Util::toUTF32(input, scratchIntsRef), scratch->toBytesRef());
  }

  shared_ptr<FST<std::shared_ptr<BytesRef>>> fst = builder->finish();
  return make_shared<SynonymMap>(fst, words, maxHorizontalContext);
}

SynonymMap::Parser::Parser(bool dedup, shared_ptr<Analyzer> analyzer)
    : Builder(dedup), analyzer(analyzer)
{
}

shared_ptr<CharsRef> SynonymMap::Parser::analyze(
    const wstring &text, shared_ptr<CharsRefBuilder> reuse) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("", text))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"", text);
    shared_ptr<CharTermAttribute> termAtt =
        ts->addAttribute(CharTermAttribute::typeid);
    shared_ptr<PositionIncrementAttribute> posIncAtt =
        ts->addAttribute(PositionIncrementAttribute::typeid);
    ts->reset();
    reuse->clear();
    while (ts->incrementToken()) {
      int length = termAtt->length();
      if (length == 0) {
        throw invalid_argument(L"term: " + text +
                               L" analyzed to a zero-length token");
      }
      if (posIncAtt->getPositionIncrement() != 1) {
        throw invalid_argument(
            L"term: " + text + L" analyzed to a token (" + termAtt +
            L") with position increment != 1 (got: " +
            to_wstring(posIncAtt->getPositionIncrement()) + L")");
      }
      reuse->grow(reuse->length() + length + 1); // current + word + separator
      int end = reuse->length();
      if (reuse->length() > 0) {
        reuse->setCharAt(end++, SynonymMap::WORD_SEPARATOR);
        reuse->setLength(reuse->length() + 1);
      }
      System::arraycopy(termAtt->buffer(), 0, reuse->chars(), end, length);
      reuse->setLength(reuse->length() + length);
    }
    ts->end();
  }
  if (reuse->length() == 0) {
    throw invalid_argument(L"term: " + text +
                           L" was completely eliminated by analyzer");
  }
  return reuse->get();
}
} // namespace org::apache::lucene::analysis::synonym