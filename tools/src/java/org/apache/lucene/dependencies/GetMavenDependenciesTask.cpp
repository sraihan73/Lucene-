using namespace std;

#include "GetMavenDependenciesTask.h"

namespace org::apache::lucene::dependencies
{
using org::apache::tools::ant::BuildException;
using org::apache::tools::ant::Project;
using org::apache::tools::ant::Task;
using org::apache::tools::ant::types::Resource;
using org::apache::tools::ant::types::ResourceCollection;
using org::apache::tools::ant::types::resources::FileResource;
using org::apache::tools::ant::types::resources::Resources;
using org::w3c::dom::Document;
using org::w3c::dom::Element;
using org::w3c::dom::NodeList;
using org::xml::sax::SAXException;
const shared_ptr<java::util::regex::Pattern>
    GetMavenDependenciesTask::PROPERTY_PREFIX_FROM_IVY_XML_FILE_PATTERN =
        java::util::regex::Pattern::compile(
            L"[/\\\\](lucene|solr)[/"
            L"\\\\](?:(?:contrib|(analysis)|(example)|(server))[/\\\\])?([^/"
            L"\\\\]+)[/\\\\]ivy\\.xml");
const shared_ptr<java::util::regex::Pattern>
    GetMavenDependenciesTask::COORDINATE_KEY_PATTERN =
        java::util::regex::Pattern::compile(L"/([^/]+)/([^/]+)");
const shared_ptr<java::util::regex::Pattern>
    GetMavenDependenciesTask::MODULE_DEPENDENCIES_COORDINATE_KEY_PATTERN =
        java::util::regex::Pattern::compile(L"(.*?)(\\.test)?\\.dependencies");
const shared_ptr<java::util::regex::Pattern>
    GetMavenDependenciesTask::COMPILATION_OUTPUT_DIRECTORY_PATTERN =
        java::util::regex::Pattern::compile(
            L"(lucene|solr)/build/(?:contrib/)?(.*)/classes/(?:java|test)");
const wstring GetMavenDependenciesTask::UNWANTED_INTERNAL_DEPENDENCIES =
    L"/(?:test-)?lib/|test-framework/classes/java|/test-files|/resources";
const shared_ptr<java::util::regex::Pattern>
    GetMavenDependenciesTask::SHARED_EXTERNAL_DEPENDENCIES_PATTERN =
        java::util::regex::Pattern::compile(
            L"((?:solr|lucene)/(?!test-framework).*)/((?:test-)?)lib/");
const wstring GetMavenDependenciesTask::DEPENDENCY_MANAGEMENT_PROPERTY =
    L"lucene.solr.dependency.management";
const wstring GetMavenDependenciesTask::IVY_USER_DIR_PROPERTY =
    L"ivy.default.ivy.user.dir";
const shared_ptr<java::util::Properties>
    GetMavenDependenciesTask::allProperties =
        make_shared<java::util::Properties>();
const shared_ptr<java::util::Set<wstring>>
    GetMavenDependenciesTask::modulesWithSeparateCompileAndTestPOMs =
        unordered_set<wstring>();
const shared_ptr<java::util::Set<wstring>>
    GetMavenDependenciesTask::globalOptionalExternalDependencies =
        unordered_set<wstring>();
const unordered_map<wstring, java::util::Set<wstring>>
    GetMavenDependenciesTask::perModuleOptionalExternalDependencies =
        unordered_map<wstring, java::util::Set<wstring>>();

GetMavenDependenciesTask::StaticConstructor::StaticConstructor()
{
  // Add modules here that have split compile and test POMs
  // - they need compile-scope deps to also be test-scope deps.
  modulesWithSeparateCompileAndTestPOMs->addAll(Arrays::asList(
      L"lucene-core", L"lucene-codecs", L"solr-core", L"solr-solrj"));

  // Add external dependencies here that should be optional for all modules
  // (i.e., not invoke Maven's transitive dependency mechanism).
  // Format is "groupId:artifactId"
  globalOptionalExternalDependencies->addAll(
      Arrays::asList(L"org.slf4j:jul-to-slf4j", L"org.slf4j:slf4j-log4j12"));
}

GetMavenDependenciesTask::StaticConstructor
    GetMavenDependenciesTask::staticConstructor;

void GetMavenDependenciesTask::add(shared_ptr<ResourceCollection> rc)
{
  ivyXmlResources->add(rc);
}

void GetMavenDependenciesTask::setVerbose(bool verbose)
{
  verboseLevel = (verbose ? Project::MSG_VERBOSE : Project::MSG_INFO);
}

void GetMavenDependenciesTask::setCentralizedVersionsFile(shared_ptr<File> file)
{
  centralizedVersionsFile = file;
}

void GetMavenDependenciesTask::setModuleDependenciesPropertiesFile(
    shared_ptr<File> file)
{
  moduleDependenciesPropertiesFile = file;
}

void GetMavenDependenciesTask::setMavenDependenciesFiltersFile(
    shared_ptr<File> file)
{
  mavenDependenciesFiltersFile = file;
}

GetMavenDependenciesTask::GetMavenDependenciesTask()
{
  try {
    documentBuilder =
        DocumentBuilderFactory::newInstance().newDocumentBuilder();
  } catch (const ParserConfigurationException &e) {
    throw make_shared<BuildException>(e);
  }
}

void GetMavenDependenciesTask::execute() 
{
  // Local:
  // lucene/build/analysis/common/lucene-analyzers-common-5.0-SNAPSHOT.jar
  // Jenkins:
  // lucene/build/analysis/common/lucene-analyzers-common-5.0-2013-10-31_18-52-24.jar
  // Also support any custom version, which won't necessarily conform to any
  // predefined pattern.
  internalJarPattern = Pattern::compile(
      L".*(lucene|solr)([^/]*?)-" +
      Pattern::quote(getProject().getProperty(L"version")) + L"\\.jar");

  ivyModuleInfo = getIvyModuleInfo(ivyXmlResources, documentBuilder, xpath);

  setInternalDependencyProperties(); // side-effect: all modules' internal deps
                                     // are recorded
  setExternalDependencyProperties(); // side-effect: all modules' external deps
                                     // are recorded
  setGrandparentDependencyManagementProperty(); // uses deps recorded in above
                                                // two methods
  writeFiltersFile();
}

void GetMavenDependenciesTask::writeFiltersFile()
{
  shared_ptr<Writer> writer = nullptr;
  try {
    shared_ptr<FileOutputStream> outputStream =
        make_shared<FileOutputStream>(mavenDependenciesFiltersFile);
    writer = make_shared<OutputStreamWriter>(outputStream,
                                             StandardCharsets::ISO_8859_1);
    allProperties->store(writer, nullptr);
  } catch (const FileNotFoundException &e) {
    throw make_shared<BuildException>(
        L"Can't find file: '" + mavenDependenciesFiltersFile->getPath() + L"'",
        e);
  } catch (const IOException &e) {
    throw make_shared<BuildException>(
        L"Exception writing out '" + mavenDependenciesFiltersFile->getPath() +
            L"'",
        e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nullptr != writer) {
      try {
        writer->close();
      } catch (const IOException &e) {
        // ignore
      }
    }
  }
}

unordered_map<wstring, wstring> GetMavenDependenciesTask::getIvyModuleInfo(
    shared_ptr<Resources> ivyXmlResources,
    shared_ptr<DocumentBuilder> documentBuilder, shared_ptr<XPath> xpath)
{
  unordered_map<wstring, wstring> ivyInfoModuleToOrganisation =
      unordered_map<wstring, wstring>();
  traverseIvyXmlResources(ivyXmlResources, [&](File f) {
    try {
      shared_ptr<Document> document = documentBuilder->parse(f);
      {
        wstring infoPath = L"/ivy-module/info";
        shared_ptr<NodeList> infos = std::static_pointer_cast<NodeList>(
            xpath->evaluate(infoPath, document, XPathConstants::NODESET));
        for (int infoNum = 0; infoNum < infos->getLength(); ++infoNum) {
          shared_ptr<Element> infoElement =
              std::static_pointer_cast<Element>(infos->item(infoNum));
          wstring infoOrg = infoElement->getAttribute(L"organisation");
          wstring infoOrgSuffix = infoOrg.substr((int)infoOrg.rfind(L'.') + 1);
          wstring infoModule = infoElement->getAttribute(L"module");
          wstring module = infoOrgSuffix + L"-" + infoModule;
          ivyInfoModuleToOrganisation.emplace(module, infoOrg);
        }
      }
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (XPathExpressionException | IOException | SAXException e) {
      throw runtime_error(e);
    }
  });
  return ivyInfoModuleToOrganisation;
}

void GetMavenDependenciesTask::setExternalDependencyProperties()
{
  traverseIvyXmlResources(ivyXmlResources, [&](File f) {
    try {
      collectExternalDependenciesFromIvyXmlFile(f);
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (XPathExpressionException | IOException | SAXException e) {
      throw runtime_error(e);
    }
  });
  addSharedExternalDependencies();
  setExternalDependencyXmlProperties();
}

void GetMavenDependenciesTask::traverseIvyXmlResources(
    shared_ptr<Resources> ivyXmlResources,
    function<void(File *)> &ivyXmlFileConsumer)
{
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked")
  // java.util.Iterator<org.apache.tools.ant.types.Resource> iter =
  // (java.util.Iterator<org.apache.tools.ant.types.Resource>)ivyXmlResources.iterator();
  Resources::const_iterator iter =
      std::static_pointer_cast<Iterator<std::shared_ptr<Resource>>>(
          ivyXmlResources->begin());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  while (iter->hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<Resource> *const resource = iter->next();
    if (!resource->isExists()) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(L"Resource does not exist: " +
                                                resource.getName());
    }
    if (!(std::dynamic_pointer_cast<FileResource>(resource) != nullptr)) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Only filesystem resources are supported: " + resource.getName() +
          L", was: " + resource.getClassName());
    }

    shared_ptr<File> ivyXmlFile =
        (std::static_pointer_cast<FileResource>(resource))->getFile();
    try {
      ivyXmlFileConsumer(ivyXmlFile);
    } catch (const BuildException &e) {
      throw e;
    } catch (const runtime_error &e) {
      throw make_shared<BuildException>(
          L"Exception reading file " + ivyXmlFile->getPath() + L": " + e, e);
    }
    iter++;
  }
}

void GetMavenDependenciesTask::addSharedExternalDependencies()
{
  // Delay adding shared compile-scope dependencies until after all have been
  // processed, so dependency sharing is limited to a depth of one.
  unordered_map<wstring, SortedSet<std::shared_ptr<ExternalDependency>>>
      sharedDependencies =
          unordered_map<wstring,
                        SortedSet<std::shared_ptr<ExternalDependency>>>();
  for (auto module : interModuleExternalCompileScopeDependencies) {
    set<std::shared_ptr<ExternalDependency>> deps =
        set<std::shared_ptr<ExternalDependency>>();
    sharedDependencies.emplace(module.first, deps);
    shared_ptr<Set<wstring>> moduleDependencies =
        interModuleExternalCompileScopeDependencies[module.first];
    if (nullptr != moduleDependencies) {
      for (auto otherArtifactId : moduleDependencies) {
        shared_ptr<SortedSet<std::shared_ptr<ExternalDependency>>>
            otherExtDeps = allExternalDependencies[otherArtifactId];
        if (nullptr != otherExtDeps) {
          for (auto otherDep : otherExtDeps) {
            if (!otherDep->isTestDependency) {
              deps.insert(otherDep);
            }
          }
        }
      }
    }
  }
  for (auto module : interModuleExternalTestScopeDependencies) {
    shared_ptr<SortedSet<std::shared_ptr<ExternalDependency>>> deps =
        sharedDependencies[module.first];
    if (nullptr == deps) {
      deps = set<>();
      sharedDependencies.emplace(module.first, deps);
    }
    shared_ptr<Set<wstring>> moduleDependencies =
        interModuleExternalTestScopeDependencies[module.first];
    if (nullptr != moduleDependencies) {
      for (auto otherArtifactId : moduleDependencies) {
        int testScopePos = otherArtifactId.find(L":test");
        bool isTestScope = false;
        if (-1 != testScopePos) {
          otherArtifactId = otherArtifactId.substr(0, testScopePos);
          isTestScope = true;
        }
        shared_ptr<SortedSet<std::shared_ptr<ExternalDependency>>>
            otherExtDeps = allExternalDependencies[otherArtifactId];
        if (nullptr != otherExtDeps) {
          for (auto otherDep : otherExtDeps) {
            if (otherDep->isTestDependency == isTestScope) {
              if (!deps->contains(otherDep) &&
                  (nullptr == allExternalDependencies[module.first] ||
                   !allExternalDependencies[module.first]->contains(
                       otherDep))) {
                // Add test-scope clone only if it's not already a compile-scope
                // dependency.
                shared_ptr<ExternalDependency> otherDepTestScope =
                    make_shared<ExternalDependency>(
                        otherDep->groupId, otherDep->artifactId,
                        otherDep->classifier, true, otherDep->isOptional);
                deps->add(otherDepTestScope);
              }
            }
          }
        }
      }
    }
  }
  for (auto module : sharedDependencies) {
    shared_ptr<SortedSet<std::shared_ptr<ExternalDependency>>> deps =
        allExternalDependencies[module.first];
    if (nullptr == deps) {
      deps = set<>();
      allExternalDependencies.emplace(module.first, deps);
    }
    for (auto dep : sharedDependencies[module.first]) {
      wstring dependencyCoordinate = dep->groupId + L":" + dep->artifactId;
      if (globalOptionalExternalDependencies->contains(dependencyCoordinate) ||
          (perModuleOptionalExternalDependencies.find(module.first) !=
               perModuleOptionalExternalDependencies.end() &&
           perModuleOptionalExternalDependencies[module.first]->contains(
               dependencyCoordinate))) {
        // make a copy of the dep and set optional=true
        dep = make_shared<ExternalDependency>(dep->groupId, dep->artifactId,
                                              dep->classifier,
                                              dep->isTestDependency, true);
      }
      deps->add(dep);
    }
  }
}

void GetMavenDependenciesTask::setExternalDependencyXmlProperties()
{
  for (auto module : internalCompileScopeDependencies) { // get full module deque
    shared_ptr<StringBuilder> compileScopeBuilder =
        make_shared<StringBuilder>();
    shared_ptr<StringBuilder> testScopeBuilder = make_shared<StringBuilder>();
    shared_ptr<SortedSet<std::shared_ptr<ExternalDependency>>> extDeps =
        allExternalDependencies[module.first];
    if (nullptr != extDeps) {
      for (auto dep : extDeps) {
        shared_ptr<StringBuilder> builder =
            dep->isTestDependency ? testScopeBuilder : compileScopeBuilder;
        appendDependencyXml(builder, dep->groupId, dep->artifactId, L"    ",
                            L"", dep->isTestDependency, dep->isOptional,
                            dep->classifier, nullptr);
        // Test POMs for solrj, solr-core, lucene-codecs and lucene-core modules
        // need to include all compile-scope dependencies as test-scope
        // dependencies since we've turned off transitive dependency resolution.
        if (!dep->isTestDependency &&
            modulesWithSeparateCompileAndTestPOMs->contains(module.first)) {
          appendDependencyXml(testScopeBuilder, dep->groupId, dep->artifactId,
                              L"    ", L"", true, dep->isOptional,
                              dep->classifier, nullptr);
        }
      }
    }
    if (compileScopeBuilder->length() > 0) {
      compileScopeBuilder->setLength(compileScopeBuilder->length() -
                                     1); // drop trailing newline
    }
    if (testScopeBuilder->length() > 0) {
      testScopeBuilder->setLength(testScopeBuilder->length() -
                                  1); // drop trailing newline
    }
    allProperties->setProperty(module.first + L".external.dependencies",
                               compileScopeBuilder->toString());
    allProperties->setProperty(module.first + L".external.test.dependencies",
                               testScopeBuilder->toString());
  }
}

void GetMavenDependenciesTask::setGrandparentDependencyManagementProperty()
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  appendAllInternalDependencies(builder);
  unordered_map<wstring, wstring> versionsMap =
      unordered_map<wstring, wstring>();
  appendAllExternalDependencies(builder, versionsMap);
  builder->setLength(builder->length() - 1); // drop trailing newline
  allProperties->setProperty(DEPENDENCY_MANAGEMENT_PROPERTY,
                             builder->toString());
  for (auto entry : versionsMap) {
    allProperties->setProperty(entry.first, entry.second);
  }
}

void GetMavenDependenciesTask::appendAllInternalDependencies(
    shared_ptr<StringBuilder> builder)
{
  for (auto artifactId : internalCompileScopeDependencies) {
    deque<wstring> exclusions = deque<wstring>();
    exclusions.addAll(internalCompileScopeDependencies->get(artifactId.first));
    shared_ptr<SortedSet<std::shared_ptr<ExternalDependency>>> extDeps =
        allExternalDependencies[artifactId.first];
    if (nullptr != extDeps) {
      for (auto externalDependency : extDeps) {
        if (!externalDependency->isTestDependency &&
            !externalDependency->isOptional) {
          exclusions.push_back(externalDependency->groupId +
                               StringHelper::toString(L':') +
                               externalDependency->artifactId);
        }
      }
    }
    wstring groupId = ivyModuleInfo[artifactId.first];
    appendDependencyXml(builder, groupId, artifactId.first, L"      ",
                        L"${project.version}", false, false, L"", exclusions);
  }
}

shared_ptr<File> GetMavenDependenciesTask::getIvyCacheDir()
{
  wstring ivyUserDirName = getProject().getUserProperty(IVY_USER_DIR_PROPERTY);
  if (L"" == ivyUserDirName) {
    ivyUserDirName = getProject().getProperty(IVY_USER_DIR_PROPERTY);
    if (L"" == ivyUserDirName) {
      ivyUserDirName = System::getProperty(L"user.home") +
                       System::getProperty(L"file.separator") + L".ivy2";
    }
  }
  shared_ptr<File> ivyUserDir = make_shared<File>(ivyUserDirName);
  if (!ivyUserDir->exists()) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(
        L"Ivy user dir does not exist: '" + ivyUserDir.getPath() + L"'");
  }
  shared_ptr<File> dir = make_shared<File>(ivyUserDir, L"cache");
  if (!dir->exists()) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(
        L"Ivy cache dir does not exist: '" + ivyCacheDir.getPath() + L"'");
  }
  return dir;
}

void GetMavenDependenciesTask::appendAllExternalDependencies(
    shared_ptr<StringBuilder> dependenciesBuilder,
    unordered_map<wstring, wstring> &versionsMap)
{
  log(L"Loading centralized ivy versions from: " + centralizedVersionsFile,
      verboseLevel);
  ivyCacheDir = getIvyCacheDir();
  shared_ptr<Properties> versions = make_shared<InterpolatedProperties>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream inputStream = new
  // java.io.FileInputStream(centralizedVersionsFile); java.io.Reader reader =
  // new java.io.InputStreamReader(inputStream,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::InputStream inputStream =
        java::io::FileInputStream(centralizedVersionsFile);
    java::io::Reader reader = java::io::InputStreamReader(
        inputStream, java::nio::charset::StandardCharsets::UTF_8);
    try {
      versions->load(reader);
    } catch (const IOException &e) {
      throw make_shared<BuildException>(
          L"Exception reading centralized versions file " +
              centralizedVersionsFile->getPath(),
          e);
    }
  }
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: java.util.SortedSet<java.util.Map.Entry<?,?>> sortedEntries
  // = new java.util.TreeSet<>(new
  // java.util.Comparator<java.util.Map.Entry<?,?>>()
  shared_ptr < SortedSet < unordered_map::Entry < ?,
      ? >>> sortedEntries = set < unordered_map::Entry < ?,
            ? >> (make_shared<ComparatorAnonymousInnerClass>(
                     shared_from_this()));
  sortedEntries->addAll(versions->entrySet());
  for (auto entry : sortedEntries) {
    wstring key = static_cast<wstring>(entry.first);
    shared_ptr<Matcher> matcher = COORDINATE_KEY_PATTERN->matcher(key);
    if (matcher->lookingAt()) {
      wstring groupId = matcher->group(1);
      wstring artifactId = matcher->group(2);
      wstring coordinate = groupId + StringHelper::toString(L':') + artifactId;
      wstring version = static_cast<wstring>(entry.second);
      versionsMap.emplace(coordinate + L".version", version);
      if (!nonJarDependencies->contains(coordinate)) {
        shared_ptr<Set<wstring>> classifiers =
            dependencyClassifiers[coordinate];
        if (nullptr != classifiers) {
          for (auto classifier : classifiers) {
            shared_ptr<deque<wstring>> exclusions =
                getTransitiveDependenciesFromIvyCache(groupId, artifactId,
                                                      version);
            appendDependencyXml(dependenciesBuilder, groupId, artifactId,
                                L"      ", version, false, false, classifier,
                                exclusions);
          }
        }
      }
    }
  }
}

GetMavenDependenciesTask::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<GetMavenDependenciesTask> outerInstance)
{
  this->outerInstance = outerInstance;
}

template <typename T1, typename T1, typename T2>
int GetMavenDependenciesTask::ComparatorAnonymousInnerClass::compare(
    unordered_map::Entry<T1> o1, unordered_map::Entry<T2> o2)
{
  return (static_cast<wstring>(o1.getKey()))
      ->compare(static_cast<wstring>(o2.getKey()));
}

shared_ptr<deque<wstring>>
GetMavenDependenciesTask::getTransitiveDependenciesFromIvyCache(
    const wstring &groupId, const wstring &artifactId, const wstring &version)
{
  shared_ptr<SortedSet<wstring>> transitiveDependencies = set<wstring>();
  //                                      E.g.
  //                                      ~/.ivy2/cache/xerces/xercesImpl/ivy-2.9.1.xml
  shared_ptr<File> ivyXmlFile = make_shared<File>(
      make_shared<File>(make_shared<File>(ivyCacheDir, groupId), artifactId),
      L"ivy-" + version + L".xml");
  if (!ivyXmlFile->exists()) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(L"File not found: " +
                                              ivyXmlFile.getPath());
  }
  try {
    shared_ptr<Document> document = documentBuilder->parse(ivyXmlFile);
    wstring dependencyPath = wstring(L"/ivy-module/dependencies/dependency") +
                             L"[   not(starts-with(@conf,'test->'))" +
                             L"and not(starts-with(@conf,'provided->'))" +
                             L"and not(starts-with(@conf,'optional->'))]";
    shared_ptr<NodeList> dependencies = std::static_pointer_cast<NodeList>(
        xpath->evaluate(dependencyPath, document, XPathConstants::NODESET));
    for (int i = 0; i < dependencies->getLength(); ++i) {
      shared_ptr<Element> dependency =
          std::static_pointer_cast<Element>(dependencies->item(i));
      transitiveDependencies->add(dependency->getAttribute(L"org") + L':' +
                                  dependency->getAttribute(L"name"));
    }
  } catch (const runtime_error &e) {
    throw make_shared<BuildException>(
        L"Exception collecting transitive dependencies for " + groupId +
            StringHelper::toString(L':') + artifactId +
            StringHelper::toString(L':') + version + L" from " +
            ivyXmlFile->getAbsolutePath(),
        e);
  }
  return transitiveDependencies;
}

void GetMavenDependenciesTask::setInternalDependencyProperties()
{
  log(L"Loading module dependencies from: " + moduleDependenciesPropertiesFile,
      verboseLevel);
  shared_ptr<Properties> moduleDependencies = make_shared<Properties>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream inputStream = new
  // java.io.FileInputStream(moduleDependenciesPropertiesFile); java.io.Reader
  // reader = new java.io.InputStreamReader(inputStream,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::InputStream inputStream =
        java::io::FileInputStream(moduleDependenciesPropertiesFile);
    java::io::Reader reader = java::io::InputStreamReader(
        inputStream, java::nio::charset::StandardCharsets::UTF_8);
    try {
      moduleDependencies->load(reader);
    } catch (const FileNotFoundException &e) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Properties file does not exist: " +
          moduleDependenciesPropertiesFile.getPath());
    } catch (const IOException &e) {
      throw make_shared<BuildException>(
          L"Exception reading properties file " +
              moduleDependenciesPropertiesFile->getPath(),
          e);
    }
  }
  unordered_map<wstring, SortedSet<wstring>> testScopeDependencies =
      unordered_map<wstring, SortedSet<wstring>>();
  unordered_map<wstring, wstring> testScopePropertyKeys =
      unordered_map<wstring, wstring>();
  for (auto entry : moduleDependencies) {
    wstring newPropertyKey = static_cast<wstring>(entry.first);
    shared_ptr<StringBuilder> newPropertyValue = make_shared<StringBuilder>();
    wstring value = static_cast<wstring>(entry.second);
    shared_ptr<Matcher> matcher =
        MODULE_DEPENDENCIES_COORDINATE_KEY_PATTERN->matcher(newPropertyKey);
    if (!matcher->matches()) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Malformed module dependencies property key: '" + newPropertyKey +
          L"'");
    }
    wstring antProjectName = matcher->group(1);
    bool isTest = nullptr != matcher->group(2);
    wstring artifactName = antProjectToArtifactName(antProjectName);
    newPropertyKey = artifactName +
                     (isTest ? L".internal.test" : L".internal") +
                     L".dependencies"; // Add ".internal"
    if (isTest) {
      testScopePropertyKeys.emplace(artifactName, newPropertyKey);
    }
    if (L"" == value || value.isEmpty()) {
      allProperties->setProperty(newPropertyKey, L"");
      unordered_map<wstring, SortedSet<wstring>> scopedDependencies =
          isTest ? testScopeDependencies : internalCompileScopeDependencies;
      scopedDependencies.emplace(artifactName, set<wstring>());
    } else {
      // Lucene analysis modules' build dirs do not include hyphens, but Solr
      // contribs' build dirs do
      wstring origModuleDir =
          StringHelper::replace(antProjectName, L"analyzers-", L"analysis/");
      // Exclude the module's own build output, in addition to
      // UNWANTED_INTERNAL_DEPENDENCIES
      shared_ptr<Pattern> unwantedInternalDependencies = Pattern::compile(
          L"(?:lucene/build/|solr/build/(?:contrib/)?)" + origModuleDir + L"/" +
          L"|" + UNWANTED_INTERNAL_DEPENDENCIES);
      shared_ptr<SortedSet<wstring>> sortedDeps = set<wstring>();
      for (wstring dependency : value.split(L",")) {
        matcher = SHARED_EXTERNAL_DEPENDENCIES_PATTERN->matcher(dependency);
        if (matcher->find()) {
          wstring otherArtifactName = matcher->group(1);
          bool isTestScope =
              nullptr != matcher->group(2) && matcher->group(2)->length() > 0;
          otherArtifactName =
              StringHelper::replace(otherArtifactName, L'/', L'-');
          otherArtifactName = StringHelper::replace(
              otherArtifactName, L"lucene-analysis", L"lucene-analyzers");
          otherArtifactName = StringHelper::replace(
              otherArtifactName, L"solr-contrib-solr-", L"solr-");
          otherArtifactName = StringHelper::replace(otherArtifactName,
                                                    L"solr-contrib-", L"solr-");
          if (otherArtifactName != artifactName) {
            unordered_map<wstring, Set<wstring>> sharedDeps =
                isTest ? interModuleExternalTestScopeDependencies
                       : interModuleExternalCompileScopeDependencies;
            shared_ptr<Set<wstring>> sharedSet = sharedDeps[artifactName];
            if (nullptr == sharedSet) {
              sharedSet = unordered_set<>();
              sharedDeps.emplace(artifactName, sharedSet);
            }
            if (isTestScope) {
              otherArtifactName += L":test";
            }
            sharedSet->add(otherArtifactName);
          }
        }
        matcher = unwantedInternalDependencies->matcher(dependency);
        if (matcher->find()) {
          continue; // skip external (/(test-)lib/), and non-jar and unwanted
                    // (self) internal deps
        }
        wstring artifactId = dependencyToArtifactId(newPropertyKey, dependency);
        wstring groupId = ivyModuleInfo[artifactId];
        wstring coordinate =
            groupId + StringHelper::toString(L':') + artifactId;
        sortedDeps->add(coordinate);
      }
      if (isTest) { // Don't set test-scope properties until all compile-scope
                    // deps have been seen
        testScopeDependencies.emplace(artifactName, sortedDeps);
      } else {
        internalCompileScopeDependencies->put(artifactName, sortedDeps);
        for (auto dependency : sortedDeps) {
          int splitPos = dependency.find(L':');
          wstring groupId = dependency.substr(0, splitPos);
          wstring artifactId = dependency.substr(splitPos + 1);
          appendDependencyXml(newPropertyValue, groupId, artifactId, L"    ",
                              L"", false, false, L"", nullptr);
        }
        if (newPropertyValue->length() > 0) {
          newPropertyValue->setLength(newPropertyValue->length() -
                                      1); // drop trailing newline
        }
        allProperties->setProperty(newPropertyKey,
                                   newPropertyValue->toString());
      }
    }
  }
  // Now that all compile-scope dependencies have been seen, include only those
  // test-scope dependencies that are not also compile-scope dependencies.
  for (auto entry : testScopeDependencies) {
    wstring module = entry.first;
    shared_ptr<SortedSet<wstring>> testDeps = entry.second;
    shared_ptr<SortedSet<wstring>> compileDeps =
        internalCompileScopeDependencies->get(module);
    if (nullptr == compileDeps) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Can't find compile scope dependencies for module " + module);
    }
    shared_ptr<StringBuilder> newPropertyValue = make_shared<StringBuilder>();
    for (auto dependency : testDeps) {
      // modules with separate compile-scope and test-scope POMs need their
      // compile-scope deps included in their test-scope deps.
      if (modulesWithSeparateCompileAndTestPOMs->contains(module) ||
          !compileDeps->contains(dependency)) {
        int splitPos = dependency.find(L':');
        wstring groupId = dependency.substr(0, splitPos);
        wstring artifactId = dependency.substr(splitPos + 1);
        appendDependencyXml(newPropertyValue, groupId, artifactId, L"    ", L"",
                            true, false, L"", nullptr);
      }
    }
    if (newPropertyValue->length() > 0) {
      newPropertyValue->setLength(newPropertyValue->length() -
                                  1); // drop trailing newline
    }
    allProperties->setProperty(testScopePropertyKeys[module],
                               newPropertyValue->toString());
  }
}

wstring
GetMavenDependenciesTask::dependencyToArtifactId(const wstring &newPropertyKey,
                                                 const wstring &dependency)
{
  shared_ptr<StringBuilder> artifactId = make_shared<StringBuilder>();
  shared_ptr<Matcher> matcher =
      COMPILATION_OUTPUT_DIRECTORY_PATTERN->matcher(dependency);
  if (matcher->matches()) {
    // Pattern.compile("(lucene|solr)/build/(.*)/classes/java");
    wstring artifact = matcher->group(2);
    artifact = StringHelper::replace(artifact, L'/', L'-');
    artifact = artifact.replaceAll(L"(?<!solr-)analysis-", L"analyzers-");
    if (L"lucene" == matcher->group(1)) {
      artifactId->append(L"lucene-");
    }
    artifactId->append(artifact);
  } else {
    matcher = internalJarPattern->matcher(dependency);
    if (matcher->matches()) {
      // internalJarPattern is /.*(lucene|solr)([^/]*?)-<version>\.jar/,
      // where <version> is the value of the Ant "version" property
      artifactId->append(matcher->group(1));
      artifactId->append(matcher->group(2));
    } else {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Malformed module dependency from '" + newPropertyKey + L"': '" +
          dependency + L"'");
    }
  }
  return artifactId->toString();
}

wstring
GetMavenDependenciesTask::antProjectToArtifactName(const wstring &origModule)
{
  wstring module = origModule;
  if (!StringHelper::startsWith(
          origModule,
          L"solr-")) { // lucene modules names don't have "lucene-" prepended
    module = L"lucene-" + module;
  }
  return module;
}

void GetMavenDependenciesTask::collectExternalDependenciesFromIvyXmlFile(
    shared_ptr<File> ivyXmlFile) throw(XPathExpressionException, IOException,
                                       SAXException)
{
  wstring module = getModuleName(ivyXmlFile);
  log(L"Collecting external dependencies from: " + ivyXmlFile->getPath(),
      verboseLevel);
  shared_ptr<Document> document = documentBuilder->parse(ivyXmlFile);
  // Exclude the 'start' configuration in solr/server/ivy.xml
  wstring dependencyPath =
      L"/ivy-module/dependencies/dependency[not(starts-with(@conf,'start'))]";
  shared_ptr<NodeList> dependencies = std::static_pointer_cast<NodeList>(
      xpath->evaluate(dependencyPath, document, XPathConstants::NODESET));
  for (int depNum = 0; depNum < dependencies->getLength(); ++depNum) {
    shared_ptr<Element> dependency =
        std::static_pointer_cast<Element>(dependencies->item(depNum));
    wstring groupId = dependency->getAttribute(L"org");
    wstring artifactId = dependency->getAttribute(L"name");
    wstring dependencyCoordinate =
        groupId + StringHelper::toString(L':') + artifactId;
    shared_ptr<Set<wstring>> classifiers =
        dependencyClassifiers[dependencyCoordinate];
    if (nullptr == classifiers) {
      classifiers = unordered_set<>();
      dependencyClassifiers.emplace(dependencyCoordinate, classifiers);
    }
    wstring conf = dependency->getAttribute(L"conf");
    bool confContainsTest = conf.find(L"test") != wstring::npos;
    bool isOptional =
        globalOptionalExternalDependencies->contains(dependencyCoordinate) ||
        (perModuleOptionalExternalDependencies.find(module) !=
             perModuleOptionalExternalDependencies.end() &&
         perModuleOptionalExternalDependencies[module]->contains(
             dependencyCoordinate));
    shared_ptr<SortedSet<std::shared_ptr<ExternalDependency>>> deps =
        allExternalDependencies[module];
    if (nullptr == deps) {
      deps = set<>();
      allExternalDependencies.emplace(module, deps);
    }
    shared_ptr<NodeList> artifacts = nullptr;
    if (dependency->hasChildNodes()) {
      artifacts = std::static_pointer_cast<NodeList>(
          xpath->evaluate(L"artifact", dependency, XPathConstants::NODESET));
    }
    if (nullptr != artifacts && artifacts->getLength() > 0) {
      for (int artifactNum = 0; artifactNum < artifacts->getLength();
           ++artifactNum) {
        shared_ptr<Element> artifact =
            std::static_pointer_cast<Element>(artifacts->item(artifactNum));
        wstring type = artifact->getAttribute(L"type");
        wstring ext = artifact->getAttribute(L"ext");
        // When conf contains BOTH "test" and "compile", and type != "test",
        // this is NOT a test dependency
        bool isTestDependency =
            confContainsTest &&
            (type == L"test" || !conf.find(L"compile") != wstring::npos);
        if ((type.isEmpty() && ext.isEmpty()) || type == L"jar" ||
            ext == L"jar") {
          wstring classifier = artifact->getAttribute(L"maven:classifier");
          if (classifier.isEmpty()) {
            classifier = L"";
          }
          classifiers->add(classifier);
          deps->add(make_shared<ExternalDependency>(
              groupId, artifactId, classifier, isTestDependency, isOptional));
        } else { // not a jar
          nonJarDependencies->add(dependencyCoordinate);
        }
      }
    } else {
      classifiers->add(nullptr);
      deps->add(make_shared<ExternalDependency>(groupId, artifactId, nullptr,
                                                confContainsTest, isOptional));
    }
  }
}

GetMavenDependenciesTask::ExternalDependency::ExternalDependency(
    const wstring &groupId, const wstring &artifactId,
    const wstring &classifier, bool isTestDependency, bool isOptional)
{
  this->groupId = groupId;
  this->artifactId = artifactId;
  this->classifier = classifier;
  this->isTestDependency = isTestDependency;
  this->isOptional = isOptional;
}

bool GetMavenDependenciesTask::ExternalDependency::equals(any o)
{
  if (!(std::dynamic_pointer_cast<ExternalDependency>(o) != nullptr)) {
    return false;
  }
  shared_ptr<ExternalDependency> other =
      any_cast<std::shared_ptr<ExternalDependency>>(o);
  return groupId == other->groupId && artifactId == other->artifactId &&
         isTestDependency == other->isTestDependency &&
         isOptional == other->isOptional && classifier == other->classifier;
}

int GetMavenDependenciesTask::ExternalDependency::hashCode()
{
  return groupId.hashCode() * 31 + artifactId.hashCode() * 31 +
         (isTestDependency ? 31 : 0) + (isOptional ? 31 : 0) +
         classifier.hashCode();
}

int GetMavenDependenciesTask::ExternalDependency::compareTo(
    shared_ptr<ExternalDependency> other)
{
  int comparison = groupId.compare(other->groupId);
  if (0 != comparison) {
    return comparison;
  }
  comparison = artifactId.compare(other->artifactId);
  if (0 != comparison) {
    return comparison;
  }
  if (L"" == classifier) {
    if (L"" != other->classifier) {
      return -1;
    }
  } else if (L"" == other->classifier) { // classifier is not null
    return 1;
  } else { // neither classifier is  null
    if (0 != (comparison = classifier.compare(other->classifier))) {
      return comparison;
    }
  }
  // test and optional don't matter in this sort
  return 0;
}

wstring GetMavenDependenciesTask::getModuleName(shared_ptr<File> ivyXmlFile)
{
  wstring path = ivyXmlFile->getAbsolutePath();
  shared_ptr<Matcher> matcher =
      PROPERTY_PREFIX_FROM_IVY_XML_FILE_PATTERN->matcher(path);
  if (!matcher->find()) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(
        L"Can't get module name from ivy.xml path: " + path);
  }
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  builder->append(matcher->group(1));
  if (nullptr != matcher->group(2)) { // "lucene/analysis/..."
    builder->append(L"-analyzers");
  } else if (nullptr != matcher->group(3)) { // "solr/example/..."
    builder->append(L"-example");
  } else if (nullptr != matcher->group(4)) { // "solr/server/..."
    builder->append(L"-server");
  }
  builder->append(L'-');
  builder->append(matcher->group(5));
  return builder->toString()->replace(L"solr-solr-", L"solr-");
}

void GetMavenDependenciesTask::appendDependencyXml(
    shared_ptr<StringBuilder> builder, const wstring &groupId,
    const wstring &artifactId, const wstring &indent, const wstring &version,
    bool isTestDependency, bool isOptional, const wstring &classifier,
    shared_ptr<deque<wstring>> exclusions)
{
  builder->append(indent)->append(L"<dependency>\n");
  builder->append(indent)
      ->append(L"  <groupId>")
      ->append(groupId)
      ->append(L"</groupId>\n");
  builder->append(indent)
      ->append(L"  <artifactId>")
      ->append(artifactId)
      ->append(L"</artifactId>\n");
  if (L"" != version) {
    builder->append(indent)
        ->append(L"  <version>")
        ->append(version)
        ->append(L"</version>\n");
  }
  if (isTestDependency) {
    builder->append(indent)->append(L"  <scope>test</scope>\n");
  }
  if (isOptional) {
    builder->append(indent)->append(L"  <optional>true</optional>\n");
  }
  if (L"" != classifier) {
    builder->append(indent)
        ->append(L"  <classifier>")
        ->append(classifier)
        ->append(L"</classifier>\n");
  }
  if (nullptr != exclusions && !exclusions->isEmpty()) {
    builder->append(indent)->append(L"  <exclusions>\n");
    for (auto dependency : exclusions) {
      int splitPos = dependency.find(L':');
      wstring excludedGroupId = dependency.substr(0, splitPos);
      wstring excludedArtifactId = dependency.substr(splitPos + 1);
      builder->append(indent)->append(L"    <exclusion>\n");
      builder->append(indent)
          ->append(L"      <groupId>")
          ->append(excludedGroupId)
          ->append(L"</groupId>\n");
      builder->append(indent)
          ->append(L"      <artifactId>")
          ->append(excludedArtifactId)
          ->append(L"</artifactId>\n");
      builder->append(indent)->append(L"    </exclusion>\n");
    }
    builder->append(indent)->append(L"  </exclusions>\n");
  }
  builder->append(indent)->append(L"</dependency>\n");
}
} // namespace org::apache::lucene::dependencies