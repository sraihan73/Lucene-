using namespace std;

#include "ToStringUtils.h"

namespace org::apache::lucene::util
{

ToStringUtils::ToStringUtils() {} // no instance

void ToStringUtils::byteArray(shared_ptr<StringBuilder> buffer,
                              std::deque<char> &bytes)
{
  for (int i = 0; i < bytes.size(); i++) {
    buffer->append(L"b[")->append(i)->append(L"]=")->append(bytes[i]);
    if (i < bytes.size() - 1) {
      buffer->append(L',');
    }
  }
}

std::deque<wchar_t> const ToStringUtils::HEX =
    (wstring(L"0123456789abcdef")).toCharArray();

wstring ToStringUtils::longHex(int64_t x)
{
  std::deque<wchar_t> asHex(16);
  for (int i = 16; --i >= 0;
       x = static_cast<int64_t>(static_cast<uint64_t>(x) >> 4)) {
    asHex[i] = HEX[static_cast<int>(x) & 0x0F];
  }
  return L"0x" + wstring(asHex);
}
} // namespace org::apache::lucene::util