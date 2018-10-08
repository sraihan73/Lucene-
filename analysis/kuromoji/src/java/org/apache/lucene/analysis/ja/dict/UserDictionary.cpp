using namespace std;

#include "UserDictionary.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../util/CSVUtil.h"
#include "TokenInfoFST.h"

namespace org::apache::lucene::analysis::ja::dict
{
using CSVUtil = org::apache::lucene::analysis::ja::util::CSVUtil;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

shared_ptr<UserDictionary>
UserDictionary::open(shared_ptr<Reader> reader) 
{

  shared_ptr<BufferedReader> br = make_shared<BufferedReader>(reader);
  wstring line = L"";
  deque<std::deque<wstring>> featureEntries = deque<std::deque<wstring>>();

  // text, segmentation, readings, POS
  while ((line = br->readLine()) != L"") {
    // Remove comments
    line = line.replaceAll(L"#.*$", L"");

    // Skip empty lines or comment lines
    if (StringHelper::trim(line)->length() == 0) {
      continue;
    }
    std::deque<wstring> values = CSVUtil::parse(line);
    featureEntries.push_back(values);
  }

  if (featureEntries.empty()) {
    return nullptr;
  } else {
    return make_shared<UserDictionary>(featureEntries);
  }
}

UserDictionary::UserDictionary(
    deque<std::deque<wstring>> &featureEntries) 
    : fst(make_shared<TokenInfoFST>(fstBuilder::finish(), false)),
      segmentations(segmentations.toArray(
          std::deque<std::deque<int>>(segmentations.size()))),
      data(data.toArray(std::deque<wstring>(data.size())))
{

  int wordId = CUSTOM_DICTIONARY_WORD_ID_OFFSET;
  // TODO: should we allow multiple segmentations per input 'phrase'?
  // the old treemap didn't support this either, and i'm not sure if it's
  // needed/useful?

  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(featureEntries, new
  // java.util.Comparator<std::wstring[]>() C++ TODO: You cannot use 'shared_from_this'
  // in a constructor:
  sort(featureEntries.begin(), featureEntries.end(),
       make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  deque<wstring> data = deque<wstring>(featureEntries.size());
  deque<std::deque<int>> segmentations =
      deque<std::deque<int>>(featureEntries.size());

  shared_ptr<PositiveIntOutputs> fstOutput = PositiveIntOutputs::getSingleton();
  shared_ptr<Builder<int64_t>> fstBuilder =
      make_shared<Builder<int64_t>>(FST::INPUT_TYPE::BYTE2, fstOutput);
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  int64_t ord = 0;

  for (auto values : featureEntries) {
    std::deque<wstring> segmentation =
        values[1].replaceAll(L"  *", L" ")->split(L" ");
    std::deque<wstring> readings =
        values[2].replaceAll(L"  *", L" ")->split(L" ");
    wstring pos = values[3];

    if (segmentation.size() != readings.size()) {
      throw runtime_error(
          L"Illegal user dictionary entry " + values[0] +
          L" - the number of segmentations (" + segmentation.size() + L")" +
          L" does not the match number of readings (" + readings.size() + L")");
    }

    std::deque<int> wordIdAndLength(segmentation.size() +
                                     1); // wordId offset, length, length....
    wordIdAndLength[0] = wordId;
    for (int i = 0; i < segmentation.size(); i++) {
      wordIdAndLength[i + 1] = segmentation[i].length();
      data.push_back(readings[i] + INTERNAL_SEPARATOR + pos);
      wordId++;
    }
    // add mapping to FST
    wstring token = values[0];
    scratch->grow(token.length());
    scratch->setLength(token.length());
    for (int i = 0; i < token.length(); i++) {
      scratch->setIntAt(i, static_cast<int>(token[i]));
    }
    fstBuilder->add(scratch->get(), ord);
    segmentations.push_back(wordIdAndLength);
    ord++;
  }
}

UserDictionary::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<UserDictionary> outerInstance)
{
  this->outerInstance = outerInstance;
}

int UserDictionary::ComparatorAnonymousInnerClass::compare(
    std::deque<wstring> &left, std::deque<wstring> &right)
{
  return left[0].compare(right[0]);
}

std::deque<std::deque<int>>
UserDictionary::lookup(std::deque<wchar_t> &chars, int off,
                       int len) 
{
  // TODO: can we avoid this treemap/toIndexArray?
  map_obj<int, std::deque<int>> result =
      map_obj<int, std::deque<int>>(); // index, [length, length...]
  bool found = false;               // true if we found any results

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
        result.emplace(startOffset - off, segmentations[finalOutput]);
        found = true;
      }
    }
  }

  return found ? toIndexArray(result) : EMPTY_RESULT;
}

shared_ptr<TokenInfoFST> UserDictionary::getFST() { return fst; }

std::deque<std::deque<int>> const UserDictionary::EMPTY_RESULT =
    std::deque<std::deque<int>>(0);

std::deque<std::deque<int>>
UserDictionary::toIndexArray(unordered_map<int, std::deque<int>> &input)
{
  deque<std::deque<int>> result = deque<std::deque<int>>();
  for (auto i : input) {
    std::deque<int> wordIdAndLength = input[i.first];
    int wordId = wordIdAndLength[0];
    // convert length to index
    int current = i.first;
    for (int j = 1; j < wordIdAndLength.size();
         j++) { // first entry is wordId offset
      std::deque<int> token = {wordId + j - 1, current, wordIdAndLength[j]};
      result.push_back(token);
      current += wordIdAndLength[j];
    }
  }
  return result.toArray(std::deque<std::deque<int>>(result.size()));
}

std::deque<int> UserDictionary::lookupSegmentation(int phraseID)
{
  return segmentations[phraseID];
}

int UserDictionary::getLeftId(int wordId) { return LEFT_ID; }

int UserDictionary::getRightId(int wordId) { return RIGHT_ID; }

int UserDictionary::getWordCost(int wordId) { return WORD_COST; }

wstring UserDictionary::getReading(int wordId, std::deque<wchar_t> &surface,
                                   int off, int len)
{
  return getFeature(wordId, {0});
}

wstring UserDictionary::getPartOfSpeech(int wordId)
{
  return getFeature(wordId, {1});
}

wstring UserDictionary::getBaseForm(int wordId, std::deque<wchar_t> &surface,
                                    int off, int len)
{
  return L""; // TODO: add support?
}

wstring UserDictionary::getPronunciation(int wordId,
                                         std::deque<wchar_t> &surface, int off,
                                         int len)
{
  return L""; // TODO: add support?
}

wstring UserDictionary::getInflectionType(int wordId)
{
  return L""; // TODO: add support?
}

wstring UserDictionary::getInflectionForm(int wordId)
{
  return L""; // TODO: add support?
}

std::deque<wstring> UserDictionary::getAllFeaturesArray(int wordId)
{
  wstring allFeatures = data[wordId - CUSTOM_DICTIONARY_WORD_ID_OFFSET];
  if (allFeatures == L"") {
    return nullptr;
  }

  return allFeatures.split(INTERNAL_SEPARATOR);
}

wstring UserDictionary::getFeature(int wordId, deque<int> &fields)
{
  std::deque<wstring> allFeatures = getAllFeaturesArray(wordId);
  if (allFeatures.empty()) {
    return L"";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  if (fields->length == 0) { // All features
    for (auto feature : allFeatures) {
      sb->append(CSVUtil::quoteEscape(feature))->append(L",");
    }
  } else if (fields->length == 1) { // One feature doesn't need to escape value
    sb->append(allFeatures[fields[0]])->append(L",");
  } else {
    for (int field : fields) {
      sb->append(CSVUtil::quoteEscape(allFeatures[field]))->append(L",");
    }
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sb->deleteCharAt(sb->length() - 1)->toString();
}
} // namespace org::apache::lucene::analysis::ja::dict