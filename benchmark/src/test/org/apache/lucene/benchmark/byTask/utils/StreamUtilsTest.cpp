using namespace std;

#include "StreamUtilsTest.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/utils/StreamUtils.h"

namespace org::apache::lucene::benchmark::byTask::utils
{
using org::apache::commons::compress::compressors::CompressorStreamFactory;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using org::junit::Before;
using org::junit::Test;
const wstring StreamUtilsTest::TEXT = L"Some-Text...";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetInputStreamPlainText() throws
// Exception
void StreamUtilsTest::testGetInputStreamPlainText() 
{
  assertReadText(rawTextFile(L"txt"));
  assertReadText(rawTextFile(L"TXT"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetInputStreamGzip() throws Exception
void StreamUtilsTest::testGetInputStreamGzip() 
{
  assertReadText(rawGzipFile(L"gz"));
  assertReadText(rawGzipFile(L"gzip"));
  assertReadText(rawGzipFile(L"GZ"));
  assertReadText(rawGzipFile(L"GZIP"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetInputStreamBzip2() throws Exception
void StreamUtilsTest::testGetInputStreamBzip2() 
{
  assertReadText(rawBzip2File(L"bz2"));
  assertReadText(rawBzip2File(L"bzip"));
  assertReadText(rawBzip2File(L"BZ2"));
  assertReadText(rawBzip2File(L"BZIP"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetOutputStreamBzip2() throws Exception
void StreamUtilsTest::testGetOutputStreamBzip2() 
{
  assertReadText(autoOutFile(L"bz2"));
  assertReadText(autoOutFile(L"bzip"));
  assertReadText(autoOutFile(L"BZ2"));
  assertReadText(autoOutFile(L"BZIP"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetOutputStreamGzip() throws Exception
void StreamUtilsTest::testGetOutputStreamGzip() 
{
  assertReadText(autoOutFile(L"gz"));
  assertReadText(autoOutFile(L"gzip"));
  assertReadText(autoOutFile(L"GZ"));
  assertReadText(autoOutFile(L"GZIP"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetOutputStreamPlain() throws Exception
void StreamUtilsTest::testGetOutputStreamPlain() 
{
  assertReadText(autoOutFile(L"txt"));
  assertReadText(autoOutFile(L"text"));
  assertReadText(autoOutFile(L"TXT"));
  assertReadText(autoOutFile(L"TEXT"));
}

shared_ptr<Path>
StreamUtilsTest::rawTextFile(const wstring &ext) 
{
  shared_ptr<Path> f = testDir->resolve(L"testfile." + ext);
  shared_ptr<BufferedWriter> w =
      Files::newBufferedWriter(f, StandardCharsets::UTF_8);
  w->write(TEXT);
  w->newLine();
  w->close();
  return f;
}

shared_ptr<Path>
StreamUtilsTest::rawGzipFile(const wstring &ext) 
{
  shared_ptr<Path> f = testDir->resolve(L"testfile." + ext);
  shared_ptr<OutputStream> os =
      (make_shared<CompressorStreamFactory>())
          ->createCompressorOutputStream(CompressorStreamFactory::GZIP,
                                         Files::newOutputStream(f));
  writeText(os);
  return f;
}

shared_ptr<Path>
StreamUtilsTest::rawBzip2File(const wstring &ext) 
{
  shared_ptr<Path> f = testDir->resolve(L"testfile." + ext);
  shared_ptr<OutputStream> os =
      (make_shared<CompressorStreamFactory>())
          ->createCompressorOutputStream(CompressorStreamFactory::BZIP2,
                                         Files::newOutputStream(f));
  writeText(os);
  return f;
}

shared_ptr<Path>
StreamUtilsTest::autoOutFile(const wstring &ext) 
{
  shared_ptr<Path> f = testDir->resolve(L"testfile." + ext);
  shared_ptr<OutputStream> os = StreamUtils::outputStream(f);
  writeText(os);
  return f;
}

void StreamUtilsTest::writeText(shared_ptr<OutputStream> os) 
{
  shared_ptr<BufferedWriter> w = make_shared<BufferedWriter>(
      make_shared<OutputStreamWriter>(os, StandardCharsets::UTF_8));
  w->write(TEXT);
  w->newLine();
  w->close();
}

void StreamUtilsTest::assertReadText(shared_ptr<Path> f) 
{
  shared_ptr<InputStream> ir = StreamUtils::inputStream(f);
  shared_ptr<InputStreamReader> in_ =
      make_shared<InputStreamReader>(ir, StandardCharsets::UTF_8);
  shared_ptr<BufferedReader> r = make_shared<BufferedReader>(in_);
  wstring line = r->readLine();
  assertEquals(L"Wrong text found in " + f->getFileName(), TEXT, line);
  r->close();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void StreamUtilsTest::setUp() 
{
  BenchmarkTestCase::setUp();
  testDir = createTempDir(L"ContentSourceTest");
}
} // namespace org::apache::lucene::benchmark::byTask::utils