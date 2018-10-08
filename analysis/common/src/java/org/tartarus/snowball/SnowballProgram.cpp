using namespace std;

#include "SnowballProgram.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "Among.h"

namespace org::tartarus::snowball
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

SnowballProgram::SnowballProgram()
{
  current = std::deque<wchar_t>(8);
  setCurrent(L"");
}

void SnowballProgram::setCurrent(const wstring &value)
{
  current = value.toCharArray();
  cursor = 0;
  limit = value.length();
  limit_backward = 0;
  bra = cursor;
  ket = limit;
}

wstring SnowballProgram::getCurrent() { return wstring(current, 0, limit); }

void SnowballProgram::setCurrent(std::deque<wchar_t> &text, int length)
{
  current = text;
  cursor = 0;
  limit = length;
  limit_backward = 0;
  bra = cursor;
  ket = limit;
}

std::deque<wchar_t> SnowballProgram::getCurrentBuffer() { return current; }

int SnowballProgram::getCurrentBufferLength() { return limit; }

void SnowballProgram::copy_from(shared_ptr<SnowballProgram> other)
{
  current = other->current;
  cursor = other->cursor;
  limit = other->limit;
  limit_backward = other->limit_backward;
  bra = other->bra;
  ket = other->ket;
}

bool SnowballProgram::in_grouping(std::deque<wchar_t> &s, int min, int max)
{
  if (cursor >= limit) {
    return false;
  }
  wchar_t ch = current[cursor];
  if (ch > max || ch < min) {
    return false;
  }
  ch -= min;
  if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) {
    return false;
  }
  cursor++;
  return true;
}

bool SnowballProgram::in_grouping_b(std::deque<wchar_t> &s, int min, int max)
{
  if (cursor <= limit_backward) {
    return false;
  }
  wchar_t ch = current[cursor - 1];
  if (ch > max || ch < min) {
    return false;
  }
  ch -= min;
  if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) {
    return false;
  }
  cursor--;
  return true;
}

bool SnowballProgram::out_grouping(std::deque<wchar_t> &s, int min, int max)
{
  if (cursor >= limit) {
    return false;
  }
  wchar_t ch = current[cursor];
  if (ch > max || ch < min) {
    cursor++;
    return true;
  }
  ch -= min;
  if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) {
    cursor++;
    return true;
  }
  return false;
}

bool SnowballProgram::out_grouping_b(std::deque<wchar_t> &s, int min, int max)
{
  if (cursor <= limit_backward) {
    return false;
  }
  wchar_t ch = current[cursor - 1];
  if (ch > max || ch < min) {
    cursor--;
    return true;
  }
  ch -= min;
  if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) {
    cursor--;
    return true;
  }
  return false;
}

bool SnowballProgram::in_range(int min, int max)
{
  if (cursor >= limit) {
    return false;
  }
  wchar_t ch = current[cursor];
  if (ch > max || ch < min) {
    return false;
  }
  cursor++;
  return true;
}

bool SnowballProgram::in_range_b(int min, int max)
{
  if (cursor <= limit_backward) {
    return false;
  }
  wchar_t ch = current[cursor - 1];
  if (ch > max || ch < min) {
    return false;
  }
  cursor--;
  return true;
}

bool SnowballProgram::out_range(int min, int max)
{
  if (cursor >= limit) {
    return false;
  }
  wchar_t ch = current[cursor];
  if (!(ch > max || ch < min)) {
    return false;
  }
  cursor++;
  return true;
}

bool SnowballProgram::out_range_b(int min, int max)
{
  if (cursor <= limit_backward) {
    return false;
  }
  wchar_t ch = current[cursor - 1];
  if (!(ch > max || ch < min)) {
    return false;
  }
  cursor--;
  return true;
}

bool SnowballProgram::eq_s(int s_size, shared_ptr<std::wstring> s)
{
  if (limit - cursor < s_size) {
    return false;
  }
  int i;
  for (i = 0; i != s_size; i++) {
    if (current[cursor + i] != s->charAt(i)) {
      return false;
    }
  }
  cursor += s_size;
  return true;
}

bool SnowballProgram::eq_s_b(int s_size, shared_ptr<std::wstring> s)
{
  if (cursor - limit_backward < s_size) {
    return false;
  }
  int i;
  for (i = 0; i != s_size; i++) {
    if (current[cursor - s_size + i] != s->charAt(i)) {
      return false;
    }
  }
  cursor -= s_size;
  return true;
}

bool SnowballProgram::eq_v(shared_ptr<std::wstring> s)
{
  return eq_s(s->length(), s);
}

bool SnowballProgram::eq_v_b(shared_ptr<std::wstring> s)
{
  return eq_s_b(s->length(), s);
}

int SnowballProgram::find_among(std::deque<std::shared_ptr<Among>> &v,
                                int v_size)
{
  int i = 0;
  int j = v_size;

  int c = cursor;
  int l = limit;

  int common_i = 0;
  int common_j = 0;

  bool first_key_inspected = false;

  while (true) {
    int k = i + ((j - i) >> 1);
    int diff = 0;
    int common = common_i < common_j ? common_i : common_j; // smaller
    shared_ptr<Among> w = v[k];
    int i2;
    for (i2 = common; i2 < w->s_size; i2++) {
      if (c + common == l) {
        diff = -1;
        break;
      }
      diff = current[c + common] - w->s[i2];
      if (diff != 0) {
        break;
      }
      common++;
    }
    if (diff < 0) {
      j = k;
      common_j = common;
    } else {
      i = k;
      common_i = common;
    }
    if (j - i <= 1) {
      if (i > 0) {
        break; // v->s has been inspected
      }
      if (j == i) {
        break; // only one item in v
      }

      // - but now we need to go round once more to get
      // v->s inspected. This looks messy, but is actually
      // the optimal approach.

      if (first_key_inspected) {
        break;
      }
      first_key_inspected = true;
    }
  }
  while (true) {
    shared_ptr<Among> w = v[i];
    if (common_i >= w->s_size) {
      cursor = c + w->s_size;
      if (w->method == nullptr) {
        return w->result;
      }
      bool res = false;
      try {
        res = static_cast<bool>(w->method->invokeExact(shared_from_this()));
      }
      // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
      catch (Error | runtime_error e) {
        throw e;
      } catch (const runtime_error &e) {
        throw make_shared<UndeclaredThrowableException>(e);
      }
      cursor = c + w->s_size;
      if (res) {
        return w->result;
      }
    }
    i = w->substring_i;
    if (i < 0) {
      return 0;
    }
  }
}

int SnowballProgram::find_among_b(std::deque<std::shared_ptr<Among>> &v,
                                  int v_size)
{
  int i = 0;
  int j = v_size;

  int c = cursor;
  int lb = limit_backward;

  int common_i = 0;
  int common_j = 0;

  bool first_key_inspected = false;

  while (true) {
    int k = i + ((j - i) >> 1);
    int diff = 0;
    int common = common_i < common_j ? common_i : common_j;
    shared_ptr<Among> w = v[k];
    int i2;
    for (i2 = w->s_size - 1 - common; i2 >= 0; i2--) {
      if (c - common == lb) {
        diff = -1;
        break;
      }
      diff = current[c - 1 - common] - w->s[i2];
      if (diff != 0) {
        break;
      }
      common++;
    }
    if (diff < 0) {
      j = k;
      common_j = common;
    } else {
      i = k;
      common_i = common;
    }
    if (j - i <= 1) {
      if (i > 0) {
        break;
      }
      if (j == i) {
        break;
      }
      if (first_key_inspected) {
        break;
      }
      first_key_inspected = true;
    }
  }
  while (true) {
    shared_ptr<Among> w = v[i];
    if (common_i >= w->s_size) {
      cursor = c - w->s_size;
      if (w->method == nullptr) {
        return w->result;
      }

      bool res = false;
      try {
        res = static_cast<bool>(w->method->invokeExact(shared_from_this()));
      }
      // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
      catch (Error | runtime_error e) {
        throw e;
      } catch (const runtime_error &e) {
        throw make_shared<UndeclaredThrowableException>(e);
      }
      cursor = c - w->s_size;
      if (res) {
        return w->result;
      }
    }
    i = w->substring_i;
    if (i < 0) {
      return 0;
    }
  }
}

int SnowballProgram::replace_s(int c_bra, int c_ket, shared_ptr<std::wstring> s)
{
  constexpr int adjustment = s->length() - (c_ket - c_bra);
  constexpr int newLength = limit + adjustment;
  // resize if necessary
  if (newLength > current.size()) {
    std::deque<wchar_t> newBuffer(
        ArrayUtil::oversize(newLength, Character::BYTES));
    System::arraycopy(current, 0, newBuffer, 0, limit);
    current = newBuffer;
  }
  // if the substring being replaced is longer or shorter than the
  // replacement, need to shift things around
  if (adjustment != 0 && c_ket < limit) {
    System::arraycopy(current, c_ket, current, c_bra + s->length(),
                      limit - c_ket);
  }
  // insert the replacement text
  // Note, faster is s.getChars(0, s.length(), current, c_bra);
  // but would have to duplicate this method for both std::wstring and StringBuilder
  for (int i = 0; i < s->length(); i++) {
    current[c_bra + i] = s->charAt(i);
  }

  limit += adjustment;
  if (cursor >= c_ket) {
    cursor += adjustment;
  } else if (cursor > c_bra) {
    cursor = c_bra;
  }
  return adjustment;
}

void SnowballProgram::slice_check()
{
  if (bra < 0 || bra > ket || ket > limit) {
    throw invalid_argument(L"faulty slice operation: bra=" + to_wstring(bra) +
                           L",ket=" + to_wstring(ket) + L",limit=" +
                           to_wstring(limit));
    // FIXME: report error somehow.
    /*
    fprintf(stderr, "faulty slice operation:\n");
    debug(z, -1, 0);
    exit(1);
    */
  }
}

void SnowballProgram::slice_from(shared_ptr<std::wstring> s)
{
  slice_check();
  replace_s(bra, ket, s);
}

void SnowballProgram::slice_del()
{
  slice_from(std::static_pointer_cast<std::wstring>(L""));
}

void SnowballProgram::insert(int c_bra, int c_ket, shared_ptr<std::wstring> s)
{
  int adjustment = replace_s(c_bra, c_ket, s);
  if (c_bra <= bra) {
    bra += adjustment;
  }
  if (c_bra <= ket) {
    ket += adjustment;
  }
}

shared_ptr<StringBuilder> SnowballProgram::slice_to(shared_ptr<StringBuilder> s)
{
  slice_check();
  int len = ket - bra;
  s->setLength(0);
  s->append(current, bra, len);
  return s;
}

shared_ptr<StringBuilder>
SnowballProgram::assign_to(shared_ptr<StringBuilder> s)
{
  s->setLength(0);
  s->append(current, 0, limit);
  return s;
}
} // namespace org::tartarus::snowball