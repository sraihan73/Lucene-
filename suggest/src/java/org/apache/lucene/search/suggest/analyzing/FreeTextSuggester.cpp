using namespace std;

#include "FreeTextSuggester.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ShingleFilter = org::apache::lucene::analysis::shingle::ShingleFilter;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MultiFields = org::apache::lucene::index::MultiFields;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;
using Outputs = org::apache::lucene::util::fst::Outputs;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
using org::apache::lucene::util::fst::Util::Result;
using org::apache::lucene::util::fst::Util::TopResults;
const wstring FreeTextSuggester::CODEC_NAME = L"freetextsuggest";

FreeTextSuggester::FreeTextSuggester(shared_ptr<Analyzer> analyzer)
    : FreeTextSuggester(analyzer, analyzer, DEFAULT_GRAMS)
{
}

FreeTextSuggester::FreeTextSuggester(shared_ptr<Analyzer> indexAnalyzer,
                                     shared_ptr<Analyzer> queryAnalyzer)
    : FreeTextSuggester(indexAnalyzer, queryAnalyzer, DEFAULT_GRAMS)
{
}

FreeTextSuggester::FreeTextSuggester(shared_ptr<Analyzer> indexAnalyzer,
                                     shared_ptr<Analyzer> queryAnalyzer,
                                     int grams)
    : FreeTextSuggester(indexAnalyzer, queryAnalyzer, grams, DEFAULT_SEPARATOR)
{
}

FreeTextSuggester::FreeTextSuggester(shared_ptr<Analyzer> indexAnalyzer,
                                     shared_ptr<Analyzer> queryAnalyzer,
                                     int grams, char separator)
    : indexAnalyzer(addShingles(indexAnalyzer)),
      queryAnalyzer(addShingles(queryAnalyzer)), grams(grams),
      separator(separator)
{
  if (grams < 1) {
    throw invalid_argument(L"grams must be >= 1");
  }
  if ((separator & 0x80) != 0) {
    throw invalid_argument(L"separator must be simple ascii character");
  }
}

int64_t FreeTextSuggester::ramBytesUsed()
{
  if (fst == nullptr) {
    return 0;
  }
  return fst->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FreeTextSuggester::getChildResources()
{
  if (fst == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"fst", fst));
  }
}

shared_ptr<Analyzer> FreeTextSuggester::addShingles(shared_ptr<Analyzer> other)
{
  if (grams == 1) {
    return other;
  } else {
    // TODO: use ShingleAnalyzerWrapper?
    // Tack on ShingleFilter to the end, to generate token ngrams:
    return make_shared<AnalyzerWrapperAnonymousInnerClass>(
        shared_from_this(), other->getReuseStrategy(), other);
  }
}

FreeTextSuggester::AnalyzerWrapperAnonymousInnerClass::
    AnalyzerWrapperAnonymousInnerClass(
        shared_ptr<FreeTextSuggester> outerInstance,
        shared_ptr<Analyzer::ReuseStrategy> getReuseStrategy,
        shared_ptr<Analyzer> other)
    : org::apache::lucene::analysis::AnalyzerWrapper(getReuseStrategy)
{
  this->outerInstance = outerInstance;
  this->other = other;
}

shared_ptr<Analyzer>
FreeTextSuggester::AnalyzerWrapperAnonymousInnerClass::getWrappedAnalyzer(
    const wstring &fieldName)
{
  return other;
}

shared_ptr<Analyzer::TokenStreamComponents>
FreeTextSuggester::AnalyzerWrapperAnonymousInnerClass::wrapComponents(
    const wstring &fieldName,
    shared_ptr<Analyzer::TokenStreamComponents> components)
{
  shared_ptr<ShingleFilter> shingles = make_shared<ShingleFilter>(
      components->getTokenStream(), 2, outerInstance->grams);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shingles->setTokenSeparator(
      Character::toString(static_cast<wchar_t>(outerInstance->separator)));
  return make_shared<Analyzer::TokenStreamComponents>(
      components->getTokenizer(), shingles);
}

void FreeTextSuggester::build(shared_ptr<InputIterator> iterator) throw(
    IOException)
{
  build(iterator, IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB);
}

void FreeTextSuggester::build(shared_ptr<InputIterator> iterator,
                              double ramBufferSizeMB) 
{
  if (iterator->hasPayloads()) {
    throw invalid_argument(L"this suggester doesn't support payloads");
  }
  if (iterator->hasContexts()) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }

  wstring prefix = getClass().getSimpleName();
  shared_ptr<Path> tempIndexPath =
      Files::createTempDirectory(prefix + L".index.");

  shared_ptr<Directory> dir = FSDirectory::open(tempIndexPath);

  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(indexAnalyzer);
  iwc->setOpenMode(IndexWriterConfig::OpenMode::CREATE);
  iwc->setRAMBufferSizeMB(ramBufferSizeMB);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  // TODO: if only we had IndexOptions.TERMS_ONLY...
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  ft->setOmitNorms(true);
  ft->freeze();

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = make_shared<Field>(L"body", L"", ft);
  doc->push_back(field);

  totTokens = 0;
  shared_ptr<IndexReader> reader = nullptr;

  bool success = false;
  count = 0;
  try {
    while (true) {
      shared_ptr<BytesRef> surfaceForm = iterator->next();
      if (surfaceForm == nullptr) {
        break;
      }
      field->setStringValue(surfaceForm->utf8ToString());
      writer->addDocument(doc);
      count++;
    }
    reader = DirectoryReader::open(writer);

    shared_ptr<Terms> terms = MultiFields::getTerms(reader, L"body");
    if (terms == nullptr) {
      throw invalid_argument(L"need at least one suggestion");
    }

    // Move all ngrams into an FST:
    shared_ptr<TermsEnum> termsEnum = terms->begin();

    shared_ptr<Outputs<int64_t>> outputs = PositiveIntOutputs::getSingleton();
    shared_ptr<Builder<int64_t>> builder =
        make_shared<Builder<int64_t>>(FST::INPUT_TYPE::BYTE1, outputs);

    shared_ptr<IntsRefBuilder> scratchInts = make_shared<IntsRefBuilder>();
    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      int ngramCount = countGrams(term);
      if (ngramCount > grams) {
        throw invalid_argument(
            L"tokens must not contain separator byte; got token=" + term +
            L" but gramCount=" + to_wstring(ngramCount) +
            L", which is greater than expected max ngram size=" +
            to_wstring(grams));
      }
      if (ngramCount == 1) {
        totTokens += termsEnum->totalTermFreq();
      }

      builder->add(Util::toIntsRef(term, scratchInts),
                   encodeWeight(termsEnum->totalTermFreq()));
    }

    fst = builder->finish();
    if (fst == nullptr) {
      throw invalid_argument(L"need at least one suggestion");
    }
    // System.out.println("FST: " + fst.getNodeCount() + " nodes");

    /*
    PrintWriter pw = new PrintWriter("/x/tmp/out.dot");
    Util.toDot(fst, pw, true, true);
    pw.close();
    */

    // Writer was only temporary, to count up bigrams,
    // which we transferred to the FST, so now we
    // rollback:
    writer->rollback();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    try {
      if (success) {
        IOUtils::close({reader, dir});
      } else {
        IOUtils::closeWhileHandlingException({reader, writer, dir});
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::rm({tempIndexPath});
    }
  }
}

bool FreeTextSuggester::store(shared_ptr<DataOutput> output) 
{
  CodecUtil::writeHeader(output, CODEC_NAME, VERSION_CURRENT);
  output->writeVLong(count);
  output->writeByte(separator);
  output->writeVInt(grams);
  output->writeVLong(totTokens);
  fst->save(output);
  return true;
}

bool FreeTextSuggester::load(shared_ptr<DataInput> input) 
{
  CodecUtil::checkHeader(input, CODEC_NAME, VERSION_START, VERSION_START);
  count = input->readVLong();
  char separatorOrig = input->readByte();
  if (separatorOrig != separator) {
    throw make_shared<IllegalStateException>(
        L"separator=" + to_wstring(separator) +
        L" is incorrect: original model was built with separator=" +
        to_wstring(separatorOrig));
  }
  int gramsOrig = input->readVInt();
  if (gramsOrig != grams) {
    throw make_shared<IllegalStateException>(
        L"grams=" + to_wstring(grams) +
        L" is incorrect: original model was built with grams=" +
        to_wstring(gramsOrig));
  }
  totTokens = input->readVLong();

  fst = make_shared<FST<int64_t>>(input, PositiveIntOutputs::getSingleton());

  return true;
}

deque<std::shared_ptr<Lookup::LookupResult>>
FreeTextSuggester::lookup(shared_ptr<std::wstring> key, bool onlyMorePopular,
                          int num)
{
  return lookup(key, nullptr, onlyMorePopular, num);
}

deque<std::shared_ptr<Lookup::LookupResult>>
FreeTextSuggester::lookup(shared_ptr<std::wstring> key, int num)
{
  return lookup(key, nullptr, true, num);
}

deque<std::shared_ptr<Lookup::LookupResult>>
FreeTextSuggester::lookup(shared_ptr<std::wstring> key,
                          shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                          bool onlyMorePopular, int num)
{
  try {
    return lookup(key, contexts, num);
  } catch (const IOException &ioe) {
    // bogus:
    throw runtime_error(ioe);
  }
}

int64_t FreeTextSuggester::getCount() { return count; }

int FreeTextSuggester::countGrams(shared_ptr<BytesRef> token)
{
  int count = 1;
  for (int i = 0; i < token->length; i++) {
    if (token->bytes[token->offset + i] == separator) {
      count++;
    }
  }

  return count;
}

deque<std::shared_ptr<Lookup::LookupResult>>
FreeTextSuggester::lookup(shared_ptr<std::wstring> key,
                          shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                          int num) 
{
  if (contexts != nullptr) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  if (fst == nullptr) {
    throw make_shared<IllegalStateException>(
        L"Lookup not supported at this time");
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // queryAnalyzer.tokenStream("", key.toString())) C++ TODO: There is no native
  // C++ equivalent to 'toString':
  {
    org::apache::lucene::analysis::TokenStream ts =
        queryAnalyzer->tokenStream(L"", key->toString());
    shared_ptr<TermToBytesRefAttribute> termBytesAtt =
        ts->addAttribute(TermToBytesRefAttribute::typeid);
    shared_ptr<OffsetAttribute> offsetAtt =
        ts->addAttribute(OffsetAttribute::typeid);
    shared_ptr<PositionLengthAttribute> posLenAtt =
        ts->addAttribute(PositionLengthAttribute::typeid);
    shared_ptr<PositionIncrementAttribute> posIncAtt =
        ts->addAttribute(PositionIncrementAttribute::typeid);
    ts->reset();

    std::deque<std::shared_ptr<BytesRefBuilder>> lastTokens(grams);
    // System.out.println("lookup: key='" + key + "'");

    // Run full analysis, but save only the
    // last 1gram, last 2gram, etc.:
    int maxEndOffset = -1;
    bool sawRealToken = false;
    while (ts->incrementToken()) {
      shared_ptr<BytesRef> tokenBytes = termBytesAtt->getBytesRef();
      sawRealToken |= tokenBytes->length > 0;
      // TODO: this is somewhat iffy; today, ShingleFilter
      // sets posLen to the gram count; maybe we should make
      // a separate dedicated att for this?
      int gramCount = posLenAtt->getPositionLength();

      assert(gramCount <= grams);

      // Safety: make sure the recalculated count "agrees":
      if (countGrams(tokenBytes) != gramCount) {
        throw invalid_argument(
            L"tokens must not contain separator byte; got token=" + tokenBytes +
            L" but gramCount=" + to_wstring(gramCount) +
            L" does not match recalculated count=" +
            to_wstring(countGrams(tokenBytes)));
      }
      maxEndOffset = max(maxEndOffset, offsetAtt->endOffset());
      shared_ptr<BytesRefBuilder> b = make_shared<BytesRefBuilder>();
      b->append(tokenBytes);
      lastTokens[gramCount - 1] = b;
    }
    ts->end();

    if (!sawRealToken) {
      throw invalid_argument(L"no tokens produced by analyzer, or the only "
                             L"tokens were empty strings");
    }

    // Carefully fill last tokens with _ tokens;
    // ShingleFilter appraently won't emit "only hole"
    // tokens:
    int endPosInc = posIncAtt->getPositionIncrement();

    // Note this will also be true if input is the empty
    // string (in which case we saw no tokens and
    // maxEndOffset is still -1), which in fact works out OK
    // because we fill the unigram with an empty BytesRef
    // below:
    bool lastTokenEnded =
        offsetAtt->endOffset() > maxEndOffset || endPosInc > 0;
    // System.out.println("maxEndOffset=" + maxEndOffset + " vs " +
    // offsetAtt.endOffset());

    if (lastTokenEnded) {
      // System.out.println("  lastTokenEnded");
      // If user hit space after the last token, then
      // "upgrade" all tokens.  This way "foo " will suggest
      // all bigrams starting w/ foo, and not any unigrams
      // starting with "foo":
      for (int i = grams - 1; i > 0; i--) {
        shared_ptr<BytesRefBuilder> token = lastTokens[i - 1];
        if (token == nullptr) {
          continue;
        }
        token->append(separator);
        lastTokens[i] = token;
      }
      lastTokens[0] = make_shared<BytesRefBuilder>();
    }

    shared_ptr<FST::Arc<int64_t>> arc = make_shared<FST::Arc<int64_t>>();

    shared_ptr<FST::BytesReader> bytesReader = fst->getBytesReader();

    // Try highest order models first, and if they return
    // results, return that; else, fallback:
    double backoff = 1.0;

    deque<std::shared_ptr<Lookup::LookupResult>> results =
        deque<std::shared_ptr<Lookup::LookupResult>>(num);

    // We only add a given suffix once, from the highest
    // order model that saw it; for subsequent lower order
    // models we skip it:
    shared_ptr<Set<std::shared_ptr<BytesRef>>> *const seen =
        unordered_set<std::shared_ptr<BytesRef>>();

    for (int gram = grams - 1; gram >= 0; gram--) {
      shared_ptr<BytesRefBuilder> token = lastTokens[gram];
      // Don't make unigram predictions from empty string:
      if (token == nullptr || (token->length() == 0 && key->length() > 0)) {
        // Input didn't have enough tokens:
        // System.out.println("  gram=" + gram + ": skip: not enough input");
        continue;
      }

      if (endPosInc > 0 && gram <= endPosInc) {
        // Skip hole-only predictions; in theory we
        // shouldn't have to do this, but we'd need to fix
        // ShingleFilter to produce only-hole tokens:
        // System.out.println("  break: only holes now");
        break;
      }

      // System.out.println("try " + (gram+1) + " gram token=" +
      // token.utf8ToString());

      // TODO: we could add fuzziness here
      // match the prefix portion exactly
      // Pair<Long,BytesRef> prefixOutput = null;
      optional<int64_t> prefixOutput = nullopt;
      try {
        prefixOutput = lookupPrefix(fst, bytesReader, token->get(), arc);
      } catch (const IOException &bogus) {
        throw runtime_error(bogus);
      }
      // System.out.println("  prefixOutput=" + prefixOutput);

      if (!prefixOutput) {
        // This model never saw this prefix, e.g. the
        // trigram model never saw context "purple mushroom"
        backoff *= ALPHA;
        continue;
      }

      // TODO: we could do this division at build time, and
      // bake it into the FST?

      // Denominator for computing scores from current
      // model's predictions:
      int64_t contextCount = totTokens;

      shared_ptr<BytesRef> lastTokenFragment = nullptr;

      for (int i = token->length() - 1; i >= 0; i--) {
        if (token->byteAt(i) == separator) {
          shared_ptr<BytesRef> context =
              make_shared<BytesRef>(token->bytes(), 0, i);
          optional<int64_t> output = Util::get(
              fst, Util::toIntsRef(context, make_shared<IntsRefBuilder>()));
          assert(output);
          contextCount = decodeWeight(output);
          lastTokenFragment = make_shared<BytesRef>(token->bytes(), i + 1,
                                                    token->length() - i - 1);
          break;
        }
      }

      shared_ptr<BytesRefBuilder> *const finalLastToken =
          make_shared<BytesRefBuilder>();
      if (lastTokenFragment == nullptr) {
        finalLastToken->copyBytes(token->get());
      } else {
        finalLastToken->copyBytes(lastTokenFragment);
      }

      shared_ptr<CharsRefBuilder> spare = make_shared<CharsRefBuilder>();

      // complete top-N
      shared_ptr<Util::TopResults<int64_t>> completions;
      try {

        // Because we store multiple models in one FST
        // (1gram, 2gram, 3gram), we must restrict the
        // search so that it only considers the current
        // model.  For highest order model, this is not
        // necessary since all completions in the FST
        // must be from this model, but for lower order
        // models we have to filter out the higher order
        // ones:

        // Must do num+seen.size() for queue depth because we may
        // reject up to seen.size() paths in acceptResult():
        shared_ptr<Util::TopNSearcher<int64_t>> searcher =
            make_shared<TopNSearcherAnonymousInnerClass>(
                shared_from_this(), fst, num, num + seen->size(),
                weightComparator, seen, finalLastToken);

        // since this search is initialized with a single start node
        // it is okay to start with an empty input path here
        searcher->addStartPaths(arc, prefixOutput, true,
                                make_shared<IntsRefBuilder>());

        completions = searcher->search();
        assert(completions->isComplete);
      } catch (const IOException &bogus) {
        throw runtime_error(bogus);
      }

      int prefixLength = token->length();

      shared_ptr<BytesRefBuilder> suffix = make_shared<BytesRefBuilder>();
      // System.out.println("    " + completions.length + " completions");

      for (auto completion : completions) {
        token->setLength(prefixLength);
        // append suffix
        Util::toBytesRef(completion->input, suffix);
        token->append(suffix);

        // System.out.println("    completion " + token.utf8ToString());

        // Skip this path if a higher-order model already
        // saw/predicted its last token:
        shared_ptr<BytesRef> lastToken = token->get();
        for (int i = token->length() - 1; i >= 0; i--) {
          if (token->byteAt(i) == separator) {
            assert(token->length() - i - 1 > 0);
            lastToken = make_shared<BytesRef>(token->bytes(), i + 1,
                                              token->length() - i - 1);
            break;
          }
        }
        if (seen->contains(lastToken)) {
          // System.out.println("      skip dup " + lastToken.utf8ToString());
          goto nextCompletionContinue;
        }
        seen->add(BytesRef::deepCopyOf(lastToken));
        spare->copyUTF8Bytes(token->get());
        // C++ TODO: There is no native C++ equivalent to 'toString':
        shared_ptr<Lookup::LookupResult> result =
            make_shared<Lookup::LookupResult>(
                spare->toString(),
                static_cast<int64_t>(
                    numeric_limits<int64_t>::max() * backoff *
                    (static_cast<double>(decodeWeight(completion->output))) /
                    contextCount));
        results.push_back(result);
        assert(results.size() == seen->size());
      // System.out.println("  add result=" + result);
      nextCompletionContinue:;
      }
    nextCompletionBreak:
      backoff *= ALPHA;
    }

    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(results, new
    // java.util.Comparator<LookupResult>()
    sort(results.begin(), results.end(),
         make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

    if (results.size() > num) {
      results.subList(num, results.size())->clear();
    }

    return results;
  }
}

FreeTextSuggester::TopNSearcherAnonymousInnerClass::
    TopNSearcherAnonymousInnerClass(
        shared_ptr<FreeTextSuggester> outerInstance,
        shared_ptr<FST<int64_t>> fst, int num, shared_ptr<UnknownType> size,
        shared_ptr<UnknownType> weightComparator,
        shared_ptr<Set<std::shared_ptr<BytesRef>>> seen,
        shared_ptr<BytesRefBuilder> finalLastToken)
    : org::apache::lucene::util::fst::Util::TopNSearcher<long>(fst, num, size,
                                                               weightComparator)
{
  this->outerInstance = outerInstance;
  this->seen = seen;
  this->finalLastToken = finalLastToken;
  scratchBytes = make_shared<BytesRefBuilder>();
}

void FreeTextSuggester::TopNSearcherAnonymousInnerClass::addIfCompetitive(
    shared_ptr<Util::FSTPath<int64_t>> path)
{
  if (path->arc->label != outerInstance->separator) {
    // System.out.println("    keep path: " + Util.toBytesRef(path.input, new
    // BytesRef()).utf8ToString() + "; " + path + "; arc=" + path.arc);
    outerInstance->super->addIfCompetitive(path);
  } else {
    // System.out.println("    prevent path: " + Util.toBytesRef(path.input, new
    // BytesRef()).utf8ToString() + "; " + path + "; arc=" + path.arc);
  }
}

bool FreeTextSuggester::TopNSearcherAnonymousInnerClass::acceptResult(
    shared_ptr<IntsRef> input, optional<int64_t> &output)
{
  Util::toBytesRef(input, scratchBytes);
  finalLastToken->grow(finalLastToken->length() + scratchBytes->length());
  int lenSav = finalLastToken->length();
  finalLastToken->append(scratchBytes);
  // System.out.println("    accept? input='" + scratchBytes.utf8ToString() +
  // "'; lastToken='" + finalLastToken.utf8ToString() + "'; return " +
  // (seen.contains(finalLastToken) == false));
  bool ret = seen->contains(finalLastToken->get()) == false;

  finalLastToken->setLength(lenSav);
  return ret;
}

FreeTextSuggester::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<FreeTextSuggester> outerInstance)
{
  this->outerInstance = outerInstance;
}

int FreeTextSuggester::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Lookup::LookupResult> a, shared_ptr<Lookup::LookupResult> b)
{
  if (a->value > b->value) {
    return -1;
  } else if (a->value < b->value) {
    return 1;
  } else {
    // Tie break by UTF16 sort order:
    return (static_cast<wstring>(a->key))
        ->compare(static_cast<wstring>(b->key));
  }
}

int64_t FreeTextSuggester::encodeWeight(int64_t ngramCount)
{
  return numeric_limits<int64_t>::max() - ngramCount;
}

int64_t FreeTextSuggester::decodeWeight(optional<int64_t> &output)
{
  assert(output);
  return static_cast<int>(numeric_limits<int64_t>::max() - output);
}

optional<int64_t> FreeTextSuggester::lookupPrefix(
    shared_ptr<FST<int64_t>> fst, shared_ptr<FST::BytesReader> bytesReader,
    shared_ptr<BytesRef> scratch,
    shared_ptr<FST::Arc<int64_t>> arc) 
{

  optional<int64_t> output = fst->outputs->getNoOutput();

  fst->getFirstArc(arc);

  std::deque<char> bytes = scratch->bytes;
  int pos = scratch->offset;
  int end = pos + scratch->length;
  while (pos < end) {
    if (fst->findTargetArc(bytes[pos++] & 0xff, arc, arc, bytesReader) ==
        nullptr) {
      return nullopt;
    } else {
      output = fst->outputs->add(output, arc->output);
    }
  }

  return output;
}

const shared_ptr<java::util::Comparator<int64_t>>
    FreeTextSuggester::weightComparator =
        make_shared<ComparatorAnonymousInnerClass2>();

FreeTextSuggester::ComparatorAnonymousInnerClass2::
    ComparatorAnonymousInnerClass2()
{
}

int FreeTextSuggester::ComparatorAnonymousInnerClass2::compare(
    optional<int64_t> &left, optional<int64_t> &right)
{
  return left.compareTo(right);
}

any FreeTextSuggester::get(shared_ptr<std::wstring> key)
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::search::suggest::analyzing