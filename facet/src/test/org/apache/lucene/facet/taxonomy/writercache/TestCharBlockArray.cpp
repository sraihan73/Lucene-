using namespace std;

#include "TestCharBlockArray.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;

void TestCharBlockArray::testArray() 
{
  shared_ptr<CharBlockArray> array_ = make_shared<CharBlockArray>();
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();

  constexpr int n = 100 * 1000;

  std::deque<char> buffer(50);

  for (int i = 0; i < n; i++) {
    random()->nextBytes(buffer);
    int size = 1 + random()->nextInt(50);
    // This test is turning random bytes into a string,
    // this is asking for trouble.
    shared_ptr<CharsetDecoder> decoder =
        StandardCharsets::UTF_8::newDecoder()
            .onUnmappableCharacter(CodingErrorAction::REPLACE)
            .onMalformedInput(CodingErrorAction::REPLACE);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring s = decoder->decode(ByteBuffer::wrap(buffer, 0, size))->toString();
    array_->append(s);
    builder->append(s);
  }

  for (int i = 0; i < n; i++) {
    random()->nextBytes(buffer);
    int size = 1 + random()->nextInt(50);
    // This test is turning random bytes into a string,
    // this is asking for trouble.
    shared_ptr<CharsetDecoder> decoder =
        StandardCharsets::UTF_8::newDecoder()
            .onUnmappableCharacter(CodingErrorAction::REPLACE)
            .onMalformedInput(CodingErrorAction::REPLACE);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring s = decoder->decode(ByteBuffer::wrap(buffer, 0, size))->toString();
    array_->append(std::static_pointer_cast<std::wstring>(s));
    builder->append(s);
  }

  for (int i = 0; i < n; i++) {
    random()->nextBytes(buffer);
    int size = 1 + random()->nextInt(50);
    // This test is turning random bytes into a string,
    // this is asking for trouble.
    shared_ptr<CharsetDecoder> decoder =
        StandardCharsets::UTF_8::newDecoder()
            .onUnmappableCharacter(CodingErrorAction::REPLACE)
            .onMalformedInput(CodingErrorAction::REPLACE);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring s = decoder->decode(ByteBuffer::wrap(buffer, 0, size))->toString();
    for (int j = 0; j < s.length(); j++) {
      array_->append(s[j]);
    }
    builder->append(s);
  }

  assertEqualsInternal(L"GrowingCharArray<->StringBuilder mismatch.", builder,
                       array_);

  shared_ptr<Path> tempDir = createTempDir(L"growingchararray");
  shared_ptr<Path> f = tempDir->resolve(L"GrowingCharArrayTest.tmp");
  shared_ptr<BufferedOutputStream> out =
      make_shared<BufferedOutputStream>(Files::newOutputStream(f));
  array_->flush(out);
  out->flush();
  out->close();

  shared_ptr<BufferedInputStream> in_ =
      make_shared<BufferedInputStream>(Files::newInputStream(f));
  array_ = CharBlockArray::open(in_);
  assertEqualsInternal(
      L"GrowingCharArray<->StringBuilder mismatch after flush/load.", builder,
      array_);
  in_->close();
}

void TestCharBlockArray::assertEqualsInternal(
    const wstring &msg, shared_ptr<StringBuilder> expected,
    shared_ptr<CharBlockArray> actual)
{
  assertEquals(msg, expected->length(), actual->length());
  for (int i = 0; i < expected->length(); i++) {
    assertEquals(msg, expected->charAt(i), actual->charAt(i));
  }
}
} // namespace org::apache::lucene::facet::taxonomy::writercache