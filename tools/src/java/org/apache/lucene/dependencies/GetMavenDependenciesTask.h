#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::dependencies
{
class GetMavenDependenciesTask;
class ExternalDependency;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::dependencies
{

using org::apache::tools::ant::BuildException;
using org::apache::tools::ant::Project;
using org::apache::tools::ant::Task;
using org::apache::tools::ant::types::ResourceCollection;
using org::apache::tools::ant::types::resources::Resources;
using org::xml::sax::SAXException;
  class ExternalDependency
      : public std::enable_shared_from_this<ExternalDependency>,
        public Comparable<std::shared_ptr<ExternalDependency>>
  {
    GET_CLASS_NAME(ExternalDependency)
  public:
    std::wstring groupId;
    std::wstring artifactId;
    bool isTestDependency = false;
    bool isOptional = false;
    std::wstring classifier;

    ExternalDependency(const std::wstring &groupId,
                       const std::wstring &artifactId,
                       const std::wstring &classifier, bool isTestDependency,
                       bool isOptional);

    virtual bool equals(std::any o);

    virtual int hashCode();

    int compareTo(std::shared_ptr<ExternalDependency> other) override;
  };

/**
 * An Ant task to generate a properties file containing maven dependency
 * declarations, used to filter the maven POMs when copying them to
 * maven-build/ via 'ant get-maven-poms', or to lucene/build/poms/
 * via the '-filter-maven-poms' target, which is called from the
 * 'generate-maven-artifacts' target.
 */
class GetMavenDependenciesTask : public Task
{
  GET_CLASS_NAME(GetMavenDependenciesTask)
private:
  static const std::shared_ptr<Pattern>
      PROPERTY_PREFIX_FROM_IVY_XML_FILE_PATTERN;
  static const std::shared_ptr<Pattern> COORDINATE_KEY_PATTERN;
  static const std::shared_ptr<Pattern>
      MODULE_DEPENDENCIES_COORDINATE_KEY_PATTERN;
  // lucene/build/core/classes/java
  static const std::shared_ptr<Pattern> COMPILATION_OUTPUT_DIRECTORY_PATTERN;
  static const std::wstring UNWANTED_INTERNAL_DEPENDENCIES;
  static const std::shared_ptr<Pattern> SHARED_EXTERNAL_DEPENDENCIES_PATTERN;

  static const std::wstring DEPENDENCY_MANAGEMENT_PROPERTY;
  static const std::wstring IVY_USER_DIR_PROPERTY;
  static const std::shared_ptr<Properties> allProperties;
  static const std::shared_ptr<Set<std::wstring>>
      modulesWithSeparateCompileAndTestPOMs;

  static const std::shared_ptr<Set<std::wstring>>
      globalOptionalExternalDependencies;
  static const std::unordered_map<std::wstring, Set<std::wstring>>
      perModuleOptionalExternalDependencies;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static GetMavenDependenciesTask::StaticConstructor staticConstructor;

  const std::shared_ptr<XPath> xpath = XPathFactory::newInstance().newXPath();
  const std::shared_ptr<SortedMap<std::wstring, SortedSet<std::wstring>>>
      internalCompileScopeDependencies =
          std::map_obj<std::wstring, SortedSet<std::wstring>>();
  const std::shared_ptr<Set<std::wstring>> nonJarDependencies =
      std::unordered_set<std::wstring>();
  const std::unordered_map<std::wstring, Set<std::wstring>>
      dependencyClassifiers =
          std::unordered_map<std::wstring, Set<std::wstring>>();
  const std::unordered_map<std::wstring, Set<std::wstring>>
      interModuleExternalCompileScopeDependencies =
          std::unordered_map<std::wstring, Set<std::wstring>>();
  const std::unordered_map<std::wstring, Set<std::wstring>>
      interModuleExternalTestScopeDependencies =
          std::unordered_map<std::wstring, Set<std::wstring>>();
  const std::unordered_map<std::wstring,
                           SortedSet<std::shared_ptr<ExternalDependency>>>
      allExternalDependencies =
          std::unordered_map<std::wstring,
                             SortedSet<std::shared_ptr<ExternalDependency>>>();
  const std::shared_ptr<DocumentBuilder> documentBuilder;
  std::shared_ptr<File> ivyCacheDir;
  std::shared_ptr<Pattern> internalJarPattern;
  std::unordered_map<std::wstring, std::wstring> ivyModuleInfo;

  /**
   * All ivy.xml files to get external dependencies from.
   */
  std::shared_ptr<Resources> ivyXmlResources = std::make_shared<Resources>();

  /**
   * Centralized Ivy versions properties file
   */
  std::shared_ptr<File> centralizedVersionsFile;

  /**
   * Module dependencies properties file, generated by task
   * -append-module-dependencies-properties.
   */
  std::shared_ptr<File> moduleDependenciesPropertiesFile;

  /**
   * Where all properties are written, to be used to filter POM templates when
   * copying them.
   */
  std::shared_ptr<File> mavenDependenciesFiltersFile;

  /**
   * A logging level associated with verbose logging.
   */
  int verboseLevel = Project::MSG_VERBOSE;

  /**
   * Adds a set of ivy.xml resources to check.
   */
public:
  virtual void add(std::shared_ptr<ResourceCollection> rc);

  virtual void setVerbose(bool verbose);

  virtual void setCentralizedVersionsFile(std::shared_ptr<File> file);

  virtual void setModuleDependenciesPropertiesFile(std::shared_ptr<File> file);

  virtual void setMavenDependenciesFiltersFile(std::shared_ptr<File> file);

  GetMavenDependenciesTask();

  /**
   * Collect dependency information from Ant build.xml and ivy.xml files
   * and from ivy-versions.properties, then write out an Ant filters file
   * to be used when copying POMs.
   */
  void execute()  override;

  /**
   * Write out an Ant filters file to be used when copying POMs.
   */
private:
  void writeFiltersFile();

  /**
   * Visits all ivy.xml files and collects module and organisation attributes
   * into a map_obj.
   */
  static std::unordered_map<std::wstring, std::wstring>
  getIvyModuleInfo(std::shared_ptr<Resources> ivyXmlResources,
                   std::shared_ptr<DocumentBuilder> documentBuilder,
                   std::shared_ptr<XPath> xpath);

  /**
   * Collects external dependencies from each ivy.xml file and sets
   * external dependency properties to be inserted into modules' POMs.
   */
  void setExternalDependencyProperties();

  static void
  traverseIvyXmlResources(std::shared_ptr<Resources> ivyXmlResources,
                          std::function<void(File *)> &ivyXmlFileConsumer);

  /**
   * For each module that includes other modules' external dependencies via
   * including all files under their ".../lib/" dirs in their (test.)classpath,
   * add the other modules' dependencies to its set of external dependencies.
   */
  void addSharedExternalDependencies();

  /**
   * For each module, sets a compile-scope and a test-scope property
   * with values that contain the appropriate &lt;dependency&gt;
   * snippets.
   */
  void setExternalDependencyXmlProperties();

  /**
   * Sets the property to be inserted into the grandparent POM's
   * &lt;dependencyManagement&gt; section.
   */
  void setGrandparentDependencyManagementProperty();

  /**
   * For each artifact in the project, append a dependency with version
   * ${project.version} to the grandparent POM's &lt;dependencyManagement&gt;
   * section.  An &lt;exclusion&gt; is added for each of the artifact's
   * dependencies.
   */
  void appendAllInternalDependencies(std::shared_ptr<StringBuilder> builder);

  /**
   * Sets the ivyCacheDir field, to either the ${ivy.default.ivy.user.dir}
   * property, or if that's not set, to the default ~/.ivy2/.
   */
  std::shared_ptr<File> getIvyCacheDir();

  /**
   * Append each dependency listed in the centralized Ivy versions file
   * to the grandparent POM's &lt;dependencyManagement&gt; section.
   * An &lt;exclusion&gt; is added for each of the artifact's dependencies,
   * which are collected from the artifact's ivy.xml from the Ivy cache.
   *
   * Also add a version property for each dependency.
   */
  void appendAllExternalDependencies(
      std::shared_ptr<StringBuilder> dependenciesBuilder,
      std::unordered_map<std::wstring, std::wstring> &versionsMap);

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private static class ComparatorAnonymousInnerClass
  // implements java.util.Comparator<java.util.Map.Entry<?,?>>
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator < std::unordered_map::Entry <
        ?,
        ? >>
  {
  private:
    std::shared_ptr<GetMavenDependenciesTask> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<GetMavenDependenciesTask> outerInstance);

    template <typename T1, typename T1, typename T2>
    int compare(std::unordered_map::Entry<T1> o1,
                std::unordered_map::Entry<T2> o2);
  };

  /**
   * Collect transitive compile-scope dependencies for the given artifact's
   * ivy.xml from the Ivy cache, using the default ivy pattern
   * "[organisation]/[module]/ivy-[revision].xml".  See
   * <a
   * href="http://ant.apache.org/ivy/history/latest-milestone/settings/caches.html"
   * >the Ivy cache documentation</a>.
   */
private:
  std::shared_ptr<std::deque<std::wstring>>
  getTransitiveDependenciesFromIvyCache(const std::wstring &groupId,
                                        const std::wstring &artifactId,
                                        const std::wstring &version);

  /**
   * Sets the internal dependencies compile and test properties to be inserted
   * into modules' POMs.
   *
   * Also collects shared external dependencies,
   * e.g. solr-core wants all of solrj's external dependencies
   */
  void setInternalDependencyProperties();

  /**
   * Converts either a compile output directory or an internal jar
   * dependency, taken from an Ant (test.)classpath, into an artifactId
   */
  std::wstring dependencyToArtifactId(const std::wstring &newPropertyKey,
                                      const std::wstring &dependency);

  /**
   * Convert Ant project names to artifact names: prepend "lucene-"
   * to Lucene project names
   */
  std::wstring antProjectToArtifactName(const std::wstring &origModule);

  /**
   * Collect external dependencies from the given ivy.xml file, constructing
   * property values containing &lt;dependency&gt; snippets, which will be
   * filtered (substituted) when copying the POM for the module corresponding
   * to the given ivy.xml file.
   */
  void collectExternalDependenciesFromIvyXmlFile(
      std::shared_ptr<File> ivyXmlFile) throw(XPathExpressionException,
                                              IOException, SAXException);

  /**
   * Stores information about an external dependency
   */
private:
  /**
   * Extract module name from ivy.xml path.
   */
private:
  std::wstring getModuleName(std::shared_ptr<File> ivyXmlFile);

  /**
   * Appends a &lt;dependency&gt; snippet to the given builder.
   */
  void appendDependencyXml(
      std::shared_ptr<StringBuilder> builder, const std::wstring &groupId,
      const std::wstring &artifactId, const std::wstring &indent,
      const std::wstring &version, bool isTestDependency, bool isOptional,
      const std::wstring &classifier,
      std::shared_ptr<std::deque<std::wstring>> exclusions);

protected:
  std::shared_ptr<GetMavenDependenciesTask> shared_from_this()
  {
    return std::static_pointer_cast<GetMavenDependenciesTask>(
        org.apache.tools.ant.Task::shared_from_this());
  }
};

} // namespace org::apache::lucene::dependencies
