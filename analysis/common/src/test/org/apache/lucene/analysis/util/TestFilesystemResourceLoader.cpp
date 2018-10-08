using namespace std;

#include "TestFilesystemResourceLoader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/WordlistLoader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/FilesystemResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::util
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestFilesystemResourceLoader::assertNotFound(
    shared_ptr<ResourceLoader> rl) 
{
  // the resource does not exist, should fail!
  expectThrows(IOException::typeid, [&]() {
    IOUtils::closeWhileHandlingException({rl->openResource(
        L"this-directory-really-really-really-should-not-exist/foo/bar.txt")});
  });

  // the class does not exist, should fail!
  expectThrows(runtime_error::typeid, [&]() {
    rl->newInstance(L"org.apache.lucene.analysis.FooBarFilterFactory",
                    TokenFilterFactory::typeid);
  });
}

void TestFilesystemResourceLoader::assertClasspathDelegation(
    shared_ptr<ResourceLoader> rl) 
{
  // try a stopwords file from classpath
  shared_ptr<CharArraySet> set =
      WordlistLoader::getSnowballWordSet(make_shared<InputStreamReader>(
          rl->openResource(
              L"org/apache/lucene/analysis/snowball/english_stop.txt"),
          StandardCharsets::UTF_8));
  assertTrue(set->contains(L"you"));
  // try to load a class; we use string comparison because classloader may be
  // different...
  assertEquals(
      L"org.apache.lucene.analysis.util.RollingCharBuffer",
      rl->newInstance(L"org.apache.lucene.analysis.util.RollingCharBuffer",
                      any::typeid)
          ->getClassName());
}

void TestFilesystemResourceLoader::testBaseDir() 
{
  shared_ptr<Path> *const base = createTempDir(L"fsResourceLoaderBase");
  shared_ptr<Writer> os = Files::newBufferedWriter(
      base->resolve(L"template.txt"), StandardCharsets::UTF_8);
  try {
    os->write(L"foobar\n");
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({os});
  }

  shared_ptr<ResourceLoader> rl = make_shared<FilesystemResourceLoader>(base);
  assertEquals(L"foobar",
               WordlistLoader::getLines(rl->openResource(L"template.txt"),
                                        StandardCharsets::UTF_8)[0]);
  // Same with full path name:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring fullPath =
      base->resolve(L"template.txt").toAbsolutePath()->toString();
  assertEquals(L"foobar", WordlistLoader::getLines(rl->openResource(fullPath),
                                                   StandardCharsets::UTF_8)[0]);
  assertClasspathDelegation(rl);
  assertNotFound(rl);
}

void TestFilesystemResourceLoader::testDelegation() 
{
  shared_ptr<ResourceLoader> rl = make_shared<FilesystemResourceLoader>(
      createTempDir(L"empty"),
      make_shared<StringMockResourceLoader>(L"foobar\n"));
  assertEquals(L"foobar",
               WordlistLoader::getLines(rl->openResource(L"template.txt"),
                                        StandardCharsets::UTF_8)[0]);
}
} // namespace org::apache::lucene::analysis::util