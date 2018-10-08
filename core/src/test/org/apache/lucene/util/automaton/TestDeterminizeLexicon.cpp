using namespace std;

#include "TestDeterminizeLexicon.h"

namespace org::apache::lucene::util::automaton
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDeterminizeLexicon::testLexicon() 
{
  int num = atLeast(1);
  for (int i = 0; i < num; i++) {
    automata.clear();
    terms.clear();
    for (int j = 0; j < 5000; j++) {
      wstring randomString = TestUtil::randomUnicodeString(random());
      terms.push_back(randomString);
      automata.push_back(Automata::makeString(randomString));
    }
    assertLexicon();
  }
}

void TestDeterminizeLexicon::assertLexicon() 
{
  Collections::shuffle(automata, random());
  shared_ptr<Automaton> lex = Operations::union_(automata);
  lex = Operations::determinize(lex, 1000000);
  assertTrue(Operations::isFinite(lex));
  for (auto s : terms) {
    assertTrue(Operations::run(lex, s));
  }
  shared_ptr<ByteRunAutomaton> *const lexByte =
      make_shared<ByteRunAutomaton>(lex, false, 1000000);
  for (auto s : terms) {
    std::deque<char> bytes = s.getBytes(StandardCharsets::UTF_8);
    assertTrue(lexByte->run(bytes, 0, bytes.size()));
  }
}
} // namespace org::apache::lucene::util::automaton