using namespace std;

#include "Algorithm.h"
#include "../PerfRunData.h"
#include "../tasks/PerfTask.h"
#include "../tasks/RepSumByPrefTask.h"
#include "../tasks/TaskSequence.h"
#include "Config.h"

namespace org::apache::lucene::benchmark::byTask::utils
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using PerfTask = org::apache::lucene::benchmark::byTask::tasks::PerfTask;
using RepSumByPrefTask =
    org::apache::lucene::benchmark::byTask::tasks::RepSumByPrefTask;
using TaskSequence =
    org::apache::lucene::benchmark::byTask::tasks::TaskSequence;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public
// Algorithm(org.apache.lucene.benchmark.byTask.PerfRunData runData) throws
// Exception
Algorithm::Algorithm(shared_ptr<PerfRunData> runData) 
    : taskPackages(initTasksPackages(config))
{
  shared_ptr<Config> config = runData->getConfig();
  wstring algTxt = config->getAlgorithmText();
  sequence = make_shared<TaskSequence>(runData, nullptr, nullptr, false);
  shared_ptr<TaskSequence> currSequence = sequence;
  shared_ptr<PerfTask> prevTask = nullptr;
  shared_ptr<StreamTokenizer> stok =
      make_shared<StreamTokenizer>(make_shared<StringReader>(algTxt));
  stok->commentChar(L'#');
  stok->eolIsSignificant(false);
  stok->quoteChar(L'"');
  stok->quoteChar(L'\'');
  stok->ordinaryChar(L'/');
  stok->ordinaryChar(L'(');
  stok->ordinaryChar(L')');
  bool colonOk = false;
  bool isDisableCountNextTask = false; // only for primitive tasks
  currSequence->setDepth(0);

  while (stok->nextToken() != StreamTokenizer::TT_EOF) {
    switch (stok->ttype) {

    case StreamTokenizer::TT_WORD: {
      wstring s = stok->sval;
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: Constructor<? extends
      // org.apache.lucene.benchmark.byTask.tasks.PerfTask> cnstr =
      // taskClass(config,s).asSubclass(org.apache.lucene.benchmark.byTask.tasks.PerfTask.class).getConstructor(org.apache.lucene.benchmark.byTask.PerfRunData.class);
      shared_ptr < Constructor < ? extends PerfTask >> cnstr =
                                       taskClass(config, s)
                                           .asSubclass(PerfTask::typeid)
                                           .getConstructor(PerfRunData::typeid);
      shared_ptr<PerfTask> task = cnstr->newInstance(runData);
      task->setAlgLineNum(stok->lineno());
      task->setDisableCounting(isDisableCountNextTask);
      isDisableCountNextTask = false;
      currSequence->addTask(task);
      if (std::dynamic_pointer_cast<RepSumByPrefTask>(task) != nullptr) {
        stok->nextToken();
        wstring prefix = stok->sval;
        if (prefix == L"" || prefix.length() == 0) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw runtime_error(L"named report prefix problem - " +
                              stok->toString());
        }
        (std::static_pointer_cast<RepSumByPrefTask>(task))->setPrefix(prefix);
      }
      // check for task param: '(' someParam ')'
      stok->nextToken();
      if (stok->ttype != L'(') {
        stok->pushBack();
      } else {
        // get params, for tasks that supports them - allow recursive
        // parenthetical expressions
        stok->eolIsSignificant(
            true); // Allow params tokenizer to keep track of line number
        shared_ptr<StringBuilder> params = make_shared<StringBuilder>();
        stok->nextToken();
        if (stok->ttype != L')') {
          int count = 1;
          while (true) {
            switch (stok->ttype) {
            case StreamTokenizer::TT_NUMBER: {
              params->append(stok->nval);
              break;
            }
            case StreamTokenizer::TT_WORD: {
              params->append(stok->sval);
              break;
            }
            case StreamTokenizer::TT_EOF: {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              throw runtime_error(L"Unexpexted EOF: - " + stok->toString());
            }
            case L'"':
            case L'\'': {
              params->append(static_cast<wchar_t>(stok->ttype));
              // re-escape delimiters, if any
              params->append(stok->sval.replaceAll(
                  L"" +
                      StringHelper::toString(static_cast<wchar_t>(stok->ttype)),
                  L"\\\\" + StringHelper::toString(
                                static_cast<wchar_t>(stok->ttype))));
              params->append(static_cast<wchar_t>(stok->ttype));
              break;
            }
            case L'(': {
              params->append(static_cast<wchar_t>(stok->ttype));
              ++count;
              break;
            }
            case L')': {
              if (--count >= 1) { // exclude final closing parenthesis
                params->append(static_cast<wchar_t>(stok->ttype));
              } else {
                goto BALANCED_PARENSBreak;
              }
              break;
            }
            default: {
              params->append(static_cast<wchar_t>(stok->ttype));
            }
            }
            stok->nextToken();
          BALANCED_PARENSContinue:;
          }
        BALANCED_PARENSBreak:;
        }
        stok->eolIsSignificant(false);
        wstring prm = params->toString()->trim();
        if (prm.length() > 0) {
          task->setParams(prm);
        }
      }

      // ---------------------------------------
      colonOk = false;
      prevTask = task;
      break;
    }
    default:
      wchar_t c = static_cast<wchar_t>(stok->ttype);

      switch (c) {

      case L':':
        if (!colonOk) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw runtime_error(L"colon unexpexted: - " + stok->toString());
        }
        colonOk = false;
        // get repetitions number
        stok->nextToken();
        if (static_cast<wchar_t>(stok->ttype) == L'*') {
          (std::static_pointer_cast<TaskSequence>(prevTask))
              ->setRepetitions(TaskSequence::REPEAT_EXHAUST);
        } else {
          if (stok->ttype != StreamTokenizer::TT_NUMBER) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            throw runtime_error(L"expected repetitions number or XXXs: - " +
                                stok->toString());
          } else {
            double num = stok->nval;
            stok->nextToken();
            if (stok->ttype == StreamTokenizer::TT_WORD &&
                stok->sval.equals(L"s")) {
              (std::static_pointer_cast<TaskSequence>(prevTask))
                  ->setRunTime(num);
            } else {
              stok->pushBack();
              (std::static_pointer_cast<TaskSequence>(prevTask))
                  ->setRepetitions(static_cast<int>(num));
            }
          }
        }
        // check for rate specification (ops/min)
        stok->nextToken();
        if (stok->ttype != L':') {
          stok->pushBack();
        } else {
          // get rate number
          stok->nextToken();
          if (stok->ttype != StreamTokenizer::TT_NUMBER) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            throw runtime_error(L"expected rate number: - " + stok->toString());
          }
          // check for unit - min or sec, sec is default
          stok->nextToken();
          if (stok->ttype != L'/') {
            stok->pushBack();
            (std::static_pointer_cast<TaskSequence>(prevTask))
                ->setRate(static_cast<int>(stok->nval),
                          false); // set rate per sec
          } else {
            stok->nextToken();
            if (stok->ttype != StreamTokenizer::TT_WORD) {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              throw runtime_error(L"expected rate unit: 'min' or 'sec' - " +
                                  stok->toString());
            }
            wstring unit = stok->sval->toLowerCase(Locale::ROOT);
            if (L"min" == unit) {
              (std::static_pointer_cast<TaskSequence>(prevTask))
                  ->setRate(static_cast<int>(stok->nval),
                            true); // set rate per min
            } else if (L"sec" == unit) {
              (std::static_pointer_cast<TaskSequence>(prevTask))
                  ->setRate(static_cast<int>(stok->nval),
                            false); // set rate per sec
            } else {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              throw runtime_error(L"expected rate unit: 'min' or 'sec' - " +
                                  stok->toString());
            }
          }
        }
        colonOk = false;
        break;

      case L'{':
      case L'[': {
        // a sequence
        // check for sequence name
        wstring name = L"";
        stok->nextToken();
        if (stok->ttype != L'"') {
          stok->pushBack();
        } else {
          name = stok->sval;
          if (stok->ttype != L'"' || name == L"" || name.length() == 0) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            throw runtime_error(L"sequence name problem - " + stok->toString());
          }
        }
        // start the sequence
        shared_ptr<TaskSequence> seq2 =
            make_shared<TaskSequence>(runData, name, currSequence, c == L'[');
        currSequence->addTask(seq2);
        currSequence = seq2;
        colonOk = false;
        break;
      }
      case L'&': {
        if (currSequence->isParallel()) {
          throw runtime_error(
              L"Can only create background tasks within a serial task");
        }
        stok->nextToken();
        constexpr int deltaPri;
        if (stok->ttype != StreamTokenizer::TT_NUMBER) {
          stok->pushBack();
          deltaPri = 0;
        } else {
          // priority
          deltaPri = static_cast<int>(stok->nval);
        }

        if (prevTask == nullptr) {
          throw runtime_error(L"& was unexpected");
        } else if (prevTask->getRunInBackground()) {
          throw runtime_error(L"double & was unexpected");
        } else {
          prevTask->setRunInBackground(deltaPri);
        }
        break;
      }
      case L'>':
        currSequence->setNoChildReport(); // intentional fallthrough
      case L'}':
      case L']':
        // end sequence
        colonOk = true;
        prevTask = currSequence;
        currSequence = currSequence->getParent();
        break;

      case L'-':
        isDisableCountNextTask = true;
        break;

      } // switch(c)
      break;

    } // switch(stok.ttype)
  }

  if (sequence != currSequence) {
    throw runtime_error(L"Unmatched sequences");
  }

  // remove redundant top level enclosing sequences
  while (sequence->isCollapsable() && sequence->getRepetitions() == 1 &&
         sequence->getRate() == 0) {
    deque<std::shared_ptr<PerfTask>> t = sequence->getTasks();
    if (t.size() > 0 && t.size() == 1) {
      shared_ptr<PerfTask> p = t[0];
      if (std::dynamic_pointer_cast<TaskSequence>(p) != nullptr) {
        sequence = std::static_pointer_cast<TaskSequence>(p);
        continue;
      }
    }
    break;
  }
}

std::deque<wstring> Algorithm::initTasksPackages(shared_ptr<Config> config)
{
  wstring alts = config->get(L"alt.tasks.packages", L"");
  wstring dfltPkg = PerfTask::typeid->getPackage().getName();
  if (alts == L"") {
    return std::deque<wstring>{dfltPkg};
  }
  deque<wstring> pkgs = deque<wstring>();
  pkgs.push_back(dfltPkg);
  for (wstring alt : alts.split(L",")) {
    pkgs.push_back(alt);
  }
  return pkgs.toArray(std::deque<wstring>(0));
}

type_info
Algorithm::taskClass(shared_ptr<Config> config,
                     const wstring &taskName) 
{
  for (auto pkg : taskPackages) {
    try {
      return type_info::forName(pkg + StringHelper::toString(L'.') + taskName +
                                L"Task");
    } catch (const ClassNotFoundException &e) {
      // failed in this package, might succeed in the next one...
    }
  }
  // can only get here if failed to instantiate
  // C++ TODO: There is no native C++ equivalent to 'toString':
  throw make_shared<ClassNotFoundException>(
      taskName + L" not found in packages " + Arrays->toString(taskPackages));
}

wstring Algorithm::toString()
{
  wstring newline = System::getProperty(L"line.separator");
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  sb->append(sequence->toString());
  sb->append(newline);
  return sb->toString();
}

void Algorithm::execute() 
{
  try {
    sequence->runAndMaybeStats(true);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    sequence->close();
  }
}

deque<std::shared_ptr<PerfTask>> Algorithm::extractTasks()
{
  deque<std::shared_ptr<PerfTask>> res = deque<std::shared_ptr<PerfTask>>();
  extractTasks(res, sequence);
  return res;
}

void Algorithm::extractTasks(deque<std::shared_ptr<PerfTask>> &extrct,
                             shared_ptr<TaskSequence> seq)
{
  if (seq == nullptr) {
    return;
  }
  extrct.push_back(seq);
  deque<std::shared_ptr<PerfTask>> t = sequence->getTasks();
  if (t.empty()) {
    return;
  }
  for (auto p : t) {
    if (std::dynamic_pointer_cast<TaskSequence>(p) != nullptr) {
      extractTasks(extrct, std::static_pointer_cast<TaskSequence>(p));
    } else {
      extrct.push_back(p);
    }
  }
}

Algorithm::~Algorithm() { sequence->close(); }
} // namespace org::apache::lucene::benchmark::byTask::utils