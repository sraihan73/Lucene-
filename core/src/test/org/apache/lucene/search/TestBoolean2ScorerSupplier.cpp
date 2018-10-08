using namespace std;

#include "TestBoolean2ScorerSupplier.h"

namespace org::apache::lucene::search
{
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

TestBoolean2ScorerSupplier::FakeScorer::FakeScorer(int64_t cost)
    : Scorer(nullptr), it(DocIdSetIterator::all(Math::toIntExact(cost)))
{
}

int TestBoolean2ScorerSupplier::FakeScorer::docID() { return it->docID(); }

float TestBoolean2ScorerSupplier::FakeScorer::score() 
{
  return 1;
}

shared_ptr<DocIdSetIterator> TestBoolean2ScorerSupplier::FakeScorer::iterator()
{
  return it;
}

wstring TestBoolean2ScorerSupplier::FakeScorer::toString()
{
  return L"FakeScorer(cost=" + to_wstring(it->cost()) + L")";
}

TestBoolean2ScorerSupplier::FakeScorerSupplier::FakeScorerSupplier(
    int64_t cost)
    : cost(cost), leadCost(nullopt)
{
}

TestBoolean2ScorerSupplier::FakeScorerSupplier::FakeScorerSupplier(
    int64_t cost, int64_t leadCost)
    : cost(cost), leadCost(leadCost)
{
}

shared_ptr<Scorer> TestBoolean2ScorerSupplier::FakeScorerSupplier::get(
    int64_t leadCost) 
{
  if (this->leadCost) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(this->toString(), this->leadCost.value(), leadCost);
  }
  return make_shared<FakeScorer>(cost_);
}

int64_t TestBoolean2ScorerSupplier::FakeScorerSupplier::cost()
{
  return cost_;
}

wstring TestBoolean2ScorerSupplier::FakeScorerSupplier::toString()
{
  return L"FakeLazyScorer(cost=" + to_wstring(cost_) + L",leadCost=" +
         leadCost + L")";
}

void TestBoolean2ScorerSupplier::testConjunctionCost()
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  subs[RandomPicks::randomFrom(random(),
                               Arrays::asList(Occur::FILTER, Occur::MUST))]
      ->add(make_shared<FakeScorerSupplier>(42));
  TestUtil::assertEquals(42, (make_shared<Boolean2ScorerSupplier>(
                                  nullptr, subs, random()->nextBoolean(), 0))
                                 ->cost());

  subs[RandomPicks::randomFrom(random(),
                               Arrays::asList(Occur::FILTER, Occur::MUST))]
      ->add(make_shared<FakeScorerSupplier>(12));
  TestUtil::assertEquals(12, (make_shared<Boolean2ScorerSupplier>(
                                  nullptr, subs, random()->nextBoolean(), 0))
                                 ->cost());

  subs[RandomPicks::randomFrom(random(),
                               Arrays::asList(Occur::FILTER, Occur::MUST))]
      ->add(make_shared<FakeScorerSupplier>(20));
  TestUtil::assertEquals(12, (make_shared<Boolean2ScorerSupplier>(
                                  nullptr, subs, random()->nextBoolean(), 0))
                                 ->cost());
}

void TestBoolean2ScorerSupplier::testDisjunctionCost() 
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(42));
  shared_ptr<ScorerSupplier> s = make_shared<Boolean2ScorerSupplier>(
      nullptr, subs, random()->nextBoolean(), 0);
  TestUtil::assertEquals(42, s->cost());
  TestUtil::assertEquals(42, s->get(random()->nextInt(100)).begin().cost());

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12));
  s = make_shared<Boolean2ScorerSupplier>(nullptr, subs,
                                          random()->nextBoolean(), 0);
  TestUtil::assertEquals(42 + 12, s->cost());
  TestUtil::assertEquals(42 + 12,
                         s->get(random()->nextInt(100)).begin().cost());

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(20));
  s = make_shared<Boolean2ScorerSupplier>(nullptr, subs,
                                          random()->nextBoolean(), 0);
  TestUtil::assertEquals(42 + 12 + 20, s->cost());
  TestUtil::assertEquals(42 + 12 + 20,
                         s->get(random()->nextInt(100)).begin().cost());
}

void TestBoolean2ScorerSupplier::testDisjunctionWithMinShouldMatchCost() throw(
    IOException)
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(42));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12));
  shared_ptr<ScorerSupplier> s = make_shared<Boolean2ScorerSupplier>(
      nullptr, subs, random()->nextBoolean(), 1);
  TestUtil::assertEquals(42 + 12, s->cost());
  TestUtil::assertEquals(42 + 12,
                         s->get(random()->nextInt(100)).begin().cost());

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(20));
  s = make_shared<Boolean2ScorerSupplier>(nullptr, subs,
                                          random()->nextBoolean(), 1);
  TestUtil::assertEquals(42 + 12 + 20, s->cost());
  TestUtil::assertEquals(42 + 12 + 20,
                         s->get(random()->nextInt(100)).begin().cost());
  s = make_shared<Boolean2ScorerSupplier>(nullptr, subs,
                                          random()->nextBoolean(), 2);
  TestUtil::assertEquals(12 + 20, s->cost());
  TestUtil::assertEquals(12 + 20,
                         s->get(random()->nextInt(100)).begin().cost());

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(30));
  s = make_shared<Boolean2ScorerSupplier>(nullptr, subs,
                                          random()->nextBoolean(), 1);
  TestUtil::assertEquals(42 + 12 + 20 + 30, s->cost());
  TestUtil::assertEquals(42 + 12 + 20 + 30,
                         s->get(random()->nextInt(100)).begin().cost());
  s = make_shared<Boolean2ScorerSupplier>(nullptr, subs,
                                          random()->nextBoolean(), 2);
  TestUtil::assertEquals(12 + 20 + 30, s->cost());
  TestUtil::assertEquals(12 + 20 + 30,
                         s->get(random()->nextInt(100)).begin().cost());
  s = make_shared<Boolean2ScorerSupplier>(nullptr, subs,
                                          random()->nextBoolean(), 3);
  TestUtil::assertEquals(12 + 20, s->cost());
  TestUtil::assertEquals(12 + 20,
                         s->get(random()->nextInt(100)).begin().cost());
}

void TestBoolean2ScorerSupplier::testDuelCost() 
{
  constexpr int iters = atLeast(1000);
  for (int iter = 0; iter < iters; ++iter) {
    unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
        make_shared<
            EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
            Occur::typeid);
    for (Occur occur : Occur::values()) {
      subs.emplace(occur, deque<>());
    }
    int numClauses = TestUtil::nextInt(random(), 1, 10);
    int numShoulds = 0;
    int numRequired = 0;
    for (int j = 0; j < numClauses; ++j) {
      Occur occur = RandomPicks::randomFrom(random(), Occur::values());
      subs[occur]->add(make_shared<FakeScorerSupplier>(random()->nextInt(100)));
      if (occur == Occur::SHOULD) {
        ++numShoulds;
      } else if (occur == Occur::FILTER || occur == Occur::MUST) {
        numRequired++;
      }
    }
    bool needsScores = random()->nextBoolean();
    if (needsScores == false && numRequired > 0) {
      numClauses -= numShoulds;
      numShoulds = 0;
      subs[Occur::SHOULD]->clear();
    }
    if (numShoulds + numRequired == 0) {
      // only negative clauses, invalid
      continue;
    }
    int minShouldMatch =
        numShoulds == 0 ? 0 : TestUtil::nextInt(random(), 0, numShoulds - 1);
    shared_ptr<Boolean2ScorerSupplier> supplier =
        make_shared<Boolean2ScorerSupplier>(nullptr, subs, needsScores,
                                            minShouldMatch);
    int64_t cost1 = supplier->cost();
    int64_t cost2 =
        supplier->get(numeric_limits<int64_t>::max()).begin().cost();
    assertEquals(L"clauses=" + subs + L", minShouldMatch=" +
                     to_wstring(minShouldMatch),
                 cost1, cost2);
  }
}

void TestBoolean2ScorerSupplier::testFakeScorerSupplier()
{
  shared_ptr<FakeScorerSupplier> randomAccessSupplier =
      make_shared<FakeScorerSupplier>(random()->nextInt(100), 30);
  expectThrows(AssertionError::typeid,
               [&]() { randomAccessSupplier->get(70); });
  shared_ptr<FakeScorerSupplier> sequentialSupplier =
      make_shared<FakeScorerSupplier>(random()->nextInt(100), 70);
  expectThrows(AssertionError::typeid, [&]() { sequentialSupplier->get(30); });
}

void TestBoolean2ScorerSupplier::testConjunctionLeadCost() 
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  // If the clauses are less costly than the lead cost, the min cost is the new
  // lead cost
  subs[RandomPicks::randomFrom(random(),
                               Arrays::asList(Occur::FILTER, Occur::MUST))]
      ->add(make_shared<FakeScorerSupplier>(42, 12));
  subs[RandomPicks::randomFrom(random(),
                               Arrays::asList(Occur::FILTER, Occur::MUST))]
      ->add(make_shared<FakeScorerSupplier>(12, 12));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       0))
      ->get(numeric_limits<int64_t>::max()); // triggers assertions as a
                                               // side-effect

  subs = make_shared<EnumMap<>>(Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  // If the lead cost is less that the clauses' cost, then we don't modify it
  subs[RandomPicks::randomFrom(random(),
                               Arrays::asList(Occur::FILTER, Occur::MUST))]
      ->add(make_shared<FakeScorerSupplier>(42, 7));
  subs[RandomPicks::randomFrom(random(),
                               Arrays::asList(Occur::FILTER, Occur::MUST))]
      ->add(make_shared<FakeScorerSupplier>(12, 7));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       0))
      ->get(7); // triggers assertions as a side-effect
}

void TestBoolean2ScorerSupplier::testDisjunctionLeadCost() 
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(42, 54));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12, 54));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       0))
      ->get(100); // triggers assertions as a side-effect

  subs[Occur::SHOULD]->clear();
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(42, 20));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12, 20));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       0))
      ->get(20); // triggers assertions as a side-effect
}

void TestBoolean2ScorerSupplier::
    testDisjunctionWithMinShouldMatchLeadCost() 
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  // minShouldMatch is 2 so the 2 least costly clauses will lead iteration
  // and their cost will be 30+12=42
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(50, 42));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12, 42));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(30, 42));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       2))
      ->get(100); // triggers assertions as a side-effect

  subs = make_shared<EnumMap<>>(Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  // If the leadCost is less than the msm cost, then it wins
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(42, 20));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12, 20));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(30, 20));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       2))
      ->get(20); // triggers assertions as a side-effect

  subs = make_shared<EnumMap<>>(Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(42, 62));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12, 62));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(30, 62));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(20, 62));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       2))
      ->get(100); // triggers assertions as a side-effect

  subs = make_shared<EnumMap<>>(Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(42, 32));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(12, 32));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(30, 32));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(20, 32));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       3))
      ->get(100); // triggers assertions as a side-effect
}

void TestBoolean2ScorerSupplier::testProhibitedLeadCost() 
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  // The MUST_NOT clause is called with the same lead cost as the MUST clause
  subs[Occur::MUST]->add(make_shared<FakeScorerSupplier>(42, 42));
  subs[Occur::MUST_NOT]->add(make_shared<FakeScorerSupplier>(30, 42));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       0))
      ->get(100); // triggers assertions as a side-effect

  subs[Occur::MUST]->clear();
  subs[Occur::MUST_NOT]->clear();
  subs[Occur::MUST]->add(make_shared<FakeScorerSupplier>(42, 42));
  subs[Occur::MUST_NOT]->add(make_shared<FakeScorerSupplier>(80, 42));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       0))
      ->get(100); // triggers assertions as a side-effect

  subs[Occur::MUST]->clear();
  subs[Occur::MUST_NOT]->clear();
  subs[Occur::MUST]->add(make_shared<FakeScorerSupplier>(42, 20));
  subs[Occur::MUST_NOT]->add(make_shared<FakeScorerSupplier>(30, 20));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, random()->nextBoolean(),
                                       0))
      ->get(20); // triggers assertions as a side-effect
}

void TestBoolean2ScorerSupplier::testMixedLeadCost() 
{
  unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> subs =
      make_shared<EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    subs.emplace(occur, deque<>());
  }

  // The SHOULD clause is always called with the same lead cost as the MUST
  // clause
  subs[Occur::MUST]->add(make_shared<FakeScorerSupplier>(42, 42));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(30, 42));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, true, 0))
      ->get(100); // triggers assertions as a side-effect

  subs[Occur::MUST]->clear();
  subs[Occur::SHOULD]->clear();
  subs[Occur::MUST]->add(make_shared<FakeScorerSupplier>(42, 42));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(80, 42));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, true, 0))
      ->get(100); // triggers assertions as a side-effect

  subs[Occur::MUST]->clear();
  subs[Occur::SHOULD]->clear();
  subs[Occur::MUST]->add(make_shared<FakeScorerSupplier>(42, 20));
  subs[Occur::SHOULD]->add(make_shared<FakeScorerSupplier>(80, 20));
  (make_shared<Boolean2ScorerSupplier>(nullptr, subs, true, 0))
      ->get(20); // triggers assertions as a side-effect
}
} // namespace org::apache::lucene::search