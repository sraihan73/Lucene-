using namespace std;

#include "TestHugeRamFile.h"

namespace org::apache::lucene::store
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

std::deque<char> TestHugeRamFile::DenseRAMFile::newBuffer(int size)
{
  capacity += size;
  if (capacity <= MAX_VALUE) {
    // below maxint we reuse buffers
    std::deque<char> buf = singleBuffers[static_cast<Integer>(size)];
    if (buf.empty()) {
      buf = std::deque<char>(size);
      // System.out.println("allocate: "+size);
      singleBuffers.emplace(static_cast<Integer>(size), buf);
    }
    return buf;
  }
  // System.out.println("allocate: "+size); System.out.flush();
  return std::deque<char>(size);
}

void TestHugeRamFile::testHugeFile() 
{
  shared_ptr<DenseRAMFile> f = make_shared<DenseRAMFile>();
  // output part
  shared_ptr<RAMOutputStream> out = make_shared<RAMOutputStream>(f, true);
  std::deque<char> b1(RAMOutputStream::BUFFER_SIZE);
  std::deque<char> b2(RAMOutputStream::BUFFER_SIZE / 3);
  for (int i = 0; i < b1.size(); i++) {
    b1[i] = static_cast<char>(i & 0x0007F);
  }
  for (int i = 0; i < b2.size(); i++) {
    b2[i] = static_cast<char>(i & 0x0003F);
  }
  int64_t n = 0;
  assertEquals(L"output length must match", n, out->getFilePointer());
  while (n <= MAX_VALUE - b1.size()) {
    out->writeBytes(b1, 0, b1.size());
    out->flush();
    n += b1.size();
    assertEquals(L"output length must match", n, out->getFilePointer());
  }
  // System.out.println("after writing b1's, length = "+out.length()+"
  // (MAX_VALUE="+MAX_VALUE+")");
  int m = b2.size();
  int64_t L = 12;
  for (int j = 0; j < L; j++) {
    for (int i = 0; i < b2.size(); i++) {
      b2[i]++;
    }
    out->writeBytes(b2, 0, m);
    out->flush();
    n += m;
    assertEquals(L"output length must match", n, out->getFilePointer());
  }
  delete out;
  // input part
  shared_ptr<RAMInputStream> in_ = make_shared<RAMInputStream>(L"testcase", f);
  assertEquals(L"input length must match", n, in_->length());
  // System.out.println("input length = "+in.length()+" % 1024 =
  // "+in.length()%1024);
  for (int j = 0; j < L; j++) {
    int64_t loc = n - (L - j) * m;
    in_->seek(loc / 3);
    in_->seek(loc);
    for (int i = 0; i < m; i++) {
      char bt = in_->readByte();
      char expected = static_cast<char>(1 + j + (i & 0x0003F));
      assertEquals(L"must read same value that was written! j=" +
                       to_wstring(j) + L" i=" + to_wstring(i),
                   expected, bt);
    }
  }
}
} // namespace org::apache::lucene::store