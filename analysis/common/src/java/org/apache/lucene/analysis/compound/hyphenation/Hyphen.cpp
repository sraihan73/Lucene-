using namespace std;

#include "Hyphen.h"

namespace org::apache::lucene::analysis::compound::hyphenation
{

Hyphen::Hyphen(const wstring &pre, const wstring &no, const wstring &post)
{
  preBreak = pre;
  noBreak = no;
  postBreak = post;
}

Hyphen::Hyphen(const wstring &pre)
{
  preBreak = pre;
  noBreak = L"";
  postBreak = L"";
}

wstring Hyphen::toString()
{
  if (noBreak == L"" && postBreak == L"" && preBreak != L"" &&
      preBreak == L"-") {
    return L"-";
  }
  shared_ptr<StringBuilder> res = make_shared<StringBuilder>(L"{");
  res->append(preBreak);
  res->append(L"}{");
  res->append(postBreak);
  res->append(L"}{");
  res->append(noBreak);
  res->append(L'}');
  return res->toString();
}
} // namespace org::apache::lucene::analysis::compound::hyphenation