using namespace std;

#include "UnknownDictionary.h"
#include "CharacterDefinition.h"

namespace org::apache::lucene::analysis::ko::dict
{

UnknownDictionary::UnknownDictionary()  : BinaryDictionary()
{
}

shared_ptr<CharacterDefinition> UnknownDictionary::getCharacterDefinition()
{
  return characterDefinition;
}

shared_ptr<UnknownDictionary> UnknownDictionary::getInstance()
{
  return SingletonHolder::INSTANCE;
}

wstring UnknownDictionary::getReading(int wordId) { return L""; }

std::deque<std::shared_ptr<Morpheme>>
UnknownDictionary::getMorphemes(int wordId, std::deque<wchar_t> &surfaceForm,
                                int off, int len)
{
  return nullptr;
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
} // namespace org::apache::lucene::analysis::ko::dict