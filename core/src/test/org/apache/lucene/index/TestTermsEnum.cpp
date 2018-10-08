using namespace std;

#include "TestTermsEnum.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;

void TestTermsEnum::test() 
{
  shared_ptr<Random> random =
      make_shared<Random>(TestTermsEnum::random()->nextLong());
  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random);
  shared_ptr<Directory> *const d = newDirectory();
  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(TestTermsEnum::random());
  analyzer->setMaxTokenLength(TestUtil::nextInt(TestTermsEnum::random(), 1,
                                                IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(TestTermsEnum::random(), d, analyzer);
  constexpr int numDocs = atLeast(10);
  for (int docCount = 0; docCount < numDocs; docCount++) {
    w->addDocument(docs->nextDoc());
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  const deque<std::shared_ptr<BytesRef>> terms =
      deque<std::shared_ptr<BytesRef>>();
  shared_ptr<TermsEnum> *const termsEnum =
      MultiFields::getTerms(r, L"body")->begin();
  shared_ptr<BytesRef> term;
  while ((term = termsEnum->next()) != nullptr) {
    terms.push_back(BytesRef::deepCopyOf(term));
  }
  if (VERBOSE) {
    wcout << L"TEST: " << terms.size() << L" terms" << endl;
  }

  int upto = -1;
  constexpr int iters = atLeast(200);
  for (int iter = 0; iter < iters; iter++) {
    constexpr bool isEnd;
    if (upto != -1 && TestTermsEnum::random()->nextBoolean()) {
      // next
      if (VERBOSE) {
        wcout << L"TEST: iter next" << endl;
      }
      isEnd = termsEnum->next() == nullptr;
      upto++;
      if (isEnd) {
        if (VERBOSE) {
          wcout << L"  end" << endl;
        }
        TestUtil::assertEquals(upto, terms.size());
        upto = -1;
      } else {
        if (VERBOSE) {
          wcout << L"  got term=" << termsEnum->term()->utf8ToString()
                << L" expected=" << terms[upto]->utf8ToString() << endl;
        }
        assertTrue(upto < terms.size());
        TestUtil::assertEquals(terms[upto], termsEnum->term());
      }
    } else {

      shared_ptr<BytesRef> *const target;
      const wstring exists;
      if (TestTermsEnum::random()->nextBoolean()) {
        // likely fake term
        if (TestTermsEnum::random()->nextBoolean()) {
          target = make_shared<BytesRef>(
              TestUtil::randomSimpleString(TestTermsEnum::random()));
        } else {
          target = make_shared<BytesRef>(
              TestUtil::randomRealisticUnicodeString(TestTermsEnum::random()));
        }
        exists = L"likely not";
      } else {
        // real term
        target = terms[TestTermsEnum::random()->nextInt(terms.size())];
        exists = L"yes";
      }

      upto = Collections::binarySearch(terms, target);

      if (TestTermsEnum::random()->nextBoolean()) {
        if (VERBOSE) {
          wcout << L"TEST: iter seekCeil target=" << target->utf8ToString()
                << L" exists=" << exists << endl;
        }
        // seekCeil
        constexpr TermsEnum::SeekStatus status = termsEnum->seekCeil(target);
        if (VERBOSE) {
          wcout << L"  got " << status << endl;
        }

        if (upto < 0) {
          upto = -(upto + 1);
          if (upto >= terms.size()) {
            TestUtil::assertEquals(TermsEnum::SeekStatus::END, status);
            upto = -1;
          } else {
            TestUtil::assertEquals(TermsEnum::SeekStatus::NOT_FOUND, status);
            TestUtil::assertEquals(terms[upto], termsEnum->term());
          }
        } else {
          TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND, status);
          TestUtil::assertEquals(terms[upto], termsEnum->term());
        }
      } else {
        if (VERBOSE) {
          wcout << L"TEST: iter seekExact target=" << target->utf8ToString()
                << L" exists=" << exists << endl;
        }
        // seekExact
        constexpr bool result = termsEnum->seekExact(target);
        if (VERBOSE) {
          wcout << L"  got " << result << endl;
        }
        if (upto < 0) {
          assertFalse(result);
          upto = -1;
        } else {
          assertTrue(result);
          TestUtil::assertEquals(target, termsEnum->term());
        }
      }
    }
  }

  delete r;
  delete d;
  delete docs;
}

void TestTermsEnum::addDoc(
    shared_ptr<RandomIndexWriter> w, shared_ptr<deque<wstring>> terms,
    unordered_map<std::shared_ptr<BytesRef>, int> &termToID,
    int id) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
  if (VERBOSE) {
    wcout << L"TEST: addDoc id:" << id << L" terms=" << terms << endl;
  }
  for (auto s2 : terms) {
    doc->push_back(newStringField(L"f", s2, Field::Store::NO));
    termToID.emplace(make_shared<BytesRef>(s2), id);
  }
  w->addDocument(doc);
  terms->clear();
}

bool TestTermsEnum::accepts(shared_ptr<CompiledAutomaton> c,
                            shared_ptr<BytesRef> b)
{
  int state = 0;
  for (int idx = 0; idx < b->length; idx++) {
    assertTrue(state != -1);
    state = c->runAutomaton->step(state, b->bytes[b->offset + idx] & 0xff);
  }
  return c->runAutomaton->isAccept(state);
}

void TestTermsEnum::testIntersectRandom() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  constexpr int numTerms = atLeast(300);
  // final int numTerms = 50;

  shared_ptr<Set<wstring>> *const terms = unordered_set<wstring>();
  shared_ptr<deque<wstring>> *const pendingTerms = deque<wstring>();
  const unordered_map<std::shared_ptr<BytesRef>, int> termToID =
      unordered_map<std::shared_ptr<BytesRef>, int>();
  int id = 0;
  while (terms->size() != numTerms) {
    const wstring s = getRandomString();
    if (!terms->contains(s)) {
      terms->add(s);
      pendingTerms->add(s);
      if (random()->nextInt(20) == 7) {
        addDoc(w, pendingTerms, termToID, id++);
      }
    }
  }
  addDoc(w, pendingTerms, termToID, id++);

  std::deque<std::shared_ptr<BytesRef>> termsArray(terms->size());
  shared_ptr<Set<std::shared_ptr<BytesRef>>> *const termsSet =
      unordered_set<std::shared_ptr<BytesRef>>();
  {
    int upto = 0;
    for (auto s : terms) {
      shared_ptr<BytesRef> *const b = make_shared<BytesRef>(s);
      termsArray[upto++] = b;
      termsSet->add(b);
    }
    Arrays::sort(termsArray);
  }

  if (VERBOSE) {
    wcout << L"\nTEST: indexed terms (unicode order):" << endl;
    for (auto t : termsArray) {
      wcout << L"  " << t->utf8ToString() << L" -> id:" << termToID[t] << endl;
    }
  }

  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  std::deque<int> docIDToID(r->maxDoc());
  shared_ptr<NumericDocValues> values =
      MultiDocValues::getNumericValues(r, L"id");
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, values->nextDoc());
    docIDToID[i] = static_cast<int>(values->longValue());
  }

  for (int iter = 0; iter < 10 * RANDOM_MULTIPLIER; iter++) {

    // TODO: can we also test infinite As here...?

    // From the random terms, pick some ratio and compile an
    // automaton:
    shared_ptr<Set<wstring>> *const acceptTerms = unordered_set<wstring>();
    const set<std::shared_ptr<BytesRef>> sortedAcceptTerms =
        set<std::shared_ptr<BytesRef>>();
    constexpr double keepPct = random()->nextDouble();
    shared_ptr<Automaton> a;
    if (iter == 0) {
      if (VERBOSE) {
        wcout << L"\nTEST: empty automaton" << endl;
      }
      a = Automata::makeEmpty();
    } else {
      if (VERBOSE) {
        wcout << L"\nTEST: keepPct=" << keepPct << endl;
      }
      for (auto s : terms) {
        const wstring s2;
        if (random()->nextDouble() <= keepPct) {
          s2 = s;
        } else {
          s2 = getRandomString();
        }
        acceptTerms->add(s2);
        sortedAcceptTerms.insert(make_shared<BytesRef>(s2));
      }
      a = Automata::makeStringUnion(sortedAcceptTerms);
    }

    shared_ptr<CompiledAutomaton> *const c =
        make_shared<CompiledAutomaton>(a, true, false, 1000000, false);

    std::deque<std::shared_ptr<BytesRef>> acceptTermsArray(
        acceptTerms->size());
    shared_ptr<Set<std::shared_ptr<BytesRef>>> *const acceptTermsSet =
        unordered_set<std::shared_ptr<BytesRef>>();
    int upto = 0;
    for (auto s : acceptTerms) {
      shared_ptr<BytesRef> *const b = make_shared<BytesRef>(s);
      acceptTermsArray[upto++] = b;
      acceptTermsSet->add(b);
      assertTrue(accepts(c, b));
    }
    Arrays::sort(acceptTermsArray);

    if (VERBOSE) {
      wcout << L"\nTEST: accept terms (unicode order):" << endl;
      for (auto t : acceptTermsArray) {
        wcout << L"  " << t->utf8ToString()
              << (termsSet->contains(t) ? L" (exists)" : L"") << endl;
      }
      wcout << a->toDot() << endl;
    }

    for (int iter2 = 0; iter2 < 100; iter2++) {
      shared_ptr<BytesRef> *const startTerm =
          acceptTermsArray.empty() || random()->nextBoolean()
              ? nullptr
              : acceptTermsArray[random()->nextInt(acceptTermsArray.size())];

      if (VERBOSE) {
        wcout << L"\nTEST: iter2=" << iter2 << L" startTerm="
              << (startTerm == nullptr ? L"<null>" : startTerm->utf8ToString())
              << endl;

        if (startTerm != nullptr) {
          int state = 0;
          for (int idx = 0; idx < startTerm->length; idx++) {
            constexpr int label =
                startTerm->bytes[startTerm->offset + idx] & 0xff;
            wcout << L"  state=" << state << L" label=" << label << endl;
            state = c->runAutomaton->step(state, label);
            assertTrue(state != -1);
          }
          wcout << L"  state=" << state << endl;
        }
      }

      shared_ptr<TermsEnum> *const te =
          MultiFields::getTerms(r, L"f")->intersect(c, startTerm);

      int loc;
      if (startTerm == nullptr) {
        loc = 0;
      } else {
        loc = Arrays::binarySearch(termsArray, BytesRef::deepCopyOf(startTerm));
        if (loc < 0) {
          loc = -(loc + 1);
        } else {
          // startTerm exists in index
          loc++;
        }
      }
      while (loc < termsArray.size() &&
             !acceptTermsSet->contains(termsArray[loc])) {
        loc++;
      }

      shared_ptr<PostingsEnum> postingsEnum = nullptr;
      while (loc < termsArray.size()) {
        shared_ptr<BytesRef> *const expected = termsArray[loc];
        shared_ptr<BytesRef> *const actual = te->next();
        if (VERBOSE) {
          wcout << L"TEST:   next() expected=" << expected->utf8ToString()
                << L" actual="
                << (actual == nullptr ? L"null" : actual->utf8ToString())
                << endl;
        }
        TestUtil::assertEquals(expected, actual);
        TestUtil::assertEquals(1, te->docFreq());
        postingsEnum =
            TestUtil::docs(random(), te, postingsEnum, PostingsEnum::NONE);
        constexpr int docID = postingsEnum->nextDoc();
        assertTrue(docID != DocIdSetIterator::NO_MORE_DOCS);
        TestUtil::assertEquals(docIDToID[docID], termToID[expected]);
        do {
          loc++;
        } while (loc < termsArray.size() &&
                 !acceptTermsSet->contains(termsArray[loc]));
      }
      assertNull(te->next());
    }
  }

  delete r;
  delete dir;
}

shared_ptr<IndexReader>
TestTermsEnum::makeIndex(deque<wstring> &terms) 
{
  d = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));

  /*
  iwc.setCodec(new StandardCodec(minTermsInBlock, maxTermsInBlock));
  */

  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), d, iwc);
  for (wstring term : terms) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> f = newStringField(FIELD, term, Field::Store::NO);
    doc->push_back(f);
    w->addDocument(doc);
  }
  if (r != nullptr) {
    close();
  }
  r = w->getReader();
  delete w;
  return r;
}

TestTermsEnum::~TestTermsEnum()
{
  delete r;
  delete d;
}

int TestTermsEnum::docFreq(shared_ptr<IndexReader> r,
                           const wstring &term) 
{
  return r->docFreq(make_shared<Term>(FIELD, term));
}

void TestTermsEnum::testEasy() 
{
  // No floor arcs:
  r = makeIndex(
      {L"aa0", L"aa1", L"aa2", L"aa3", L"bb0", L"bb1", L"bb2", L"bb3", L"aa"});

  // First term in block:
  TestUtil::assertEquals(1, docFreq(r, L"aa0"));

  // Scan forward to another term in same block
  TestUtil::assertEquals(1, docFreq(r, L"aa2"));

  TestUtil::assertEquals(1, docFreq(r, L"aa"));

  // Reset same block then scan forwards
  TestUtil::assertEquals(1, docFreq(r, L"aa1"));

  // Not found, in same block
  TestUtil::assertEquals(0, docFreq(r, L"aa5"));

  // Found, in same block
  TestUtil::assertEquals(1, docFreq(r, L"aa2"));

  // Not found in index:
  TestUtil::assertEquals(0, docFreq(r, L"b0"));

  // Found:
  TestUtil::assertEquals(1, docFreq(r, L"aa2"));

  // Found, rewind:
  TestUtil::assertEquals(1, docFreq(r, L"aa0"));

  // First term in block:
  TestUtil::assertEquals(1, docFreq(r, L"bb0"));

  // Scan forward to another term in same block
  TestUtil::assertEquals(1, docFreq(r, L"bb2"));

  // Reset same block then scan forwards
  TestUtil::assertEquals(1, docFreq(r, L"bb1"));

  // Not found, in same block
  TestUtil::assertEquals(0, docFreq(r, L"bb5"));

  // Found, in same block
  TestUtil::assertEquals(1, docFreq(r, L"bb2"));

  // Not found in index:
  TestUtil::assertEquals(0, docFreq(r, L"b0"));

  // Found:
  TestUtil::assertEquals(1, docFreq(r, L"bb2"));

  // Found, rewind:
  TestUtil::assertEquals(1, docFreq(r, L"bb0"));

  close();
}

void TestTermsEnum::testFloorBlocks() 
{
  const std::deque<wstring> terms =
      std::deque<wstring>{L"aa0", L"aa1", L"aa2", L"aa3", L"aa4", L"aa5",
                           L"aa6", L"aa7", L"aa8", L"aa9", L"aa",  L"xx"};
  r = makeIndex(terms);
  // r = makeIndex("aa0", "aa1", "aa2", "aa3", "aa4", "aa5", "aa6", "aa7",
  // "aa8", "aa9");

  // First term in first block:
  TestUtil::assertEquals(1, docFreq(r, L"aa0"));
  TestUtil::assertEquals(1, docFreq(r, L"aa4"));

  // No block
  TestUtil::assertEquals(0, docFreq(r, L"bb0"));

  // Second block
  TestUtil::assertEquals(1, docFreq(r, L"aa4"));

  // Backwards to prior floor block:
  TestUtil::assertEquals(1, docFreq(r, L"aa0"));

  // Forwards to last floor block:
  TestUtil::assertEquals(1, docFreq(r, L"aa9"));

  TestUtil::assertEquals(0, docFreq(r, L"a"));
  TestUtil::assertEquals(1, docFreq(r, L"aa"));
  TestUtil::assertEquals(0, docFreq(r, L"a"));
  TestUtil::assertEquals(1, docFreq(r, L"aa"));

  // Forwards to last floor block:
  TestUtil::assertEquals(1, docFreq(r, L"xx"));
  TestUtil::assertEquals(1, docFreq(r, L"aa1"));
  TestUtil::assertEquals(0, docFreq(r, L"yy"));

  TestUtil::assertEquals(1, docFreq(r, L"xx"));
  TestUtil::assertEquals(1, docFreq(r, L"aa9"));

  TestUtil::assertEquals(1, docFreq(r, L"xx"));
  TestUtil::assertEquals(1, docFreq(r, L"aa4"));

  shared_ptr<TermsEnum> *const te = MultiFields::getTerms(r, FIELD)->begin();
  while (te->next() != nullptr) {
    // System.out.println("TEST: next term=" + te.term().utf8ToString());
  }

  assertTrue(seekExact(te, L"aa1"));
  TestUtil::assertEquals(L"aa2", next(te));
  assertTrue(seekExact(te, L"aa8"));
  TestUtil::assertEquals(L"aa9", next(te));
  TestUtil::assertEquals(L"xx", next(te));

  testRandomSeeks(r, terms);
  close();
}

void TestTermsEnum::testZeroTerms() 
{
  d = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"one two three", Field::Store::NO));
  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field2", L"one two three", Field::Store::NO));
  w->addDocument(doc);
  w->commit();
  w->deleteDocuments(make_shared<Term>(L"field", L"one"));
  w->forceMerge(1);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  TestUtil::assertEquals(1, r->numDocs());
  TestUtil::assertEquals(1, r->maxDoc());
  shared_ptr<Terms> terms = MultiFields::getTerms(r, L"field");
  if (terms != nullptr) {
    assertNull(terms->begin()->next());
  }
  delete r;
  delete d;
}

wstring TestTermsEnum::getRandomString()
{
  // return _TestUtil.randomSimpleString(random());
  return TestUtil::randomRealisticUnicodeString(random());
}

void TestTermsEnum::testRandomTerms() 
{
  const std::deque<wstring> terms =
      std::deque<wstring>(TestUtil::nextInt(random(), 1, atLeast(1000)));
  shared_ptr<Set<wstring>> *const seen = unordered_set<wstring>();

  constexpr bool allowEmptyString = random()->nextBoolean();

  if (random()->nextInt(10) == 7 && terms.size() > 2) {
    // Sometimes add a bunch of terms sharing a longish common prefix:
    constexpr int numTermsSamePrefix = random()->nextInt(terms.size() / 2);
    if (numTermsSamePrefix > 0) {
      wstring prefix;
      while (true) {
        prefix = getRandomString();
        if (prefix.length() < 5) {
          continue;
        } else {
          break;
        }
      }
      while (seen->size() < numTermsSamePrefix) {
        const wstring t = prefix + getRandomString();
        if (!seen->contains(t)) {
          terms[seen->size()] = t;
          seen->add(t);
        }
      }
    }
  }

  while (seen->size() < terms.size()) {
    const wstring t = getRandomString();
    if (!seen->contains(t) && (allowEmptyString || t.length() != 0)) {
      terms[seen->size()] = t;
      seen->add(t);
    }
  }
  r = makeIndex(terms);
  testRandomSeeks(r, terms);
  close();
}

bool TestTermsEnum::seekExact(shared_ptr<TermsEnum> te,
                              const wstring &term) 
{
  return te->seekExact(make_shared<BytesRef>(term));
}

wstring TestTermsEnum::next(shared_ptr<TermsEnum> te) 
{
  shared_ptr<BytesRef> *const br = te->next();
  if (br == nullptr) {
    return L"";
  } else {
    return br->utf8ToString();
  }
}

shared_ptr<BytesRef>
TestTermsEnum::getNonExistTerm(std::deque<std::shared_ptr<BytesRef>> &terms)
{
  shared_ptr<BytesRef> t = nullptr;
  while (true) {
    const wstring ts = getRandomString();
    t = make_shared<BytesRef>(ts);
    if (Arrays::binarySearch(terms, t) < 0) {
      return t;
    }
  }
}

TestTermsEnum::TermAndState::TermAndState(shared_ptr<BytesRef> term,
                                          shared_ptr<TermState> state)
    : term(term), state(state)
{
}

void TestTermsEnum::testRandomSeeks(
    shared_ptr<IndexReader> r,
    deque<wstring> &validTermStrings) 
{
  std::deque<std::shared_ptr<BytesRef>> validTerms(validTermStrings->length);
  for (int termIDX = 0; termIDX < validTermStrings->length; termIDX++) {
    validTerms[termIDX] = make_shared<BytesRef>(validTermStrings[termIDX]);
  }
  Arrays::sort(validTerms);
  if (VERBOSE) {
    wcout << L"TEST: " << validTerms.size() << L" terms:" << endl;
    for (auto t : validTerms) {
      wcout << L"  " << t->utf8ToString() << L" " << t << endl;
    }
  }
  shared_ptr<TermsEnum> *const te = MultiFields::getTerms(r, FIELD)->begin();

  constexpr int END_LOC = -validTerms.size() - 1;

  const deque<std::shared_ptr<TermAndState>> termStates =
      deque<std::shared_ptr<TermAndState>>();

  for (int iter = 0; iter < 100 * RANDOM_MULTIPLIER; iter++) {

    shared_ptr<BytesRef> *const t;
    int loc;
    shared_ptr<TermState> *const termState;
    if (random()->nextInt(6) == 4) {
      // pick term that doens't exist:
      t = getNonExistTerm(validTerms);
      termState.reset();
      if (VERBOSE) {
        wcout << L"\nTEST: invalid term=" << t->utf8ToString() << endl;
      }
      loc = Arrays::binarySearch(validTerms, t);
    } else if (termStates.size() != 0 && random()->nextInt(4) == 1) {
      shared_ptr<TermAndState> *const ts =
          termStates[random()->nextInt(termStates.size())];
      t = ts->term;
      loc = Arrays::binarySearch(validTerms, t);
      assertTrue(loc >= 0);
      termState = ts->state;
      if (VERBOSE) {
        wcout << L"\nTEST: valid termState term=" << t->utf8ToString() << endl;
      }
    } else {
      // pick valid term
      loc = random()->nextInt(validTerms.size());
      t = BytesRef::deepCopyOf(validTerms[loc]);
      termState.reset();
      if (VERBOSE) {
        wcout << L"\nTEST: valid term=" << t->utf8ToString() << endl;
      }
    }

    // seekCeil or seekExact:
    constexpr bool doSeekExact = random()->nextBoolean();
    if (termState != nullptr) {
      if (VERBOSE) {
        wcout << L"  seekExact termState" << endl;
      }
      te->seekExact(t, termState);
    } else if (doSeekExact) {
      if (VERBOSE) {
        wcout << L"  seekExact" << endl;
      }
      TestUtil::assertEquals(loc >= 0, te->seekExact(t));
    } else {
      if (VERBOSE) {
        wcout << L"  seekCeil" << endl;
      }

      constexpr TermsEnum::SeekStatus result = te->seekCeil(t);
      if (VERBOSE) {
        wcout << L"  got " << result << endl;
      }

      if (loc >= 0) {
        TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND, result);
      } else if (loc == END_LOC) {
        TestUtil::assertEquals(TermsEnum::SeekStatus::END, result);
      } else {
        assert(loc >= -validTerms.size());
        TestUtil::assertEquals(TermsEnum::SeekStatus::NOT_FOUND, result);
      }
    }

    if (loc >= 0) {
      TestUtil::assertEquals(t, te->term());
    } else if (doSeekExact) {
      // TermsEnum is unpositioned if seekExact returns false
      continue;
    } else if (loc == END_LOC) {
      continue;
    } else {
      loc = -loc - 1;
      TestUtil::assertEquals(validTerms[loc], te->term());
    }

    // Do a bunch of next's after the seek
    constexpr int numNext = random()->nextInt(validTerms.size());

    for (int nextCount = 0; nextCount < numNext; nextCount++) {
      if (VERBOSE) {
        wcout << L"\nTEST: next loc=" << loc << L" of " << validTerms.size()
              << endl;
      }
      shared_ptr<BytesRef> *const t2 = te->next();
      loc++;
      if (loc == validTerms.size()) {
        assertNull(t2);
        break;
      } else {
        TestUtil::assertEquals(validTerms[loc], t2);
        if (random()->nextInt(40) == 17 && termStates.size() < 100) {
          termStates.push_back(
              make_shared<TermAndState>(validTerms[loc], te->termState()));
        }
      }
    }
  }
}

void TestTermsEnum::testIntersectBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(make_shared<LogDocMergePolicy>());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"aaa", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"bbb", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"ccc", Field::Store::NO));
  w->addDocument(doc);

  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = w->getReader();
  delete w;
  shared_ptr<LeafReader> sub = getOnlyLeafReader(r);
  shared_ptr<Terms> terms = sub->terms(L"field");
  shared_ptr<Automaton> automaton =
      (make_shared<RegExp>(L".*", RegExp::NONE))->toAutomaton();
  shared_ptr<CompiledAutomaton> ca =
      make_shared<CompiledAutomaton>(automaton, false, false);
  shared_ptr<TermsEnum> te = terms->intersect(ca, nullptr);
  TestUtil::assertEquals(L"aaa", te->next().utf8ToString());
  TestUtil::assertEquals(0,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  TestUtil::assertEquals(L"bbb", te->next().utf8ToString());
  TestUtil::assertEquals(1,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  TestUtil::assertEquals(L"ccc", te->next().utf8ToString());
  TestUtil::assertEquals(2,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  assertNull(te->next());

  te = terms->intersect(ca, make_shared<BytesRef>(L"abc"));
  TestUtil::assertEquals(L"bbb", te->next().utf8ToString());
  TestUtil::assertEquals(1,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  TestUtil::assertEquals(L"ccc", te->next().utf8ToString());
  TestUtil::assertEquals(2,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  assertNull(te->next());

  te = terms->intersect(ca, make_shared<BytesRef>(L"aaa"));
  TestUtil::assertEquals(L"bbb", te->next().utf8ToString());
  TestUtil::assertEquals(1,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  TestUtil::assertEquals(L"ccc", te->next().utf8ToString());
  TestUtil::assertEquals(2,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  assertNull(te->next());

  r->close();
  delete dir;
}

void TestTermsEnum::testIntersectStartTerm() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(make_shared<LogDocMergePolicy>());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"abc", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"abd", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"acd", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"bcd", Field::Store::NO));
  w->addDocument(doc);

  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = w->getReader();
  delete w;
  shared_ptr<LeafReader> sub = getOnlyLeafReader(r);
  shared_ptr<Terms> terms = sub->terms(L"field");

  shared_ptr<Automaton> automaton =
      (make_shared<RegExp>(L".*d", RegExp::NONE))->toAutomaton();
  shared_ptr<CompiledAutomaton> ca =
      make_shared<CompiledAutomaton>(automaton, false, false);
  shared_ptr<TermsEnum> te;

  // should seek to startTerm
  te = terms->intersect(ca, make_shared<BytesRef>(L"aad"));
  TestUtil::assertEquals(L"abd", te->next().utf8ToString());
  TestUtil::assertEquals(1,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  TestUtil::assertEquals(L"acd", te->next().utf8ToString());
  TestUtil::assertEquals(2,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  TestUtil::assertEquals(L"bcd", te->next().utf8ToString());
  TestUtil::assertEquals(3,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  assertNull(te->next());

  // should fail to find ceil label on second arc, rewind
  te = terms->intersect(ca, make_shared<BytesRef>(L"add"));
  TestUtil::assertEquals(L"bcd", te->next().utf8ToString());
  TestUtil::assertEquals(3,
                         te->postings(nullptr, PostingsEnum::NONE)->nextDoc());
  assertNull(te->next());

  // should reach end
  te = terms->intersect(ca, make_shared<BytesRef>(L"bcd"));
  assertNull(te->next());
  te = terms->intersect(ca, make_shared<BytesRef>(L"ddd"));
  assertNull(te->next());

  r->close();
  delete dir;
}

void TestTermsEnum::testIntersectEmptyString() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(make_shared<LogDocMergePolicy>());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"", Field::Store::NO));
  doc->push_back(newStringField(L"field", L"abc", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  // add empty string to both documents, so that singletonDocID == -1.
  // For a FST-based term dict, we'll expect to see the first arc is
  // flaged with HAS_FINAL_OUTPUT
  doc->push_back(newStringField(L"field", L"abc", Field::Store::NO));
  doc->push_back(newStringField(L"field", L"", Field::Store::NO));
  w->addDocument(doc);

  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = w->getReader();
  delete w;
  shared_ptr<LeafReader> sub = getOnlyLeafReader(r);
  shared_ptr<Terms> terms = sub->terms(L"field");

  shared_ptr<Automaton> automaton =
      (make_shared<RegExp>(L".*", RegExp::NONE))->toAutomaton(); // accept ALL
  shared_ptr<CompiledAutomaton> ca =
      make_shared<CompiledAutomaton>(automaton, false, false);

  shared_ptr<TermsEnum> te = terms->intersect(ca, nullptr);
  shared_ptr<PostingsEnum> de;

  TestUtil::assertEquals(L"", te->next().utf8ToString());
  de = te->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(0, de->nextDoc());
  TestUtil::assertEquals(1, de->nextDoc());

  TestUtil::assertEquals(L"abc", te->next().utf8ToString());
  de = te->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(0, de->nextDoc());
  TestUtil::assertEquals(1, de->nextDoc());

  assertNull(te->next());

  // pass empty string
  te = terms->intersect(ca, make_shared<BytesRef>(L""));

  TestUtil::assertEquals(L"abc", te->next().utf8ToString());
  de = te->postings(nullptr, PostingsEnum::NONE);
  TestUtil::assertEquals(0, de->nextDoc());
  TestUtil::assertEquals(1, de->nextDoc());

  assertNull(te->next());

  r->close();
  delete dir;
}

void TestTermsEnum::testCommonPrefixTerms() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Set<wstring>> terms = unordered_set<wstring>();
  // std::wstring prefix = TestUtil.randomSimpleString(random(), 1, 20);
  wstring prefix = TestUtil::randomRealisticUnicodeString(random(), 1, 20);
  int numTerms = atLeast(1000);
  if (VERBOSE) {
    wcout << L"TEST: " << numTerms << L" terms; prefix=" << prefix << endl;
  }
  while (terms->size() < numTerms) {
    // terms.add(prefix + TestUtil.randomSimpleString(random(), 1, 20));
    terms->add(prefix +
               TestUtil::randomRealisticUnicodeString(random(), 1, 20));
  }
  for (auto term : terms) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", term, Field::Store::YES));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> r = w->getReader();
  if (VERBOSE) {
    wcout << L"\nTEST: reader=" << r << endl;
  }

  shared_ptr<TermsEnum> termsEnum = MultiFields::getTerms(r, L"id")->begin();
  shared_ptr<PostingsEnum> postingsEnum = nullptr;
  shared_ptr<PerThreadPKLookup> pkLookup =
      make_shared<PerThreadPKLookup>(r, L"id");

  int iters = atLeast(numTerms * 3);
  deque<wstring> termsList = deque<wstring>(terms);
  for (int iter = 0; iter < iters; iter++) {
    wstring term;
    bool shouldExist;
    if (random()->nextBoolean()) {
      term = termsList[random()->nextInt(terms->size())];
      shouldExist = true;
    } else {
      term = prefix + TestUtil::randomSimpleString(random(), 1, 20);
      shouldExist = terms->contains(term);
    }

    if (VERBOSE) {
      wcout << L"\nTEST: try term=" << term << endl;
      wcout << L"  shouldExist?=" << shouldExist << endl;
    }

    shared_ptr<BytesRef> termBytesRef = make_shared<BytesRef>(term);

    bool actualResult = termsEnum->seekExact(termBytesRef);
    TestUtil::assertEquals(shouldExist, actualResult);
    if (shouldExist) {
      postingsEnum = termsEnum->postings(postingsEnum, 0);
      int docID = postingsEnum->nextDoc();
      assertTrue(docID != PostingsEnum::NO_MORE_DOCS);
      TestUtil::assertEquals(docID, pkLookup->lookup(termBytesRef));
      shared_ptr<Document> doc = r->document(docID);
      TestUtil::assertEquals(term, doc[L"id"]);

      if (random()->nextInt(7) == 1) {
        termsEnum->next();
      }
    } else {
      TestUtil::assertEquals(-1, pkLookup->lookup(termBytesRef));
    }

    if (random()->nextInt(7) == 1) {
      TermsEnum::SeekStatus status = termsEnum->seekCeil(termBytesRef);
      if (shouldExist) {
        TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND, status);
      } else {
        assertNotSame(TermsEnum::SeekStatus::FOUND, status);
      }
    }
  }

  delete r;
  delete w;
  delete d;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testVaryingTermsPerSegment() throws
// Exception
void TestTermsEnum::testVaryingTermsPerSegment() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Set<std::shared_ptr<BytesRef>>> terms =
      unordered_set<std::shared_ptr<BytesRef>>();
  int MAX_TERMS = atLeast(1000);
  while (terms->size() < MAX_TERMS) {
    terms->add(
        make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 1, 40)));
  }
  deque<std::shared_ptr<BytesRef>> termsList =
      deque<std::shared_ptr<BytesRef>>(terms);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int termCount = 0; termCount < MAX_TERMS; termCount++) {
    if (VERBOSE) {
      wcout << L"\nTEST: termCount=" << termCount << L" add term="
            << termsList[termCount]->utf8ToString() << endl;
    }
    sb->append(L' ');
    sb->append(termsList[termCount]->utf8ToString());
    shared_ptr<IndexWriterConfig> iwc =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    iwc->setOpenMode(IndexWriterConfig::OpenMode::CREATE);
    shared_ptr<RandomIndexWriter> w =
        make_shared<RandomIndexWriter>(random(), dir, iwc);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"field", sb->toString(), Field::Store::NO));
    w->addDocument(doc);
    shared_ptr<IndexReader> r = w->getReader();
    TestUtil::assertEquals(1, r->leaves().size());
    shared_ptr<TermsEnum> te = r->leaves()[0]->reader().terms(L"field").begin();
    for (int i = 0; i <= termCount; i++) {
      assertTrue(L"term '" + termsList[i]->utf8ToString() +
                     L"' should exist but doesn't",
                 te->seekExact(termsList[i]));
    }
    for (int i = termCount + 1; i < termsList.size(); i++) {
      assertFalse(L"term '" + termsList[i] + L"' shouldn't exist but does",
                  te->seekExact(termsList[i]));
    }
    delete r;
    delete w;
  }
  delete dir;
}

void TestTermsEnum::testIntersectRegexp() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"foobar", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Terms> terms = MultiFields::getTerms(r, L"field");
  shared_ptr<CompiledAutomaton> automaton = make_shared<CompiledAutomaton>(
      (make_shared<RegExp>(L"do_not_match_anything"))->toAutomaton());
  wstring message =
      expectThrows(invalid_argument::typeid,
                   [&]() { terms->intersect(automaton, nullptr); })
          .what();
  TestUtil::assertEquals(L"please use CompiledAutomaton.getTermsEnum instead",
                         message);
  delete r;
  delete w;
  delete d;
}

void TestTermsEnum::testInvalidAutomatonTermsEnum() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AutomatonTermsEnum>(
        TermsEnum::EMPTY,
        make_shared<CompiledAutomaton>(Automata::makeString(L"foo")));
  });
}
} // namespace org::apache::lucene::index