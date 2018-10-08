using namespace std;

#include "TestRegExp.h"

namespace org::apache::lucene::util::automaton
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestRegExp::testSmoke()
{
  shared_ptr<RegExp> r = make_shared<RegExp>(L"a(b+|c+)d");
  shared_ptr<Automaton> a = r->toAutomaton();
  assertTrue(a->isDeterministic());
  shared_ptr<CharacterRunAutomaton> run = make_shared<CharacterRunAutomaton>(a);
  assertTrue(run->run(L"abbbbbd"));
  assertTrue(run->run(L"acd"));
  assertFalse(run->run(L"ad"));
}

void TestRegExp::testDeterminizeTooManyStates()
{
  // LUCENE-6046
  wstring source = L"[ac]*a[ac]{50,200}";
  shared_ptr<TooComplexToDeterminizeException> expected =
      expectThrows(TooComplexToDeterminizeException::typeid,
                   [&]() { (make_shared<RegExp>(source))->toAutomaton(); });
  assertTrue(expected->what()->contains(source));
}

void TestRegExp::testSerializeTooManyStatesToRepeat() 
{
  wstring source = L"a{50001}";
  shared_ptr<TooComplexToDeterminizeException> expected =
      expectThrows(TooComplexToDeterminizeException::typeid, [&]() {
        (make_shared<RegExp>(source))->toAutomaton(50000);
      });
  assertTrue(expected->what()->contains(source));
}

void TestRegExp::testSerializeTooManyStatesToDeterminizeExc() throw(
    runtime_error)
{
  // LUCENE-6046
  wstring source = L"[ac]*a[ac]{50,200}";
  shared_ptr<TooComplexToDeterminizeException> expected =
      expectThrows(TooComplexToDeterminizeException::typeid,
                   [&]() { (make_shared<RegExp>(source))->toAutomaton(); });
  assertTrue(expected->what()->contains(source));
}

void TestRegExp::testRepeatWithEmptyString() 
{
  shared_ptr<Automaton> a =
      (make_shared<RegExp>(L"[^y]*{1,2}"))->toAutomaton(1000);
  // paranoia:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(a->toString()->length() > 0);
}

void TestRegExp::testRepeatWithEmptyLanguage() 
{
  shared_ptr<Automaton> a = (make_shared<RegExp>(L"#*"))->toAutomaton(1000);
  // paranoia:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(a->toString()->length() > 0);
  a = (make_shared<RegExp>(L"#+"))->toAutomaton(1000);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(a->toString()->length() > 0);
  a = (make_shared<RegExp>(L"#{2,10}"))->toAutomaton(1000);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(a->toString()->length() > 0);
  a = (make_shared<RegExp>(L"#?"))->toAutomaton(1000);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(a->toString()->length() > 0);
}
} // namespace org::apache::lucene::util::automaton