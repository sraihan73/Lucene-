using namespace std;

#include "CharacterDefinitionWriter.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/OutputStreamDataOutput.h"

namespace org::apache::lucene::analysis::ja::util
{
using CharacterDefinition =
    org::apache::lucene::analysis::ja::dict::CharacterDefinition;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataOutput = org::apache::lucene::store::DataOutput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;

CharacterDefinitionWriter::CharacterDefinitionWriter()
{
  Arrays::fill(characterCategoryMap, CharacterDefinition::DEFAULT);
}

void CharacterDefinitionWriter::putCharacterCategory(
    int codePoint, const wstring &characterClassName)
{
  characterClassName = characterClassName.split(L" ")[0]; // use first
  // category
  // class

  // Override Nakaguro
  if (codePoint == 0x30FB) {
    characterClassName = L"SYMBOL";
  }
  characterCategoryMap[codePoint] =
      CharacterDefinition::lookupCharacterClass(characterClassName);
}

void CharacterDefinitionWriter::putInvokeDefinition(
    const wstring &characterClassName, int invoke, int group, int length)
{
  constexpr char characterClass =
      CharacterDefinition::lookupCharacterClass(characterClassName);
  invokeMap[characterClass] = invoke == 1;
  groupMap[characterClass] = group == 1;
  // TODO: length def ignored
}

void CharacterDefinitionWriter::write(const wstring &baseDir) 
{
  wstring filename = baseDir + File::separator +
                     CharacterDefinition::typeid->getName()->replace(
                         L'.', FileSystem::preferredSeparator()) +
                     CharacterDefinition::FILENAME_SUFFIX;
  (make_shared<File>(filename))->getParentFile().mkdirs();
  shared_ptr<OutputStream> os = make_shared<FileOutputStream>(filename);
  try {
    os = make_shared<BufferedOutputStream>(os);
    shared_ptr<DataOutput> *const out = make_shared<OutputStreamDataOutput>(os);
    CodecUtil::writeHeader(out, CharacterDefinition::HEADER,
                           CharacterDefinition::VERSION);
    out->writeBytes(characterCategoryMap, 0, characterCategoryMap.size());
    for (int i = 0; i < CharacterDefinition::CLASS_COUNT; i++) {
      constexpr char b = static_cast<char>((invokeMap[i] ? 0x01 : 0x00) |
                                           (groupMap[i] ? 0x02 : 0x00));
      out->writeByte(b);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    os->close();
  }
}
} // namespace org::apache::lucene::analysis::ja::util