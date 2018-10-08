using namespace std;

#include "TestCodecUtil.h"

namespace org::apache::lucene::codecs
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMFile = org::apache::lucene::store::RAMFile;
using RAMInputStream = org::apache::lucene::store::RAMInputStream;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;

void TestCodecUtil::testHeaderLength() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  CodecUtil::writeHeader(output, L"FooBar", 5);
  output->writeString(L"this is the data");
  delete output;

  shared_ptr<IndexInput> input = make_shared<RAMInputStream>(L"file", file);
  input->seek(CodecUtil::headerLength(L"FooBar"));
  assertEquals(L"this is the data", input->readString());
  delete input;
}

void TestCodecUtil::testWriteTooLongHeader() 
{
  shared_ptr<StringBuilder> tooLong = make_shared<StringBuilder>();
  for (int i = 0; i < 128; i++) {
    tooLong->append(L'a');
  }
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  expectThrows(invalid_argument::typeid, [&]() {
    CodecUtil::writeHeader(output, tooLong->toString(), 5);
  });
}

void TestCodecUtil::testWriteNonAsciiHeader() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  expectThrows(invalid_argument::typeid,
               [&]() { CodecUtil::writeHeader(output, L"\u1234", 5); });
}

void TestCodecUtil::testReadHeaderWrongMagic() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  output->writeInt(1234);
  delete output;

  shared_ptr<IndexInput> input = make_shared<RAMInputStream>(L"file", file);
  expectThrows(CorruptIndexException::typeid,
               [&]() { CodecUtil::checkHeader(input, L"bogus", 1, 1); });
}

void TestCodecUtil::testChecksumEntireFile() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  CodecUtil::writeHeader(output, L"FooBar", 5);
  output->writeString(L"this is the data");
  CodecUtil::writeFooter(output);
  delete output;

  shared_ptr<IndexInput> input = make_shared<RAMInputStream>(L"file", file);
  CodecUtil::checksumEntireFile(input);
  delete input;
}

void TestCodecUtil::testCheckFooterValid() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  CodecUtil::writeHeader(output, L"FooBar", 5);
  output->writeString(L"this is the data");
  CodecUtil::writeFooter(output);
  delete output;

  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(
          make_shared<RAMInputStream>(L"file", file));
  runtime_error mine = runtime_error(L"fake exception");
  runtime_error expected = expectThrows(
      runtime_error::typeid, [&]() { CodecUtil::checkFooter(input, mine); });
  assertEquals(L"fake exception", expected.what());
  std::deque<runtime_error> suppressed = expected.getSuppressed();
  assertEquals(1, suppressed.size());
  assertTrue(suppressed[0].what()->contains(L"checksum passed"));
  delete input;
}

void TestCodecUtil::testCheckFooterValidAtFooter() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  CodecUtil::writeHeader(output, L"FooBar", 5);
  output->writeString(L"this is the data");
  CodecUtil::writeFooter(output);
  delete output;

  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(
          make_shared<RAMInputStream>(L"file", file));
  CodecUtil::checkHeader(input, L"FooBar", 5, 5);
  assertEquals(L"this is the data", input->readString());
  runtime_error mine = runtime_error(L"fake exception");
  runtime_error expected = expectThrows(
      runtime_error::typeid, [&]() { CodecUtil::checkFooter(input, mine); });
  assertEquals(L"fake exception", expected.what());
  std::deque<runtime_error> suppressed = expected.getSuppressed();
  assertEquals(1, suppressed.size());
  assertTrue(suppressed[0].what()->contains(L"checksum passed"));
  delete input;
}

void TestCodecUtil::testCheckFooterValidPastFooter() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  CodecUtil::writeHeader(output, L"FooBar", 5);
  output->writeString(L"this is the data");
  CodecUtil::writeFooter(output);
  delete output;

  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(
          make_shared<RAMInputStream>(L"file", file));
  CodecUtil::checkHeader(input, L"FooBar", 5, 5);
  assertEquals(L"this is the data", input->readString());
  // bogusly read a byte too far (can happen)
  input->readByte();
  runtime_error mine = runtime_error(L"fake exception");
  runtime_error expected = expectThrows(
      runtime_error::typeid, [&]() { CodecUtil::checkFooter(input, mine); });
  assertEquals(L"fake exception", expected.what());
  std::deque<runtime_error> suppressed = expected.getSuppressed();
  assertEquals(1, suppressed.size());
  assertTrue(suppressed[0].what()->contains(L"checksum status indeterminate"));
  delete input;
}

void TestCodecUtil::testCheckFooterInvalid() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  CodecUtil::writeHeader(output, L"FooBar", 5);
  output->writeString(L"this is the data");
  output->writeInt(CodecUtil::FOOTER_MAGIC);
  output->writeInt(0);
  output->writeLong(1234567); // write a bogus checksum
  delete output;

  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(
          make_shared<RAMInputStream>(L"file", file));
  CodecUtil::checkHeader(input, L"FooBar", 5, 5);
  assertEquals(L"this is the data", input->readString());
  runtime_error mine = runtime_error(L"fake exception");
  runtime_error expected = expectThrows(
      runtime_error::typeid, [&]() { CodecUtil::checkFooter(input, mine); });
  assertEquals(L"fake exception", expected.what());
  std::deque<runtime_error> suppressed = expected.getSuppressed();
  assertEquals(1, suppressed.size());
  assertTrue(suppressed[0].what()->contains(L"checksum failed"));
  delete input;
}

void TestCodecUtil::testSegmentHeaderLength() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  CodecUtil::writeIndexHeader(output, L"FooBar", 5, StringHelper::randomId(),
                              L"xyz");
  output->writeString(L"this is the data");
  delete output;

  shared_ptr<IndexInput> input = make_shared<RAMInputStream>(L"file", file);
  input->seek(CodecUtil::indexHeaderLength(L"FooBar", L"xyz"));
  assertEquals(L"this is the data", input->readString());
  delete input;
}

void TestCodecUtil::testWriteTooLongSuffix() 
{
  shared_ptr<StringBuilder> tooLong = make_shared<StringBuilder>();
  for (int i = 0; i < 256; i++) {
    tooLong->append(L'a');
  }
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  expectThrows(invalid_argument::typeid, [&]() {
    CodecUtil::writeIndexHeader(output, L"foobar", 5, StringHelper::randomId(),
                                tooLong->toString());
  });
}

void TestCodecUtil::testWriteVeryLongSuffix() 
{
  shared_ptr<StringBuilder> justLongEnough = make_shared<StringBuilder>();
  for (int i = 0; i < 255; i++) {
    justLongEnough->append(L'a');
  }
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  std::deque<char> id = StringHelper::randomId();
  CodecUtil::writeIndexHeader(output, L"foobar", 5, id,
                              justLongEnough->toString());
  delete output;

  shared_ptr<IndexInput> input = make_shared<RAMInputStream>(L"file", file);
  CodecUtil::checkIndexHeader(input, L"foobar", 5, 5, id,
                              justLongEnough->toString());
  assertEquals(input->getFilePointer(), input->length());
  assertEquals(
      input->getFilePointer(),
      CodecUtil::indexHeaderLength(L"foobar", justLongEnough->toString()));
  delete input;
}

void TestCodecUtil::testWriteNonAsciiSuffix() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, true);
  expectThrows(invalid_argument::typeid, [&]() {
    CodecUtil::writeIndexHeader(output, L"foobar", 5, StringHelper::randomId(),
                                L"\u1234");
  });
}

void TestCodecUtil::testReadBogusCRC() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, false);
  output->writeLong(-1LL);            // bad
  output->writeLong(1LL << 32);       // bad
  output->writeLong(-(1LL << 32));    // bad
  output->writeLong((1LL << 32) - 1); // ok
  delete output;
  shared_ptr<IndexInput> input = make_shared<RAMInputStream>(L"file", file);
  // read 3 bogus values
  for (int i = 0; i < 3; i++) {
    expectThrows(CorruptIndexException::typeid,
                 [&]() { CodecUtil::readCRC(input); });
  }
  // good value
  CodecUtil::readCRC(input);
}

void TestCodecUtil::testWriteBogusCRC() 
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> *const output =
      make_shared<RAMOutputStream>(file, false);
  shared_ptr<AtomicLong> fakeChecksum = make_shared<AtomicLong>();
  // wrap the index input where we control the checksum for mocking
  shared_ptr<IndexOutput> fakeOutput =
      make_shared<IndexOutputAnonymousInnerClass>(shared_from_this(), output,
                                                  fakeChecksum);

  fakeChecksum->set(-1LL); // bad
  expectThrows(IllegalStateException::typeid,
               [&]() { CodecUtil::writeCRC(fakeOutput); });

  fakeChecksum->set(1LL << 32); // bad
  expectThrows(IllegalStateException::typeid,
               [&]() { CodecUtil::writeCRC(fakeOutput); });

  fakeChecksum->set(-(1LL << 32)); // bad
  expectThrows(IllegalStateException::typeid,
               [&]() { CodecUtil::writeCRC(fakeOutput); });

  fakeChecksum->set((1LL << 32) - 1); // ok
  CodecUtil::writeCRC(fakeOutput);
}

TestCodecUtil::IndexOutputAnonymousInnerClass::IndexOutputAnonymousInnerClass(
    shared_ptr<TestCodecUtil> outerInstance, shared_ptr<IndexOutput> output,
    shared_ptr<AtomicLong> fakeChecksum)
    : org::apache::lucene::store::IndexOutput(L"fake", L"fake")
{
  this->outerInstance = outerInstance;
  this->output = output;
  this->fakeChecksum = fakeChecksum;
}

TestCodecUtil::IndexOutputAnonymousInnerClass::~IndexOutputAnonymousInnerClass()
{
  delete output;
}

int64_t TestCodecUtil::IndexOutputAnonymousInnerClass::getFilePointer()
{
  return output->getFilePointer();
}

int64_t
TestCodecUtil::IndexOutputAnonymousInnerClass::getChecksum() 
{
  return fakeChecksum->get();
}

void TestCodecUtil::IndexOutputAnonymousInnerClass::writeByte(char b) throw(
    IOException)
{
  output->writeByte(b);
}

void TestCodecUtil::IndexOutputAnonymousInnerClass::writeBytes(
    std::deque<char> &b, int offset, int length) 
{
  output->writeBytes(b, offset, length);
}

void TestCodecUtil::testTruncatedFileThrowsCorruptIndexException() throw(
    IOException)
{
  shared_ptr<RAMFile> file = make_shared<RAMFile>();
  shared_ptr<IndexOutput> output = make_shared<RAMOutputStream>(file, false);
  delete output;
  shared_ptr<IndexInput> input = make_shared<RAMInputStream>(L"file", file);
  shared_ptr<CorruptIndexException> e =
      expectThrows(CorruptIndexException::typeid, [&]() {
        org::apache::lucene::codecs::CodecUtil::checksumEntireFile(input);
      });
  assertEquals(L"misplaced codec footer (file truncated?): length=0 but "
               L"footerLength==16 (resource=RAMInputStream(name=file))",
               e->getMessage());
  e = expectThrows(CorruptIndexException::typeid, [&]() {
    org::apache::lucene::codecs::CodecUtil::retrieveChecksum(input);
  });
  assertEquals(L"misplaced codec footer (file truncated?): length=0 but "
               L"footerLength==16 (resource=RAMInputStream(name=file))",
               e->getMessage());
}
} // namespace org::apache::lucene::codecs