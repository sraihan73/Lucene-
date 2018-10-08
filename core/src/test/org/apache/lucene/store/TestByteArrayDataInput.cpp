using namespace std;

#include "TestByteArrayDataInput.h"

namespace org::apache::lucene::store
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestByteArrayDataInput::testBasic() 
{
  std::deque<char> bytes = {1, 65};
  shared_ptr<ByteArrayDataInput> in_ = make_shared<ByteArrayDataInput>(bytes);
  assertEquals(L"A", in_->readString());

  bytes = std::deque<char>{1, 1, 65};
  in_->reset(bytes, 1, 2);
  assertEquals(L"A", in_->readString());
}
} // namespace org::apache::lucene::store