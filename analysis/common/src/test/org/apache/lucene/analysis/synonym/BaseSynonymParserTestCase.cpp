using namespace std;

#include "BaseSynonymParserTestCase.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Util = org::apache::lucene::util::fst::Util;

void BaseSynonymParserTestCase::assertEntryEquals(
    shared_ptr<SynonymMap> synonynMap, const wstring &word, bool includeOrig,
    std::deque<wstring> &synonyms) 
{
  word = StringHelper::replace(word, L' ', SynonymMap::WORD_SEPARATOR);
  shared_ptr<BytesRef> value =
      Util::get(synonynMap->fst, Util::toUTF32(make_shared<CharsRef>(word),
                                               make_shared<IntsRefBuilder>()));
  assertNotNull(L"No synonyms found for: " + word, value);

  shared_ptr<ByteArrayDataInput> bytesReader = make_shared<ByteArrayDataInput>(
      value->bytes, value->offset, value->length);
  constexpr int code = bytesReader->readVInt();

  constexpr bool keepOrig = (code & 0x1) == 0;
  assertEquals(L"Include original different than expected. Expected " +
                   StringHelper::toString(includeOrig) + L" was " +
                   StringHelper::toString(keepOrig),
               includeOrig, keepOrig);

  constexpr int count = static_cast<int>(static_cast<unsigned int>(code) >> 1);
  assertEquals(L"Invalid synonym count. Expected " + synonyms.size() +
                   L" was " + to_wstring(count),
               synonyms.size(), count);

  shared_ptr<Set<wstring>> synonymSet =
      unordered_set<wstring>(Arrays::asList(synonyms));

  shared_ptr<BytesRef> scratchBytes = make_shared<BytesRef>();
  for (int i = 0; i < count; i++) {
    synonynMap->words->get(bytesReader->readVInt(), scratchBytes);
    wstring synonym = StringHelper::replace(scratchBytes->utf8ToString(),
                                            SynonymMap::WORD_SEPARATOR, L' ');
    assertTrue(L"Unexpected synonym found: " + synonym,
               synonymSet->contains(synonym));
  }
}

void BaseSynonymParserTestCase::assertEntryAbsent(
    shared_ptr<SynonymMap> synonynMap, const wstring &word) 
{
  word = StringHelper::replace(word, L' ', SynonymMap::WORD_SEPARATOR);
  shared_ptr<BytesRef> value =
      Util::get(synonynMap->fst, Util::toUTF32(make_shared<CharsRef>(word),
                                               make_shared<IntsRefBuilder>()));
  assertNull(L"There should be no synonyms for: " + word, value);
}

void BaseSynonymParserTestCase::assertEntryEquals(
    shared_ptr<SynonymMap> synonynMap, const wstring &word, bool includeOrig,
    const wstring &synonym) 
{
  assertEntryEquals(synonynMap, word, includeOrig,
                    std::deque<wstring>{synonym});
}

void BaseSynonymParserTestCase::assertAnalyzesToPositions(
    shared_ptr<Analyzer> a, const wstring &input, std::deque<wstring> &output,
    std::deque<wstring> &types, std::deque<int> &posIncrements,
    std::deque<int> &posLengths) 
{
  assertAnalyzesTo(a, input, output, nullptr, nullptr, types, posIncrements,
                   posLengths);
}
} // namespace org::apache::lucene::analysis::synonym