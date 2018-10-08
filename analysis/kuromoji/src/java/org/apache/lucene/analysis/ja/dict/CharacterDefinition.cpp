using namespace std;

#include "CharacterDefinition.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/store/InputStreamDataInput.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "BinaryDictionary.h"

namespace org::apache::lucene::analysis::ja::dict
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataInput = org::apache::lucene::store::DataInput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring CharacterDefinition::FILENAME_SUFFIX = L".dat";
const wstring CharacterDefinition::HEADER = L"kuromoji_cd";

CharacterDefinition::CharacterDefinition() 
{
  shared_ptr<InputStream> is = nullptr;
  bool success = false;
  try {
    is = BinaryDictionary::getClassResource(getClass(), FILENAME_SUFFIX);
    is = make_shared<BufferedInputStream>(is);
    shared_ptr<DataInput> *const in_ = make_shared<InputStreamDataInput>(is);
    CodecUtil::checkHeader(in_, HEADER, VERSION, VERSION);
    in_->readBytes(characterCategoryMap, 0, characterCategoryMap.size());
    for (int i = 0; i < CLASS_COUNT; i++) {
      constexpr char b = in_->readByte();
      invokeMap[i] = (b & 0x01) != 0;
      groupMap[i] = (b & 0x02) != 0;
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({is});
    } else {
      IOUtils::closeWhileHandlingException({is});
    }
  }
}

char CharacterDefinition::getCharacterClass(wchar_t c)
{
  return characterCategoryMap[c];
}

bool CharacterDefinition::isInvoke(wchar_t c)
{
  return invokeMap[characterCategoryMap[c]];
}

bool CharacterDefinition::isGroup(wchar_t c)
{
  return groupMap[characterCategoryMap[c]];
}

bool CharacterDefinition::isKanji(wchar_t c)
{
  constexpr char characterClass = characterCategoryMap[c];
  return characterClass == KANJI || characterClass == KANJINUMERIC;
}

char CharacterDefinition::lookupCharacterClass(
    const wstring &characterClassName)
{
  return static_cast<char>(
      CharacterClass::valueOf(characterClassName).ordinal());
}

shared_ptr<CharacterDefinition> CharacterDefinition::getInstance()
{
  return SingletonHolder::INSTANCE;
}

const shared_ptr<CharacterDefinition>
    CharacterDefinition::SingletonHolder::INSTANCE;

CharacterDefinition::SingletonHolder::StaticConstructor::StaticConstructor()
{
  try {
    INSTANCE = make_shared<CharacterDefinition>();
  } catch (const IOException &ioe) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Cannot load
    // CharacterDefinition.", ioe);
    throw runtime_error(L"Cannot load CharacterDefinition.");
  }
}

SingletonHolder::StaticConstructor
    CharacterDefinition::SingletonHolder::staticConstructor;
} // namespace org::apache::lucene::analysis::ja::dict