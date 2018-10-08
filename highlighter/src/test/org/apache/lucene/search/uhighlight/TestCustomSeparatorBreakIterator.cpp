using namespace std;

#include "TestCustomSeparatorBreakIterator.h"

namespace org::apache::lucene::search::uhighlight
{
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static
//    org.apache.lucene.search.uhighlight.TestWholeBreakIterator.assertSameBreaks;
//    import static org.hamcrest.CoreMatchers.equalTo;
std::deque<optional<wchar_t>> const
    TestCustomSeparatorBreakIterator::SEPARATORS =
        std::deque<optional<wchar_t>>{L' ', L'\u0000', 8233};

void TestCustomSeparatorBreakIterator::testBreakOnCustomSeparator() throw(
    runtime_error)
{
  optional<wchar_t> separator = randomSeparator();
  shared_ptr<BreakIterator> bi =
      make_shared<CustomSeparatorBreakIterator>(separator);
  wstring source = L"this" + separator + L"is" + separator + L"the" +
                   separator + L"first" + separator + L"sentence";
  bi->setText(source);
  assertThat(bi->current(), equalTo(0));
  assertThat(bi->first(), equalTo(0));
  assertThat(source.substr(bi->current(), bi->next() - bi->current()),
             equalTo(L"this" + separator));
  assertThat(source.substr(bi->current(), bi->next() - bi->current()),
             equalTo(L"is" + separator));
  assertThat(source.substr(bi->current(), bi->next() - bi->current()),
             equalTo(L"the" + separator));
  assertThat(source.substr(bi->current(), bi->next() - bi->current()),
             equalTo(L"first" + separator));
  assertThat(source.substr(bi->current(), bi->next() - bi->current()),
             equalTo(L"sentence"));
  assertThat(bi->next(), equalTo(BreakIterator::DONE));

  assertThat(bi->last(), equalTo(source.length()));
  int current = bi->current();
  assertThat(source.substr(bi->previous(), current - bi->previous()),
             equalTo(L"sentence"));
  current = bi->current();
  assertThat(source.substr(bi->previous(), current - bi->previous()),
             equalTo(L"first" + separator));
  current = bi->current();
  assertThat(source.substr(bi->previous(), current - bi->previous()),
             equalTo(L"the" + separator));
  current = bi->current();
  assertThat(source.substr(bi->previous(), current - bi->previous()),
             equalTo(L"is" + separator));
  current = bi->current();
  assertThat(source.substr(bi->previous(), current - bi->previous()),
             equalTo(L"this" + separator));
  assertThat(bi->previous(), equalTo(BreakIterator::DONE));
  assertThat(bi->current(), equalTo(0));

  assertThat(
      source.substr(0, bi->following(9)),
      equalTo(L"this" + separator + L"is" + separator + L"the" + separator));

  assertThat(source.substr(0, bi->preceding(9)),
             equalTo(L"this" + separator + L"is" + separator));

  assertThat(bi->first(), equalTo(0));
  assertThat(
      source.substr(0, bi->next(3)),
      equalTo(L"this" + separator + L"is" + separator + L"the" + separator));
}

void TestCustomSeparatorBreakIterator::testSingleSentences() throw(
    runtime_error)
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual =
      make_shared<CustomSeparatorBreakIterator>(randomSeparator());
  assertSameBreaks(L"a", expected, actual);
  assertSameBreaks(L"ab", expected, actual);
  assertSameBreaks(L"abc", expected, actual);
  assertSameBreaks(L"", expected, actual);
}

void TestCustomSeparatorBreakIterator::testSliceEnd() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual =
      make_shared<CustomSeparatorBreakIterator>(randomSeparator());
  assertSameBreaks(L"a000", 0, 1, expected, actual);
  assertSameBreaks(L"ab000", 0, 1, expected, actual);
  assertSameBreaks(L"abc000", 0, 1, expected, actual);
  assertSameBreaks(L"000", 0, 0, expected, actual);
}

void TestCustomSeparatorBreakIterator::testSliceStart() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual =
      make_shared<CustomSeparatorBreakIterator>(randomSeparator());
  assertSameBreaks(L"000a", 3, 1, expected, actual);
  assertSameBreaks(L"000ab", 3, 2, expected, actual);
  assertSameBreaks(L"000abc", 3, 3, expected, actual);
  assertSameBreaks(L"000", 3, 0, expected, actual);
}

void TestCustomSeparatorBreakIterator::testSliceMiddle() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual =
      make_shared<CustomSeparatorBreakIterator>(randomSeparator());
  assertSameBreaks(L"000a000", 3, 1, expected, actual);
  assertSameBreaks(L"000ab000", 3, 2, expected, actual);
  assertSameBreaks(L"000abc000", 3, 3, expected, actual);
  assertSameBreaks(L"000000", 3, 0, expected, actual);
}

void TestCustomSeparatorBreakIterator::testFirstPosition() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual =
      make_shared<CustomSeparatorBreakIterator>(randomSeparator());
  assertSameBreaks(L"000ab000", 3, 2, 4, expected, actual);
}

wchar_t TestCustomSeparatorBreakIterator::randomSeparator()
{
  return RandomPicks::randomFrom(random(), SEPARATORS);
}
} // namespace org::apache::lucene::search::uhighlight