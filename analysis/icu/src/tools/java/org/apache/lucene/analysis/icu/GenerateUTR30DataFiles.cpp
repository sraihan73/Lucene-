using namespace std;

#include "GenerateUTR30DataFiles.h"

namespace org::apache::lucene::analysis::icu
{
using com::ibm::icu::lang::UCharacter;
using com::ibm::icu::lang::UProperty;
using com::ibm::icu::text::UnicodeSet;
using com::ibm::icu::text::UnicodeSetIterator;
const wstring GenerateUTR30DataFiles::ICU_SVN_TAG_URL =
    L"http://source.icu-project.org/repos/icu/tags";
const wstring GenerateUTR30DataFiles::ICU_RELEASE_TAG = L"release-60-2";
const wstring GenerateUTR30DataFiles::ICU_DATA_NORM2_PATH =
    L"icu4c/source/data/unidata/norm2";
const wstring GenerateUTR30DataFiles::NFC_TXT = L"nfc.txt";
const wstring GenerateUTR30DataFiles::NFKC_TXT = L"nfkc.txt";
const wstring GenerateUTR30DataFiles::NFKC_CF_TXT = L"nfkc_cf.txt";
std::deque<char> GenerateUTR30DataFiles::DOWNLOAD_BUFFER(8192);
const shared_ptr<java::util::regex::Pattern>
    GenerateUTR30DataFiles::ROUND_TRIP_MAPPING_LINE_PATTERN =
        java::util::regex::Pattern::compile(L"^\\s*([^=]+?)\\s*=\\s*(.*)$");
const shared_ptr<java::util::regex::Pattern>
    GenerateUTR30DataFiles::VERBATIM_RULE_LINE_PATTERN =
        java::util::regex::Pattern::compile(
            L"^#\\s*Rule:\\s*verbatim\\s*$",
            java::util::regex::Pattern::CASE_INSENSITIVE);
const shared_ptr<java::util::regex::Pattern>
    GenerateUTR30DataFiles::RULE_LINE_PATTERN =
        java::util::regex::Pattern::compile(
            L"^#\\s*Rule:\\s*(.*)>(.*)",
            java::util::regex::Pattern::CASE_INSENSITIVE);
const shared_ptr<java::util::regex::Pattern>
    GenerateUTR30DataFiles::BLANK_OR_COMMENT_LINE_PATTERN =
        java::util::regex::Pattern::compile(L"^\\s*(?:#.*)?$");
const shared_ptr<java::util::regex::Pattern>
    GenerateUTR30DataFiles::NUMERIC_VALUE_PATTERN =
        java::util::regex::Pattern::compile(
            L"Numeric[-\\s_]*Value",
            java::util::regex::Pattern::CASE_INSENSITIVE);

void GenerateUTR30DataFiles::main(std::deque<wstring> &args)
{
  try {
    getNFKCDataFilesFromIcuProject();
    expandRulesInUTR30DataFiles();
  } catch (const runtime_error &t) {
    t.printStackTrace(System::err);
    exit(1);
  }
}

void GenerateUTR30DataFiles::expandRulesInUTR30DataFiles() 
{
  shared_ptr<FileFilter> filter = make_shared<FileFilterAnonymousInnerClass>();
  for (auto file : (make_shared<File>(L"."))->listFiles(filter)) {
    expandDataFileRules(file);
  }
}

GenerateUTR30DataFiles::FileFilterAnonymousInnerClass::
    FileFilterAnonymousInnerClass()
{
}

bool GenerateUTR30DataFiles::FileFilterAnonymousInnerClass::accept(
    shared_ptr<File> pathname)
{
  wstring name = pathname->getName();
  return pathname->isFile() && regex_match(name, regex(L".*\\.(?s:txt)")) &&
         name != NFC_TXT && name != NFKC_TXT && name != NFKC_CF_TXT;
}

void GenerateUTR30DataFiles::expandDataFileRules(shared_ptr<File> file) throw(
    IOException)
{
  shared_ptr<FileInputStream> *const stream =
      make_shared<FileInputStream>(file);
  shared_ptr<InputStreamReader> *const reader =
      make_shared<InputStreamReader>(stream, StandardCharsets::UTF_8);
  shared_ptr<BufferedReader> *const bufferedReader =
      make_shared<BufferedReader>(reader);
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  wstring line;
  bool verbatim = false;
  bool modified = false;
  int lineNum = 0;
  try {
    while (L"" != (line = bufferedReader->readLine())) {
      ++lineNum;
      if (VERBATIM_RULE_LINE_PATTERN->matcher(line).matches()) {
        verbatim = true;
        builder->append(line)->append(L"\n");
      } else {
        shared_ptr<Matcher> ruleMatcher = RULE_LINE_PATTERN->matcher(line);
        if (ruleMatcher->matches()) {
          verbatim = false;
          builder->append(line)->append(L"\n");
          try {
            wstring leftHandSide = ruleMatcher->group(1)->trim();
            wstring rightHandSide = ruleMatcher->group(2)->trim();
            expandSingleRule(builder, leftHandSide, rightHandSide);
          } catch (const invalid_argument &e) {
            System::err::println(L"ERROR in " + file->getName() + L" line #" +
                                 to_wstring(lineNum) + L":");
            e.printStackTrace(System::err);
            exit(1);
          }
          modified = true;
        } else {
          if (BLANK_OR_COMMENT_LINE_PATTERN->matcher(line).matches()) {
            builder->append(line)->append(L"\n");
          } else {
            if (verbatim) {
              builder->append(line)->append(L"\n");
            } else {
              modified = true;
            }
          }
        }
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    bufferedReader->close();
  }
  if (modified) {
    System::err::println(L"Expanding rules in and overwriting " +
                         file->getName());
    shared_ptr<FileOutputStream> *const out =
        make_shared<FileOutputStream>(file, false);
    shared_ptr<Writer> writer =
        make_shared<OutputStreamWriter>(out, StandardCharsets::UTF_8);
    try {
      writer->write(builder->toString());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      writer->close();
    }
  }
}

void GenerateUTR30DataFiles::getNFKCDataFilesFromIcuProject() 
{
  shared_ptr<URL> icuTagsURL = make_shared<URL>(ICU_SVN_TAG_URL + L"/");
  shared_ptr<URL> icuReleaseTagURL =
      make_shared<URL>(icuTagsURL, ICU_RELEASE_TAG + L"/");
  shared_ptr<URL> norm2url =
      make_shared<URL>(icuReleaseTagURL, ICU_DATA_NORM2_PATH + L"/");

  System::err::print(L"Downloading " + NFKC_TXT + L" ... ");
  download(make_shared<URL>(norm2url, NFKC_TXT), NFKC_TXT);
  System::err::println(L"done.");
  System::err::print(L"Downloading " + NFKC_CF_TXT + L" ... ");
  download(make_shared<URL>(norm2url, NFKC_CF_TXT), NFKC_CF_TXT);
  System::err::println(L"done.");

  System::err::print(L"Downloading " + NFKC_CF_TXT +
                     L" and making diacritic rules one-way ... ");
  shared_ptr<URLConnection> connection =
      openConnection(make_shared<URL>(norm2url, NFC_TXT));
  shared_ptr<BufferedReader> reader =
      make_shared<BufferedReader>(make_shared<InputStreamReader>(
          connection->getInputStream(), StandardCharsets::UTF_8));
  shared_ptr<Writer> writer = make_shared<OutputStreamWriter>(
      make_shared<FileOutputStream>(NFC_TXT), StandardCharsets::UTF_8);
  try {
    wstring line;

    while (L"" != (line = reader->readLine())) {
      shared_ptr<Matcher> matcher =
          ROUND_TRIP_MAPPING_LINE_PATTERN->matcher(line);
      if (matcher->matches()) {
        const wstring leftHandSide = matcher->group(1);
        const wstring rightHandSide = matcher->group(2)->trim();
        deque<wstring> diacritics = deque<wstring>();
        for (wstring outputCodePoint : rightHandSide.split(L"\\s+")) {
          // C++ TODO: Only single-argument parse and valueOf methods are
          // converted: ORIGINAL LINE: int ch = Integer.parseInt(outputCodePoint,
          // 16);
          int ch = Integer::valueOf(outputCodePoint, 16);
          if (UCharacter::hasBinaryProperty(ch, UProperty::DIACRITIC) ||
              (ch >= 0x653 && ch <= 0x656)) {
            diacritics.push_back(outputCodePoint);
          }
        }
        if (!diacritics.empty()) {
          shared_ptr<StringBuilder> replacementLine =
              make_shared<StringBuilder>();
          replacementLine->append(leftHandSide)
              ->append(L">")
              ->append(rightHandSide);
          replacementLine->append(L"  # one-way: diacritic");
          if (diacritics.size() > 1) {
            replacementLine->append(L"s");
          }
          for (auto diacritic : diacritics) {
            replacementLine->append(L" ")->append(diacritic);
          }
          line = replacementLine->toString();
        }
      }
      writer->write(line);
      writer->write(L"\n");
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reader->close();
    writer->close();
  }
  System::err::println(L"done.");
}

void GenerateUTR30DataFiles::download(
    shared_ptr<URL> url, const wstring &outputFile) 
{
  shared_ptr<URLConnection> *const connection = openConnection(url);
  shared_ptr<InputStream> *const inputStream = connection->getInputStream();
  shared_ptr<OutputStream> *const outputStream =
      make_shared<FileOutputStream>(outputFile);
  int numBytes;
  try {
    while (-1 != (numBytes = inputStream->read(DOWNLOAD_BUFFER))) {
      outputStream->write(DOWNLOAD_BUFFER, 0, numBytes);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    inputStream->close();
    outputStream->close();
  }
}

shared_ptr<URLConnection>
GenerateUTR30DataFiles::openConnection(shared_ptr<URL> url) 
{
  shared_ptr<URLConnection> *const connection = url->openConnection();
  connection->setUseCaches(false);
  connection->addRequestProperty(L"Cache-Control", L"no-cache");
  connection->connect();
  return connection;
}

void GenerateUTR30DataFiles::expandSingleRule(
    shared_ptr<StringBuilder> builder, const wstring &leftHandSide,
    const wstring &rightHandSide) 
{
  shared_ptr<UnicodeSet> set =
      make_shared<UnicodeSet>(leftHandSide, UnicodeSet::IGNORE_SPACE);
  bool numericValue = NUMERIC_VALUE_PATTERN->matcher(rightHandSide).matches();
  for (shared_ptr<UnicodeSetIterator> it = make_shared<UnicodeSetIterator>(set);
       it->nextRange();) {
    if (it->codepoint != UnicodeSetIterator::IS_STRING) {
      if (numericValue) {
        for (int cp = it->codepoint; cp <= it->codepointEnd; ++cp) {
          builder->append(wstring::format(Locale::ROOT, L"%04X", cp))
              ->append(L'>');
          builder->append(wstring::format(
              Locale::ROOT, L"%04X", 0x30 + UCharacter::getNumericValue(cp)));
          builder->append(L"   # ")->append(UCharacter::getName(cp));
          builder->append(L"\n");
        }
      } else {
        builder->append(wstring::format(Locale::ROOT, L"%04X", it->codepoint));
        if (it->codepointEnd > it->codepoint) {
          builder->append(L"..")->append(
              wstring::format(Locale::ROOT, L"%04X", it->codepointEnd));
        }
        builder->append(L'>')->append(rightHandSide)->append(L"\n");
      }
    } else {
      System::err::println(L"ERROR: std::wstring '" + it->getString() +
                           L"' found in UnicodeSet");
      exit(1);
    }
  }
}
} // namespace org::apache::lucene::analysis::icu