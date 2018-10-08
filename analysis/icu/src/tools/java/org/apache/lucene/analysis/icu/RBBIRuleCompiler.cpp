using namespace std;

#include "RBBIRuleCompiler.h"

namespace org::apache::lucene::analysis::icu
{
using com::ibm::icu::text::RuleBasedBreakIterator;

wstring RBBIRuleCompiler::getRules(shared_ptr<File> ruleFile) 
{
  shared_ptr<StringBuilder> rules = make_shared<StringBuilder>();
  shared_ptr<InputStream> in_ = make_shared<FileInputStream>(ruleFile);
  shared_ptr<BufferedReader> cin = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(in_, StandardCharsets::UTF_8));
  wstring line = L"";
  while ((line = cin->readLine()) != L"") {
    if (!StringHelper::startsWith(line, L"#")) {
      rules->append(line);
    }
    rules->append(L'\n');
  }
  cin->close();
  in_->close();
  return rules->toString();
}

void RBBIRuleCompiler::compile(shared_ptr<File> srcDir,
                               shared_ptr<File> destDir) 
{
  std::deque<std::shared_ptr<File>> files =
      srcDir->listFiles(make_shared<FilenameFilterAnonymousInnerClass>());
  if (files.empty()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Path does not exist: " + srcDir);
  }
  for (int i = 0; i < files.size(); i++) {
    shared_ptr<File> file = files[i];
    shared_ptr<File> outputFile = make_shared<File>(
        destDir, file->getName().replaceAll(L"rbbi$", L"brk"));
    wstring rules = getRules(file);
    System::err::print(L"Compiling " + file->getName() + L" to " +
                       outputFile->getName() + L": ");
    /*
     * if there is a syntax error, compileRules() may succeed. the way to
     * check is to try to instantiate from the string. additionally if the
     * rules are invalid, you can get a useful syntax error.
     */
    try {
      make_shared<RuleBasedBreakIterator>(rules);
    } catch (const invalid_argument &e) {
      /*
       * do this intentionally, so you don't get a massive stack trace
       * instead, get a useful syntax error!
       */
      System::err::println(e.what());
      exit(1);
    }
    shared_ptr<FileOutputStream> os = make_shared<FileOutputStream>(outputFile);
    RuleBasedBreakIterator::compileRules(rules, os);
    os->close();
    System::err::println(to_wstring(outputFile->length()) + L" bytes.");
  }
}

RBBIRuleCompiler::FilenameFilterAnonymousInnerClass::
    FilenameFilterAnonymousInnerClass()
{
}

bool RBBIRuleCompiler::FilenameFilterAnonymousInnerClass::accept(
    shared_ptr<File> dir, const wstring &name)
{
  return StringHelper::endsWith(name, L"rbbi");
}

void RBBIRuleCompiler::main(std::deque<wstring> &args) 
{
  if (args.size() < 2) {
    System::err::println(L"Usage: RBBIRuleComputer <sourcedir> <destdir>");
    exit(1);
  }
  compile(make_shared<File>(args[0]), make_shared<File>(args[1]));
  exit(0);
}
} // namespace org::apache::lucene::analysis::icu