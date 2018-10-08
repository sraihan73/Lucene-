using namespace std;

#include "TestFSTsMisc.h"

namespace org::apache::lucene::util::fst
{
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using TwoLongs =
    org::apache::lucene::util::fst::UpToTwoPositiveIntOutputs::TwoLongs;
//    import static org.apache.lucene.util.fst.FSTTester.getRandomString;
//    import static org.apache.lucene.util.fst.FSTTester.toIntsRef;

void TestFSTsMisc::setUp() 
{
  LuceneTestCase::setUp();
  dir = newMockDirectory();
}

void TestFSTsMisc::tearDown() 
{
  // can be null if we force simpletext (funky, some kind of bug in test runner
  // maybe)
  if (dir != nullptr) {
    delete dir;
  }
  LuceneTestCase::tearDown();
}

void TestFSTsMisc::testRandomWords() 
{
  testRandomWords(1000, LuceneTestCase::atLeast(random(), 2));
  // testRandomWords(100, 1);
}

void TestFSTsMisc::testRandomWords(int maxNumWords,
                                   int numIter) 
{
  shared_ptr<Random> random =
      make_shared<Random>(TestFSTsMisc::random()->nextLong());
  for (int iter = 0; iter < numIter; iter++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter " << iter << endl;
    }
    for (int inputMode = 0; inputMode < 2; inputMode++) {
      constexpr int numWords = random->nextInt(maxNumWords + 1);
      shared_ptr<Set<std::shared_ptr<IntsRef>>> termsSet =
          unordered_set<std::shared_ptr<IntsRef>>();
      std::deque<std::shared_ptr<IntsRef>> terms(numWords);
      while (termsSet->size() < numWords) {
        const wstring term = getRandomString(random);
        termsSet->add(toIntsRef(term, inputMode));
      }
      doTest(inputMode, termsSet->toArray(std::deque<std::shared_ptr<IntsRef>>(
                            termsSet->size())));
    }
  }
}

void TestFSTsMisc::doTest(
    int inputMode,
    std::deque<std::shared_ptr<IntsRef>> &terms) 
{
  Arrays::sort(terms);

  // Up to two positive ints, shared, generally but not
  // monotonically increasing
  {
    if (VERBOSE) {
      wcout << L"TEST: now test UpToTwoPositiveIntOutputs" << endl;
    }
    shared_ptr<UpToTwoPositiveIntOutputs> *const outputs =
        UpToTwoPositiveIntOutputs::getSingleton(true);
    const deque<FSTTester::InputOutput<any>> pairs =
        deque<FSTTester::InputOutput<any>>(terms.size());
    int64_t lastOutput = 0;
    for (int idx = 0; idx < terms.size(); idx++) {
      // Sometimes go backwards
      int64_t value = lastOutput + TestUtil::nextInt(random(), -100, 1000);
      while (value < 0) {
        value = lastOutput + TestUtil::nextInt(random(), -100, 1000);
      }
      constexpr any output;
      if (random()->nextInt(5) == 3) {
        int64_t value2 = lastOutput + TestUtil::nextInt(random(), -100, 1000);
        while (value2 < 0) {
          value2 = lastOutput + TestUtil::nextInt(random(), -100, 1000);
        }
        deque<int64_t> values = deque<int64_t>();
        values.push_back(value);
        values.push_back(value2);
        output = values;
      } else {
        output = outputs->get(value);
      }
      pairs.push_back(
          make_shared<FSTTester::InputOutput<>>(terms[idx], output));
    }
    make_shared<FSTTesterAnonymousInnerClass>(shared_from_this(), random(), dir,
                                              inputMode)
        .doTest(false);
  }

  // ListOfOutputs(PositiveIntOutputs), generally but not
  // monotonically increasing
  {
    if (VERBOSE) {
      wcout << L"TEST: now test OneOrMoreOutputs" << endl;
    }
    shared_ptr<PositiveIntOutputs> *const _outputs =
        PositiveIntOutputs::getSingleton();
    shared_ptr<ListOfOutputs<int64_t>> *const outputs =
        make_shared<ListOfOutputs<int64_t>>(_outputs);
    const deque<FSTTester::InputOutput<any>> pairs =
        deque<FSTTester::InputOutput<any>>(terms.size());
    int64_t lastOutput = 0;
    for (int idx = 0; idx < terms.size(); idx++) {

      int outputCount = TestUtil::nextInt(random(), 1, 7);
      deque<int64_t> values = deque<int64_t>();
      for (int i = 0; i < outputCount; i++) {
        // Sometimes go backwards
        int64_t value = lastOutput + TestUtil::nextInt(random(), -100, 1000);
        while (value < 0) {
          value = lastOutput + TestUtil::nextInt(random(), -100, 1000);
        }
        values.push_back(value);
        lastOutput = value;
      }

      constexpr any output;
      if (values.size() == 1) {
        output = values[0];
      } else {
        output = values;
      }

      pairs.push_back(
          make_shared<FSTTester::InputOutput<>>(terms[idx], output));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, false))
        ->doTest(false);
  }
}

TestFSTsMisc::FSTTesterAnonymousInnerClass::FSTTesterAnonymousInnerClass(
    shared_ptr<TestFSTsMisc> outerInstance, shared_ptr<Random> random,
    shared_ptr<MockDirectoryWrapper> dir, int inputMode)
    : FSTTester<Object>(random, dir, inputMode, pairs, outputs, false)
{
  this->outerInstance = outerInstance;
}

bool TestFSTsMisc::FSTTesterAnonymousInnerClass::outputsEqual(any output1,
                                                              any output2)
{
  if (std::dynamic_pointer_cast<TwoLongs>(output1) != nullptr &&
      dynamic_cast<deque>(output2) != nullptr) {
    shared_ptr<TwoLongs> twoLongs1 =
        any_cast<std::shared_ptr<TwoLongs>>(output1);
    return Arrays::asList(std::deque<optional<int64_t>>{twoLongs1->first,
                                                           twoLongs1->second})
        .equals(output2);
  } else if (std::dynamic_pointer_cast<TwoLongs>(output2) != nullptr &&
             dynamic_cast<deque>(output1) != nullptr) {
    shared_ptr<TwoLongs> twoLongs2 =
        any_cast<std::shared_ptr<TwoLongs>>(output2);
    return Arrays::asList(std::deque<optional<int64_t>>{twoLongs2->first,
                                                           twoLongs2->second})
        .equals(output1);
  }
  return output1.equals(output2);
}

void TestFSTsMisc::testListOfOutputs() 
{
  shared_ptr<PositiveIntOutputs> _outputs = PositiveIntOutputs::getSingleton();
  shared_ptr<ListOfOutputs<int64_t>> outputs =
      make_shared<ListOfOutputs<int64_t>>(_outputs);
  shared_ptr<Builder<any>> *const builder =
      make_shared<Builder<any>>(FST::INPUT_TYPE::BYTE1, outputs);

  shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();
  // Add the same input more than once and the outputs
  // are merged:
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"a"), scratch), 1LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"a"), scratch), 3LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"a"), scratch), 0LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"b"), scratch), 17LL);
  shared_ptr<FST<any>> *const fst = builder->finish();

  any output = Util::get(fst, make_shared<BytesRef>(L"a"));
  assertNotNull(output);
  deque<int64_t> outputList = outputs->asList(output);
  TestUtil::assertEquals(3, outputList.size());
  TestUtil::assertEquals(1LL, outputList[0]);
  TestUtil::assertEquals(3LL, outputList[1]);
  TestUtil::assertEquals(0LL, outputList[2]);

  output = Util::get(fst, make_shared<BytesRef>(L"b"));
  assertNotNull(output);
  outputList = outputs->asList(output);
  TestUtil::assertEquals(1, outputList.size());
  TestUtil::assertEquals(17LL, outputList[0]);
}

void TestFSTsMisc::testListOfOutputsEmptyString() 
{
  shared_ptr<PositiveIntOutputs> _outputs = PositiveIntOutputs::getSingleton();
  shared_ptr<ListOfOutputs<int64_t>> outputs =
      make_shared<ListOfOutputs<int64_t>>(_outputs);
  shared_ptr<Builder<any>> *const builder =
      make_shared<Builder<any>>(FST::INPUT_TYPE::BYTE1, outputs);

  shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();
  builder->add(scratch->get(), 0LL);
  builder->add(scratch->get(), 1LL);
  builder->add(scratch->get(), 17LL);
  builder->add(scratch->get(), 1LL);

  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"a"), scratch), 1LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"a"), scratch), 3LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"a"), scratch), 0LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"b"), scratch), 0LL);

  shared_ptr<FST<any>> *const fst = builder->finish();

  any output = Util::get(fst, make_shared<BytesRef>(L""));
  assertNotNull(output);
  deque<int64_t> outputList = outputs->asList(output);
  TestUtil::assertEquals(4, outputList.size());
  TestUtil::assertEquals(0LL, outputList[0]);
  TestUtil::assertEquals(1LL, outputList[1]);
  TestUtil::assertEquals(17LL, outputList[2]);
  TestUtil::assertEquals(1LL, outputList[3]);

  output = Util::get(fst, make_shared<BytesRef>(L"a"));
  assertNotNull(output);
  outputList = outputs->asList(output);
  TestUtil::assertEquals(3, outputList.size());
  TestUtil::assertEquals(1LL, outputList[0]);
  TestUtil::assertEquals(3LL, outputList[1]);
  TestUtil::assertEquals(0LL, outputList[2]);

  output = Util::get(fst, make_shared<BytesRef>(L"b"));
  assertNotNull(output);
  outputList = outputs->asList(output);
  TestUtil::assertEquals(1, outputList.size());
  TestUtil::assertEquals(0LL, outputList[0]);
}
} // namespace org::apache::lucene::util::fst