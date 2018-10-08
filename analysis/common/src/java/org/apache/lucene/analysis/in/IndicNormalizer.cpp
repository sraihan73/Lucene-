using namespace std;

#include "IndicNormalizer.h"

namespace org::apache::lucene::analysis::in_
{
//    import static Character.UnicodeBlock.*;
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

IndicNormalizer::ScriptData::ScriptData(int flag, int base)
    : flag(flag), base(base)
{
}

const shared_ptr<java::util::IdentityHashMap<
    std::shared_ptr<Character::UnicodeBlock>, std::shared_ptr<ScriptData>>>
    IndicNormalizer::scripts = make_shared<java::util::IdentityHashMap<
        std::shared_ptr<Character::UnicodeBlock>, std::shared_ptr<ScriptData>>>(
        9);

int IndicNormalizer::flag(shared_ptr<Character::UnicodeBlock> ub)
{
  return scripts->get(ub).flag;
}

IndicNormalizer::StaticConstructor::StaticConstructor()
{
  scripts->put(DEVANAGARI, make_shared<ScriptData>(1, 0x0900));
  scripts->put(BENGALI, make_shared<ScriptData>(2, 0x0980));
  scripts->put(GURMUKHI, make_shared<ScriptData>(4, 0x0A00));
  scripts->put(GUJARATI, make_shared<ScriptData>(8, 0x0A80));
  scripts->put(ORIYA, make_shared<ScriptData>(16, 0x0B00));
  scripts->put(TAMIL, make_shared<ScriptData>(32, 0x0B80));
  scripts->put(TELUGU, make_shared<ScriptData>(64, 0x0C00));
  scripts->put(KANNADA, make_shared<ScriptData>(128, 0x0C80));
  scripts->put(MALAYALAM, make_shared<ScriptData>(256, 0x0D00));
  for (auto sd : scripts) {
    sd->second->decompMask = make_shared<BitSet>(0x7F);
    for (int i = 0; i < decompositions.size(); i++) {
      constexpr int ch = decompositions[i][0];
      constexpr int flags = decompositions[i][4];
      if ((flags & sd->second.flag) != 0) {
        sd->second.decompMask.set(ch);
      }
    }
  }
}

IndicNormalizer::StaticConstructor IndicNormalizer::staticConstructor;
std::deque<std::deque<int>> const IndicNormalizer::decompositions = {
    {0x05, 0x3E, 0x45, 0x11, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x05, 0x3E, 0x46, 0x12, flag(DEVANAGARI)},
    {0x05, 0x3E, 0x47, 0x13, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x05, 0x3E, 0x48, 0x14, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x05, 0x3E, -1, 0x06,
     flag(DEVANAGARI) | flag(BENGALI) | flag(GURMUKHI) | flag(GUJARATI) |
         flag(ORIYA)},
    {0x05, 0x45, -1, 0x72, flag(DEVANAGARI)},
    {0x05, 0x45, -1, 0x0D, flag(GUJARATI)},
    {0x05, 0x46, -1, 0x04, flag(DEVANAGARI)},
    {0x05, 0x47, -1, 0x0F, flag(GUJARATI)},
    {0x05, 0x48, -1, 0x10, flag(GURMUKHI) | flag(GUJARATI)},
    {0x05, 0x49, -1, 0x11, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x05, 0x4A, -1, 0x12, flag(DEVANAGARI)},
    {0x05, 0x4B, -1, 0x13, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x05, 0x4C, -1, 0x14, flag(DEVANAGARI) | flag(GURMUKHI) | flag(GUJARATI)},
    {0x06, 0x45, -1, 0x11, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x06, 0x46, -1, 0x12, flag(DEVANAGARI)},
    {0x06, 0x47, -1, 0x13, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x06, 0x48, -1, 0x14, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x07, 0x57, -1, 0x08, flag(MALAYALAM)},
    {0x09, 0x41, -1, 0x0A, flag(DEVANAGARI)},
    {0x09, 0x57, -1, 0x0A, flag(TAMIL) | flag(MALAYALAM)},
    {0x0E, 0x46, -1, 0x10, flag(MALAYALAM)},
    {0x0F, 0x45, -1, 0x0D, flag(DEVANAGARI)},
    {0x0F, 0x46, -1, 0x0E, flag(DEVANAGARI)},
    {0x0F, 0x47, -1, 0x10, flag(DEVANAGARI)},
    {0x0F, 0x57, -1, 0x10, flag(ORIYA)},
    {0x12, 0x3E, -1, 0x13, flag(MALAYALAM)},
    {0x12, 0x4C, -1, 0x14, flag(TELUGU) | flag(KANNADA)},
    {0x12, 0x55, -1, 0x13, flag(TELUGU)},
    {0x12, 0x57, -1, 0x14, flag(TAMIL) | flag(MALAYALAM)},
    {0x13, 0x57, -1, 0x14, flag(ORIYA)},
    {0x15, 0x3C, -1, 0x58, flag(DEVANAGARI)},
    {0x16, 0x3C, -1, 0x59, flag(DEVANAGARI) | flag(GURMUKHI)},
    {0x17, 0x3C, -1, 0x5A, flag(DEVANAGARI) | flag(GURMUKHI)},
    {0x1C, 0x3C, -1, 0x5B, flag(DEVANAGARI) | flag(GURMUKHI)},
    {0x21, 0x3C, -1, 0x5C, flag(DEVANAGARI) | flag(BENGALI) | flag(ORIYA)},
    {0x22, 0x3C, -1, 0x5D, flag(DEVANAGARI) | flag(BENGALI) | flag(ORIYA)},
    {0x23, 0x4D, 0xFF, 0x7A, flag(MALAYALAM)},
    {0x24, 0x4D, 0xFF, 0x4E, flag(BENGALI)},
    {0x28, 0x3C, -1, 0x29, flag(DEVANAGARI)},
    {0x28, 0x4D, 0xFF, 0x7B, flag(MALAYALAM)},
    {0x2B, 0x3C, -1, 0x5E, flag(DEVANAGARI) | flag(GURMUKHI)},
    {0x2F, 0x3C, -1, 0x5F, flag(DEVANAGARI) | flag(BENGALI)},
    {0x2C, 0x41, 0x41, 0x0B, flag(TELUGU)},
    {0x30, 0x3C, -1, 0x31, flag(DEVANAGARI)},
    {0x30, 0x4D, 0xFF, 0x7C, flag(MALAYALAM)},
    {0x32, 0x4D, 0xFF, 0x7D, flag(MALAYALAM)},
    {0x33, 0x3C, -1, 0x34, flag(DEVANAGARI)},
    {0x33, 0x4D, 0xFF, 0x7E, flag(MALAYALAM)},
    {0x35, 0x41, -1, 0x2E, flag(TELUGU)},
    {0x3E, 0x45, -1, 0x49, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x3E, 0x46, -1, 0x4A, flag(DEVANAGARI)},
    {0x3E, 0x47, -1, 0x4B, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x3E, 0x48, -1, 0x4C, flag(DEVANAGARI) | flag(GUJARATI)},
    {0x3F, 0x55, -1, 0x40, flag(KANNADA)},
    {0x41, 0x41, -1, 0x42, flag(GURMUKHI)},
    {0x46, 0x3E, -1, 0x4A, flag(TAMIL) | flag(MALAYALAM)},
    {0x46, 0x42, 0x55, 0x4B, flag(KANNADA)},
    {0x46, 0x42, -1, 0x4A, flag(KANNADA)},
    {0x46, 0x46, -1, 0x48, flag(MALAYALAM)},
    {0x46, 0x55, -1, 0x47, flag(TELUGU) | flag(KANNADA)},
    {0x46, 0x56, -1, 0x48, flag(TELUGU) | flag(KANNADA)},
    {0x46, 0x57, -1, 0x4C, flag(TAMIL) | flag(MALAYALAM)},
    {0x47, 0x3E, -1, 0x4B,
     flag(BENGALI) | flag(ORIYA) | flag(TAMIL) | flag(MALAYALAM)},
    {0x47, 0x57, -1, 0x4C, flag(BENGALI) | flag(ORIYA)},
    {0x4A, 0x55, -1, 0x4B, flag(KANNADA)},
    {0x72, 0x3F, -1, 0x07, flag(GURMUKHI)},
    {0x72, 0x40, -1, 0x08, flag(GURMUKHI)},
    {0x72, 0x47, -1, 0x0F, flag(GURMUKHI)},
    {0x73, 0x41, -1, 0x09, flag(GURMUKHI)},
    {0x73, 0x42, -1, 0x0A, flag(GURMUKHI)},
    {0x73, 0x4B, -1, 0x13, flag(GURMUKHI)}};

int IndicNormalizer::normalize(std::deque<wchar_t> &text, int len)
{
  for (int i = 0; i < len; i++) {
    shared_ptr<Character::UnicodeBlock> *const block =
        Character::UnicodeBlock::of(text[i]);
    shared_ptr<ScriptData> *const sd = scripts->get(block);
    if (sd != nullptr) {
      constexpr int ch = text[i] - sd->base;
      if (sd->decompMask->get(ch)) {
        len = compose(ch, block, sd, text, i, len);
      }
    }
  }
  return len;
}

int IndicNormalizer::compose(int ch0,
                             shared_ptr<Character::UnicodeBlock> block0,
                             shared_ptr<ScriptData> sd,
                             std::deque<wchar_t> &text, int pos, int len)
{
  if (pos + 1 >= len) // need at least 2 chars!
  {
    return len;
  }

  constexpr int ch1 = text[pos + 1] - sd->base;
  shared_ptr<Character::UnicodeBlock> *const block1 =
      Character::UnicodeBlock::of(text[pos + 1]);
  if (block1 != block0) // needs to be the same writing system
  {
    return len;
  }

  int ch2 = -1;

  if (pos + 2 < len) {
    ch2 = text[pos + 2] - sd->base;
    shared_ptr<Character::UnicodeBlock> block2 =
        Character::UnicodeBlock::of(text[pos + 2]);
    if (text[pos + 2] == L'\u200D') // ZWJ
    {
      ch2 = 0xFF;
    } else if (block2 != block1) // still allow a 2-char match
    {
      ch2 = -1;
    }
  }

  for (int i = 0; i < decompositions.size(); i++) {
    if (decompositions[i][0] == ch0 && (decompositions[i][4] & sd->flag) != 0) {
      if (decompositions[i][1] == ch1 &&
          (decompositions[i][2] < 0 || decompositions[i][2] == ch2)) {
        text[pos] = static_cast<wchar_t>(sd->base + decompositions[i][3]);
        len = StemmerUtil::delete (text, pos + 1, len);
        if (decompositions[i][2] >= 0) {
          len = StemmerUtil::delete (text, pos + 1, len);
        }
        return len;
      }
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::in_