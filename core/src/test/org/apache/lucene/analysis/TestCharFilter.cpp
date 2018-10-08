using namespace std;

#include "TestCharFilter.h"

namespace org::apache::lucene::analysis
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCharFilter::testCharFilter1() 
{
  shared_ptr<CharFilter> cs =
      make_shared<CharFilter1>(make_shared<StringReader>(L""));
  assertEquals(L"corrected offset is invalid", 1, cs->correctOffset(0));
}

void TestCharFilter::testCharFilter2() 
{
  shared_ptr<CharFilter> cs =
      make_shared<CharFilter2>(make_shared<StringReader>(L""));
  assertEquals(L"corrected offset is invalid", 2, cs->correctOffset(0));
}

void TestCharFilter::testCharFilter12() 
{
  shared_ptr<CharFilter> cs = make_shared<CharFilter2>(
      make_shared<CharFilter1>(make_shared<StringReader>(L"")));
  assertEquals(L"corrected offset is invalid", 3, cs->correctOffset(0));
}

void TestCharFilter::testCharFilter11() 
{
  shared_ptr<CharFilter> cs = make_shared<CharFilter1>(
      make_shared<CharFilter1>(make_shared<StringReader>(L"")));
  assertEquals(L"corrected offset is invalid", 2, cs->correctOffset(0));
}

TestCharFilter::CharFilter1::CharFilter1(shared_ptr<Reader> in_)
    : CharFilter(in_)
{
}

int TestCharFilter::CharFilter1::read(std::deque<wchar_t> &cbuf, int off,
                                      int len) 
{
  return input->read(cbuf, off, len);
}

int TestCharFilter::CharFilter1::correct(int currentOff)
{
  return currentOff + 1;
}

TestCharFilter::CharFilter2::CharFilter2(shared_ptr<Reader> in_)
    : CharFilter(in_)
{
}

int TestCharFilter::CharFilter2::read(std::deque<wchar_t> &cbuf, int off,
                                      int len) 
{
  return input->read(cbuf, off, len);
}

int TestCharFilter::CharFilter2::correct(int currentOff)
{
  return currentOff + 2;
}
} // namespace org::apache::lucene::analysis