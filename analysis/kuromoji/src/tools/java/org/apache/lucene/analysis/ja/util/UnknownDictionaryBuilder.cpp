using namespace std;

#include "UnknownDictionaryBuilder.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/dict/CharacterDefinition.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/util/CSVUtil.h"
#include "UnknownDictionaryWriter.h"

namespace org::apache::lucene::analysis::ja::util
{
using CharacterDefinition =
    org::apache::lucene::analysis::ja::dict::CharacterDefinition;
const wstring UnknownDictionaryBuilder::NGRAM_DICTIONARY_ENTRY =
    L"NGRAM,5,5,-32768,記号,一般,*,*,*,*,*,*,*";

UnknownDictionaryBuilder::UnknownDictionaryBuilder(const wstring &encoding)
{
  this->encoding = encoding;
}

shared_ptr<UnknownDictionaryWriter>
UnknownDictionaryBuilder::build(const wstring &dirname) 
{
  shared_ptr<UnknownDictionaryWriter> unkDictionary = readDictionaryFile(
      dirname + File::separator + L"unk.def"); // Should be only one file
  readCharacterDefinition(dirname + File::separator + L"char.def",
                          unkDictionary);
  return unkDictionary;
}

shared_ptr<UnknownDictionaryWriter>
UnknownDictionaryBuilder::readDictionaryFile(const wstring &filename) throw(
    IOException)
{
  return readDictionaryFile(filename, encoding);
}

shared_ptr<UnknownDictionaryWriter>
UnknownDictionaryBuilder::readDictionaryFile(
    const wstring &filename, const wstring &encoding) 
{
  shared_ptr<UnknownDictionaryWriter> dictionary =
      make_shared<UnknownDictionaryWriter>(5 * 1024 * 1024);

  shared_ptr<FileInputStream> inputStream =
      make_shared<FileInputStream>(filename);
  shared_ptr<Charset> cs = Charset::forName(encoding);
  shared_ptr<CharsetDecoder> decoder =
      cs->newDecoder()
          .onMalformedInput(CodingErrorAction::REPORT)
          .onUnmappableCharacter(CodingErrorAction::REPORT);
  shared_ptr<InputStreamReader> streamReader =
      make_shared<InputStreamReader>(inputStream, decoder);
  shared_ptr<LineNumberReader> lineReader =
      make_shared<LineNumberReader>(streamReader);

  dictionary->put(CSVUtil::parse(NGRAM_DICTIONARY_ENTRY));

  deque<std::deque<wstring>> lines = deque<std::deque<wstring>>();
  wstring line = L"";
  while ((line = lineReader->readLine()) != L"") {
    // note: unk.def only has 10 fields, it simplifies the writer to just append
    // empty reading and pronunciation, even though the unknown dictionary
    // returns hardcoded null here.
    const std::deque<wstring> parsed = CSVUtil::parse(
        line + L",*,*"); // Probably we don't need to validate entry
    lines.push_back(parsed);
  }

  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(lines, new java.util.Comparator<std::wstring[]>()
  sort(lines.begin(), lines.end(),
       make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  for (auto entry : lines) {
    dictionary->put(entry);
  }

  return dictionary;
}

UnknownDictionaryBuilder::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<UnknownDictionaryBuilder> outerInstance)
{
  this->outerInstance = outerInstance;
}

int UnknownDictionaryBuilder::ComparatorAnonymousInnerClass::compare(
    std::deque<wstring> &left, std::deque<wstring> &right)
{
  int leftId = CharacterDefinition::lookupCharacterClass(left[0]);
  int rightId = CharacterDefinition::lookupCharacterClass(right[0]);
  return leftId - rightId;
}

void UnknownDictionaryBuilder::readCharacterDefinition(
    const wstring &filename,
    shared_ptr<UnknownDictionaryWriter> dictionary) 
{
  shared_ptr<FileInputStream> inputStream =
      make_shared<FileInputStream>(filename);
  shared_ptr<InputStreamReader> streamReader =
      make_shared<InputStreamReader>(inputStream, encoding);
  shared_ptr<LineNumberReader> lineReader =
      make_shared<LineNumberReader>(streamReader);

  wstring line = L"";

  while ((line = lineReader->readLine()) != L"") {
    line = line.replaceAll(L"^\\s", L"");
    line = line.replaceAll(L"\\s*#.*", L"");
    line = line.replaceAll(L"\\s+", L" ");

    // Skip empty line or comment line
    if (line.length() == 0) {
      continue;
    }

    if (StringHelper::startsWith(line, L"0x")) { // Category mapping
      std::deque<wstring> values =
          line.split(L" ", 2); // Split only first space

      if (!values[0].find(L"..") != wstring::npos) {
        int cp = Integer::decode(values[0]).intValue();
        dictionary->putCharacterCategory(cp, values[1]);
      } else {
        std::deque<wstring> codePoints = values[0].split(L"\\.\\.");
        int cpFrom = Integer::decode(codePoints[0]).intValue();
        int cpTo = Integer::decode(codePoints[1]).intValue();

        for (int i = cpFrom; i <= cpTo; i++) {
          dictionary->putCharacterCategory(i, values[1]);
        }
      }
    } else { // Invoke definition
      std::deque<wstring> values =
          line.split(L" "); // Consecutive space is merged above
      wstring characterClassName = values[0];
      int invoke = stoi(values[1]);
      int group = stoi(values[2]);
      int length = stoi(values[3]);
      dictionary->putInvokeDefinition(characterClassName, invoke, group,
                                      length);
    }
  }
}
} // namespace org::apache::lucene::analysis::ja::util