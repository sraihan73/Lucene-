using namespace std;

#include "Diff.h"

namespace org::egothor::stemmer
{

Diff::Diff() : Diff(1, 1, 1, 0) {}

Diff::Diff(int ins, int del, int rep, int noop)
{
  INSERT = ins;
  DELETE = del;
  REPLACE = rep;
  NOOP = noop;
}

void Diff::apply(shared_ptr<StringBuilder> dest, shared_ptr<std::wstring> diff)
{
  try {

    if (diff == nullptr) {
      return;
    }

    int pos = dest->length() - 1;
    if (pos < 0) {
      return;
    }
    // orig == ""
    for (int i = 0; i < diff->length() / 2; i++) {
      wchar_t cmd = diff->charAt(2 * i);
      wchar_t param = diff->charAt(2 * i + 1);
      int par_num = (param - L'a' + 1);
      switch (cmd) {
      case L'-':
        pos = pos - par_num + 1;
        break;
      case L'R':
        dest->setCharAt(pos, param);
        break;
      case L'D': {
        int o = pos;
        pos -= par_num - 1;
        /*
         * delete par_num chars from index pos
         */
        // std::wstring s = orig.toString();
        // s = s.substring( 0, pos ) + s.substring( o + 1 );
        // orig = new StringBuffer( s );
        dest->remove(pos, o + 1);
        break;
      }
      case L'I':
        dest->insert(pos += 1, param);
        break;
      }
      pos--;
    }
  } catch (const StringIndexOutOfBoundsException &x) {
    // x.printStackTrace();
  } catch (const out_of_range &x) {
    // x.printStackTrace();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
wstring Diff::exec(const wstring &a, const wstring &b)
{
  if (a == L"" || b == L"") {
    return L"";
  }

  int x;
  int y;
  int maxx;
  int maxy;
  std::deque<int> go(4);
  constexpr int X = 1;
  constexpr int Y = 2;
  constexpr int R = 3;
  constexpr int D = 0;

  /*
   * setup memory if needed => processing speed up
   */
  maxx = a.length() + 1;
  maxy = b.length() + 1;
  if ((maxx >= sizex) || (maxy >= sizey)) {
    sizex = maxx + 8;
    sizey = maxy + 8;
    // C++ NOTE: The following call to the 'RectangularVectors' helper class
    // reproduces the rectangular array initialization that is automatic in Java:
    // ORIGINAL LINE: net = new int[sizex][sizey];
    net = RectangularVectors::ReturnRectangularIntVector(sizex, sizey);
    // C++ NOTE: The following call to the 'RectangularVectors' helper class
    // reproduces the rectangular array initialization that is automatic in Java:
    // ORIGINAL LINE: way = new int[sizex][sizey];
    way = RectangularVectors::ReturnRectangularIntVector(sizex, sizey);
  }

  /*
   * clear the network
   */
  for (x = 0; x < maxx; x++) {
    for (y = 0; y < maxy; y++) {
      net[x][y] = 0;
    }
  }

  /*
   * set known persistent values
   */
  for (x = 1; x < maxx; x++) {
    net[x][0] = x;
    way[x][0] = X;
  }
  for (y = 1; y < maxy; y++) {
    net[0][y] = y;
    way[0][y] = Y;
  }

  for (x = 1; x < maxx; x++) {
    for (y = 1; y < maxy; y++) {
      go[X] = net[x - 1][y] + DELETE;
      // way on x costs 1 unit
      go[Y] = net[x][y - 1] + INSERT;
      // way on y costs 1 unit
      go[R] = net[x - 1][y - 1] + REPLACE;
      go[D] = net[x - 1][y - 1] + ((a[x - 1] == b[y - 1]) ? NOOP : 100);
      // diagonal costs 0, when no change
      short min = D;
      if (go[min] >= go[X]) {
        min = X;
      }
      if (go[min] > go[Y]) {
        min = Y;
      }
      if (go[min] > go[R]) {
        min = R;
      }
      way[x][y] = min;
      net[x][y] = static_cast<short>(go[min]);
    }
  }

  // read the patch string
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  constexpr wchar_t base = L'a' - 1;
  wchar_t deletes = base;
  wchar_t equals = base;
  for (x = maxx - 1, y = maxy - 1; x + y != 0;) {
    switch (way[x][y]) {
    case X:
      if (equals != base) {
        result->append(L"-" + StringHelper::toString(equals));
        equals = base;
      }
      deletes++;
      x--;
      break;
    // delete
    case Y:
      if (deletes != base) {
        result->append(L"D" + StringHelper::toString(deletes));
        deletes = base;
      }
      if (equals != base) {
        result->append(L"-" + StringHelper::toString(equals));
        equals = base;
      }
      result->append(L'I');
      result->append(b[--y]);
      break;
    // insert
    case R:
      if (deletes != base) {
        result->append(L"D" + StringHelper::toString(deletes));
        deletes = base;
      }
      if (equals != base) {
        result->append(L"-" + StringHelper::toString(equals));
        equals = base;
      }
      result->append(L'R');
      result->append(b[--y]);
      x--;
      break;
    // replace
    case D:
      if (deletes != base) {
        result->append(L"D" + StringHelper::toString(deletes));
        deletes = base;
      }
      equals++;
      x--;
      y--;
      break;
      // no change
    }
  }
  if (deletes != base) {
    result->append(L"D" + StringHelper::toString(deletes));
    deletes = base;
  }

  return result->toString();
}
} // namespace org::egothor::stemmer