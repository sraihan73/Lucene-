using namespace std;

#include "TestPlainTextDictionary.h"

namespace org::apache::lucene::search::spell
{
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPlainTextDictionary::testBuild() 
{
  const wstring LF = System::getProperty(L"line.separator");
  wstring input = L"oneword" + LF + L"twoword" + LF + L"threeword";
  shared_ptr<PlainTextDictionary> ptd =
      make_shared<PlainTextDictionary>(make_shared<StringReader>(input));
  shared_ptr<Directory> ramDir = newDirectory();
  shared_ptr<SpellChecker> spellChecker = make_shared<SpellChecker>(ramDir);
  spellChecker->indexDictionary(ptd, newIndexWriterConfig(nullptr), false);
  std::deque<wstring> similar = spellChecker->suggestSimilar(L"treeword", 2);
  assertEquals(2, similar.size());
  assertEquals(similar[0], L"threeword");
  assertEquals(similar[1], L"oneword");
  delete spellChecker;
  delete ramDir;
}
} // namespace org::apache::lucene::search::spell