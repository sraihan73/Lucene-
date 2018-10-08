using namespace std;

#include "TestAllDictionaries.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/RamUsageTester.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/Dictionary.h"

namespace org::apache::lucene::analysis::hunspell
{
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Ignore;
const shared_ptr<java::nio::file::Path> TestAllDictionaries::DICTIONARY_HOME =
    java::nio::file::Paths->get(L"/data/archive.services.openoffice.org/pub/"
                                L"mirror/OpenOffice.org/contrib/dictionaries");

void TestAllDictionaries::test() 
{
  shared_ptr<Path> tmp = LuceneTestCase::createTempDir();

  for (int i = 0; i < tests.size(); i += 3) {
    shared_ptr<Path> f = DICTIONARY_HOME->resolve(tests[i]);
    assert(Files::exists(f));

    IOUtils::rm({tmp});
    Files::createDirectory(tmp);

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream in =
    // java.nio.file.Files.newInputStream(f); org.apache.lucene.store.Directory
    // tempDir = getDirectory())
    {
      java::io::InputStream in_ = java::nio::file::Files::newInputStream(f);
      org::apache::lucene::store::Directory tempDir = getDirectory();
      TestUtil::unzip(in_, tmp);
      shared_ptr<Path> dicEntry = tmp->resolve(tests[i + 1]);
      shared_ptr<Path> affEntry = tmp->resolve(tests[i + 2]);

      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (java.io.InputStream dictionary =
      // java.nio.file.Files.newInputStream(dicEntry); java.io.InputStream affix
      // = java.nio.file.Files.newInputStream(affEntry))
      {
        java::io::InputStream dictionary =
            java::nio::file::Files::newInputStream(dicEntry);
        java::io::InputStream affix =
            java::nio::file::Files::newInputStream(affEntry);
        shared_ptr<Dictionary> dic =
            make_shared<Dictionary>(tempDir, L"dictionary", affix, dictionary);
        wcout << tests[i] << L"\t" << RamUsageTester::humanSizeOf(dic) << L"\t("
              << L"words=" << RamUsageTester::humanSizeOf(dic->words) << L", "
              << L"flags=" << RamUsageTester::humanSizeOf(dic->flagLookup)
              << L", " << L"strips="
              << RamUsageTester::humanSizeOf(dic->stripData) << L", "
              << L"conditions=" << RamUsageTester::humanSizeOf(dic->patterns)
              << L", " << L"affixData="
              << RamUsageTester::humanSizeOf(dic->affixData) << L", "
              << L"prefixes=" << RamUsageTester::humanSizeOf(dic->prefixes)
              << L", " << L"suffixes="
              << RamUsageTester::humanSizeOf(dic->suffixes) << L")" << endl;
      }
    }
  }
}

void TestAllDictionaries::testOneDictionary() 
{
  shared_ptr<Path> tmp = LuceneTestCase::createTempDir();

  wstring toTest = L"zu_ZA.zip";
  for (int i = 0; i < tests.size(); i++) {
    if (tests[i] == toTest) {
      shared_ptr<Path> f = DICTIONARY_HOME->resolve(tests[i]);
      assert(Files::exists(f));

      IOUtils::rm({tmp});
      Files::createDirectory(tmp);

      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (java.io.InputStream in =
      // java.nio.file.Files.newInputStream(f))
      {
        java::io::InputStream in_ = java::nio::file::Files::newInputStream(f);
        TestUtil::unzip(in_, tmp);
        shared_ptr<Path> dicEntry = tmp->resolve(tests[i + 1]);
        shared_ptr<Path> affEntry = tmp->resolve(tests[i + 2]);

        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try (java.io.InputStream dictionary =
        // java.nio.file.Files.newInputStream(dicEntry); java.io.InputStream
        // affix = java.nio.file.Files.newInputStream(affEntry);
        // org.apache.lucene.store.Directory tempDir = getDirectory())
        {
          java::io::InputStream dictionary =
              java::nio::file::Files::newInputStream(dicEntry);
          java::io::InputStream affix =
              java::nio::file::Files::newInputStream(affEntry);
          org::apache::lucene::store::Directory tempDir = getDirectory();
          make_shared<Dictionary>(tempDir, L"dictionary", affix, dictionary);
        }
      }
    }
  }
}

shared_ptr<Directory> TestAllDictionaries::getDirectory()
{
  return newDirectory();
}
} // namespace org::apache::lucene::analysis::hunspell