using namespace std;

#include "TestFSTs.h"

namespace org::apache::lucene::util::fst
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using org::apache::lucene::util::LuceneTestCase::Slow;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using org::apache::lucene::util::fst::BytesRefFSTEnum::InputOutput;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;
using org::apache::lucene::util::fst::PairOutputs::Pair;
using org::apache::lucene::util::fst::Util::Result;
//    import static org.apache.lucene.util.fst.FSTTester.getRandomString;
//    import static org.apache.lucene.util.fst.FSTTester.simpleRandomString;
//    import static org.apache.lucene.util.fst.FSTTester.toIntsRef;

void TestFSTs::setUp() 
{
  LuceneTestCase::setUp();
  dir = newMockDirectory();
}

void TestFSTs::tearDown() 
{
  // can be null if we force simpletext (funky, some kind of bug in test runner
  // maybe)
  if (dir != nullptr) {
    delete dir;
  }
  LuceneTestCase::tearDown();
}

void TestFSTs::testBasicFSA() 
{
  std::deque<wstring> strings = {L"station", L"commotion", L"elation",
                                  L"elastic", L"plastic",   L"stop",
                                  L"ftop",    L"ftation",   L"stat"};
  std::deque<wstring> strings2 = {L"station", L"commotion", L"elation",
                                   L"elastic", L"plastic",   L"stop",
                                   L"ftop",    L"ftation"};
  std::deque<std::shared_ptr<IntsRef>> terms(strings.size());
  std::deque<std::shared_ptr<IntsRef>> terms2(strings2.size());
  for (int inputMode = 0; inputMode < 2; inputMode++) {
    if (VERBOSE) {
      wcout << L"TEST: inputMode=" << inputModeToString(inputMode) << endl;
    }

    for (int idx = 0; idx < strings.size(); idx++) {
      terms[idx] = toIntsRef(strings[idx], inputMode);
    }
    for (int idx = 0; idx < strings2.size(); idx++) {
      terms2[idx] = toIntsRef(strings2[idx], inputMode);
    }
    Arrays::sort(terms2);

    doTest(inputMode, terms);

    // Test pre-determined FST sizes to make sure we haven't lost minimality (at
    // least on this trivial set of terms):

    {
      // FSA
      shared_ptr<Outputs<any>> *const outputs = NoOutputs::getSingleton();
      constexpr any NO_OUTPUT = outputs->getNoOutput();
      const deque<FSTTester::InputOutput<any>> pairs =
          deque<FSTTester::InputOutput<any>>(terms2.size());
      for (auto term : terms2) {
        pairs.push_back(make_shared<FSTTester::InputOutput<>>(term, NO_OUTPUT));
      }
      shared_ptr<FSTTester<any>> tester = make_shared<FSTTester<any>>(
          random(), dir, inputMode, pairs, outputs, false);
      shared_ptr<FST<any>> fst = tester->doTest(0, 0, false);
      assertNotNull(fst);
      TestUtil::assertEquals(22, tester->nodeCount);
      TestUtil::assertEquals(27, tester->arcCount);
    }

    {
      // FST ord pos int
      shared_ptr<PositiveIntOutputs> *const outputs =
          PositiveIntOutputs::getSingleton();
      const deque<FSTTester::InputOutput<int64_t>> pairs =
          deque<FSTTester::InputOutput<int64_t>>(terms2.size());
      for (int idx = 0; idx < terms2.size(); idx++) {
        pairs.push_back(make_shared<FSTTester::InputOutput<>>(
            terms2[idx], static_cast<int64_t>(idx)));
      }
      shared_ptr<FSTTester<int64_t>> tester =
          make_shared<FSTTester<int64_t>>(random(), dir, inputMode, pairs,
                                            outputs, true);
      shared_ptr<FST<int64_t>> *const fst = tester->doTest(0, 0, false);
      assertNotNull(fst);
      TestUtil::assertEquals(22, tester->nodeCount);
      TestUtil::assertEquals(27, tester->arcCount);
    }

    {
      // FST byte sequence ord
      shared_ptr<ByteSequenceOutputs> *const outputs =
          ByteSequenceOutputs::getSingleton();
      shared_ptr<BytesRef> *const NO_OUTPUT = outputs->getNoOutput();
      const deque<FSTTester::InputOutput<std::shared_ptr<BytesRef>>> pairs =
          deque<FSTTester::InputOutput<std::shared_ptr<BytesRef>>>(
              terms2.size());
      for (int idx = 0; idx < terms2.size(); idx++) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        shared_ptr<BytesRef> *const output =
            random()->nextInt(30) == 17
                ? NO_OUTPUT
                : make_shared<BytesRef>(Integer::toString(idx));
        pairs.push_back(
            make_shared<FSTTester::InputOutput<>>(terms2[idx], output));
      }
      shared_ptr<FSTTester<std::shared_ptr<BytesRef>>> tester =
          make_shared<FSTTester<std::shared_ptr<BytesRef>>>(
              random(), dir, inputMode, pairs, outputs, false);
      shared_ptr<FST<std::shared_ptr<BytesRef>>> *const fst =
          tester->doTest(0, 0, false);
      assertNotNull(fst);
      TestUtil::assertEquals(24, tester->nodeCount);
      TestUtil::assertEquals(30, tester->arcCount);
    }
  }
}

void TestFSTs::doTest(
    int inputMode,
    std::deque<std::shared_ptr<IntsRef>> &terms) 
{
  Arrays::sort(terms);

  {
    // NoOutputs (simple FSA)
    shared_ptr<Outputs<any>> *const outputs = NoOutputs::getSingleton();
    constexpr any NO_OUTPUT = outputs->getNoOutput();
    const deque<FSTTester::InputOutput<any>> pairs =
        deque<FSTTester::InputOutput<any>>(terms.size());
    for (auto term : terms) {
      pairs.push_back(make_shared<FSTTester::InputOutput<>>(term, NO_OUTPUT));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, false))
        ->doTest(true);
  }

  {
    // PositiveIntOutput (ord)
    shared_ptr<PositiveIntOutputs> *const outputs =
        PositiveIntOutputs::getSingleton();
    const deque<FSTTester::InputOutput<int64_t>> pairs =
        deque<FSTTester::InputOutput<int64_t>>(terms.size());
    for (int idx = 0; idx < terms.size(); idx++) {
      pairs.push_back(make_shared<FSTTester::InputOutput<>>(
          terms[idx], static_cast<int64_t>(idx)));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, true))
        ->doTest(true);
  }

  {
    // PositiveIntOutput (random monotonically increasing positive number)
    shared_ptr<PositiveIntOutputs> *const outputs =
        PositiveIntOutputs::getSingleton();
    const deque<FSTTester::InputOutput<int64_t>> pairs =
        deque<FSTTester::InputOutput<int64_t>>(terms.size());
    int64_t lastOutput = 0;
    for (int idx = 0; idx < terms.size(); idx++) {
      constexpr int64_t value =
          lastOutput + TestUtil::nextInt(random(), 1, 1000);
      lastOutput = value;
      pairs.push_back(make_shared<FSTTester::InputOutput<>>(terms[idx], value));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, true))
        ->doTest(true);
  }

  {
    // PositiveIntOutput (random positive number)
    shared_ptr<PositiveIntOutputs> *const outputs =
        PositiveIntOutputs::getSingleton();
    const deque<FSTTester::InputOutput<int64_t>> pairs =
        deque<FSTTester::InputOutput<int64_t>>(terms.size());
    for (int idx = 0; idx < terms.size(); idx++) {
      pairs.push_back(make_shared<FSTTester::InputOutput<>>(
          terms[idx],
          TestUtil::nextLong(random(), 0, numeric_limits<int64_t>::max())));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, false))
        ->doTest(true);
  }

  {
    // Pair<ord, (random monotonically increasing positive number>
    shared_ptr<PositiveIntOutputs> *const o1 =
        PositiveIntOutputs::getSingleton();
    shared_ptr<PositiveIntOutputs> *const o2 =
        PositiveIntOutputs::getSingleton();
    shared_ptr<PairOutputs<int64_t, int64_t>> *const outputs =
        make_shared<PairOutputs<int64_t, int64_t>>(o1, o2);
    const deque<
        FSTTester::InputOutput<PairOutputs::Pair<int64_t, int64_t>>>
        pairs = deque<
            FSTTester::InputOutput<PairOutputs::Pair<int64_t, int64_t>>>(
            terms.size());
    int64_t lastOutput = 0;
    for (int idx = 0; idx < terms.size(); idx++) {
      constexpr int64_t value =
          lastOutput + TestUtil::nextInt(random(), 1, 1000);
      lastOutput = value;
      pairs.push_back(make_shared<FSTTester::InputOutput<>>(
          terms[idx], outputs->newPair(static_cast<int64_t>(idx), value)));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, false))
        ->doTest(true);
  }

  {
    // Sequence-of-bytes
    shared_ptr<ByteSequenceOutputs> *const outputs =
        ByteSequenceOutputs::getSingleton();
    shared_ptr<BytesRef> *const NO_OUTPUT = outputs->getNoOutput();
    const deque<FSTTester::InputOutput<std::shared_ptr<BytesRef>>> pairs =
        deque<FSTTester::InputOutput<std::shared_ptr<BytesRef>>>(terms.size());
    for (int idx = 0; idx < terms.size(); idx++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      shared_ptr<BytesRef> *const output =
          random()->nextInt(30) == 17
              ? NO_OUTPUT
              : make_shared<BytesRef>(Integer::toString(idx));
      pairs.push_back(
          make_shared<FSTTester::InputOutput<>>(terms[idx], output));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, false))
        ->doTest(true);
  }

  {
    // Sequence-of-ints
    shared_ptr<IntSequenceOutputs> *const outputs =
        IntSequenceOutputs::getSingleton();
    const deque<FSTTester::InputOutput<std::shared_ptr<IntsRef>>> pairs =
        deque<FSTTester::InputOutput<std::shared_ptr<IntsRef>>>(terms.size());
    for (int idx = 0; idx < terms.size(); idx++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      const wstring s = Integer::toString(idx);
      shared_ptr<IntsRef> *const output = make_shared<IntsRef>(s.length());
      output->length = s.length();
      for (int idx2 = 0; idx2 < output->length; idx2++) {
        output->ints[idx2] = s[idx2];
      }
      pairs.push_back(
          make_shared<FSTTester::InputOutput<>>(terms[idx], output));
    }
    (make_shared<FSTTester<>>(random(), dir, inputMode, pairs, outputs, false))
        ->doTest(true);
  }
}

void TestFSTs::testRandomWords() 
{
  if (TEST_NIGHTLY) {
    testRandomWords(1000, atLeast(2));
  } else {
    testRandomWords(100, 1);
  }
}

wstring TestFSTs::inputModeToString(int mode)
{
  if (mode == 0) {
    return L"utf8";
  } else {
    return L"utf32";
  }
}

void TestFSTs::testRandomWords(int maxNumWords, int numIter) 
{
  shared_ptr<Random> random =
      make_shared<Random>(TestFSTs::random()->nextLong());
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

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testBigSet() throws java.io.IOException
void TestFSTs::testBigSet() 
{
  testRandomWords(TestUtil::nextInt(random(), 50000, 60000), 1);
}

void TestFSTs::testRealTerms() 
{

  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random());
  constexpr int numDocs = TEST_NIGHTLY ? atLeast(1000) : atLeast(100);
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));

  shared_ptr<IndexWriterConfig> *const conf = newIndexWriterConfig(analyzer)
                                                  ->setMaxBufferedDocs(-1)
                                                  ->setRAMBufferSizeMB(64);
  shared_ptr<Path> *const tempDir = createTempDir(L"fstlines");
  shared_ptr<Directory> *const dir = newFSDirectory(tempDir);
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc;
  int docCount = 0;
  while ((doc = docs->nextDoc())->size() > 0 && docCount < numDocs) {
    writer->addDocument(doc);
    docCount++;
  }
  shared_ptr<IndexReader> r = DirectoryReader::open(writer);
  delete writer;
  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();

  shared_ptr<Automaton::Builder<int64_t>> builder =
      make_shared<Automaton::Builder<int64_t>>(
          FST::INPUT_TYPE::BYTE1, 0, 0, true, true, numeric_limits<int>::max(),
          outputs, true, 15);

  bool storeOrd = random()->nextBoolean();
  if (VERBOSE) {
    if (storeOrd) {
      wcout << L"FST stores ord" << endl;
    } else {
      wcout << L"FST stores docFreq" << endl;
    }
  }
  shared_ptr<Terms> terms = MultiFields::getTerms(r, L"body");
  if (terms != nullptr) {
    shared_ptr<IntsRefBuilder> *const scratchIntsRef =
        make_shared<IntsRefBuilder>();
    shared_ptr<TermsEnum> *const termsEnum = terms->begin();
    if (VERBOSE) {
      wcout << L"TEST: got termsEnum=" << termsEnum << endl;
    }
    shared_ptr<BytesRef> term;
    int ord = 0;

    shared_ptr<Automaton> automaton =
        (make_shared<RegExp>(L".*", RegExp::NONE))->toAutomaton();
    shared_ptr<TermsEnum> *const termsEnum2 = terms->intersect(
        make_shared<CompiledAutomaton>(automaton, false, false), nullptr);

    while ((term = termsEnum->next()) != nullptr) {
      shared_ptr<BytesRef> term2 = termsEnum2->next();
      assertNotNull(term2);
      TestUtil::assertEquals(term, term2);
      TestUtil::assertEquals(termsEnum->docFreq(), termsEnum2->docFreq());
      TestUtil::assertEquals(termsEnum->totalTermFreq(),
                             termsEnum2->totalTermFreq());

      if (ord == 0) {
        try {
          termsEnum->ord();
        } catch (const UnsupportedOperationException &uoe) {
          if (VERBOSE) {
            wcout << L"TEST: codec doesn't support ord; FST stores docFreq"
                  << endl;
          }
          storeOrd = false;
        }
      }
      constexpr int output;
      if (storeOrd) {
        output = ord;
      } else {
        output = termsEnum->docFreq();
      }
      builder->add(Util::toIntsRef(term, scratchIntsRef),
                   static_cast<int64_t>(output));
      ord++;
      if (VERBOSE && ord % 100000 == 0 && LuceneTestCase::TEST_NIGHTLY) {
        wcout << ord << L" terms..." << endl;
      }
    }
    shared_ptr<FST<int64_t>> fst = builder->finish();
    if (VERBOSE) {
      wcout << L"FST: " << docCount << L" docs; " << ord << L" terms; "
            << builder->getNodeCount() << L" nodes; " << builder->getArcCount()
            << L" arcs;" << L" " << fst->ramBytesUsed() << L" bytes" << endl;
    }

    if (ord > 0) {
      shared_ptr<Random> *const random =
          make_shared<Random>(TestFSTs::random()->nextLong());
      // Now confirm BytesRefFSTEnum and TermsEnum act the
      // same:
      shared_ptr<BytesRefFSTEnum<int64_t>> *const fstEnum =
          make_shared<BytesRefFSTEnum<int64_t>>(fst);
      int num = atLeast(1000);
      for (int iter = 0; iter < num; iter++) {
        shared_ptr<BytesRef> *const randomTerm =
            make_shared<BytesRef>(getRandomString(random));

        if (VERBOSE) {
          wcout << L"TEST: seek non-exist " << randomTerm->utf8ToString()
                << L" " << randomTerm << endl;
        }

        constexpr TermsEnum::SeekStatus seekResult =
            termsEnum->seekCeil(randomTerm);
        shared_ptr<InputOutput<int64_t>> *const fstSeekResult =
            fstEnum->seekCeil(randomTerm);

        if (seekResult == TermsEnum::SeekStatus::END) {
          assertNull(L"got " +
                         (fstSeekResult == nullptr
                              ? L"null"
                              : fstSeekResult->input->utf8ToString()) +
                         L" but expected null",
                     fstSeekResult);
        } else {
          assertSame(termsEnum, fstEnum, storeOrd);
          for (int nextIter = 0; nextIter < 10; nextIter++) {
            if (VERBOSE) {
              wcout << L"TEST: next" << endl;
              if (storeOrd) {
                wcout << L"  ord=" << termsEnum->ord() << endl;
              }
            }
            if (termsEnum->next() != nullptr) {
              if (VERBOSE) {
                wcout << L"  term=" << termsEnum->term()->utf8ToString()
                      << endl;
              }
              assertNotNull(fstEnum->next());
              assertSame(termsEnum, fstEnum, storeOrd);
            } else {
              if (VERBOSE) {
                wcout << L"  end!" << endl;
              }
              shared_ptr<BytesRefFSTEnum::InputOutput<int64_t>> nextResult =
                  fstEnum->next();
              if (nextResult != nullptr) {
                wcout << L"expected null but got: input="
                      << nextResult->input->utf8ToString() << L" output="
                      << outputs->outputToString(nextResult->output) << endl;
                fail();
              }
              break;
            }
          }
        }
      }
    }
  }

  delete r;
  delete dir;
}

template <typename T1>
void TestFSTs::assertSame(shared_ptr<TermsEnum> termsEnum,
                          shared_ptr<BytesRefFSTEnum<T1>> fstEnum,
                          bool storeOrd) 
{
  if (termsEnum->term() == nullptr) {
    assertNull(fstEnum->current());
  } else {
    assertNotNull(fstEnum->current());
    assertEquals(termsEnum->term()->utf8ToString() + L" != " +
                     fstEnum->current()->input->utf8ToString(),
                 termsEnum->term(), fstEnum->current()->input);
    if (storeOrd) {
      // fst stored the ord
      assertEquals(
          L"term=" + termsEnum->term()->utf8ToString() + L" " +
              termsEnum->term(),
          termsEnum->ord(),
          (static_cast<optional<int64_t>>(fstEnum->current()->output))
              ->value());
    } else {
      // fst stored the docFreq
      assertEquals(L"term=" + termsEnum->term()->utf8ToString() + L" " +
                       termsEnum->term(),
                   termsEnum->docFreq(),
                   static_cast<int>((static_cast<optional<int64_t>>(
                                         fstEnum->current()->output))
                                        ->value()));
    }
  }
}

void TestFSTs::main(std::deque<wstring> &args) 
{
  int prune = 0;
  int limit = numeric_limits<int>::max();
  int inputMode = 0; // utf8
  bool storeOrds = false;
  bool storeDocFreqs = false;
  bool verify = true;
  bool noArcArrays = false;
  shared_ptr<Path> wordsFileIn = nullptr;
  shared_ptr<Path> dirOut = nullptr;

  int idx = 0;
  while (idx < args.size()) {
    if (args[idx] == L"-prune") {
      prune = stoi(args[1 + idx]);
      idx++;
    } else if (args[idx] == L"-limit") {
      limit = stoi(args[1 + idx]);
      idx++;
    } else if (args[idx] == L"-utf8") {
      inputMode = 0;
    } else if (args[idx] == L"-utf32") {
      inputMode = 1;
    } else if (args[idx] == L"-docFreq") {
      storeDocFreqs = true;
    } else if (args[idx] == L"-noArcArrays") {
      noArcArrays = true;
    } else if (args[idx] == L"-ords") {
      storeOrds = true;
    } else if (args[idx] == L"-noverify") {
      verify = false;
    } else if (StringHelper::startsWith(args[idx], L"-")) {
      System::err::println(L"Unrecognized option: " + args[idx]);
      exit(-1);
    } else {
      if (wordsFileIn == nullptr) {
        wordsFileIn = Paths->get(args[idx]);
      } else if (dirOut == nullptr) {
        dirOut = Paths->get(args[idx]);
      } else {
        System::err::println(L"Too many arguments, expected: input [output]");
        exit(-1);
      }
    }
    idx++;
  }

  if (wordsFileIn == nullptr) {
    System::err::println(L"No input file.");
    exit(-1);
  }

  // ord benefits from share, docFreqs don't:

  if (storeOrds && storeDocFreqs) {
    // Store both ord & docFreq:
    shared_ptr<PositiveIntOutputs> *const o1 =
        PositiveIntOutputs::getSingleton();
    shared_ptr<PositiveIntOutputs> *const o2 =
        PositiveIntOutputs::getSingleton();
    shared_ptr<PairOutputs<int64_t, int64_t>> *const outputs =
        make_shared<PairOutputs<int64_t, int64_t>>(o1, o2);
    make_shared<VisitTermsAnonymousInnerClass>(dirOut, wordsFileIn, inputMode,
                                               prune, noArcArrays, outputs)
        .run(limit, verify, false);
  } else if (storeOrds) {
    // Store only ords
    shared_ptr<PositiveIntOutputs> *const outputs =
        PositiveIntOutputs::getSingleton();
    make_shared<VisitTermsAnonymousInnerClass2>(dirOut, wordsFileIn, inputMode,
                                                prune, noArcArrays)
        .run(limit, verify, true);
  } else if (storeDocFreqs) {
    // Store only docFreq
    shared_ptr<PositiveIntOutputs> *const outputs =
        PositiveIntOutputs::getSingleton();
    make_shared<VisitTermsAnonymousInnerClass3>(dirOut, wordsFileIn, inputMode,
                                                prune, noArcArrays)
        .run(limit, verify, false);
  } else {
    // Store nothing
    shared_ptr<NoOutputs> *const outputs = NoOutputs::getSingleton();
    constexpr any NO_OUTPUT = outputs->getNoOutput();
    make_shared<VisitTermsAnonymousInnerClass4>(dirOut, wordsFileIn, inputMode,
                                                prune, noArcArrays, NO_OUTPUT)
        .run(limit, verify, false);
  }
}

TestFSTs::VisitTermsAnonymousInnerClass::VisitTermsAnonymousInnerClass(
    shared_ptr<Path> dirOut, shared_ptr<Path> wordsFileIn, int inputMode,
    int prune, bool noArcArrays,
    shared_ptr<
        org::apache::lucene::util::fst::PairOutputs<int64_t, int64_t>>
        outputs)
    : VisitTerms<PairOutputs::Pair<long, long>>(dirOut, wordsFileIn, inputMode,
                                                prune, outputs, noArcArrays)
{
  this->outputs = outputs;
}

shared_ptr<PairOutputs::Pair<int64_t, int64_t>>
TestFSTs::VisitTermsAnonymousInnerClass::getOutput(shared_ptr<IntsRef> input,
                                                   int ord)
{
  if (ord == 0) {
    rand = make_shared<Random>(17);
  }
  return outputs->newPair(
      static_cast<int64_t>(ord),
      static_cast<int64_t>(TestUtil::nextInt(rand, 1, 5000)));
}

TestFSTs::VisitTermsAnonymousInnerClass2::VisitTermsAnonymousInnerClass2(
    shared_ptr<Path> dirOut, shared_ptr<Path> wordsFileIn, int inputMode,
    int prune, bool noArcArrays)
    : VisitTerms<long>(dirOut, wordsFileIn, inputMode, prune, outputs,
                       noArcArrays)
{
}

optional<int64_t>
TestFSTs::VisitTermsAnonymousInnerClass2::getOutput(shared_ptr<IntsRef> input,
                                                    int ord)
{
  return static_cast<int64_t>(ord);
}

TestFSTs::VisitTermsAnonymousInnerClass3::VisitTermsAnonymousInnerClass3(
    shared_ptr<Path> dirOut, shared_ptr<Path> wordsFileIn, int inputMode,
    int prune, bool noArcArrays)
    : VisitTerms<long>(dirOut, wordsFileIn, inputMode, prune, outputs,
                       noArcArrays)
{
}

optional<int64_t>
TestFSTs::VisitTermsAnonymousInnerClass3::getOutput(shared_ptr<IntsRef> input,
                                                    int ord)
{
  if (ord == 0) {
    rand = make_shared<Random>(17);
  }
  return static_cast<int64_t>(TestUtil::nextInt(rand, 1, 5000));
}

TestFSTs::VisitTermsAnonymousInnerClass4::VisitTermsAnonymousInnerClass4(
    shared_ptr<Path> dirOut, shared_ptr<Path> wordsFileIn, int inputMode,
    int prune, bool noArcArrays, any NO_OUTPUT)
    : VisitTerms<Object>(dirOut, wordsFileIn, inputMode, prune, outputs,
                         noArcArrays)
{
  this->NO_OUTPUT = NO_OUTPUT;
}

any TestFSTs::VisitTermsAnonymousInnerClass4::getOutput(
    shared_ptr<IntsRef> input, int ord)
{
  return NO_OUTPUT;
}

void TestFSTs::testSingleString() 
{
  shared_ptr<Outputs<any>> *const outputs = NoOutputs::getSingleton();
  shared_ptr<Automaton::Builder<any>> *const b =
      make_shared<Automaton::Builder<any>>(FST::INPUT_TYPE::BYTE1, outputs);
  b->add(Util::toIntsRef(make_shared<BytesRef>(L"foobar"),
                         make_shared<IntsRefBuilder>()),
         outputs->getNoOutput());
  shared_ptr<BytesRefFSTEnum<any>> *const fstEnum =
      make_shared<BytesRefFSTEnum<any>>(b->finish());
  assertNull(fstEnum->seekFloor(make_shared<BytesRef>(L"foo")));
  assertNull(fstEnum->seekCeil(make_shared<BytesRef>(L"foobaz")));
}

void TestFSTs::testDuplicateFSAString() 
{
  wstring str = L"foobar";
  shared_ptr<Outputs<any>> *const outputs = NoOutputs::getSingleton();
  shared_ptr<Automaton::Builder<any>> *const b =
      make_shared<Automaton::Builder<any>>(FST::INPUT_TYPE::BYTE1, outputs);
  shared_ptr<IntsRefBuilder> ints = make_shared<IntsRefBuilder>();
  for (int i = 0; i < 10; i++) {
    b->add(Util::toIntsRef(make_shared<BytesRef>(str), ints),
           outputs->getNoOutput());
  }
  shared_ptr<FST<any>> fst = b->finish();

  // count the input paths
  int count = 0;
  shared_ptr<BytesRefFSTEnum<any>> *const fstEnum =
      make_shared<BytesRefFSTEnum<any>>(fst);
  while (fstEnum->next() != nullptr) {
    count++;
  }
  TestUtil::assertEquals(1, count);

  assertNotNull(Util::get(fst, make_shared<BytesRef>(str)));
  assertNull(Util::get(fst, make_shared<BytesRef>(L"foobaz")));
}

void TestFSTs::testSimple() 
{

  // Get outputs -- passing true means FST will share
  // (delta code) the outputs.  This should result in
  // smaller FST if the outputs grow monotonically.  But
  // if numbers are "random", false should give smaller
  // final size:
  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();

  // Build an FST mapping BytesRef -> Long
  shared_ptr<Automaton::Builder<int64_t>> *const builder =
      make_shared<Automaton::Builder<int64_t>>(FST::INPUT_TYPE::BYTE1,
                                                 outputs);

  shared_ptr<BytesRef> *const a = make_shared<BytesRef>(L"a");
  shared_ptr<BytesRef> *const b = make_shared<BytesRef>(L"b");
  shared_ptr<BytesRef> *const c = make_shared<BytesRef>(L"c");

  builder->add(Util::toIntsRef(a, make_shared<IntsRefBuilder>()), 17LL);
  builder->add(Util::toIntsRef(b, make_shared<IntsRefBuilder>()), 42LL);
  builder->add(Util::toIntsRef(c, make_shared<IntsRefBuilder>()),
               13824324872317238LL);

  shared_ptr<FST<int64_t>> *const fst = builder->finish();

  TestUtil::assertEquals(13824324872317238LL,
                         static_cast<int64_t>(Util::get(fst, c)));
  TestUtil::assertEquals(42, static_cast<int64_t>(Util::get(fst, b)));
  TestUtil::assertEquals(17, static_cast<int64_t>(Util::get(fst, a)));

  shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum =
      make_shared<BytesRefFSTEnum<int64_t>>(fst);
  shared_ptr<BytesRefFSTEnum::InputOutput<int64_t>> seekResult;
  seekResult = fstEnum->seekFloor(a);
  assertNotNull(seekResult);
  TestUtil::assertEquals(17, static_cast<int64_t>(seekResult->output));

  // goes to a
  seekResult = fstEnum->seekFloor(make_shared<BytesRef>(L"aa"));
  assertNotNull(seekResult);
  TestUtil::assertEquals(17, static_cast<int64_t>(seekResult->output));

  // goes to b
  seekResult = fstEnum->seekCeil(make_shared<BytesRef>(L"aa"));
  assertNotNull(seekResult);
  TestUtil::assertEquals(b, seekResult->input);
  TestUtil::assertEquals(42, static_cast<int64_t>(seekResult->output));

  TestUtil::assertEquals(Util::toIntsRef(make_shared<BytesRef>(L"c"),
                                         make_shared<IntsRefBuilder>()),
                         Util::getByOutput(fst, 13824324872317238LL));
  assertNull(Util::getByOutput(fst, 47));
  TestUtil::assertEquals(Util::toIntsRef(make_shared<BytesRef>(L"b"),
                                         make_shared<IntsRefBuilder>()),
                         Util::getByOutput(fst, 42));
  TestUtil::assertEquals(Util::toIntsRef(make_shared<BytesRef>(L"a"),
                                         make_shared<IntsRefBuilder>()),
                         Util::getByOutput(fst, 17));
}

void TestFSTs::testPrimaryKeys() 
{
  shared_ptr<Directory> dir = newDirectory();

  for (int cycle = 0; cycle < 2; cycle++) {
    if (VERBOSE) {
      wcout << L"TEST: cycle=" << cycle << endl;
    }
    shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
        random(), dir,
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::NO);
    doc->push_back(idField);

    constexpr int NUM_IDS = atLeast(200);
    // final int NUM_IDS = (int) (377 * (1.0+random.nextDouble()));
    if (VERBOSE) {
      wcout << L"TEST: NUM_IDS=" << NUM_IDS << endl;
    }
    shared_ptr<Set<wstring>> *const allIDs = unordered_set<wstring>();
    for (int id = 0; id < NUM_IDS; id++) {
      wstring idString;
      if (cycle == 0) {
        // PKs are assigned sequentially
        idString = wstring::format(Locale::ROOT, L"%07d", id);
      } else {
        while (true) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          const wstring s = Long::toString(random()->nextLong());
          if (!allIDs->contains(s)) {
            idString = s;
            break;
          }
        }
      }
      allIDs->add(idString);
      idField->setStringValue(idString);
      w->addDocument(doc);
    }

    // w.forceMerge(1);

    // turn writer into reader:
    shared_ptr<IndexReader> *const r = w->getReader();
    shared_ptr<IndexSearcher> *const s = newSearcher(r);
    delete w;

    const deque<wstring> allIDsList = deque<wstring>(allIDs);
    const deque<wstring> sortedAllIDsList = deque<wstring>(allIDsList);
    sort(sortedAllIDsList.begin(), sortedAllIDsList.end());

    // Sprinkle in some non-existent PKs:
    shared_ptr<Set<wstring>> outOfBounds = unordered_set<wstring>();
    for (int idx = 0; idx < NUM_IDS / 10; idx++) {
      wstring idString;
      if (cycle == 0) {
        idString = wstring::format(Locale::ROOT, L"%07d", (NUM_IDS + idx));
      } else {
        while (true) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          idString = Long::toString(random()->nextLong());
          if (!allIDs->contains(idString)) {
            break;
          }
        }
      }
      outOfBounds->add(idString);
      allIDsList.push_back(idString);
    }

    // Verify w/ TermQuery
    for (int iter = 0; iter < 2 * NUM_IDS; iter++) {
      const wstring id = allIDsList[random()->nextInt(allIDsList.size())];
      constexpr bool exists = !outOfBounds->contains(id);
      if (VERBOSE) {
        wcout << L"TEST: TermQuery " << (exists ? L"" : L"non-exist ")
              << L" id=" << id << endl;
      }
      assertEquals(
          (exists ? L"" : L"non-exist ") + L"id=" + id, exists ? 1 : 0,
          s->search(make_shared<TermQuery>(make_shared<Term>(L"id", id)), 1)
              ->totalHits);
    }

    // Verify w/ MultiTermsEnum
    shared_ptr<TermsEnum> *const termsEnum =
        MultiFields::getTerms(r, L"id")->begin();
    for (int iter = 0; iter < 2 * NUM_IDS; iter++) {
      const wstring id;
      const wstring nextID;
      constexpr bool exists;

      if (random()->nextBoolean()) {
        id = allIDsList[random()->nextInt(allIDsList.size())];
        exists = !outOfBounds->contains(id);
        nextID = L"";
        if (VERBOSE) {
          wcout << L"TEST: exactOnly " << (exists ? L"" : L"non-exist ")
                << L"id=" << id << endl;
        }
      } else {
        // Pick ID between two IDs:
        exists = false;
        constexpr int idv = random()->nextInt(NUM_IDS - 1);
        if (cycle == 0) {
          id = wstring::format(Locale::ROOT, L"%07da", idv);
          nextID = wstring::format(Locale::ROOT, L"%07d", idv + 1);
        } else {
          id = sortedAllIDsList[idv] + L"a";
          nextID = sortedAllIDsList[idv + 1];
        }
        if (VERBOSE) {
          wcout << L"TEST: not exactOnly id=" << id << L" nextID=" << nextID
                << endl;
        }
      }

      constexpr TermsEnum::SeekStatus status;
      if (nextID == L"") {
        if (termsEnum->seekExact(make_shared<BytesRef>(id))) {
          status = TermsEnum::SeekStatus::FOUND;
        } else {
          status = TermsEnum::SeekStatus::NOT_FOUND;
        }
      } else {
        status = termsEnum->seekCeil(make_shared<BytesRef>(id));
      }

      if (nextID != L"") {
        TestUtil::assertEquals(TermsEnum::SeekStatus::NOT_FOUND, status);
        assertEquals(L"expected=" + nextID + L" actual=" +
                         termsEnum->term()->utf8ToString(),
                     make_shared<BytesRef>(nextID), termsEnum->term());
      } else if (!exists) {
        assertTrue(status == TermsEnum::SeekStatus::NOT_FOUND ||
                   status == TermsEnum::SeekStatus::END);
      } else {
        TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND, status);
      }
    }

    delete r;
  }
  delete dir;
}

void TestFSTs::testRandomTermLookup() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newStringField(L"field", L"", Field::Store::NO);
  doc->push_back(f);

  constexpr int NUM_TERMS =
      static_cast<int>(1000 * RANDOM_MULTIPLIER * (1 + random()->nextDouble()));
  if (VERBOSE) {
    wcout << L"TEST: NUM_TERMS=" << NUM_TERMS << endl;
  }

  shared_ptr<Set<wstring>> *const allTerms = unordered_set<wstring>();
  while (allTerms->size() < NUM_TERMS) {
    allTerms->add(simpleRandomString(random()));
  }

  for (auto term : allTerms) {
    f->setStringValue(term);
    w->addDocument(doc);
  }

  // turn writer into reader:
  if (VERBOSE) {
    wcout << L"TEST: get reader" << endl;
  }
  shared_ptr<IndexReader> r = w->getReader();
  if (VERBOSE) {
    wcout << L"TEST: got reader=" << r << endl;
  }
  shared_ptr<IndexSearcher> s = newSearcher(r);
  delete w;

  const deque<wstring> allTermsList = deque<wstring>(allTerms);
  Collections::shuffle(allTermsList, random());

  // verify exact lookup
  for (auto term : allTermsList) {
    if (VERBOSE) {
      wcout << L"TEST: term=" << term << endl;
    }
    assertEquals(
        L"term=" + term, 1,
        s->search(make_shared<TermQuery>(make_shared<Term>(L"field", term)), 1)
            ->totalHits);
  }

  delete r;
  delete dir;
}

void TestFSTs::testExpandedCloseToRoot() 
{
  // C++ TODO: Local classes are not converted by Java to C++ Converter:
  //      class SyntheticData
  //    {
  //      FST<Object> compile(std::wstring[] lines) throws IOException
  //      {
  //        final NoOutputs outputs = NoOutputs.getSingleton();
  //        final Object nothing = outputs.getNoOutput();
  //        final Builder<Object> b = new Builder<>(FST.INPUT_TYPE.BYTE1,
  //        outputs);
  //
  //        int line = 0;
  //        final BytesRefBuilder term = new BytesRefBuilder();
  //        final IntsRefBuilder scratchIntsRef = new IntsRefBuilder();
  //        while (line < lines.length)
  //        {
  //          std::wstring w = lines[line++];
  //          if (w == nullptr)
  //          {
  //            break;
  //          }
  //          term.copyChars(w);
  //          b.add(Util.toIntsRef(term.get(), scratchIntsRef), nothing);
  //        }
  //
  //        return b.finish();
  //      }
  //
  //      void generate(ArrayList<std::wstring> out, StringBuilder b, char from, char
  //      to, int depth)
  //      {
  //        if (depth == 0 || from == to)
  //        {
  //          std::wstring seq = b.toString() + "_" + out.size() + "_end";
  //          out.add(seq);
  //        }
  //        else
  //        {
  //          for (char c = from; c <= to; c++)
  //          {
  //            b.append(c);
  //            generate(out, b, from, c == to ? to : from, depth - 1);
  //            b.deleteCharAt(b.length() - 1);
  //          }
  //        }
  //      }
  //
  //      public int verifyStateAndBelow(FST<Object> fst, Arc<Object> arc, int
  //      depth) throws IOException
  //      {
  //        if (FST.targetHasArcs(arc))
  //        {
  //          int childCount = 0;
  //          BytesReader fstReader = fst.getBytesReader();
  //          for (arc = fst.readFirstTargetArc(arc, arc, fstReader); arc =
  //          fst.readNextArc(arc, fstReader), childCount++)
  //          {
  //            bool expanded = fst.isExpandedTarget(arc, fstReader);
  //            int children = verifyStateAndBelow(fst, new
  //            FST.Arc<>().copyFrom(arc), depth + 1);
  //
  //            assertEquals(expanded, (depth <=
  //            FST.FIXED_ARRAY_SHALLOW_DISTANCE && children >=
  //            FST.FIXED_ARRAY_NUM_ARCS_SHALLOW) || children >=
  //            FST.FIXED_ARRAY_NUM_ARCS_DEEP); if (arc.isLast())
  //                break;
  //          }
  //
  //          return childCount;
  //        }
  //        return 0;
  //      }
  //    }

  // Sanity check.
  assertTrue(FST::FIXED_ARRAY_NUM_ARCS_SHALLOW <
             FST::FIXED_ARRAY_NUM_ARCS_DEEP);
  assertTrue(FST::FIXED_ARRAY_SHALLOW_DISTANCE >= 0);

  shared_ptr<SyntheticData> s = make_shared<SyntheticData>();

  deque<wstring> out = deque<wstring>();
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  s->generate(out, b, L'a', L'i', 10);
  std::deque<wstring> input = out.toArray(std::deque<wstring>(out.size()));
  Arrays::sort(input);
  shared_ptr<FST<any>> fst = s->compile(input);
  shared_ptr<FST::Arc<any>> arc =
      fst->getFirstArc(make_shared<FST::Arc<any>>());
  s->verifyStateAndBelow(fst, arc, 1);
}

void TestFSTs::testFinalOutputOnEndState() 
{
  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();

  shared_ptr<Automaton::Builder<int64_t>> *const builder =
      make_shared<Automaton::Builder<int64_t>>(
          FST::INPUT_TYPE::BYTE4, 2, 0, true, true, numeric_limits<int>::max(),
          outputs, true, 15);
  builder->add(Util::toUTF32(L"stat", make_shared<IntsRefBuilder>()), 17LL);
  builder->add(Util::toUTF32(L"station", make_shared<IntsRefBuilder>()), 10LL);
  shared_ptr<FST<int64_t>> *const fst = builder->finish();
  // Writer w = new OutputStreamWriter(new FileOutputStream("/x/tmp3/out.dot"));
  shared_ptr<StringWriter> w = make_shared<StringWriter>();
  Util::toDot(fst, w, false, false);
  w->close();
  // System.out.println(w.toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(w->toString()->find(L"label=\"t/[7]\"") != -1);
}

void TestFSTs::testInternalFinalState() 
{
  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();
  shared_ptr<Automaton::Builder<int64_t>> *const builder =
      make_shared<Automaton::Builder<int64_t>>(
          FST::INPUT_TYPE::BYTE1, 0, 0, true, true, numeric_limits<int>::max(),
          outputs, true, 15);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"stat"),
                               make_shared<IntsRefBuilder>()),
               outputs->getNoOutput());
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"station"),
                               make_shared<IntsRefBuilder>()),
               outputs->getNoOutput());
  shared_ptr<FST<int64_t>> *const fst = builder->finish();
  shared_ptr<StringWriter> w = make_shared<StringWriter>();
  // Writer w = new OutputStreamWriter(new FileOutputStream("/x/tmp/out.dot"));
  Util::toDot(fst, w, false, false);
  w->close();
  // System.out.println(w.toString());

  // check for accept state at label t
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(w->toString()->find(L"[label=\"t\" style=\"bold\"") != -1);
  // check for accept state at label n
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(w->toString()->find(L"[label=\"n\" style=\"bold\"") != -1);
}

void TestFSTs::testNonFinalStopNode() 
{
  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();
  const optional<int64_t> nothing = outputs->getNoOutput();
  shared_ptr<Automaton::Builder<int64_t>> *const b =
      make_shared<Automaton::Builder<int64_t>>(FST::INPUT_TYPE::BYTE1,
                                                 outputs);

  // final FST<Long> fst = new FST<>(FST.INPUT_TYPE.BYTE1, outputs, false,
  // PackedInts.COMPACT, 15);
  shared_ptr<FST<int64_t>> *const fst = b->fst;

  shared_ptr<Automaton::Builder::UnCompiledNode<int64_t>> *const rootNode =
      make_shared<Automaton::Builder::UnCompiledNode<int64_t>>(b, 0);

  {
    // Add final stop node
    shared_ptr<Automaton::Builder::UnCompiledNode<int64_t>> *const node =
        make_shared<Automaton::Builder::UnCompiledNode<int64_t>>(b, 0);
    node->isFinal = true;
    rootNode->addArc(L'a', node);
    shared_ptr<Automaton::Builder::CompiledNode> *const frozen =
        make_shared<Automaton::Builder::CompiledNode>();
    frozen->node = fst->addNode(b, node);
    rootNode->arcs[0]->nextFinalOutput = 17LL;
    rootNode->arcs[0]->isFinal = true;
    rootNode->arcs[0]->output = nothing;
    rootNode->arcs[0]->target = frozen;
  }

  {
    // Add non-final stop node
    shared_ptr<Automaton::Builder::UnCompiledNode<int64_t>> *const node =
        make_shared<Automaton::Builder::UnCompiledNode<int64_t>>(b, 0);
    rootNode->addArc(L'b', node);
    shared_ptr<Automaton::Builder::CompiledNode> *const frozen =
        make_shared<Automaton::Builder::CompiledNode>();
    frozen->node = fst->addNode(b, node);
    rootNode->arcs[1]->nextFinalOutput = nothing;
    rootNode->arcs[1]->output = 42LL;
    rootNode->arcs[1]->target = frozen;
  }

  fst->finish(fst->addNode(b, rootNode));

  shared_ptr<StringWriter> w = make_shared<StringWriter>();
  // Writer w = new OutputStreamWriter(new FileOutputStream("/x/tmp3/out.dot"));
  Util::toDot(fst, w, false, false);
  w->close();

  checkStopNodes(fst, outputs);

  // Make sure it still works after save/load:
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(L"fst", IOContext::DEFAULT);
  fst->save(out);
  delete out;

  shared_ptr<IndexInput> in_ = dir->openInput(L"fst", IOContext::DEFAULT);
  shared_ptr<FST<int64_t>> *const fst2 =
      make_shared<FST<int64_t>>(in_, outputs);
  checkStopNodes(fst2, outputs);
  delete in_;
  delete dir;
}

void TestFSTs::checkStopNodes(
    shared_ptr<FST<int64_t>> fst,
    shared_ptr<PositiveIntOutputs> outputs) 
{
  const optional<int64_t> nothing = outputs->getNoOutput();
  shared_ptr<FST::Arc<int64_t>> startArc =
      fst->getFirstArc(make_shared<FST::Arc<int64_t>>());
  TestUtil::assertEquals(nothing, startArc->output);
  TestUtil::assertEquals(nothing, startArc->nextFinalOutput);

  shared_ptr<FST::Arc<int64_t>> arc = fst->readFirstTargetArc(
      startArc, make_shared<FST::Arc<int64_t>>(), fst->getBytesReader());
  TestUtil::assertEquals(L'a', arc->label);
  TestUtil::assertEquals(17, arc->nextFinalOutput.value());
  assertTrue(arc->isFinal());

  arc = fst->readNextArc(arc, fst->getBytesReader());
  TestUtil::assertEquals(L'b', arc->label);
  assertFalse(arc->isFinal());
  TestUtil::assertEquals(42, arc->output.value());
}

const shared_ptr<java::util::Comparator<int64_t>>
    TestFSTs::minLongComparator = make_shared<ComparatorAnonymousInnerClass>();

TestFSTs::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass() {}

int TestFSTs::ComparatorAnonymousInnerClass::compare(optional<int64_t> &left,
                                                     optional<int64_t> &right)
{
  return left.compareTo(right);
}

void TestFSTs::testShortestPaths() 
{
  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();
  shared_ptr<Automaton::Builder<int64_t>> *const builder =
      make_shared<Automaton::Builder<int64_t>>(FST::INPUT_TYPE::BYTE1,
                                                 outputs);

  shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"aab"), scratch), 22LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"aac"), scratch), 7LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"ax"), scratch), 17LL);
  shared_ptr<FST<int64_t>> *const fst = builder->finish();
  // Writer w = new OutputStreamWriter(new FileOutputStream("out.dot"));
  // Util.toDot(fst, w, false, false);
  // w.close();

  shared_ptr<Util::TopResults<int64_t>> res = Util::shortestPaths(
      fst, fst->getFirstArc(make_shared<FST::Arc<int64_t>>()),
      outputs->getNoOutput(), minLongComparator, 3, true);
  assertTrue(res->isComplete);
  TestUtil::assertEquals(3, res->topN.size());
  TestUtil::assertEquals(
      Util::toIntsRef(make_shared<BytesRef>(L"aac"), scratch),
      res->topN[0]->input);
  TestUtil::assertEquals(7LL, res->topN[0]->output.longValue());

  TestUtil::assertEquals(Util::toIntsRef(make_shared<BytesRef>(L"ax"), scratch),
                         res->topN[1]->input);
  TestUtil::assertEquals(17LL, res->topN[1]->output.longValue());

  TestUtil::assertEquals(
      Util::toIntsRef(make_shared<BytesRef>(L"aab"), scratch),
      res->topN[2]->input);
  TestUtil::assertEquals(22LL, res->topN[2]->output.longValue());
}

void TestFSTs::testRejectNoLimits() 
{
  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();
  shared_ptr<Automaton::Builder<int64_t>> *const builder =
      make_shared<Automaton::Builder<int64_t>>(FST::INPUT_TYPE::BYTE1,
                                                 outputs);

  shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"aab"), scratch), 22LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"aac"), scratch), 7LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"adcd"), scratch), 17LL);
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"adcde"), scratch), 17LL);

  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"ax"), scratch), 17LL);
  shared_ptr<FST<int64_t>> *const fst = builder->finish();
  shared_ptr<AtomicInteger> *const rejectCount = make_shared<AtomicInteger>();
  shared_ptr<Util::TopNSearcher<int64_t>> searcher =
      make_shared<TopNSearcherAnonymousInnerClass>(
          shared_from_this(), minLongComparator, rejectCount);

  searcher->addStartPaths(fst->getFirstArc(make_shared<FST::Arc<int64_t>>()),
                          outputs->getNoOutput(), true,
                          make_shared<IntsRefBuilder>());
  shared_ptr<Util::TopResults<int64_t>> res = searcher->search();
  TestUtil::assertEquals(rejectCount->get(), 4);
  assertTrue(res->isComplete); // rejected(4) + topN(2) <= maxQueueSize(6)

  TestUtil::assertEquals(1, res->topN.size());
  TestUtil::assertEquals(
      Util::toIntsRef(make_shared<BytesRef>(L"aac"), scratch),
      res->topN[0]->input);
  TestUtil::assertEquals(7LL, res->topN[0]->output.longValue());
  rejectCount->set(0);
  searcher = make_shared<TopNSearcherAnonymousInnerClass2>(
      shared_from_this(), minLongComparator, rejectCount);

  searcher->addStartPaths(fst->getFirstArc(make_shared<FST::Arc<int64_t>>()),
                          outputs->getNoOutput(), true,
                          make_shared<IntsRefBuilder>());
  res = searcher->search();
  TestUtil::assertEquals(rejectCount->get(), 4);
  assertFalse(res->isComplete); // rejected(4) + topN(2) > maxQueueSize(5)
}

TestFSTs::TopNSearcherAnonymousInnerClass::TopNSearcherAnonymousInnerClass(
    shared_ptr<TestFSTs> outerInstance,
    shared_ptr<UnknownType> minLongComparator,
    shared_ptr<AtomicInteger> rejectCount)
    : Util::TopNSearcher<long>(fst, 2, 6, minLongComparator)
{
  this->outerInstance = outerInstance;
  this->rejectCount = rejectCount;
}

bool TestFSTs::TopNSearcherAnonymousInnerClass::acceptResult(
    shared_ptr<IntsRef> input, optional<int64_t> &output)
{
  bool accept = output.value() == 7;
  if (!accept) {
    rejectCount->incrementAndGet();
  }
  return accept;
}

TestFSTs::TopNSearcherAnonymousInnerClass2::TopNSearcherAnonymousInnerClass2(
    shared_ptr<TestFSTs> outerInstance,
    shared_ptr<UnknownType> minLongComparator,
    shared_ptr<AtomicInteger> rejectCount)
    : Util::TopNSearcher<long>(fst, 2, 5, minLongComparator)
{
  this->outerInstance = outerInstance;
  this->rejectCount = rejectCount;
}

bool TestFSTs::TopNSearcherAnonymousInnerClass2::acceptResult(
    shared_ptr<IntsRef> input, optional<int64_t> &output)
{
  bool accept = output.value() == 7;
  if (!accept) {
    rejectCount->incrementAndGet();
  }
  return accept;
}

const shared_ptr<java::util::Comparator<
    org::apache::lucene::util::fst::PairOutputs::Pair<int64_t, int64_t>>>
    TestFSTs::minPairWeightComparator =
        make_shared<ComparatorAnonymousInnerClass2>();

TestFSTs::ComparatorAnonymousInnerClass2::ComparatorAnonymousInnerClass2() {}

int TestFSTs::ComparatorAnonymousInnerClass2::compare(
    shared_ptr<Pair<int64_t, int64_t>> left,
    shared_ptr<Pair<int64_t, int64_t>> right)
{
  return left->output1.compareTo(right->output1);
}

void TestFSTs::testShortestPathsWFST() 
{

  shared_ptr<PairOutputs<int64_t, int64_t>> outputs =
      make_shared<PairOutputs<int64_t, int64_t>>(
          PositiveIntOutputs::getSingleton(),
          PositiveIntOutputs::getSingleton());

  shared_ptr<Automaton::Builder<Pair<int64_t, int64_t>>> *const builder =
      make_shared<Automaton::Builder<Pair<int64_t, int64_t>>>(
          FST::INPUT_TYPE::BYTE1, outputs);

  shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"aab"), scratch),
               outputs->newPair(22LL, 57LL));
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"aac"), scratch),
               outputs->newPair(7LL, 36LL));
  builder->add(Util::toIntsRef(make_shared<BytesRef>(L"ax"), scratch),
               outputs->newPair(17LL, 85LL));
  shared_ptr<FST<Pair<int64_t, int64_t>>> *const fst = builder->finish();
  // Writer w = new OutputStreamWriter(new FileOutputStream("out.dot"));
  // Util.toDot(fst, w, false, false);
  // w.close();

  shared_ptr<Util::TopResults<Pair<int64_t, int64_t>>> res =
      Util::shortestPaths(
          fst,
          fst->getFirstArc(make_shared<FST::Arc<Pair<int64_t, int64_t>>>()),
          outputs->getNoOutput(), minPairWeightComparator, 3, true);
  assertTrue(res->isComplete);
  TestUtil::assertEquals(3, res->topN.size());

  TestUtil::assertEquals(
      Util::toIntsRef(make_shared<BytesRef>(L"aac"), scratch),
      res->topN[0]->input);
  TestUtil::assertEquals(7LL,
                         res->topN[0]->output.output1.longValue()); // weight
  TestUtil::assertEquals(36LL,
                         res->topN[0]->output.output2.longValue()); // output

  TestUtil::assertEquals(Util::toIntsRef(make_shared<BytesRef>(L"ax"), scratch),
                         res->topN[1]->input);
  TestUtil::assertEquals(17LL,
                         res->topN[1]->output.output1.longValue()); // weight
  TestUtil::assertEquals(85LL,
                         res->topN[1]->output.output2.longValue()); // output

  TestUtil::assertEquals(
      Util::toIntsRef(make_shared<BytesRef>(L"aab"), scratch),
      res->topN[2]->input);
  TestUtil::assertEquals(22LL,
                         res->topN[2]->output.output1.longValue()); // weight
  TestUtil::assertEquals(57LL,
                         res->topN[2]->output.output2.longValue()); // output
}

void TestFSTs::testShortestPathsRandom() 
{
  shared_ptr<Random> *const random = TestFSTs::random();
  int numWords = atLeast(1000);

  const map_obj<wstring, int64_t> slowCompletor = map_obj<wstring, int64_t>();
  const set<wstring> allPrefixes = set<wstring>();

  shared_ptr<PositiveIntOutputs> *const outputs =
      PositiveIntOutputs::getSingleton();
  shared_ptr<Automaton::Builder<int64_t>> *const builder =
      make_shared<Automaton::Builder<int64_t>>(FST::INPUT_TYPE::BYTE1,
                                                 outputs);
  shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();

  for (int i = 0; i < numWords; i++) {
    wstring s;
    while (true) {
      s = TestUtil::randomSimpleString(random);
      if (slowCompletor.find(s) == slowCompletor.end()) {
        break;
      }
    }

    for (int j = 1; j < s.length(); j++) {
      allPrefixes.insert(s.substr(0, j));
    }
    int weight = TestUtil::nextInt(random, 1, 100); // weights 1..100
    slowCompletor.emplace(s, static_cast<int64_t>(weight));
  }

  for (auto e : slowCompletor) {
    // System.out.println("add: " + e);
    builder->add(Util::toIntsRef(make_shared<BytesRef>(e.first), scratch),
                 e.second);
  }

  shared_ptr<FST<int64_t>> *const fst = builder->finish();
  // System.out.println("SAVE out.dot");
  // Writer w = new OutputStreamWriter(new FileOutputStream("out.dot"));
  // Util.toDot(fst, w, false, false);
  // w.close();

  shared_ptr<BytesReader> reader = fst->getBytesReader();

  // System.out.println("testing: " + allPrefixes.size() + " prefixes");
  for (auto prefix : allPrefixes) {
    // 1. run prefix against fst, then complete by value
    // System.out.println("TEST: " + prefix);

    int64_t prefixOutput = 0;
    shared_ptr<FST::Arc<int64_t>> arc =
        fst->getFirstArc(make_shared<FST::Arc<int64_t>>());
    for (int idx = 0; idx < prefix.length(); idx++) {
      if (fst->findTargetArc(static_cast<int>(prefix.charAt(idx)), arc, arc,
                             reader) == nullptr) {
        fail();
      }
      prefixOutput += arc->output;
    }

    constexpr int topN = TestUtil::nextInt(random, 1, 10);

    shared_ptr<Util::TopResults<int64_t>> r = Util::shortestPaths(
        fst, arc, fst->outputs.getNoOutput(), minLongComparator, topN, true);
    assertTrue(r->isComplete);

    // 2. go thru whole treemap (slowCompletor) and check it's actually the best
    // suggestion
    const deque<Result<int64_t>> matches = deque<Result<int64_t>>();

    // TODO: could be faster... but it's slowCompletor for a reason
    for (auto e : slowCompletor) {
      if (e.first->startsWith(prefix)) {
        // System.out.println("  consider " + e.getKey());
        matches.push_back(make_shared<Result<>>(
            Util::toIntsRef(
                make_shared<BytesRef>(e.first->substr(prefix.length())),
                make_shared<IntsRefBuilder>()),
            e.second - prefixOutput));
      }
    }

    assertTrue(matches.size() > 0);
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(matches, new
    // TieBreakByInputComparator<>(minLongComparator));
    sort(matches.begin(), matches.end(),
         make_shared<TieBreakByInputComparator<>>(minLongComparator));
    if (matches.size() > topN) {
      matches.subList(topN, matches.size())->clear();
    }

    TestUtil::assertEquals(matches.size(), r->topN.size());

    for (int hit = 0; hit < r->topN.size(); hit++) {
      // System.out.println("  check hit " + hit);
      TestUtil::assertEquals(matches[hit]->input, r->topN[hit]->input);
      TestUtil::assertEquals(matches[hit]->output, r->topN[hit]->output);
    }
  }
}

TestFSTs::TwoLongs::TwoLongs(int64_t a, int64_t b)
{
  this->a = a;
  this->b = b;
}

void TestFSTs::testShortestPathsWFSTRandom() 
{
  int numWords = atLeast(1000);

  const map_obj<wstring, std::shared_ptr<TwoLongs>> slowCompletor =
      map_obj<wstring, std::shared_ptr<TwoLongs>>();
  const set<wstring> allPrefixes = set<wstring>();

  shared_ptr<PairOutputs<int64_t, int64_t>> outputs =
      make_shared<PairOutputs<int64_t, int64_t>>(
          PositiveIntOutputs::getSingleton(),
          PositiveIntOutputs::getSingleton());
  shared_ptr<Automaton::Builder<Pair<int64_t, int64_t>>> *const builder =
      make_shared<Automaton::Builder<Pair<int64_t, int64_t>>>(
          FST::INPUT_TYPE::BYTE1, outputs);
  shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();

  shared_ptr<Random> random = TestFSTs::random();
  for (int i = 0; i < numWords; i++) {
    wstring s;
    while (true) {
      s = TestUtil::randomSimpleString(random);
      if (slowCompletor.find(s) == slowCompletor.end()) {
        break;
      }
    }

    for (int j = 1; j < s.length(); j++) {
      allPrefixes.insert(s.substr(0, j));
    }
    int weight = TestUtil::nextInt(random, 1, 100); // weights 1..100
    int output = TestUtil::nextInt(random, 0, 500); // outputs 0..500
    slowCompletor.emplace(s, make_shared<TwoLongs>(weight, output));
  }

  for (auto e : slowCompletor) {
    // System.out.println("add: " + e);
    int64_t weight = e.second::a;
    int64_t output = e.second::b;
    builder->add(Util::toIntsRef(make_shared<BytesRef>(e.first), scratch),
                 outputs->newPair(weight, output));
  }

  shared_ptr<FST<Pair<int64_t, int64_t>>> *const fst = builder->finish();
  // System.out.println("SAVE out.dot");
  // Writer w = new OutputStreamWriter(new FileOutputStream("out.dot"));
  // Util.toDot(fst, w, false, false);
  // w.close();

  shared_ptr<BytesReader> reader = fst->getBytesReader();

  // System.out.println("testing: " + allPrefixes.size() + " prefixes");
  for (auto prefix : allPrefixes) {
    // 1. run prefix against fst, then complete by value
    // System.out.println("TEST: " + prefix);

    shared_ptr<Pair<int64_t, int64_t>> prefixOutput =
        outputs->getNoOutput();
    shared_ptr<FST::Arc<Pair<int64_t, int64_t>>> arc =
        fst->getFirstArc(make_shared<FST::Arc<Pair<int64_t, int64_t>>>());
    for (int idx = 0; idx < prefix.length(); idx++) {
      if (fst->findTargetArc(static_cast<int>(prefix.charAt(idx)), arc, arc,
                             reader) == nullptr) {
        fail();
      }
      prefixOutput = outputs->add(prefixOutput, arc->output);
    }

    constexpr int topN = TestUtil::nextInt(random, 1, 10);

    shared_ptr<Util::TopResults<Pair<int64_t, int64_t>>> r =
        Util::shortestPaths(fst, arc, fst->outputs.getNoOutput(),
                            minPairWeightComparator, topN, true);
    assertTrue(r->isComplete);
    // 2. go thru whole treemap (slowCompletor) and check it's actually the best
    // suggestion
    const deque<Result<Pair<int64_t, int64_t>>> matches =
        deque<Result<Pair<int64_t, int64_t>>>();

    // TODO: could be faster... but it's slowCompletor for a reason
    for (auto e : slowCompletor) {
      if (e.first->startsWith(prefix)) {
        // System.out.println("  consider " + e.getKey());
        matches.push_back(make_shared<Result<>>(
            Util::toIntsRef(
                make_shared<BytesRef>(e.first->substr(prefix.length())),
                make_shared<IntsRefBuilder>()),
            outputs->newPair(e.second::a - prefixOutput->output1,
                             e.second::b - prefixOutput->output2)));
      }
    }

    assertTrue(matches.size() > 0);
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(matches, new
    // TieBreakByInputComparator<>(minPairWeightComparator));
    sort(matches.begin(), matches.end(),
         make_shared<TieBreakByInputComparator<>>(minPairWeightComparator));
    if (matches.size() > topN) {
      matches.subList(topN, matches.size())->clear();
    }

    TestUtil::assertEquals(matches.size(), r->topN.size());

    for (int hit = 0; hit < r->topN.size(); hit++) {
      // System.out.println("  check hit " + hit);
      TestUtil::assertEquals(matches[hit]->input, r->topN[hit]->input);
      TestUtil::assertEquals(matches[hit]->output, r->topN[hit]->output);
    }
  }
}

void TestFSTs::testLargeOutputsOnArrayArcs() 
{
  shared_ptr<ByteSequenceOutputs> *const outputs =
      ByteSequenceOutputs::getSingleton();
  shared_ptr<Automaton::Builder<std::shared_ptr<BytesRef>>> *const builder =
      make_shared<Automaton::Builder<std::shared_ptr<BytesRef>>>(
          FST::INPUT_TYPE::BYTE1, outputs);

  const std::deque<char> bytes = std::deque<char>(300);
  shared_ptr<IntsRefBuilder> *const input = make_shared<IntsRefBuilder>();
  input->append(0);
  shared_ptr<BytesRef> *const output = make_shared<BytesRef>(bytes);
  for (int arc = 0; arc < 6; arc++) {
    input->setIntAt(0, arc);
    output->bytes[0] = static_cast<char>(arc);
    builder->add(input->get(), BytesRef::deepCopyOf(output));
  }

  shared_ptr<FST<std::shared_ptr<BytesRef>>> *const fst = builder->finish();
  for (int arc = 0; arc < 6; arc++) {
    input->setIntAt(0, arc);
    shared_ptr<BytesRef> *const result = Util::get(fst, input->get());
    assertNotNull(result);
    TestUtil::assertEquals(300, result->length);
    TestUtil::assertEquals(result->bytes[result->offset], arc);
    for (int byteIDX = 1; byteIDX < result->length; byteIDX++) {
      TestUtil::assertEquals(0, result->bytes[result->offset + byteIDX]);
    }
  }
}

void TestFSTs::testIllegallyModifyRootArc() 
{
  assumeTrue(L"test relies on assertions", assertsAreEnabled);

  shared_ptr<Set<std::shared_ptr<BytesRef>>> terms =
      unordered_set<std::shared_ptr<BytesRef>>();
  for (int i = 0; i < 100; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring prefix = Character::toString(static_cast<wchar_t>(L'a' + i));
    terms->add(make_shared<BytesRef>(prefix));
    if (prefix == L"m" == false) {
      for (int j = 0; j < 20; j++) {
        // Make a big enough FST that the root cache will be created:
        wstring suffix =
            TestUtil::randomRealisticUnicodeString(random(), 10, 20);
        terms->add(make_shared<BytesRef>(prefix + suffix));
      }
    }
  }

  deque<std::shared_ptr<BytesRef>> termsList =
      deque<std::shared_ptr<BytesRef>>(terms);
  sort(termsList.begin(), termsList.end());

  shared_ptr<ByteSequenceOutputs> outputs = ByteSequenceOutputs::getSingleton();
  shared_ptr<Automaton::Builder<std::shared_ptr<BytesRef>>> builder =
      make_shared<Automaton::Builder<std::shared_ptr<BytesRef>>>(
          FST::INPUT_TYPE::BYTE1, outputs);

  shared_ptr<IntsRefBuilder> input = make_shared<IntsRefBuilder>();
  for (auto term : termsList) {
    Util::toIntsRef(term, input);
    builder->add(input->get(), term);
  }

  shared_ptr<FST<std::shared_ptr<BytesRef>>> fst = builder->finish();

  shared_ptr<Arc<std::shared_ptr<BytesRef>>> arc =
      make_shared<FST::Arc<std::shared_ptr<BytesRef>>>();
  fst->getFirstArc(arc);
  shared_ptr<FST::BytesReader> reader = fst->getBytesReader();
  arc = fst->findTargetArc(static_cast<int>(L'm'), arc, arc, reader);
  assertNotNull(arc);
  TestUtil::assertEquals(make_shared<BytesRef>(L"m"), arc->output);

  // NOTE: illegal:
  arc->output->length = 0;

  fst->getFirstArc(arc);
  try {
    arc = fst->findTargetArc(static_cast<int>(L'm'), arc, arc, reader);
  } catch (const AssertionError &ae) {
    // expected
  }
}
} // namespace org::apache::lucene::util::fst