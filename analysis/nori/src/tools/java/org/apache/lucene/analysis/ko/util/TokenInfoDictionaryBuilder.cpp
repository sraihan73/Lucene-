using namespace std;

#include "TokenInfoDictionaryBuilder.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ko/util/CSVUtil.h"
#include "TokenInfoDictionaryWriter.h"

namespace org::apache::lucene::analysis::ko::util
{
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using com::ibm::icu::text::Normalizer2;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

TokenInfoDictionaryBuilder::TokenInfoDictionaryBuilder(const wstring &encoding,
                                                       bool normalizeEntries)
{
  this->encoding = encoding;
  this->normalizeEntries = normalizeEntries;
  this->normalizer = normalizeEntries
                         ? Normalizer2::getInstance(nullptr, L"nfkc",
                                                    Normalizer2::Mode::COMPOSE)
                         : nullptr;
}

shared_ptr<TokenInfoDictionaryWriter>
TokenInfoDictionaryBuilder::build(const wstring &dirname) 
{
  shared_ptr<FilenameFilter> filter = [&](dir, name) {
    name->endsWith(L".csv");
  };
  deque<std::shared_ptr<File>> csvFiles = deque<std::shared_ptr<File>>();
  for (auto file : (make_shared<File>(dirname))->listFiles(filter)) {
    csvFiles.push_back(file);
  }
  sort(csvFiles.begin(), csvFiles.end());
  return buildDictionary(csvFiles);
}

shared_ptr<TokenInfoDictionaryWriter>
TokenInfoDictionaryBuilder::buildDictionary(
    deque<std::shared_ptr<File>> &csvFiles) 
{
  shared_ptr<TokenInfoDictionaryWriter> dictionary =
      make_shared<TokenInfoDictionaryWriter>(10 * 1024 * 1024);

  // all lines in the file
  wcout << L"  parse..." << endl;
  deque<std::deque<wstring>> lines = deque<std::deque<wstring>>(400000);
  for (auto file : csvFiles) {
    shared_ptr<FileInputStream> inputStream =
        make_shared<FileInputStream>(file);
    shared_ptr<Charset> cs = Charset::forName(encoding);
    shared_ptr<CharsetDecoder> decoder =
        cs->newDecoder()
            .onMalformedInput(CodingErrorAction::REPORT)
            .onUnmappableCharacter(CodingErrorAction::REPORT);
    shared_ptr<InputStreamReader> streamReader =
        make_shared<InputStreamReader>(inputStream, decoder);
    shared_ptr<BufferedReader> reader =
        make_shared<BufferedReader>(streamReader);

    wstring line = L"";
    while ((line = reader->readLine()) != L"") {
      std::deque<wstring> entry = CSVUtil::parse(line);

      if (entry.size() < 12) {
        wcout << L"Entry in CSV is not valid: " << line << endl;
        continue;
      }

      // NFKC normalize dictionary entry
      if (normalizeEntries) {
        std::deque<wstring> normalizedEntry(entry.size());
        for (int i = 0; i < entry.size(); i++) {
          normalizedEntry[i] = normalizer->normalize(entry[i]);
        }
        lines.push_back(normalizedEntry);
      } else {
        lines.push_back(entry);
      }
    }
  }

  wcout << L"  sort..." << endl;

  // sort by term: we sorted the files already and use a stable sort.
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(lines, java.util.Comparator.comparing(left
  // -> left[0]));
  sort(lines.begin(), lines.end(),
       Comparator::comparing([&](any left) { left[0]; }));

  wcout << L"  encode..." << endl;

  shared_ptr<PositiveIntOutputs> fstOutput = PositiveIntOutputs::getSingleton();
  shared_ptr<Builder<int64_t>> fstBuilder = make_shared<Builder<int64_t>>(
      FST::INPUT_TYPE::BYTE2, 0, 0, true, true, numeric_limits<int>::max(),
      fstOutput, true, 15);
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  int64_t ord = -1; // first ord will be 0
  wstring lastValue = L"";

  // build tokeninfo dictionary
  for (auto entry : lines) {
    int next = dictionary->put(entry);

    if (next == offset) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"Failed to process line: " << Arrays->toString(entry) << endl;
      continue;
    }

    wstring token = entry[0];
    if (token != lastValue) {
      // new word to add to fst
      ord++;
      lastValue = token;
      scratch->grow(token.length());
      scratch->setLength(token.length());
      for (int i = 0; i < token.length(); i++) {
        scratch->setIntAt(i, static_cast<int>(token[i]));
      }
      fstBuilder->add(scratch->get(), ord);
    }
    dictionary->addMapping(static_cast<int>(ord), offset);
    offset = next;
  }

  shared_ptr<FST<int64_t>> *const fst = fstBuilder->finish();

  wcout << L"  " << fstBuilder->getNodeCount() << L" nodes, "
        << fstBuilder->getArcCount() << L" arcs, " << fst->ramBytesUsed()
        << L" bytes...  ";
  dictionary->setFST(fst);
  wcout << L" done" << endl;

  return dictionary;
}
} // namespace org::apache::lucene::analysis::ko::util