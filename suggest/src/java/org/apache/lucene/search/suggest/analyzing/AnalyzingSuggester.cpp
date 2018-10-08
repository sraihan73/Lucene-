using namespace std;

#include "AnalyzingSuggester.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using LimitedFiniteStringsIterator =
    org::apache::lucene::util::automaton::LimitedFiniteStringsIterator;
using Operations = org::apache::lucene::util::automaton::Operations;
using Transition = org::apache::lucene::util::automaton::Transition;
using Builder = org::apache::lucene::util::fst::Builder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using org::apache::lucene::util::fst::Util::Result;
using org::apache::lucene::util::fst::Util::TopResults;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

AnalyzingSuggester::AnalyzingSuggester(shared_ptr<Directory> tempDir,
                                       const wstring &tempFileNamePrefix,
                                       shared_ptr<Analyzer> analyzer)
    : AnalyzingSuggester(tempDir, tempFileNamePrefix, analyzer, analyzer,
                         EXACT_FIRST | PRESERVE_SEP, 256, -1, true)
{
}

AnalyzingSuggester::AnalyzingSuggester(shared_ptr<Directory> tempDir,
                                       const wstring &tempFileNamePrefix,
                                       shared_ptr<Analyzer> indexAnalyzer,
                                       shared_ptr<Analyzer> queryAnalyzer)
    : AnalyzingSuggester(tempDir, tempFileNamePrefix, indexAnalyzer,
                         queryAnalyzer, EXACT_FIRST | PRESERVE_SEP, 256, -1,
                         true)
{
}

AnalyzingSuggester::AnalyzingSuggester(
    shared_ptr<Directory> tempDir, const wstring &tempFileNamePrefix,
    shared_ptr<Analyzer> indexAnalyzer, shared_ptr<Analyzer> queryAnalyzer,
    int options, int maxSurfaceFormsPerAnalyzedForm, int maxGraphExpansions,
    bool preservePositionIncrements)
    : indexAnalyzer(indexAnalyzer), queryAnalyzer(queryAnalyzer),
      exactFirst((options & EXACT_FIRST) != 0),
      preserveSep((options & PRESERVE_SEP) != 0),
      maxSurfaceFormsPerAnalyzedForm(maxSurfaceFormsPerAnalyzedForm),
      maxGraphExpansions(maxGraphExpansions), tempDir(tempDir),
      tempFileNamePrefix(tempFileNamePrefix)
{
  if ((options & ~(EXACT_FIRST | PRESERVE_SEP)) != 0) {
    throw invalid_argument(
        L"options should only contain EXACT_FIRST and PRESERVE_SEP; got " +
        to_wstring(options));
  }

  // NOTE: this is just an implementation limitation; if
  // somehow this is a problem we could fix it by using
  // more than one byte to disambiguate ... but 256 seems
  // like it should be way more then enough.
  if (maxSurfaceFormsPerAnalyzedForm <= 0 ||
      maxSurfaceFormsPerAnalyzedForm > 256) {
    throw invalid_argument(
        L"maxSurfaceFormsPerAnalyzedForm must be > 0 and < 256 (got: " +
        to_wstring(maxSurfaceFormsPerAnalyzedForm) + L")");
  }

  if (maxGraphExpansions < 1 && maxGraphExpansions != -1) {
    throw invalid_argument(
        L"maxGraphExpansions must -1 (no limit) or > 0 (got: " +
        to_wstring(maxGraphExpansions) + L")");
  }
  this->preservePositionIncrements = preservePositionIncrements;
}

int64_t AnalyzingSuggester::ramBytesUsed()
{
  return fst == nullptr ? 0 : fst->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AnalyzingSuggester::getChildResources()
{
  if (fst == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"fst", fst));
  }
}

shared_ptr<Automaton> AnalyzingSuggester::replaceSep(shared_ptr<Automaton> a)
{

  int numStates = a->getNumStates();
  shared_ptr<Automaton::Builder> result =
      make_shared<Automaton::Builder>(numStates, a->getNumTransitions());
  // Copy all states over
  result->copyStates(a);

  // Go in reverse topo sort so we know we only have to
  // make one pass:
  shared_ptr<Transition> t = make_shared<Transition>();
  std::deque<int> topoSortStates = Operations::topoSortStates(a);
  for (int i = 0; i < topoSortStates.size(); i++) {
    int state = topoSortStates[topoSortStates.size() - 1 - i];
    int count = a->initTransition(state, t);
    for (int j = 0; j < count; j++) {
      a->getNextTransition(t);
      if (t->min == TokenStreamToAutomaton::POS_SEP) {
        assert(t->max == TokenStreamToAutomaton::POS_SEP);
        if (preserveSep) {
          // Remap to SEP_LABEL:
          result->addTransition(state, t->dest, SEP_LABEL);
        } else {
          result->addEpsilon(state, t->dest);
        }
      } else if (t->min == TokenStreamToAutomaton::HOLE) {
        assert(t->max == TokenStreamToAutomaton::HOLE);

        // Just remove the hole: there will then be two
        // SEP tokens next to each other, which will only
        // match another hole at search time.  Note that
        // it will also match an empty-string token ... if
        // that's somehow a problem we can always map_obj HOLE
        // to a dedicated byte (and escape it in the
        // input).
        result->addEpsilon(state, t->dest);
      } else {
        result->addTransition(state, t->dest, t->min, t->max);
      }
    }
  }

  return result->finish();
}

shared_ptr<Automaton>
AnalyzingSuggester::convertAutomaton(shared_ptr<Automaton> a)
{
  return a;
}

shared_ptr<TokenStreamToAutomaton>
AnalyzingSuggester::getTokenStreamToAutomaton()
{
  shared_ptr<TokenStreamToAutomaton> *const tsta =
      make_shared<TokenStreamToAutomaton>();
  tsta->setPreservePositionIncrements(preservePositionIncrements);
  tsta->setFinalOffsetGapAsHole(true);
  return tsta;
}

AnalyzingSuggester::AnalyzingComparator::AnalyzingComparator(bool hasPayloads)
    : hasPayloads(hasPayloads)
{
}

int AnalyzingSuggester::AnalyzingComparator::compare(shared_ptr<BytesRef> a,
                                                     shared_ptr<BytesRef> b)
{

  // First by analyzed form:
  readerA->reset(a->bytes, a->offset, a->length);
  scratchA->length = readerA->readShort();
  scratchA->bytes = a->bytes;
  scratchA->offset = readerA->getPosition();

  readerB->reset(b->bytes, b->offset, b->length);
  scratchB->bytes = b->bytes;
  scratchB->length = readerB->readShort();
  scratchB->offset = readerB->getPosition();

  int cmp = scratchA->compareTo(scratchB);
  if (cmp != 0) {
    return cmp;
  }
  readerA->skipBytes(scratchA->length);
  readerB->skipBytes(scratchB->length);

  // Next by cost:
  int64_t aCost = readerA->readInt();
  int64_t bCost = readerB->readInt();
  assert(decodeWeight(aCost) >= 0);
  assert(decodeWeight(bCost) >= 0);
  if (aCost < bCost) {
    return -1;
  } else if (aCost > bCost) {
    return 1;
  }

  // Finally by surface form:
  if (hasPayloads) {
    scratchA->length = readerA->readShort();
    scratchB->length = readerB->readShort();
    scratchA->offset = readerA->getPosition();
    scratchB->offset = readerB->getPosition();
  } else {
    scratchA->offset = readerA->getPosition();
    scratchB->offset = readerB->getPosition();
    scratchA->length = readerA->length() - readerA->getPosition();
    scratchB->length = readerB->length() - readerB->getPosition();
  }
  assert(scratchA->isValid());
  assert(scratchB->isValid());

  return scratchA->compareTo(scratchB);
}

void AnalyzingSuggester::build(shared_ptr<InputIterator> iterator) throw(
    IOException)
{
  if (iterator->hasContexts()) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }

  hasPayloads = iterator->hasPayloads();

  shared_ptr<OfflineSorter> sorter =
      make_shared<OfflineSorter>(tempDir, tempFileNamePrefix,
                                 make_shared<AnalyzingComparator>(hasPayloads));

  shared_ptr<IndexOutput> tempInput = tempDir->createTempOutput(
      tempFileNamePrefix, L"input", IOContext::DEFAULT);

  shared_ptr<OfflineSorter::ByteSequencesWriter> writer =
      make_shared<OfflineSorter::ByteSequencesWriter>(tempInput);
  shared_ptr<OfflineSorter::ByteSequencesReader> reader = nullptr;
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();

  shared_ptr<TokenStreamToAutomaton> ts2a = getTokenStreamToAutomaton();

  wstring tempSortedFileName = L"";

  count = 0;
  std::deque<char> buffer(8);
  try {
    shared_ptr<ByteArrayDataOutput> output =
        make_shared<ByteArrayDataOutput>(buffer);

    for (BytesRef surfaceForm; (surfaceForm = iterator->next()) != nullptr;) {
      shared_ptr<LimitedFiniteStringsIterator> finiteStrings =
          make_shared<LimitedFiniteStringsIterator>(
              toAutomaton(surfaceForm, ts2a), maxGraphExpansions);

      for (IntsRef string; (string = finiteStrings->next()) != nullptr;
           count++) {
        Util::toBytesRef(string, scratch);

        // length of the analyzed text (FST input)
        if (scratch->length() > numeric_limits<short>::max() - 2) {
          throw invalid_argument(L"cannot handle analyzed forms > " +
                                 (numeric_limits<short>::max() - 2) +
                                 L" in length (got " +
                                 to_wstring(scratch->length()) + L")");
        }
        short analyzedLength = static_cast<short>(scratch->length());

        // compute the required length:
        // analyzed sequence + weight (4) + surface + analyzedLength (short)
        int requiredLength = analyzedLength + 4 + surfaceForm->length + 2;

        shared_ptr<BytesRef> payload;

        if (hasPayloads) {
          if (surfaceForm->length > (numeric_limits<short>::max() - 2)) {
            throw invalid_argument(L"cannot handle surface form > " +
                                   (numeric_limits<short>::max() - 2) +
                                   L" in length (got " + surfaceForm->length +
                                   L")");
          }
          payload = iterator->payload();
          // payload + surfaceLength (short)
          requiredLength += payload->length + 2;
        } else {
          payload.reset();
        }

        buffer = ArrayUtil::grow(buffer, requiredLength);

        output->reset(buffer);

        output->writeShort(analyzedLength);

        output->writeBytes(scratch->bytes(), 0, scratch->length());

        output->writeInt(encodeWeight(iterator->weight()));

        if (hasPayloads) {
          for (int i = 0; i < surfaceForm->length; i++) {
            if (surfaceForm::bytes[i] == PAYLOAD_SEP) {
              throw invalid_argument(
                  L"surface form cannot contain unit separator character "
                  L"U+001F; this character is reserved");
            }
          }
          output->writeShort(static_cast<short>(surfaceForm->length));
          output->writeBytes(surfaceForm::bytes, surfaceForm::offset,
                             surfaceForm->length);
          output->writeBytes(payload->bytes, payload->offset, payload->length);
        } else {
          output->writeBytes(surfaceForm::bytes, surfaceForm::offset,
                             surfaceForm->length);
        }

        assert((output->getPosition() == requiredLength,
                to_wstring(output->getPosition()) + L" vs " +
                    to_wstring(requiredLength)));
        writer->write(buffer, 0, output->getPosition());
      }

      maxAnalyzedPathsForOneInput =
          max(maxAnalyzedPathsForOneInput, finiteStrings->size());
    }
    CodecUtil::writeFooter(tempInput);
    delete writer;

    // Sort all input/output pairs (required by FST.Builder):
    tempSortedFileName = sorter->sort(tempInput->getName());

    // Free disk space:
    tempDir->deleteFile(tempInput->getName());

    reader = make_shared<OfflineSorter::ByteSequencesReader>(
        tempDir->openChecksumInput(tempSortedFileName, IOContext::READONCE),
        tempSortedFileName);

    shared_ptr<PairOutputs<int64_t, std::shared_ptr<BytesRef>>> outputs =
        make_shared<PairOutputs<int64_t, std::shared_ptr<BytesRef>>>(
            PositiveIntOutputs::getSingleton(),
            ByteSequenceOutputs::getSingleton());
    shared_ptr<Builder<Pair<int64_t, std::shared_ptr<BytesRef>>>> builder =
        make_shared<Builder<Pair<int64_t, std::shared_ptr<BytesRef>>>>(
            FST::INPUT_TYPE::BYTE1, outputs);

    // Build FST:
    shared_ptr<BytesRefBuilder> previousAnalyzed = nullptr;
    shared_ptr<BytesRefBuilder> analyzed = make_shared<BytesRefBuilder>();
    shared_ptr<BytesRef> surface = make_shared<BytesRef>();
    shared_ptr<IntsRefBuilder> scratchInts = make_shared<IntsRefBuilder>();
    shared_ptr<ByteArrayDataInput> input = make_shared<ByteArrayDataInput>();

    // Used to remove duplicate surface forms (but we
    // still index the hightest-weight one).  We clear
    // this when we see a new analyzed form, so it cannot
    // grow unbounded (at most 256 entries):
    shared_ptr<Set<std::shared_ptr<BytesRef>>> seenSurfaceForms =
        unordered_set<std::shared_ptr<BytesRef>>();

    int dedup = 0;
    while (true) {
      shared_ptr<BytesRef> bytes = reader->next();
      if (bytes == nullptr) {
        break;
      }
      input->reset(bytes->bytes, bytes->offset, bytes->length);
      short analyzedLength = input->readShort();
      analyzed->grow(analyzedLength + 2);
      input->readBytes(analyzed->bytes(), 0, analyzedLength);
      analyzed->setLength(analyzedLength);

      int64_t cost = input->readInt();

      surface->bytes = bytes->bytes;
      if (hasPayloads) {
        surface->length = input->readShort();
        surface->offset = input->getPosition();
      } else {
        surface->offset = input->getPosition();
        surface->length = bytes->length - surface->offset;
      }

      if (previousAnalyzed == nullptr) {
        previousAnalyzed = make_shared<BytesRefBuilder>();
        previousAnalyzed->copyBytes(analyzed->get());
        seenSurfaceForms->add(BytesRef::deepCopyOf(surface));
      } else if (analyzed->get().equals(previousAnalyzed->get())) {
        dedup++;
        if (dedup >= maxSurfaceFormsPerAnalyzedForm) {
          // More than maxSurfaceFormsPerAnalyzedForm
          // dups: skip the rest:
          continue;
        }
        if (seenSurfaceForms->contains(surface)) {
          continue;
        }
        seenSurfaceForms->add(BytesRef::deepCopyOf(surface));
      } else {
        dedup = 0;
        previousAnalyzed->copyBytes(analyzed);
        seenSurfaceForms->clear();
        seenSurfaceForms->add(BytesRef::deepCopyOf(surface));
      }

      // TODO: I think we can avoid the extra 2 bytes when
      // there is no dup (dedup==0), but we'd have to fix
      // the exactFirst logic ... which would be sort of
      // hairy because we'd need to special case the two
      // (dup/not dup)...

      // NOTE: must be byte 0 so we sort before whatever
      // is next
      analyzed->append(static_cast<char>(0));
      analyzed->append(static_cast<char>(dedup));

      Util::toIntsRef(analyzed->get(), scratchInts);
      // System.out.println("ADD: " + scratchInts + " -> " + cost + ": " +
      // surface.utf8ToString());
      if (!hasPayloads) {
        builder->add(scratchInts->get(),
                     outputs->newPair(cost, BytesRef::deepCopyOf(surface)));
      } else {
        int payloadOffset = input->getPosition() + surface->length;
        int payloadLength = bytes->length - payloadOffset;
        shared_ptr<BytesRef> br =
            make_shared<BytesRef>(surface->length + 1 + payloadLength);
        System::arraycopy(surface->bytes, surface->offset, br->bytes, 0,
                          surface->length);
        br->bytes[surface->length] = PAYLOAD_SEP;
        System::arraycopy(bytes->bytes, payloadOffset, br->bytes,
                          surface->length + 1, payloadLength);
        br->length = br->bytes.size();
        builder->add(scratchInts->get(), outputs->newPair(cost, br));
      }
    }
    fst = builder->finish();

    // Util.dotToFile(fst, "/tmp/suggest.dot");
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({reader, writer});
    IOUtils::deleteFilesIgnoringExceptions(
        tempDir, {tempInput->getName(), tempSortedFileName});
  }
}

bool AnalyzingSuggester::store(shared_ptr<DataOutput> output) 
{
  output->writeVLong(count);
  if (fst == nullptr) {
    return false;
  }

  fst->save(output);
  output->writeVInt(maxAnalyzedPathsForOneInput);
  output->writeByte(static_cast<char>(hasPayloads ? 1 : 0));
  return true;
}

bool AnalyzingSuggester::load(shared_ptr<DataInput> input) 
{
  count = input->readVLong();
  this->fst = make_shared<FST<Pair<int64_t, BytesRef>>>(
      input, make_shared<PairOutputs<>>(PositiveIntOutputs::getSingleton(),
                                        ByteSequenceOutputs::getSingleton()));
  maxAnalyzedPathsForOneInput = input->readVInt();
  hasPayloads = input->readByte() == 1;
  return true;
}

shared_ptr<Lookup::LookupResult>
AnalyzingSuggester::getLookupResult(optional<int64_t> &output1,
                                    shared_ptr<BytesRef> output2,
                                    shared_ptr<CharsRefBuilder> spare)
{
  shared_ptr<Lookup::LookupResult> result;
  if (hasPayloads) {
    int sepIndex = -1;
    for (int i = 0; i < output2->length; i++) {
      if (output2->bytes[output2->offset + i] == PAYLOAD_SEP) {
        sepIndex = i;
        break;
      }
    }
    assert(sepIndex != -1);
    spare->grow(sepIndex);
    constexpr int payloadLen = output2->length - sepIndex - 1;
    spare->copyUTF8Bytes(output2->bytes, output2->offset, sepIndex);
    shared_ptr<BytesRef> payload = make_shared<BytesRef>(payloadLen);
    System::arraycopy(output2->bytes, sepIndex + 1, payload->bytes, 0,
                      payloadLen);
    payload->length = payloadLen;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    result = make_shared<Lookup::LookupResult>(spare->toString(),
                                               decodeWeight(output1), payload);
  } else {
    spare->grow(output2->length);
    spare->copyUTF8Bytes(output2);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    result = make_shared<Lookup::LookupResult>(spare->toString(),
                                               decodeWeight(output1));
  }

  return result;
}

bool AnalyzingSuggester::sameSurfaceForm(shared_ptr<BytesRef> key,
                                         shared_ptr<BytesRef> output2)
{
  if (hasPayloads) {
    // output2 has at least PAYLOAD_SEP byte:
    if (key->length >= output2->length) {
      return false;
    }
    for (int i = 0; i < key->length; i++) {
      if (key->bytes[key->offset + i] != output2->bytes[output2->offset + i]) {
        return false;
      }
    }
    return output2->bytes[output2->offset + key->length] == PAYLOAD_SEP;
  } else {
    return key->bytesEquals(output2);
  }
}

deque<std::shared_ptr<Lookup::LookupResult>>
AnalyzingSuggester::lookup(shared_ptr<std::wstring> key,
                           shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                           bool onlyMorePopular, int num)
{
  assert(num > 0);

  if (onlyMorePopular) {
    throw invalid_argument(
        L"this suggester only works with onlyMorePopular=false");
  }
  if (contexts != nullptr) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  if (fst == nullptr) {
    return Collections::emptyList();
  }

  // System.out.println("lookup key=" + key + " num=" + num);
  for (int i = 0; i < key->length(); i++) {
    if (key->charAt(i) == 0x1E) {
      throw invalid_argument(L"lookup key cannot contain HOLE character "
                             L"U+001E; this character is reserved");
    }
    if (key->charAt(i) == 0x1F) {
      throw invalid_argument(L"lookup key cannot contain unit separator "
                             L"character U+001F; this character is reserved");
    }
  }
  shared_ptr<BytesRef> *const utf8Key = make_shared<BytesRef>(key);
  try {
    shared_ptr<Automaton> lookupAutomaton = toLookupAutomaton(key);

    shared_ptr<CharsRefBuilder> *const spare = make_shared<CharsRefBuilder>();

    // System.out.println("  now intersect exactFirst=" + exactFirst);

    // Intersect automaton w/ suggest wFST and get all
    // prefix starting nodes & their outputs:
    // final PathIntersector intersector = getPathIntersector(lookupAutomaton,
    // fst);

    // System.out.println("  prefixPaths: " + prefixPaths.size());

    shared_ptr<BytesReader> bytesReader = fst->getBytesReader();

    shared_ptr<FST::Arc<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        scratchArc =
            make_shared<FST::Arc<Pair<int64_t, std::shared_ptr<BytesRef>>>>();

    const deque<std::shared_ptr<Lookup::LookupResult>> results =
        deque<std::shared_ptr<Lookup::LookupResult>>();

    deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        prefixPaths = FSTUtil::intersectPrefixPaths(
            convertAutomaton(lookupAutomaton), fst);

    if (exactFirst) {

      int count = 0;
      for (auto path : prefixPaths) {
        if (fst->findTargetArc(END_BYTE, path->fstNode, scratchArc,
                               bytesReader) != nullptr) {
          // This node has END_BYTE arc leaving, meaning it's an
          // "exact" match:
          count++;
        }
      }

      // Searcher just to find the single exact only
      // match, if present:
      shared_ptr<Util::TopNSearcher<Pair<int64_t, std::shared_ptr<BytesRef>>>>
          searcher;
      searcher = make_shared<Util::TopNSearcher<Pair<int64_t, BytesRef>>>(
          fst, count * maxSurfaceFormsPerAnalyzedForm,
          count * maxSurfaceFormsPerAnalyzedForm, weightComparator);

      // NOTE: we could almost get away with only using
      // the first start node.  The only catch is if
      // maxSurfaceFormsPerAnalyzedForm had kicked in and
      // pruned our exact match from one of these nodes
      // ...:
      for (auto path : prefixPaths) {
        if (fst->findTargetArc(END_BYTE, path->fstNode, scratchArc,
                               bytesReader) != nullptr) {
          // This node has END_BYTE arc leaving, meaning it's an
          // "exact" match:
          searcher->addStartPaths(
              scratchArc, fst->outputs->add(path->output, scratchArc->output),
              false, path->input);
        }
      }

      shared_ptr<TopResults<Pair<int64_t, std::shared_ptr<BytesRef>>>>
          completions = searcher->search();
      assert(completions->isComplete);

      // NOTE: this is rather inefficient: we enumerate
      // every matching "exactly the same analyzed form"
      // path, and then do linear scan to see if one of
      // these exactly matches the input.  It should be
      // possible (though hairy) to do something similar
      // to getByOutput, since the surface form is encoded
      // into the FST output, so we more efficiently hone
      // in on the exact surface-form match.  Still, I
      // suspect very little time is spent in this linear
      // seach: it's bounded by how many prefix start
      // nodes we have and the
      // maxSurfaceFormsPerAnalyzedForm:
      for (auto completion : completions) {
        shared_ptr<BytesRef> output2 = completion->output->output2;
        if (sameSurfaceForm(utf8Key, output2)) {
          results.push_back(
              getLookupResult(completion->output->output1, output2, spare));
          break;
        }
      }

      if (results.size() == num) {
        // That was quick:
        return results;
      }
    }

    shared_ptr<Util::TopNSearcher<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        searcher;
    searcher = make_shared<TopNSearcherAnonymousInnerClass>(
        shared_from_this(), fst, num - results.size(),
        num * maxAnalyzedPathsForOneInput, weightComparator, utf8Key, results);

    prefixPaths = getFullPrefixPaths(prefixPaths, lookupAutomaton, fst);

    for (auto path : prefixPaths) {
      searcher->addStartPaths(path->fstNode, path->output, true, path->input);
    }

    shared_ptr<TopResults<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        completions = searcher->search();
    assert(completions->isComplete);

    for (auto completion : completions) {

      shared_ptr<Lookup::LookupResult> result = getLookupResult(
          completion->output->output1, completion->output->output2, spare);

      // TODO: for fuzzy case would be nice to return
      // how many edits were required

      // System.out.println("    result=" + result);
      results.push_back(result);

      if (results.size() == num) {
        // In the exactFirst=true case the search may
        // produce one extra path
        break;
      }
    }

    return results;
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

AnalyzingSuggester::TopNSearcherAnonymousInnerClass::
    TopNSearcherAnonymousInnerClass(
        shared_ptr<AnalyzingSuggester> outerInstance,
        shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst,
        shared_ptr<UnknownType> size, int num,
        shared_ptr<UnknownType> weightComparator, shared_ptr<BytesRef> utf8Key,
        deque<std::shared_ptr<Lookup::LookupResult>> &results)
    : org::apache::lucene::util::fst::Util::TopNSearcher<
          org::apache::lucene::util::fst::PairOutputs::Pair<
              long, org::apache::lucene::util::BytesRef>>(
          fst, size, num * maxAnalyzedPathsForOneInput, weightComparator)
{
  this->outerInstance = outerInstance;
  this->utf8Key = utf8Key;
  this->results = results;
  seen = unordered_set<>();
}

bool AnalyzingSuggester::TopNSearcherAnonymousInnerClass::acceptResult(
    shared_ptr<IntsRef> input,
    shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> output)
{

  // Dedup: when the input analyzes to a graph we
  // can get duplicate surface forms:
  if (seen->contains(output->output2)) {
    return false;
  }
  seen->add(output->output2);

  if (!outerInstance->exactFirst) {
    return true;
  } else {
    // In exactFirst mode, don't accept any paths
    // matching the surface form since that will
    // create duplicate results:
    if (outerInstance->sameSurfaceForm(utf8Key, output->output2)) {
      // We found exact match, which means we should
      // have already found it in the first search:
      assert(results.size() == 1);
      return false;
    } else {
      return true;
    }
  }
}

int64_t AnalyzingSuggester::getCount() { return count; }

deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
AnalyzingSuggester::getFullPrefixPaths(
    deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        &prefixPaths,
    shared_ptr<Automaton> lookupAutomaton,
    shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        fst) 
{
  return prefixPaths;
}

shared_ptr<Automaton> AnalyzingSuggester::toAutomaton(
    shared_ptr<BytesRef> surfaceForm,
    shared_ptr<TokenStreamToAutomaton> ts2a) 
{
  // Analyze surface form:
  shared_ptr<Automaton> automaton;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // indexAnalyzer.tokenStream("", surfaceForm.utf8ToString()))
  {
    org::apache::lucene::analysis::TokenStream ts =
        indexAnalyzer->tokenStream(L"", surfaceForm->utf8ToString());

    // Create corresponding automaton: labels are bytes
    // from each analyzed token, with byte 0 used as
    // separator between tokens:
    automaton = ts2a->toAutomaton(ts);
  }

  automaton = replaceSep(automaton);
  automaton = convertAutomaton(automaton);

  // TODO: LUCENE-5660 re-enable this once we disallow massive suggestion
  // strings assert SpecialOperations.isFinite(automaton);

  // Get all paths from the automaton (there can be
  // more than one path, eg if the analyzer created a
  // graph using SynFilter or WDF):
  return automaton;
}

shared_ptr<Automaton> AnalyzingSuggester::toLookupAutomaton(
    shared_ptr<std::wstring> key) 
{
  // TODO: is there a Reader from a std::wstring?
  // Turn tokenstream into automaton:
  shared_ptr<Automaton> automaton = nullptr;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // queryAnalyzer.tokenStream("", key.toString())) C++ TODO: There is no native
  // C++ equivalent to 'toString':
  {
    org::apache::lucene::analysis::TokenStream ts =
        queryAnalyzer->tokenStream(L"", key->toString());
    automaton = getTokenStreamToAutomaton()->toAutomaton(ts);
  }

  automaton = replaceSep(automaton);

  // TODO: we can optimize this somewhat by determinizing
  // while we convert
  automaton = Operations::determinize(
      automaton, Operations::DEFAULT_MAX_DETERMINIZED_STATES);
  return automaton;
}

any AnalyzingSuggester::get(shared_ptr<std::wstring> key)
{
  throw make_shared<UnsupportedOperationException>();
}

int AnalyzingSuggester::decodeWeight(int64_t encoded)
{
  return static_cast<int>(numeric_limits<int>::max() - encoded);
}

int AnalyzingSuggester::encodeWeight(int64_t value)
{
  if (value < 0 || value > numeric_limits<int>::max()) {
    throw make_shared<UnsupportedOperationException>(L"cannot encode value: " +
                                                     to_wstring(value));
  }
  return numeric_limits<int>::max() - static_cast<int>(value);
}

const shared_ptr<
    java::util::Comparator<org::apache::lucene::util::fst::PairOutputs::Pair<
        int64_t, std::shared_ptr<org::apache::lucene::util::BytesRef>>>>
    AnalyzingSuggester::weightComparator =
        make_shared<ComparatorAnonymousInnerClass>();

AnalyzingSuggester::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int AnalyzingSuggester::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> left,
    shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> right)
{
  return left->output1.compareTo(right->output1);
}
} // namespace org::apache::lucene::search::suggest::analyzing