using namespace std;

#include "IndexFiles.h"

namespace org::apache::lucene::demo
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using LongPoint = org::apache::lucene::document::LongPoint;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;

IndexFiles::IndexFiles() {}

void IndexFiles::main(std::deque<wstring> &args)
{
  wstring usage =
      wstring(L"java org.apache.lucene.demo.IndexFiles") +
      L" [-index INDEX_PATH] [-docs DOCS_PATH] [-update]\n\n" +
      L"This indexes the documents in DOCS_PATH, creating a Lucene index" +
      L"in INDEX_PATH that can be searched with SearchFiles";
  wstring indexPath = L"index";
  wstring docsPath = L"";
  bool create = true;
  for (int i = 0; i < args.size(); i++) {
    if (L"-index" == args[i]) {
      indexPath = args[i + 1];
      i++;
    } else if (L"-docs" == args[i]) {
      docsPath = args[i + 1];
      i++;
    } else if (L"-update" == args[i]) {
      create = false;
    }
  }

  if (docsPath == L"") {
    System::err::println(L"Usage: " + usage);
    exit(1);
  }

  shared_ptr<Path> *const docDir = Paths->get(docsPath);
  if (!Files::isReadable(docDir)) {
    wcout << L"Document directory '" << docDir->toAbsolutePath()
          << L"' does not exist or is not readable, please check the path"
          << endl;
    exit(1);
  }

  Date start = Date();
  try {
    wcout << L"Indexing to directory '" << indexPath << L"'..." << endl;

    shared_ptr<Directory> dir = FSDirectory::open(Paths->get(indexPath));
    shared_ptr<Analyzer> analyzer = make_shared<StandardAnalyzer>();
    shared_ptr<IndexWriterConfig> iwc =
        make_shared<IndexWriterConfig>(analyzer);

    if (create) {
      // Create a new index in the directory, removing any
      // previously indexed documents:
      iwc->setOpenMode(OpenMode::CREATE);
    } else {
      // Add new documents to an existing index:
      iwc->setOpenMode(OpenMode::CREATE_OR_APPEND);
    }

    // Optional: for better indexing performance, if you
    // are indexing many documents, increase the RAM
    // buffer.  But if you do this, increase the max heap
    // size to the JVM (eg add -Xmx512m or -Xmx1g):
    //
    // iwc.setRAMBufferSizeMB(256.0);

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
    indexDocs(writer, docDir);

    // NOTE: if you want to maximize search performance,
    // you can optionally call forceMerge here.  This can be
    // a terribly costly operation, so generally it's only
    // worth it when your index is relatively static (ie
    // you're done adding documents to it):
    //
    // writer.forceMerge(1);

    delete writer;

    Date end = Date();
    wcout << end.getTime() - start.getTime() << L" total milliseconds" << endl;

  } catch (const IOException &e) {
    wcout << L" caught a " << e->getClass() << L"\n with message: "
          << e->getMessage() << endl;
  }
}

void IndexFiles::indexDocs(shared_ptr<IndexWriter> writer,
                           shared_ptr<Path> path) 
{
  if (Files::isDirectory(path)) {
    Files::walkFileTree(
        path, make_shared<SimpleFileVisitorAnonymousInnerClass>(writer));
  } else {
    indexDoc(writer, path, Files::getLastModifiedTime(path).toMillis());
  }
}

IndexFiles::SimpleFileVisitorAnonymousInnerClass::
    SimpleFileVisitorAnonymousInnerClass(shared_ptr<IndexWriter> writer)
{
  this->writer = writer;
}

shared_ptr<FileVisitResult>
IndexFiles::SimpleFileVisitorAnonymousInnerClass::visitFile(
    shared_ptr<Path> file,
    shared_ptr<BasicFileAttributes> attrs) 
{
  try {
    indexDoc(writer, file, attrs->lastModifiedTime().toMillis());
  } catch (const IOException &ignore) {
    // don't index files that can't be read.
  }
  return FileVisitResult::CONTINUE;
}

void IndexFiles::indexDoc(shared_ptr<IndexWriter> writer, shared_ptr<Path> file,
                          int64_t lastModified) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream stream =
  // java.nio.file.Files.newInputStream(file))
  {
    java::io::InputStream stream = java::nio::file::Files::newInputStream(file);
    // make a new, empty document
    shared_ptr<Document> doc = make_shared<Document>();

    // Add the path of the file as a field named "path".  Use a
    // field that is indexed (i.e. searchable), but don't tokenize
    // the field into separate words and don't index term frequency
    // or positional information:
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Field> pathField =
        make_shared<StringField>(L"path", file->toString(), Field::Store::YES);
    doc->push_back(pathField);

    // Add the last modified date of the file a field named "modified".
    // Use a LongPoint that is indexed (i.e. efficiently filterable with
    // PointRangeQuery).  This indexes to milli-second resolution, which
    // is often too fine.  You could instead create a number based on
    // year/month/day/hour/minutes/seconds, down the resolution you require.
    // For example the long value 2011021714 would mean
    // February 17, 2011, 2-3 PM.
    doc->push_back(make_shared<LongPoint>(L"modified", lastModified));

    // Add the contents of the file to a field named "contents".  Specify a
    // Reader, so that the text of the file is tokenized and indexed, but not
    // stored. Note that FileReader expects the file to be in UTF-8 encoding. If
    // that's not the case searching for special characters will fail.
    doc->push_back(make_shared<TextField>(
        L"contents", make_shared<BufferedReader>(make_shared<InputStreamReader>(
                         stream, StandardCharsets::UTF_8))));

    if (writer->getConfig()->getOpenMode() == OpenMode::CREATE) {
      // New index, so we just add the document (no old document can be there):
      wcout << L"adding " << file << endl;
      writer->addDocument(doc);
    } else {
      // Existing index (an old copy of this document may have been indexed) so
      // we use updateDocument instead to replace the old one matching the exact
      // path, if present:
      wcout << L"updating " << file << endl;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      writer->updateDocument(make_shared<Term>(L"path", file->toString()), doc);
    }
  }
}
} // namespace org::apache::lucene::demo