using namespace std;

#include "LineFileDocs.h"

namespace org::apache::lucene::util
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexableField = org::apache::lucene::index::IndexableField;

LineFileDocs::LineFileDocs(shared_ptr<Random> random,
                           const wstring &path) 
    : path(path), random(make_shared<Random>(random->nextLong()))
{
  open(random);
}

LineFileDocs::LineFileDocs(shared_ptr<Random> random) 
    : LineFileDocs(random, LuceneTestCase::TEST_LINE_DOCS_FILE)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
LineFileDocs::~LineFileDocs()
{
  IOUtils::close({reader, threadDocs});
  reader.reset();
}

int64_t LineFileDocs::randomSeekPos(shared_ptr<Random> random, int64_t size)
{
  if (random == nullptr || size <= 3LL) {
    return 0LL;
  }
  return (random->nextLong() & numeric_limits<int64_t>::max()) % (size / 3);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LineFileDocs::open(shared_ptr<Random> random) 
{
  shared_ptr<InputStream> is = getClass().getResourceAsStream(path);
  bool needSkip = true;
  int64_t size = 0LL, seekTo = 0LL;
  if (is == nullptr) {
    // if it's not in classpath, we load it as absolute filesystem path (e.g.
    // Hudson's home dir)
    shared_ptr<Path> file = Paths->get(path);
    size = Files->size(file);
    if (StringHelper::endsWith(path, L".gz")) {
      // if it is a gzip file, we need to use InputStream and slowly skipTo:
      is = Files::newInputStream(file);
    } else {
      // optimized seek using SeekableByteChannel
      seekTo = randomSeekPos(random, size);
      shared_ptr<SeekableByteChannel> *const channel =
          Files::newByteChannel(file);
      if (LuceneTestCase::VERBOSE) {
        wcout << L"TEST: LineFileDocs: file seek to fp=" << seekTo
              << L" on open" << endl;
      }
      channel->position(seekTo);
      is = Channels::newInputStream(channel);
      needSkip = false;
    }
  } else {
    // if the file comes from Classpath:
    size = is->available();
  }

  if (StringHelper::endsWith(path, L".gz")) {
    is = make_shared<GZIPInputStream>(is);
    // guestimate:
    size *= 2.8;
  }

  // If we only have an InputStream, we need to seek now,
  // but this seek is a scan, so very inefficient!!!
  if (needSkip) {
    seekTo = randomSeekPos(random, size);
    if (LuceneTestCase::VERBOSE) {
      wcout << L"TEST: LineFileDocs: stream skip to fp=" << seekTo
            << L" on open" << endl;
    }
    is->skip(seekTo);
  }

  // if we seeked somewhere, read until newline char
  if (seekTo > 0LL) {
    int b;
    do {
      b = is->read();
    } while (b >= 0 && b != 13 && b != 10);
  }

  shared_ptr<CharsetDecoder> decoder =
      StandardCharsets::UTF_8::newDecoder()
          .onMalformedInput(CodingErrorAction::REPORT)
          .onUnmappableCharacter(CodingErrorAction::REPORT);
  reader = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(is, decoder), BUFFER_SIZE);

  if (seekTo > 0LL) {
    // read one more line, to make sure we are not inside a Windows linebreak
    // (\r\n):
    reader->readLine();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LineFileDocs::reset(shared_ptr<Random> random) 
{
  reader->close();
  reader.reset();
  open(random);
  id->set(0);
}

LineFileDocs::DocState::DocState()
    : doc(make_shared<Document>()),
      titleTokenized(make_shared<Field>(L"titleTokenized", L"", ft)),
      title(make_shared<StringField>(L"title", L"", Field::Store::NO)),
      titleDV(make_shared<SortedDocValuesField>(L"titleDV",
                                                make_shared<BytesRef>())),
      body(make_shared<Field>(L"body", L"", ft)),
      id(make_shared<StringField>(L"docid", L"", Field::Store::YES)),
      idNum(make_shared<IntPoint>(L"docid_int", 0)),
      idNumDV(make_shared<NumericDocValuesField>(L"docid_intDV", 0)),
      date(make_shared<StringField>(L"date", L"", Field::Store::YES))
{

  doc->push_back(title);

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  ft->setStoreTermVectorPositions(true);

  doc->push_back(titleTokenized);

  doc->push_back(body);

  doc->push_back(id);

  doc->push_back(idNum);

  doc->push_back(date);

  doc->push_back(titleDV);
  doc->push_back(idNumDV);
}

shared_ptr<Document> LineFileDocs::nextDoc() 
{
  wstring line;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    line = reader->readLine();
    if (line == L"") {
      // Always rewind at end:
      if (LuceneTestCase::VERBOSE) {
        wcout << L"TEST: LineFileDocs: now rewind file..." << endl;
      }
      reader->close();
      reader.reset();
      open(nullptr);
      line = reader->readLine();
    }
  }

  shared_ptr<DocState> docState = threadDocs->get();
  if (docState == nullptr) {
    docState = make_shared<DocState>();
    threadDocs->set(docState);
  }

  int spot = (int)line.find(SEP);
  if (spot == -1) {
    throw runtime_error(L"line: [" + line + L"] is in an invalid format !");
  }
  int spot2 = (int)line.find(SEP, 1 + spot);
  if (spot2 == -1) {
    throw runtime_error(L"line: [" + line + L"] is in an invalid format !");
  }

  docState->body->setStringValue(
      line.substr(1 + spot2, line.length() - (1 + spot2)));
  const wstring title = line.substr(0, spot);
  docState->title->setStringValue(title);
  if (docState->titleDV != nullptr) {
    docState->titleDV->setBytesValue(make_shared<BytesRef>(title));
  }
  docState->titleTokenized->setStringValue(title);
  docState->date->setStringValue(line.substr(1 + spot, spot2 - (1 + spot)));
  constexpr int i = id->getAndIncrement();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  docState->id->setStringValue(Integer::toString(i));
  docState->idNum->setIntValue(i);
  if (docState->idNumDV != nullptr) {
    docState->idNumDV->setLongValue(i);
  }

  if (random->nextInt(5) == 4) {
    // Make some sparse fields
    shared_ptr<Document> doc = make_shared<Document>();
    for (auto field : docState->doc) {
      doc->push_back(field);
    }

    if (random->nextInt(3) == 1) {
      int x = random->nextInt(4);
      doc->push_back(
          make_shared<IntPoint>(L"docLength" + to_wstring(x), line.length()));
    }

    if (random->nextInt(3) == 1) {
      int x = random->nextInt(4);
      doc->push_back(make_shared<IntPoint>(L"docTitleLength" + to_wstring(x),
                                           title.length()));
    }

    if (random->nextInt(3) == 1) {
      int x = random->nextInt(4);
      doc->push_back(make_shared<NumericDocValuesField>(
          L"docLength" + to_wstring(x), line.length()));
    }

    // TODO: more random sparse fields here too
  }

  return docState->doc;
}
} // namespace org::apache::lucene::util