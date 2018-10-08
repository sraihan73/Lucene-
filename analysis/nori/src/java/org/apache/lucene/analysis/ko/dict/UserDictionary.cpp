using namespace std;

#include "UserDictionary.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "CharacterDefinition.h"
#include "TokenInfoFST.h"

namespace org::apache::lucene::analysis::ko::dict
{
using POS = org::apache::lucene::analysis::ko::POS;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

shared_ptr<UserDictionary>
UserDictionary::open(shared_ptr<Reader> reader) 
{

  shared_ptr<BufferedReader> br = make_shared<BufferedReader>(reader);
  wstring line = L"";
  deque<wstring> entries = deque<wstring>();

  // text + optional segmentations
  while ((line = br->readLine()) != L"") {
    // Remove comments
    line = line.replaceAll(L"#.*$", L"");

    // Skip empty lines or comment lines
    if (StringHelper::trim(line)->length() == 0) {
      continue;
    }
    entries.push_back(line);
  }

  if (entries.empty()) {
    return nullptr;
  } else {
    return make_shared<UserDictionary>(entries);
  }
}

UserDictionary::UserDictionary(deque<wstring> &entries) 
    : fst(make_shared<TokenInfoFST>(fstBuilder::finish())),
      segmentations(segmentations.toArray(
          std::deque<std::deque<int>>(segmentations.size()))),
      rightIds(std::deque<short>(rightIds.size()))
{
  shared_ptr<CharacterDefinition> *const charDef =
      CharacterDefinition::getInstance();
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(entries, java.util.Comparator.comparing(e
  // -> e.split("\\s+")[0]));
  sort(entries.begin(), entries.end(),
       Comparator::comparing([&](any e) { e->split(L"\\s+")[0]; }));

  shared_ptr<PositiveIntOutputs> fstOutput = PositiveIntOutputs::getSingleton();
  shared_ptr<Builder<int64_t>> fstBuilder =
      make_shared<Builder<int64_t>>(FST::INPUT_TYPE::BYTE2, fstOutput);
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();

  wstring lastToken = L"";
  deque<std::deque<int>> segmentations =
      deque<std::deque<int>>(entries.size());
  deque<short> rightIds = deque<short>(entries.size());
  int64_t ord = 0;
  for (auto entry : entries) {
    std::deque<wstring> splits = entry.split(L"\\s+");
    wstring token = splits[0];
    if (lastToken != L"" && token == lastToken) {
      continue;
    }
    wchar_t lastChar = entry.charAt(entry.length() - 1);
    if (charDef->isHangul(lastChar)) {
      if (charDef->hasCoda(lastChar)) {
        rightIds.push_back(RIGHT_ID_T);
      } else {
        rightIds.push_back(RIGHT_ID_F);
      }
    } else {
      rightIds.push_back(RIGHT_ID);
    }

    if (splits.size() == 1) {
      segmentations.push_back(nullptr);
    } else {
      std::deque<int> length(splits.size() - 1);
      int offset = 0;
      for (int i = 1; i < splits.size(); i++) {
        length[i - 1] = splits[i].length();
        offset += splits[i].length();
      }
      if (offset > token.length()) {
        throw invalid_argument(
            L"Illegal user dictionary entry " + entry +
            L" - the segmentation is bigger than the surface form (" + token +
            L")");
      }
      segmentations.push_back(length);
    }

    // add mapping to FST
    scratch->grow(token.length());
    scratch->setLength(token.length());
    for (int i = 0; i < token.length(); i++) {
      scratch->setIntAt(i, static_cast<int>(token[i]));
    }
    fstBuilder->add(scratch->get(), ord);
    lastToken = token;
    ord++;
  }
  for (int i = 0; i < rightIds.size(); i++) {
    this->rightIds[i] = rightIds[i];
  }
}

shared_ptr<TokenInfoFST> UserDictionary::getFST() { return fst; }

int UserDictionary::getLeftId(int wordId) { return LEFT_ID; }

int UserDictionary::getRightId(int wordId) { return rightIds[wordId]; }

int UserDictionary::getWordCost(int wordId) { return WORD_COST; }

POS::Type UserDictionary::getPOSType(int wordId)
{
  if (segmentations[wordId] == nullptr) {
    return POS::Type::MORPHEME;
  } else {
    return POS::Type::COMPOUND;
  }
}

POS::Tag UserDictionary::getLeftPOS(int wordId) { return POS::Tag::NNG; }

POS::Tag UserDictionary::getRightPOS(int wordId) { return POS::Tag::NNG; }

wstring UserDictionary::getReading(int wordId) { return L""; }

std::deque<std::shared_ptr<Morpheme>>
UserDictionary::getMorphemes(int wordId, std::deque<wchar_t> &surfaceForm,
                             int off, int len)
{
  std::deque<int> segs = segmentations[wordId];
  if (segs.empty()) {
    return nullptr;
  }
  int offset = 0;
  std::deque<std::shared_ptr<Morpheme>> morphemes(segs.size());
  for (int i = 0; i < segs.size(); i++) {
    morphemes[i] = make_shared<Morpheme>(
        POS::Tag::NNG, wstring(surfaceForm, off + offset, segs[i]));
    offset += segs[i];
  }
  return morphemes;
}

deque<int> UserDictionary::lookup(std::deque<wchar_t> &chars, int off,
                                   int len) 
{
  deque<int> result = deque<int>();
  shared_ptr<FST::BytesReader> *const fstReader = fst->getBytesReader();

  shared_ptr<FST::Arc<int64_t>> arc = make_shared<FST::Arc<int64_t>>();
  int end = off + len;
  for (int startOffset = off; startOffset < end; startOffset++) {
    arc = fst->getFirstArc(arc);
    int output = 0;
    int remaining = end - startOffset;
    for (int i = 0; i < remaining; i++) {
      int ch = chars[startOffset + i];
      if (fst->findTargetArc(ch, arc, arc, i == 0, fstReader) == nullptr) {
        break; // continue to next position
      }
      output += arc->output.intValue();
      if (arc->isFinal()) {
        constexpr int finalOutput = output + arc->nextFinalOutput.intValue();
        result.push_back(finalOutput);
      }
    }
  }
  return result;
}
} // namespace org::apache::lucene::analysis::ko::dict