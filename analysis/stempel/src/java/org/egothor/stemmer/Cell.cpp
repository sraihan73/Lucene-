using namespace std;

#include "Cell.h"

namespace org::egothor::stemmer
{

Cell::Cell() {}

Cell::Cell(shared_ptr<Cell> a)
{
  ref = a->ref;
  cmd = a->cmd;
  cnt = a->cnt;
  skip = a->skip;
}

wstring Cell::toString()
{
  return L"ref(" + to_wstring(ref) + L")cmd(" + to_wstring(cmd) + L")cnt(" +
         to_wstring(cnt) + L")skp(" + to_wstring(skip) + L")";
}
} // namespace org::egothor::stemmer