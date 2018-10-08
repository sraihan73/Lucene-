using namespace std;

#include "TestTokenInfoDictionary.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/UnicodeUtil.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/IntsRefFSTEnum.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ja/dict/ConnectionCosts.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ja/dict/TokenInfoDictionary.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ja/util/ToStringUtil.h"

namespace org::apache::lucene::analysis::ja::dict
{
using ToStringUtil = org::apache::lucene::analysis::ja::util::ToStringUtil;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using FST = org::apache::lucene::util::fst::FST;
using IntsRefFSTEnum = org::apache::lucene::util::fst::IntsRefFSTEnum;
using org::apache::lucene::util::fst::IntsRefFSTEnum::InputOutput;

void TestTokenInfoDictionary::testEnumerateAll() 
{
  // just for debugging
  int numTerms = 0;
  int numWords = 0;
  int lastWordId = -1;
  int lastSourceId = -1;
  shared_ptr<TokenInfoDictionary> tid = TokenInfoDictionary::getInstance();
  shared_ptr<ConnectionCosts> matrix = ConnectionCosts::getInstance();
  shared_ptr<FST<int64_t>> fst = tid->getFST()->getInternalFST();
  shared_ptr<IntsRefFSTEnum<int64_t>> fstEnum =
      make_shared<IntsRefFSTEnum<int64_t>>(fst);
  shared_ptr<IntsRefFSTEnum::InputOutput<int64_t>> mapping;
  shared_ptr<IntsRef> scratch = make_shared<IntsRef>();
  while ((mapping = fstEnum->next()) != nullptr) {
    numTerms++;
    shared_ptr<IntsRef> input = mapping->input;
    std::deque<wchar_t> chars(input->length);
    for (int i = 0; i < chars.size(); i++) {
      chars[i] = static_cast<wchar_t>(input->ints[input->offset + i]);
    }
    assertTrue(UnicodeUtil::validUTF16String(wstring(chars)));

    optional<int64_t> output = mapping->output;
    int sourceId = output.value();
    // we walk in order, terms, sourceIds, and wordIds should always be
    // increasing
    assertTrue(sourceId > lastSourceId);
    lastSourceId = sourceId;
    tid->lookupWordIds(sourceId, scratch);
    for (int i = 0; i < scratch->length; i++) {
      numWords++;
      int wordId = scratch->ints[scratch->offset + i];
      assertTrue(wordId > lastWordId);
      lastWordId = wordId;

      wstring baseForm = tid->getBaseForm(wordId, chars, 0, chars.size());
      assertTrue(baseForm == L"" || UnicodeUtil::validUTF16String(baseForm));

      wstring inflectionForm = tid->getInflectionForm(wordId);
      assertTrue(inflectionForm == L"" ||
                 UnicodeUtil::validUTF16String(inflectionForm));
      if (inflectionForm != L"") {
        // check that it's actually an ipadic inflection form
        assertNotNull(
            ToStringUtil::getInflectedFormTranslation(inflectionForm));
      }

      wstring inflectionType = tid->getInflectionType(wordId);
      assertTrue(inflectionType == L"" ||
                 UnicodeUtil::validUTF16String(inflectionType));
      if (inflectionType != L"") {
        // check that it's actually an ipadic inflection type
        assertNotNull(
            ToStringUtil::getInflectionTypeTranslation(inflectionType));
      }

      int leftId = tid->getLeftId(wordId);
      int rightId = tid->getRightId(wordId);

      matrix->get(rightId, leftId);

      tid->getWordCost(wordId);

      wstring pos = tid->getPartOfSpeech(wordId);
      assertNotNull(pos);
      assertTrue(UnicodeUtil::validUTF16String(pos));
      // check that it's actually an ipadic pos tag
      assertNotNull(ToStringUtil::getPOSTranslation(pos));

      wstring pronunciation =
          tid->getPronunciation(wordId, chars, 0, chars.size());
      assertNotNull(pronunciation);
      assertTrue(UnicodeUtil::validUTF16String(pronunciation));

      wstring reading = tid->getReading(wordId, chars, 0, chars.size());
      assertNotNull(reading);
      assertTrue(UnicodeUtil::validUTF16String(reading));
    }
  }
  if (VERBOSE) {
    wcout << L"checked " << numTerms << L" terms, " << numWords << L" words."
          << endl;
  }
}
} // namespace org::apache::lucene::analysis::ja::dict