using namespace std;

#include "UnknownDictionaryWriter.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/dict/CharacterDefinition.h"
#include "CharacterDefinitionWriter.h"

namespace org::apache::lucene::analysis::ja::util
{
using CharacterDefinition =
    org::apache::lucene::analysis::ja::dict::CharacterDefinition;
using UnknownDictionary =
    org::apache::lucene::analysis::ja::dict::UnknownDictionary;

UnknownDictionaryWriter::UnknownDictionaryWriter(int size)
    : BinaryDictionaryWriter(UnknownDictionary::class, size)
{
}

int UnknownDictionaryWriter::put(std::deque<wstring> &entry)
{
  // Get wordId of current entry
  int wordId = buffer->position();

  // Put entry
  int result = BinaryDictionaryWriter::put(entry);

  // Put entry in targetMap
  int characterId = CharacterDefinition::lookupCharacterClass(entry[0]);
  addMapping(characterId, wordId);
  return result;
}

void UnknownDictionaryWriter::putCharacterCategory(
    int codePoint, const wstring &characterClassName)
{
  characterDefinition->putCharacterCategory(codePoint, characterClassName);
}

void UnknownDictionaryWriter::putInvokeDefinition(
    const wstring &characterClassName, int invoke, int group, int length)
{
  characterDefinition->putInvokeDefinition(characterClassName, invoke, group,
                                           length);
}

void UnknownDictionaryWriter::write(const wstring &baseDir) 
{
  BinaryDictionaryWriter::write(baseDir);
  characterDefinition->write(baseDir);
}
} // namespace org::apache::lucene::analysis::ja::util