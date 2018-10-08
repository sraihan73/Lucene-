using namespace std;

#include "Test64kAffixes.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/Dictionary.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/Stemmer.h"

namespace org::apache::lucene::analysis::hunspell
{
using Directory = org::apache::lucene::store::Directory;
using CharsRef = org::apache::lucene::util::CharsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void Test64kAffixes::test() 
{
  shared_ptr<Path> tempDir = createTempDir(L"64kaffixes");
  shared_ptr<Path> affix = tempDir->resolve(L"64kaffixes.aff");
  shared_ptr<Path> dict = tempDir->resolve(L"64kaffixes.dic");

  shared_ptr<BufferedWriter> affixWriter =
      Files::newBufferedWriter(affix, StandardCharsets::UTF_8);

  // 65k affixes with flag 1, then an affix with flag 2
  affixWriter->write(L"SET UTF-8\nFLAG num\nSFX 1 Y 65536\n");
  for (int i = 0; i < 65536; i++) {
    affixWriter->write(L"SFX 1 0 " + Integer::toHexString(i) + L" .\n");
  }
  affixWriter->write(L"SFX 2 Y 1\nSFX 2 0 s\n");
  affixWriter->close();

  shared_ptr<BufferedWriter> dictWriter =
      Files::newBufferedWriter(dict, StandardCharsets::UTF_8);

  // drink signed with affix 2 (takes -s)
  dictWriter->write(L"1\ndrink/2\n");
  dictWriter->close();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream affStream =
  // java.nio.file.Files.newInputStream(affix); java.io.InputStream dictStream =
  // java.nio.file.Files.newInputStream(dict); org.apache.lucene.store.Directory
  // tempDir2 = newDirectory())
  {
    java::io::InputStream affStream =
        java::nio::file::Files::newInputStream(affix);
    java::io::InputStream dictStream =
        java::nio::file::Files::newInputStream(dict);
    org::apache::lucene::store::Directory tempDir2 = newDirectory();
    shared_ptr<Dictionary> dictionary =
        make_shared<Dictionary>(tempDir2, L"dictionary", affStream, dictStream);
    shared_ptr<Stemmer> stemmer = make_shared<Stemmer>(dictionary);
    // drinks should still stem to drink
    deque<std::shared_ptr<CharsRef>> stems = stemmer->stem(L"drinks");
    assertEquals(1, stems.size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"drink", stems[0]->toString());
  }
}
} // namespace org::apache::lucene::analysis::hunspell