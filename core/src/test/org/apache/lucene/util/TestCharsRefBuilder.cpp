using namespace std;

#include "TestCharsRefBuilder.h"

namespace org::apache::lucene::util
{

void TestCharsRefBuilder::testAppend()
{
  const wstring s = TestUtil::randomUnicodeString(random(), 100);
  shared_ptr<CharsRefBuilder> builder = make_shared<CharsRefBuilder>();
  while (builder->length() < s.length()) {
    if (random()->nextBoolean()) {
      builder->append(s[builder->length()]);
    } else {
      constexpr int start = builder->length();
      constexpr int end = TestUtil::nextInt(random(), start, s.length());
      if (random()->nextBoolean()) {
        builder->append(s.substr(start, end - start));
      } else {
        builder->append(s, start, end);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(s, builder->toString());
}

void TestCharsRefBuilder::testAppendNull()
{
  shared_ptr<CharsRefBuilder> builder = make_shared<CharsRefBuilder>();
  builder->append(nullptr);
  builder->append(std::static_pointer_cast<std::wstring>(nullptr), 1, 3);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"nullnull", builder->toString());
}
} // namespace org::apache::lucene::util