using namespace std;

#include "ExtractReuters.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"

namespace org::apache::lucene::benchmark::utils
{
using IOUtils = org::apache::lucene::util::IOUtils;

ExtractReuters::ExtractReuters(shared_ptr<Path> reutersDir,
                               shared_ptr<Path> outputDir) 
{
  this->reutersDir = reutersDir;
  this->outputDir = outputDir;
  wcout << L"Deleting all files in " << outputDir << endl;
  IOUtils::rm({outputDir});
}

void ExtractReuters::extract() 
{
  int64_t count = 0;
  Files::createDirectories(outputDir);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(reutersDir, "*.sgm"))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream =
            java::nio::file::Files::newDirectoryStream(reutersDir, L"*.sgm");
    for (auto sgmFile : stream) {
      extractFile(sgmFile);
      count++;
    }
  }
  if (count == 0) {
    System::err::println(L"No .sgm files in " + reutersDir);
  }
}

std::deque<wstring> ExtractReuters::META_CHARS = {L"&", L"<", L">", L"\"",
                                                   L"'"};
std::deque<wstring> ExtractReuters::META_CHARS_SERIALIZATIONS = {
    L"&amp;", L"&lt;", L"&gt;", L"&quot;", L"&apos;"};

void ExtractReuters::extractFile(shared_ptr<Path> sgmFile)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader reader =
  // java.nio.file.Files.newBufferedReader(sgmFile,
  // java.nio.charset.StandardCharsets.ISO_8859_1))
  {
    java::io::BufferedReader reader = java::nio::file::Files::newBufferedReader(
        sgmFile, java::nio::charset::StandardCharsets::ISO_8859_1);
    try {
      shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>(1024);
      shared_ptr<StringBuilder> outBuffer = make_shared<StringBuilder>(1024);

      wstring line = L"";
      int docNumber = 0;
      while ((line = reader.readLine()) != L"") {
        // when we see a closing reuters tag, flush the file

        if (line.find(L"</REUTERS") == wstring::npos) {
          // Replace the SGM escape sequences

          buffer->append(line)->append(L' '); // accumulate the strings for now,
                                              // then apply regular expression
                                              // to get the pieces,
        } else {
          // Extract the relevant pieces and write to a file in the output dir
          shared_ptr<Matcher> matcher = EXTRACTION_PATTERN->matcher(buffer);
          while (matcher->find()) {
            for (int i = 1; i <= matcher->groupCount(); i++) {
              if (matcher->group(i) != nullptr) {
                outBuffer->append(matcher->group(i));
              }
            }
            outBuffer->append(L"\r\n")->append(L"\r\n");
          }
          wstring out = outBuffer->toString();
          for (int i = 0; i < META_CHARS_SERIALIZATIONS.size(); i++) {
            out = out.replaceAll(META_CHARS_SERIALIZATIONS[i], META_CHARS[i]);
          }
          shared_ptr<Path> outFile =
              outputDir->resolve(sgmFile->getFileName() + L"-" +
                                 to_wstring(docNumber++) + L".txt");
          // System.out.println("Writing " + outFile);
          // C++ NOTE: The following 'try with resources' block is replaced by
          // its C++ equivalent: ORIGINAL LINE: try (java.io.BufferedWriter
          // writer = java.nio.file.Files.newBufferedWriter(outFile,
          // java.nio.charset.StandardCharsets.UTF_8))
          {
            java::io::BufferedWriter writer =
                java::nio::file::Files::newBufferedWriter(
                    outFile, java::nio::charset::StandardCharsets::UTF_8);
            writer.write(out);
          }
          outBuffer->setLength(0);
          buffer->setLength(0);
        }
      }
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  }
}

void ExtractReuters::main(std::deque<wstring> &args) 
{
  if (args.size() != 2) {
    usage(L"Wrong number of arguments (" + args.size() + L")");
    return;
  }
  shared_ptr<Path> reutersDir = Paths->get(args[0]);
  if (!Files::exists(reutersDir)) {
    usage(L"Cannot find Path to Reuters SGM files (" + reutersDir + L")");
    return;
  }

  // First, extract to a tmp directory and only if everything succeeds, rename
  // to output directory.
  shared_ptr<Path> outputDir = Paths->get(args[1] + L"-tmp");
  Files::createDirectories(outputDir);
  shared_ptr<ExtractReuters> extractor =
      make_shared<ExtractReuters>(reutersDir, outputDir);
  extractor->extract();
  // Now rename to requested output dir
  Files::move(outputDir, Paths->get(args[1]), StandardCopyOption::ATOMIC_MOVE);
}

void ExtractReuters::usage(const wstring &msg)
{
  System::err::println(
      L"Usage: " + msg +
      L" :: java -cp <...> org.apache.lucene.benchmark.utils.ExtractReuters "
      L"<Path to Reuters SGM files> <Output Path>");
}
} // namespace org::apache::lucene::benchmark::utils