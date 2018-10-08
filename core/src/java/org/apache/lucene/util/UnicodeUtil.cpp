using namespace std;

#include "UnicodeUtil.h"

namespace org::apache::lucene::util
{

const shared_ptr<BytesRef> UnicodeUtil::BIG_TERM = make_shared<BytesRef>(
    std::deque<char>{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1});

UnicodeUtil::UnicodeUtil() {} // no instance

int UnicodeUtil::UTF16toUTF8(std::deque<wchar_t> &source, int const offset,
                             int const length, std::deque<char> &out)
{

  int upto = 0;
  int i = offset;
  constexpr int end = offset + length;

  while (i < end) {

    constexpr int code = static_cast<int>(source[i++]);

    if (code < 0x80) {
      out[upto++] = static_cast<char>(code);
    } else if (code < 0x800) {
      out[upto++] = static_cast<char>(0xC0 | (code >> 6));
      out[upto++] = static_cast<char>(0x80 | (code & 0x3F));
    } else if (code < 0xD800 || code > 0xDFFF) {
      out[upto++] = static_cast<char>(0xE0 | (code >> 12));
      out[upto++] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
      out[upto++] = static_cast<char>(0x80 | (code & 0x3F));
    } else {
      // surrogate pair
      // confirm valid high surrogate
      if (code < 0xDC00 && i < end) {
        int utf32 = static_cast<int>(source[i]);
        // confirm valid low surrogate and write pair
        if (utf32 >= 0xDC00 && utf32 <= 0xDFFF) {
          utf32 = (code << 10) + utf32 + SURROGATE_OFFSET;
          i++;
          out[upto++] = static_cast<char>(0xF0 | (utf32 >> 18));
          out[upto++] = static_cast<char>(0x80 | ((utf32 >> 12) & 0x3F));
          out[upto++] = static_cast<char>(0x80 | ((utf32 >> 6) & 0x3F));
          out[upto++] = static_cast<char>(0x80 | (utf32 & 0x3F));
          continue;
        }
      }
      // replace unpaired surrogate or out-of-order low surrogate
      // with substitution character
      out[upto++] = static_cast<char>(0xEF);
      out[upto++] = static_cast<char>(0xBF);
      out[upto++] = static_cast<char>(0xBD);
    }
  }
  // assert matches(source, offset, length, out, upto);
  return upto;
}

int UnicodeUtil::UTF16toUTF8(shared_ptr<std::wstring> s, int const offset,
                             int const length, std::deque<char> &out)
{
  return UTF16toUTF8(s, offset, length, out, 0);
}

int UnicodeUtil::UTF16toUTF8(shared_ptr<std::wstring> s, int const offset,
                             int const length, std::deque<char> &out,
                             int outOffset)
{
  constexpr int end = offset + length;

  int upto = outOffset;
  for (int i = offset; i < end; i++) {
    constexpr int code = static_cast<int>(s->charAt(i));

    if (code < 0x80) {
      out[upto++] = static_cast<char>(code);
    } else if (code < 0x800) {
      out[upto++] = static_cast<char>(0xC0 | (code >> 6));
      out[upto++] = static_cast<char>(0x80 | (code & 0x3F));
    } else if (code < 0xD800 || code > 0xDFFF) {
      out[upto++] = static_cast<char>(0xE0 | (code >> 12));
      out[upto++] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
      out[upto++] = static_cast<char>(0x80 | (code & 0x3F));
    } else {
      // surrogate pair
      // confirm valid high surrogate
      if (code < 0xDC00 && (i < end - 1)) {
        int utf32 = static_cast<int>(s->charAt(i + 1));
        // confirm valid low surrogate and write pair
        if (utf32 >= 0xDC00 && utf32 <= 0xDFFF) {
          utf32 = (code << 10) + utf32 + SURROGATE_OFFSET;
          i++;
          out[upto++] = static_cast<char>(0xF0 | (utf32 >> 18));
          out[upto++] = static_cast<char>(0x80 | ((utf32 >> 12) & 0x3F));
          out[upto++] = static_cast<char>(0x80 | ((utf32 >> 6) & 0x3F));
          out[upto++] = static_cast<char>(0x80 | (utf32 & 0x3F));
          continue;
        }
      }
      // replace unpaired surrogate or out-of-order low surrogate
      // with substitution character
      out[upto++] = static_cast<char>(0xEF);
      out[upto++] = static_cast<char>(0xBF);
      out[upto++] = static_cast<char>(0xBD);
    }
  }
  // assert matches(s, offset, length, out, upto);
  return upto;
}

int UnicodeUtil::calcUTF16toUTF8Length(shared_ptr<std::wstring> s,
                                       int const offset, int const len)
{
  constexpr int end = offset + len;

  int res = 0;
  for (int i = offset; i < end; i++) {
    constexpr int code = static_cast<int>(s->charAt(i));

    if (code < 0x80) {
      res++;
    } else if (code < 0x800) {
      res += 2;
    } else if (code < 0xD800 || code > 0xDFFF) {
      res += 3;
    } else {
      // surrogate pair
      // confirm valid high surrogate
      if (code < 0xDC00 && (i < end - 1)) {
        int utf32 = static_cast<int>(s->charAt(i + 1));
        // confirm valid low surrogate and write pair
        if (utf32 >= 0xDC00 && utf32 <= 0xDFFF) {
          i++;
          res += 4;
          continue;
        }
      }
      res += 3;
    }
  }

  return res;
}

bool UnicodeUtil::validUTF16String(shared_ptr<std::wstring> s)
{
  constexpr int size = s->length();
  for (int i = 0; i < size; i++) {
    wchar_t ch = s->charAt(i);
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
      if (i < size - 1) {
        i++;
        wchar_t nextCH = s->charAt(i);
        if (nextCH >= UNI_SUR_LOW_START && nextCH <= UNI_SUR_LOW_END) {
          // Valid surrogate pair
        } else {
          // Unmatched high surrogate
          return false;
        }
      } else {
        // Unmatched high surrogate
        return false;
      }
    } else if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
      // Unmatched low surrogate
      return false;
    }
  }

  return true;
}

bool UnicodeUtil::validUTF16String(std::deque<wchar_t> &s, int size)
{
  for (int i = 0; i < size; i++) {
    wchar_t ch = s[i];
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
      if (i < size - 1) {
        i++;
        wchar_t nextCH = s[i];
        if (nextCH >= UNI_SUR_LOW_START && nextCH <= UNI_SUR_LOW_END) {
          // Valid surrogate pair
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
      // Unmatched low surrogate
      return false;
    }
  }

  return true;
}

std::deque<int> const UnicodeUtil::utf8CodeLength;

UnicodeUtil::StaticConstructor::StaticConstructor()
{
  constexpr int v = numeric_limits<int>::min();
  utf8CodeLength = std::deque<int>{
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4};
}

UnicodeUtil::StaticConstructor UnicodeUtil::staticConstructor;

int UnicodeUtil::codePointCount(shared_ptr<BytesRef> utf8)
{
  int pos = utf8->offset;
  constexpr int limit = pos + utf8->length;
  const std::deque<char> bytes = utf8->bytes;

  int codePointCount = 0;
  for (; pos < limit; codePointCount++) {
    int v = bytes[pos] & 0xFF;
    if (v < 0x80) {
      pos += 1;
      continue;
    }
    if (v >= 0xc0) {
      if (v < 0xe0) {
        pos += 2;
        continue;
      }
      if (v < 0xf0) {
        pos += 3;
        continue;
      }
      if (v < 0xf8) {
        pos += 4;
        continue;
      }
      // fallthrough, consider 5 and 6 byte sequences invalid.
    }

    // Anything not covered above is invalid UTF8.
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }

  // Check if we didn't go over the limit on the last character.
  if (pos > limit) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }

  return codePointCount;
}

int UnicodeUtil::UTF8toUTF32(shared_ptr<BytesRef> utf8, std::deque<int> &ints)
{
  // TODO: ints must not be null, should be an assert
  int utf32Count = 0;
  int utf8Upto = utf8->offset;
  const std::deque<char> bytes = utf8->bytes;
  constexpr int utf8Limit = utf8->offset + utf8->length;
  while (utf8Upto < utf8Limit) {
    constexpr int numBytes = utf8CodeLength[bytes[utf8Upto] & 0xFF];
    int v = 0;
    switch (numBytes) {
    case 1:
      ints[utf32Count++] = bytes[utf8Upto++];
      continue;
    case 2:
      // 5 useful bits
      v = bytes[utf8Upto++] & 31;
      break;
    case 3:
      // 4 useful bits
      v = bytes[utf8Upto++] & 15;
      break;
    case 4:
      // 3 useful bits
      v = bytes[utf8Upto++] & 7;
      break;
    default:
      throw invalid_argument(L"invalid utf8");
    }

    // TODO: this may read past utf8's limit.
    constexpr int limit = utf8Upto + numBytes - 1;
    while (utf8Upto < limit) {
      v = v << 6 | bytes[utf8Upto++] & 63;
    }
    ints[utf32Count++] = v;
  }

  return utf32Count;
}

wstring UnicodeUtil::newString(std::deque<int> &codePoints, int offset,
                               int count)
{
  if (count < 0) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }
  std::deque<wchar_t> chars(count);
  int w = 0;
  for (int r = offset, e = offset + count; r < e; ++r) {
    int cp = codePoints[r];
    if (cp < 0 || cp > 0x10ffff) {
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: throw new IllegalArgumentException();
      throw invalid_argument();
    }
    while (true) {
      try {
        if (cp < 0x010000) {
          chars[w] = static_cast<wchar_t>(cp);
          w++;
        } else {
          chars[w] = static_cast<wchar_t>(LEAD_SURROGATE_OFFSET_ +
                                          (cp >> LEAD_SURROGATE_SHIFT_));
          chars[w + 1] = static_cast<wchar_t>(TRAIL_SURROGATE_MIN_VALUE +
                                              (cp & TRAIL_SURROGATE_MASK_));
          w += 2;
        }
        break;
      } catch (const out_of_range &ex) {
        int newlen =
            static_cast<int>(ceil(static_cast<double>(codePoints.size()) *
                                  (w + 2) / (r - offset + 1)));
        std::deque<wchar_t> temp(newlen);
        System::arraycopy(chars, 0, temp, 0, w);
        chars = temp;
      }
    }
  }
  return wstring(chars, 0, w);
}

wstring UnicodeUtil::toHexString(const wstring &s)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < s.length(); i++) {
    wchar_t ch = s[i];
    if (i > 0) {
      sb->append(L' ');
    }
    if (ch < 128) {
      sb->append(ch);
    } else {
      if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
        sb->append(L"H:");
      } else if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
        sb->append(L"L:");
      } else if (ch > UNI_SUR_LOW_END) {
        if (ch == 0xffff) {
          sb->append(L"F:");
        } else {
          sb->append(L"E:");
        }
      }

      sb->append(L"0x" + Integer::toHexString(ch));
    }
  }
  return sb->toString();
}

int UnicodeUtil::UTF8toUTF16(std::deque<char> &utf8, int offset, int length,
                             std::deque<wchar_t> &out)
{
  int out_offset = 0;
  constexpr int limit = offset + length;
  while (offset < limit) {
    int b = utf8[offset++] & 0xff;
    if (b < 0xc0) {
      assert(b < 0x80);
      out[out_offset++] = static_cast<wchar_t>(b);
    } else if (b < 0xe0) {
      out[out_offset++] =
          static_cast<wchar_t>(((b & 0x1f) << 6) + (utf8[offset++] & 0x3f));
    } else if (b < 0xf0) {
      out[out_offset++] = static_cast<wchar_t>(((b & 0xf) << 12) +
                                               ((utf8[offset] & 0x3f) << 6) +
                                               (utf8[offset + 1] & 0x3f));
      offset += 2;
    } else {
      assert((b < 0xf8, L"b = 0x" + Integer::toHexString(b)));
      int ch = ((b & 0x7) << 18) + ((utf8[offset] & 0x3f) << 12) +
               ((utf8[offset + 1] & 0x3f) << 6) + (utf8[offset + 2] & 0x3f);
      offset += 3;
      if (ch < UNI_MAX_BMP) {
        out[out_offset++] = static_cast<wchar_t>(ch);
      } else {
        int chHalf = ch - 0x0010000;
        out[out_offset++] = static_cast<wchar_t>((chHalf >> 10) + 0xD800);
        out[out_offset++] = static_cast<wchar_t>((chHalf & HALF_MASK) + 0xDC00);
      }
    }
  }
  return out_offset;
}

int UnicodeUtil::maxUTF8Length(int utf16Length)
{
  return Math::multiplyExact(utf16Length, MAX_UTF8_BYTES_PER_CHAR);
}

int UnicodeUtil::UTF8toUTF16(shared_ptr<BytesRef> bytesRef,
                             std::deque<wchar_t> &chars)
{
  return UTF8toUTF16(bytesRef->bytes, bytesRef->offset, bytesRef->length,
                     chars);
}
} // namespace org::apache::lucene::util