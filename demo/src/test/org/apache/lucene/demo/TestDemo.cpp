using namespace std;

#include "TestDemo.h"

namespace org::apache::lucene::demo
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDemo::testOneSearch(shared_ptr<Path> indexPath, const wstring &query,
                             int expectedHitCount) 
{
  shared_ptr<PrintStream> outSave = System::out;
  try {
    shared_ptr<ByteArrayOutputStream> bytes =
        make_shared<ByteArrayOutputStream>();
    shared_ptr<PrintStream> fakeSystemOut = make_shared<PrintStream>(
        bytes, false, Charset::defaultCharset().name());
    System::setOut(fakeSystemOut);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SearchFiles::main(std::deque<wstring>{L"-query", query, L"-index",
                                           indexPath->toString()});
    fakeSystemOut->flush();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring output = bytes->toString(
        Charset::defaultCharset().name()); // intentionally use default encoding
    assertTrue(L"output=" + output,
               output.find(to_wstring(expectedHitCount) +
                           L" total matching documents") != wstring::npos);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    System::setOut(outSave);
  }
}

void TestDemo::testIndexSearch() 
{
  shared_ptr<Path> dir = getDataPath(L"test-files/docs");
  shared_ptr<Path> indexDir = createTempDir(L"ContribDemoTest");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  IndexFiles::main(std::deque<wstring>{L"-create", L"-docs", dir->toString(),
                                        L"-index", indexDir->toString()});
  testOneSearch(indexDir, L"apache", 3);
  testOneSearch(indexDir, L"patent", 8);
  testOneSearch(indexDir, L"lucene", 0);
  testOneSearch(indexDir, L"gnu", 6);
  testOneSearch(indexDir, L"derivative", 8);
  testOneSearch(indexDir, L"license", 13);
}
} // namespace org::apache::lucene::demo