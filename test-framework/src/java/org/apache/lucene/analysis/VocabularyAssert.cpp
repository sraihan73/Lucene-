using namespace std;

#include "VocabularyAssert.h"

namespace org::apache::lucene::analysis
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Assert;

void VocabularyAssert::assertVocabulary(
    shared_ptr<Analyzer> a, shared_ptr<InputStream> voc,
    shared_ptr<InputStream> out) 
{
  shared_ptr<BufferedReader> vocReader = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(voc, StandardCharsets::UTF_8));
  shared_ptr<BufferedReader> outputReader = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(out, StandardCharsets::UTF_8));
  wstring inputWord = L"";
  while ((inputWord = vocReader->readLine()) != L"") {
    wstring expectedWord = outputReader->readLine();
    Assert::assertNotNull(expectedWord);
    BaseTokenStreamTestCase::checkOneTerm(a, inputWord, expectedWord);
  }
}

void VocabularyAssert::assertVocabulary(
    shared_ptr<Analyzer> a, shared_ptr<InputStream> vocOut) 
{
  shared_ptr<BufferedReader> vocReader = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(vocOut, StandardCharsets::UTF_8));
  wstring inputLine = L"";
  while ((inputLine = vocReader->readLine()) != L"") {
    if (StringHelper::startsWith(inputLine, L"#") ||
        StringHelper::trim(inputLine)->length() == 0) {
      continue; // comment
    }
    std::deque<wstring> words = inputLine.split(L"\t");
    BaseTokenStreamTestCase::checkOneTerm(a, words[0], words[1]);
  }
}

void VocabularyAssert::assertVocabulary(shared_ptr<Analyzer> a,
                                        shared_ptr<Path> zipFile,
                                        const wstring &voc,
                                        const wstring &out) 
{
  shared_ptr<Path> tmp = LuceneTestCase::createTempDir();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream in =
  // java.nio.file.Files.newInputStream(zipFile))
  {
    java::io::InputStream in_ = java::nio::file::Files::newInputStream(zipFile);
    TestUtil::unzip(in_, tmp);
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream v =
  // java.nio.file.Files.newInputStream(tmp.resolve(voc)); java.io.InputStream o
  // = java.nio.file.Files.newInputStream(tmp.resolve(out)))
  {
    java::io::InputStream v =
        java::nio::file::Files::newInputStream(tmp->resolve(voc));
    java::io::InputStream o =
        java::nio::file::Files::newInputStream(tmp->resolve(out));
    assertVocabulary(a, v, o);
  }
}

void VocabularyAssert::assertVocabulary(
    shared_ptr<Analyzer> a, shared_ptr<Path> zipFile,
    const wstring &vocOut) 
{
  shared_ptr<Path> tmp = LuceneTestCase::createTempDir();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream in =
  // java.nio.file.Files.newInputStream(zipFile))
  {
    java::io::InputStream in_ = java::nio::file::Files::newInputStream(zipFile);
    TestUtil::unzip(in_, tmp);
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream in =
  // java.nio.file.Files.newInputStream(tmp.resolve(vocOut)))
  {
    java::io::InputStream in_ =
        java::nio::file::Files::newInputStream(tmp->resolve(vocOut));
    assertVocabulary(a, in_);
  }
}
} // namespace org::apache::lucene::analysis