using namespace std;

#include "TestTokenInfoDictionary.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/UnicodeUtil.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/IntsRefFSTEnum.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/POS.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/dict/CharacterDefinition.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/dict/ConnectionCosts.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/dict/Dictionary.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/dict/TokenInfoDictionary.h"

namespace org::apache::lucene::analysis::ko::dict
{
using POS = org::apache::lucene::analysis::ko::POS;
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
  shared_ptr<CharacterDefinition> charDef = CharacterDefinition::getInstance();
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
    wstring surfaceForm = wstring(chars);
    assertTrue(UnicodeUtil::validUTF16String(surfaceForm));

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

      int leftId = tid->getLeftId(wordId);
      int rightId = tid->getRightId(wordId);

      matrix->get(rightId, leftId);

      tid->getWordCost(wordId);

      POS::Type type = tid->getPOSType(wordId);
      POS::Tag leftPOS = tid->getLeftPOS(wordId);
      POS::Tag rightPOS = tid->getRightPOS(wordId);

      if (type == POS::Type::MORPHEME) {
        assertTrue(leftPOS == rightPOS);
        wstring reading = tid->getReading(wordId);
        bool isHanja = charDef->isHanja(surfaceForm[0]);
        if (isHanja) {
          assertTrue(reading != L"");
          for (int j = 0; j < reading.length(); j++) {
            assertTrue(charDef->isHangul(reading[j]));
          }
        }
        if (reading != L"") {
          assertTrue(UnicodeUtil::validUTF16String(reading));
        }
      } else {
        if (type == POS::Type::COMPOUND) {
          assertTrue(leftPOS == rightPOS);
          assertTrue(leftPOS == POS::Tag::NNG || rightPOS == POS::Tag::NNP);
        }
        std::deque<std::shared_ptr<Dictionary::Morpheme>> decompound =
            tid->getMorphemes(wordId, chars, 0, chars.size());
        if (decompound.size() > 0) {
          int offset = 0;
          for (auto morph : decompound) {
            assertTrue(UnicodeUtil::validUTF16String(morph->surfaceForm));
            if (type != POS::Type::INFLECT) {
              assertEquals(
                  morph->surfaceForm,
                  surfaceForm.substr(offset, morph->surfaceForm.length()));
              offset += morph->surfaceForm.length();
            }
          }
          assertTrue(offset <= surfaceForm.length());
        }
      }
    }
  }
  if (VERBOSE) {
    wcout << L"checked " << numTerms << L" terms, " << numWords << L" words."
          << endl;
  }
}
} // namespace org::apache::lucene::analysis::ko::dict