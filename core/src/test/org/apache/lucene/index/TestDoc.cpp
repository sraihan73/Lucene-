using namespace std;

#include "TestDoc.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using MergeInfo = org::apache::lucene::store::MergeInfo;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;

void TestDoc::setUp() 
{
  LuceneTestCase::setUp();
  if (VERBOSE) {
    wcout << L"TEST: setUp" << endl;
  }
  workDir = createTempDir(L"TestDoc");
  indexDir = createTempDir(L"testIndex");

  shared_ptr<Directory> directory = newFSDirectory(indexDir);
  delete directory;

  files = deque<std::shared_ptr<Path>>();
  files.push_back(createOutput(L"test.txt", L"This is the first test file"));

  files.push_back(createOutput(L"test2.txt", L"This is the second test file"));
}

shared_ptr<Path> TestDoc::createOutput(const wstring &name,
                                       const wstring &text) 
{
  shared_ptr<Writer> fw = nullptr;
  shared_ptr<PrintWriter> pw = nullptr;

  try {
    shared_ptr<Path> path = workDir->resolve(name);
    Files::deleteIfExists(path);

    fw = make_shared<OutputStreamWriter>(Files::newOutputStream(path),
                                         StandardCharsets::UTF_8);
    pw = make_shared<PrintWriter>(fw);
    pw->println(text);
    return path;

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (pw != nullptr) {
      pw->close();
    }
    if (fw != nullptr) {
      fw->close();
    }
  }
}

void TestDoc::testIndexAndMerge() 
{
  shared_ptr<StringWriter> sw = make_shared<StringWriter>();
  shared_ptr<PrintWriter> out = make_shared<PrintWriter>(sw, true);

  shared_ptr<Directory> directory = newFSDirectory(indexDir);

  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(directory) != nullptr) {
    // We create unreferenced files (we don't even write
    // a segments file):
    (std::static_pointer_cast<MockDirectoryWrapper>(directory))
        ->setAssertNoUnrefencedFilesOnClose(false);
  }

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setOpenMode(OpenMode::CREATE)
                     ->setMaxBufferedDocs(-1)
                     ->setMergePolicy(newLogMergePolicy(10)));

  shared_ptr<SegmentCommitInfo> si1 = indexDoc(writer, L"test.txt");
  printSegment(out, si1);

  shared_ptr<SegmentCommitInfo> si2 = indexDoc(writer, L"test2.txt");
  printSegment(out, si2);
  delete writer;

  shared_ptr<SegmentCommitInfo> siMerge =
      merge(directory, si1, si2, L"_merge", false);
  printSegment(out, siMerge);

  shared_ptr<SegmentCommitInfo> siMerge2 =
      merge(directory, si1, si2, L"_merge2", false);
  printSegment(out, siMerge2);

  shared_ptr<SegmentCommitInfo> siMerge3 =
      merge(directory, siMerge, siMerge2, L"_merge3", false);
  printSegment(out, siMerge3);

  delete directory;
  out->close();
  sw->close();

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring multiFileOutput = sw->toString();
  // System.out.println(multiFileOutput);

  sw = make_shared<StringWriter>();
  out = make_shared<PrintWriter>(sw, true);

  directory = newFSDirectory(indexDir);

  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(directory) != nullptr) {
    // We create unreferenced files (we don't even write
    // a segments file):
    (std::static_pointer_cast<MockDirectoryWrapper>(directory))
        ->setAssertNoUnrefencedFilesOnClose(false);
  }

  writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setOpenMode(OpenMode::CREATE)
                     ->setMaxBufferedDocs(-1)
                     ->setMergePolicy(newLogMergePolicy(10)));

  si1 = indexDoc(writer, L"test.txt");
  printSegment(out, si1);

  si2 = indexDoc(writer, L"test2.txt");
  printSegment(out, si2);
  delete writer;

  siMerge = merge(directory, si1, si2, L"_merge", true);
  printSegment(out, siMerge);

  siMerge2 = merge(directory, si1, si2, L"_merge2", true);
  printSegment(out, siMerge2);

  siMerge3 = merge(directory, siMerge, siMerge2, L"_merge3", true);
  printSegment(out, siMerge3);

  delete directory;
  out->close();
  sw->close();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring singleFileOutput = sw->toString();

  assertEquals(multiFileOutput, singleFileOutput);
}

shared_ptr<SegmentCommitInfo>
TestDoc::indexDoc(shared_ptr<IndexWriter> writer,
                  const wstring &fileName) 
{
  shared_ptr<Path> path = workDir->resolve(fileName);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<InputStreamReader> is = make_shared<InputStreamReader>(
      Files::newInputStream(path), StandardCharsets::UTF_8);
  doc->push_back(make_shared<TextField>(L"contents", is));
  writer->addDocument(doc);
  writer->commit();
  is->close();
  return writer->newestSegment();
}

shared_ptr<SegmentCommitInfo>
TestDoc::merge(shared_ptr<Directory> dir, shared_ptr<SegmentCommitInfo> si1,
               shared_ptr<SegmentCommitInfo> si2, const wstring &merged,
               bool useCompoundFile) 
{
  shared_ptr<IOContext> context = newIOContext(
      random(),
      make_shared<IOContext>(make_shared<MergeInfo>(-1, -1, false, -1)));
  shared_ptr<SegmentReader> r1 =
      make_shared<SegmentReader>(si1, Version::LATEST->major, context);
  shared_ptr<SegmentReader> r2 =
      make_shared<SegmentReader>(si2, Version::LATEST->major, context);

  shared_ptr<Codec> *const codec = Codec::getDefault();
  shared_ptr<TrackingDirectoryWrapper> trackingDir =
      make_shared<TrackingDirectoryWrapper>(si1->info->dir);
  shared_ptr<SegmentInfo> *const si = make_shared<SegmentInfo>(
      si1->info->dir, Version::LATEST, nullptr, merged, -1, false, codec,
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);

  shared_ptr<SegmentMerger> merger = make_shared<SegmentMerger>(
      Arrays::asList<std::shared_ptr<CodecReader>>(r1, r2), si,
      InfoStream::getDefault(), trackingDir,
      make_shared<FieldInfos::FieldNumbers>(nullptr), context);

  shared_ptr<MergeState> mergeState = merger->merge();
  delete r1;
  delete r2;
  si->setFiles(unordered_set<>(trackingDir->getCreatedFiles()));

  if (useCompoundFile) {
    shared_ptr<deque<wstring>> filesToDelete = si->files();
    codec->compoundFormat()->write(dir, si, context);
    si->setUseCompoundFile(true);
    for (auto name : filesToDelete) {
      si1->info->dir->deleteFile(name);
    }
  }

  return make_shared<SegmentCommitInfo>(si, 0, 0, -1LL, -1LL, -1LL);
}

void TestDoc::printSegment(
    shared_ptr<PrintWriter> out,
    shared_ptr<SegmentCommitInfo> si) 
{
  shared_ptr<SegmentReader> reader = make_shared<SegmentReader>(
      si, Version::LATEST->major, newIOContext(random()));

  for (int i = 0; i < reader->numDocs(); i++) {
    out->println(reader->document(i));
  }

  for (auto fieldInfo : reader->getFieldInfos()) {
    if (fieldInfo->getIndexOptions() == IndexOptions::NONE) {
      continue;
    }
    shared_ptr<Terms> terms = reader->terms(fieldInfo->name);
    assertNotNull(terms);
    shared_ptr<TermsEnum> tis = terms->begin();
    while (tis->next() != nullptr) {

      out->print(L"  term=" + fieldInfo->name + L":" + tis->term());
      out->println(L"    DF=" + to_wstring(tis->docFreq()));

      shared_ptr<PostingsEnum> positions =
          tis->postings(nullptr, PostingsEnum::POSITIONS);

      shared_ptr<Bits> *const liveDocs = reader->getLiveDocs();
      while (positions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        if (liveDocs != nullptr && liveDocs->get(positions->docID()) == false) {
          continue;
        }
        out->print(L" doc=" + to_wstring(positions->docID()));
        out->print(L" TF=" + to_wstring(positions->freq()));
        out->print(L" pos=");
        out->print(positions->nextPosition());
        for (int j = 1; j < positions->freq(); j++) {
          out->print(L"," + to_wstring(positions->nextPosition()));
        }
        out->println(L"");
      }
    }
  }
  delete reader;
}
} // namespace org::apache::lucene::index