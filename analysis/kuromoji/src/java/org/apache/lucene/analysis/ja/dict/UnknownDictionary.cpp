using namespace std;

#include "UnknownDictionary.h"
#include "CharacterDefinition.h"

namespace org::apache::lucene::analysis::ja::dict
{

UnknownDictionary::UnknownDictionary()  : BinaryDictionary()
{
}

int UnknownDictionary::lookup(std::deque<wchar_t> &text, int offset, int len)
{
  if (!characterDefinition->isGroup(text[offset])) {
    return 1;
  }

  // Extract unknown word. Characters with the same character class are
  // considered to be part of unknown word
  char characterIdOfFirstCharacter =
      characterDefinition->getCharacterClass(text[offset]);
  int length = 1;
  for (int i = 1; i < len; i++) {
    if (characterIdOfFirstCharacter ==
        characterDefinition->getCharacterClass(text[offset + i])) {
      length++;
    } else {
      break;
    }
  }

  return length;
}

shared_ptr<CharacterDefinition> UnknownDictionary::getCharacterDefinition()
{
  return characterDefinition;
}

wstring UnknownDictionary::getReading(int wordId, std::deque<wchar_t> &surface,
                                      int off, int len)
{
  return L"";
}

wstring UnknownDictionary::getInflectionType(int wordId) { return L""; }

wstring UnknownDictionary::getInflectionForm(int wordId) { return L""; }

shared_ptr<UnknownDictionary> UnknownDictionary::getInstance()
{
  return SingletonHolder::INSTANCE;
}

const shared_ptr<UnknownDictionary>
    UnknownDictionary::SingletonHolder::INSTANCE;

UnknownDictionary::SingletonHolder::StaticConstructor::StaticConstructor()
{
  try {
    INSTANCE = make_shared<UnknownDictionary>();
  } catch (const IOException &ioe) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Cannot load
    // UnknownDictionary.", ioe);
    throw runtime_error(L"Cannot load UnknownDictionary.");
  }
}

SingletonHolder::StaticConstructor
    UnknownDictionary::SingletonHolder::staticConstructor;
} // namespace org::apache::lucene::analysis::ja::dict