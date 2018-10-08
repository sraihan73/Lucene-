using namespace std;

#include "TestStemmer.h"
#include "../../../../java/org/egothor/stemmer/Gener.h"
#include "../../../../java/org/egothor/stemmer/Lift.h"
#include "../../../../java/org/egothor/stemmer/MultiTrie.h"
#include "../../../../java/org/egothor/stemmer/MultiTrie2.h"
#include "../../../../java/org/egothor/stemmer/Optimizer.h"
#include "../../../../java/org/egothor/stemmer/Optimizer2.h"
#include "../../../../java/org/egothor/stemmer/Trie.h"

namespace org::egothor::stemmer
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestStemmer::testTrie()
{
  shared_ptr<Trie> t = make_shared<Trie>(true);

  std::deque<wstring> keys = {L"a", L"ba", L"bb", L"c"};
  std::deque<wstring> vals = {L"1", L"2", L"2", L"4"};

  for (int i = 0; i < keys.size(); i++) {
    t->add(keys[i], vals[i]);
  }

  assertEquals(0, t->root);
  assertEquals(2, t->rows.size());
  assertEquals(3, t->cmds.size());
  assertTrieContents(t, keys, vals);
}

void TestStemmer::testTrieBackwards()
{
  shared_ptr<Trie> t = make_shared<Trie>(false);

  std::deque<wstring> keys = {L"a", L"ba", L"bb", L"c"};
  std::deque<wstring> vals = {L"1", L"2", L"2", L"4"};

  for (int i = 0; i < keys.size(); i++) {
    t->add(keys[i], vals[i]);
  }

  assertTrieContents(t, keys, vals);
}

void TestStemmer::testMultiTrie()
{
  shared_ptr<Trie> t = make_shared<MultiTrie>(true);

  std::deque<wstring> keys = {L"a", L"ba", L"bb", L"c"};
  std::deque<wstring> vals = {L"1", L"2", L"2", L"4"};

  for (int i = 0; i < keys.size(); i++) {
    t->add(keys[i], vals[i]);
  }

  assertTrieContents(t, keys, vals);
}

void TestStemmer::testMultiTrieBackwards()
{
  shared_ptr<Trie> t = make_shared<MultiTrie>(false);

  std::deque<wstring> keys = {L"a", L"ba", L"bb", L"c"};
  std::deque<wstring> vals = {L"1", L"2", L"2", L"4"};

  for (int i = 0; i < keys.size(); i++) {
    t->add(keys[i], vals[i]);
  }

  assertTrieContents(t, keys, vals);
}

void TestStemmer::testMultiTrie2()
{
  shared_ptr<Trie> t = make_shared<MultiTrie2>(true);

  std::deque<wstring> keys = {L"a", L"ba", L"bb", L"c"};
  /*
   * short vals won't work, see line 155 for example
   * the IOOBE is caught (wierd), but shouldnt affect patch cmds?
   */
  std::deque<wstring> vals = {L"1111", L"2222", L"2223", L"4444"};

  for (int i = 0; i < keys.size(); i++) {
    t->add(keys[i], vals[i]);
  }

  assertTrieContents(t, keys, vals);
}

void TestStemmer::testMultiTrie2Backwards()
{
  shared_ptr<Trie> t = make_shared<MultiTrie2>(false);

  std::deque<wstring> keys = {L"a", L"ba", L"bb", L"c"};
  /*
   * short vals won't work, see line 155 for example
   * the IOOBE is caught (wierd), but shouldnt affect patch cmds?
   */
  std::deque<wstring> vals = {L"1111", L"2222", L"2223", L"4444"};

  for (int i = 0; i < keys.size(); i++) {
    t->add(keys[i], vals[i]);
  }

  assertTrieContents(t, keys, vals);
}

void TestStemmer::assertTrieContents(shared_ptr<Trie> trie,
                                     std::deque<wstring> &keys,
                                     std::deque<wstring> &vals)
{
  std::deque<std::shared_ptr<Trie>> tries = {
      trie,
      trie->reduce(make_shared<Optimizer>()),
      trie->reduce(make_shared<Optimizer2>()),
      trie->reduce(make_shared<Gener>()),
      trie->reduce(make_shared<Lift>(true)),
      trie->reduce(make_shared<Lift>(false))};

  for (auto t : tries) {
    for (int i = 0; i < keys.size(); i++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(vals[i], t->getFully(keys[i])->toString());
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(vals[i], t->getLastOnPath(keys[i])->toString());
    }
  }
}
} // namespace org::egothor::stemmer