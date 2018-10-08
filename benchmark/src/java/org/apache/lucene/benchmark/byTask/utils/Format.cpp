using namespace std;

#include "Format.h"

namespace org::apache::lucene::benchmark::byTask::utils
{

std::deque<std::shared_ptr<java::text::NumberFormat>> Format::numFormat = {
    java::text::NumberFormat::getInstance(java::util::Locale::ROOT),
    java::text::NumberFormat::getInstance(java::util::Locale::ROOT),
    java::text::NumberFormat::getInstance(java::util::Locale::ROOT)};
const wstring Format::padd =
    L"                                                 ";

Format::StaticConstructor::StaticConstructor()
{
  numFormat[0]->setMaximumFractionDigits(0);
  numFormat[0]->setMinimumFractionDigits(0);
  numFormat[1]->setMaximumFractionDigits(1);
  numFormat[1]->setMinimumFractionDigits(1);
  numFormat[2]->setMaximumFractionDigits(2);
  numFormat[2]->setMinimumFractionDigits(2);
}

Format::StaticConstructor Format::staticConstructor;

wstring Format::format(int numFracDigits, float f, const wstring &col)
{
  wstring res = padd + numFormat[numFracDigits]->format(f);
  return res.substr(res.length() - col.length());
}

wstring Format::format(int numFracDigits, double f, const wstring &col)
{
  wstring res = padd + numFormat[numFracDigits]->format(f);
  return res.substr(res.length() - col.length());
}

wstring Format::formatPaddRight(int numFracDigits, float f, const wstring &col)
{
  wstring res = numFormat[numFracDigits]->format(f) + padd;
  return res.substr(0, col.length());
}

wstring Format::formatPaddRight(int numFracDigits, double f, const wstring &col)
{
  wstring res = numFormat[numFracDigits]->format(f) + padd;
  return res.substr(0, col.length());
}

wstring Format::format(int n, const wstring &col)
{
  wstring res = padd + to_wstring(n);
  return res.substr(res.length() - col.length());
}

wstring Format::format(const wstring &s, const wstring &col)
{
  wstring s1 = (s + padd);
  return s1.substr(0, min(col.length(), s1.length()));
}

wstring Format::formatPaddLeft(const wstring &s, const wstring &col)
{
  wstring res = padd + s;
  return res.substr(res.length() - col.length());
}
} // namespace org::apache::lucene::benchmark::byTask::utils