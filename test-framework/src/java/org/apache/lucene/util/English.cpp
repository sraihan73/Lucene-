using namespace std;

#include "English.h"

namespace org::apache::lucene::util
{

English::English() {} // no instance

wstring English::longToEnglish(int64_t i)
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  longToEnglish(i, result);
  return result->toString();
}

void English::longToEnglish(int64_t i, shared_ptr<StringBuilder> result)
{
  if (i == 0) {
    result->append(L"zero");
    return;
  }
  if (i < 0) {
    result->append(L"minus ");
    i = -i;
  }
  if (i >= 1000000000000000000LL) { // quadrillion
    longToEnglish(i / 1000000000000000000LL, result);
    result->append(L"quintillion, ");
    i = i % 1000000000000000000LL;
  }
  if (i >= 1000000000000000LL) { // quadrillion
    longToEnglish(i / 1000000000000000LL, result);
    result->append(L"quadrillion, ");
    i = i % 1000000000000000LL;
  }
  if (i >= 1000000000000LL) { // trillions
    longToEnglish(i / 1000000000000LL, result);
    result->append(L"trillion, ");
    i = i % 1000000000000LL;
  }
  if (i >= 1000000000) { // billions
    longToEnglish(i / 1000000000, result);
    result->append(L"billion, ");
    i = i % 1000000000;
  }
  if (i >= 1000000) { // millions
    longToEnglish(i / 1000000, result);
    result->append(L"million, ");
    i = i % 1000000;
  }
  if (i >= 1000) { // thousands
    longToEnglish(i / 1000, result);
    result->append(L"thousand, ");
    i = i % 1000;
  }
  if (i >= 100) { // hundreds
    longToEnglish(i / 100, result);
    result->append(L"hundred ");
    i = i % 100;
  }
  // we know we are smaller here so we can cast
  if (i >= 20) {
    switch ((static_cast<int>(i)) / 10) {
    case 9:
      result->append(L"ninety");
      break;
    case 8:
      result->append(L"eighty");
      break;
    case 7:
      result->append(L"seventy");
      break;
    case 6:
      result->append(L"sixty");
      break;
    case 5:
      result->append(L"fifty");
      break;
    case 4:
      result->append(L"forty");
      break;
    case 3:
      result->append(L"thirty");
      break;
    case 2:
      result->append(L"twenty");
      break;
    }
    i = i % 10;
    if (i == 0) {
      result->append(L" ");
    } else {
      result->append(L"-");
    }
  }
  switch (static_cast<int>(i)) {
  case 19:
    result->append(L"nineteen ");
    break;
  case 18:
    result->append(L"eighteen ");
    break;
  case 17:
    result->append(L"seventeen ");
    break;
  case 16:
    result->append(L"sixteen ");
    break;
  case 15:
    result->append(L"fifteen ");
    break;
  case 14:
    result->append(L"fourteen ");
    break;
  case 13:
    result->append(L"thirteen ");
    break;
  case 12:
    result->append(L"twelve ");
    break;
  case 11:
    result->append(L"eleven ");
    break;
  case 10:
    result->append(L"ten ");
    break;
  case 9:
    result->append(L"nine ");
    break;
  case 8:
    result->append(L"eight ");
    break;
  case 7:
    result->append(L"seven ");
    break;
  case 6:
    result->append(L"six ");
    break;
  case 5:
    result->append(L"five ");
    break;
  case 4:
    result->append(L"four ");
    break;
  case 3:
    result->append(L"three ");
    break;
  case 2:
    result->append(L"two ");
    break;
  case 1:
    result->append(L"one ");
    break;
  case 0:
    result->append(L"");
    break;
  }
}

wstring English::intToEnglish(int i)
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  longToEnglish(i, result);
  return result->toString();
}

void English::intToEnglish(int i, shared_ptr<StringBuilder> result)
{
  longToEnglish(i, result);
}
} // namespace org::apache::lucene::util