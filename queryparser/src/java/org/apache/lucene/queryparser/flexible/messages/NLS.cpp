using namespace std;

#include "NLS.h"

namespace org::apache::lucene::queryparser::flexible::messages
{

unordered_map<wstring, type_info> NLS::bundles =
    unordered_map<wstring, type_info>(0);

NLS::NLS()
{
  // Do not instantiate
}

wstring NLS::getLocalizedMessage(const wstring &key)
{
  return getLocalizedMessage(key, Locale::getDefault());
}

wstring NLS::getLocalizedMessage(const wstring &key, shared_ptr<Locale> locale)
{
  any message = getResourceBundleObject(key, locale);
  if (message == nullptr) {
    return L"Message with key:" + key + L" and locale: " + locale +
           L" not found.";
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return message.toString();
}

wstring NLS::getLocalizedMessage(const wstring &key, shared_ptr<Locale> locale,
                                 deque<any> &args)
{
  wstring str = getLocalizedMessage(key, locale);

  if (args->length > 0) {
    str = (make_shared<MessageFormat>(str, Locale::ROOT))->format(args);
  }

  return str;
}

wstring NLS::getLocalizedMessage(const wstring &key, deque<any> &args)
{
  return getLocalizedMessage(key, Locale::getDefault(), args);
}

void NLS::initializeMessages(const wstring &bundleName, type_info clazz)
{
  try {
    load(clazz);
    if (bundles.find(bundleName) == bundles.end()) {
      bundles.emplace(bundleName, clazz);
    }
  } catch (const runtime_error &e) {
    // ignore all errors and exceptions
    // because this function is supposed to be called at class load time.
  }
}

any NLS::getResourceBundleObject(const wstring &messageKey,
                                 shared_ptr<Locale> locale)
{

  // slow resource checking
  // need to loop thru all registered resource bundles
  for (shared_ptr<Iterator<wstring>> it = bundles.keySet().begin();
       it->hasNext();) {
    type_info clazz = bundles[it->next()];
    shared_ptr<ResourceBundle> resourceBundle =
        ResourceBundle::getBundle(clazz.getName(), locale);
    if (resourceBundle != nullptr) {
      try {
        any obj = resourceBundle->getObject(messageKey);
        if (obj != nullptr) {
          return obj;
        }
      } catch (const MissingResourceException &e) {
        // just continue it might be on the next resource bundle
      }
    }
  }
  // if resource is not found
  return nullptr;
}

void NLS::load(type_info clazz)
{
  std::deque<std::shared_ptr<Field>> fieldArray = clazz.getDeclaredFields();

  // build a map_obj of field names to Field objects
  constexpr int len = fieldArray.size();
  unordered_map<wstring, std::shared_ptr<Field>> fields =
      unordered_map<wstring, std::shared_ptr<Field>>(len * 2);
  for (int i = 0; i < len; i++) {
    fields.emplace(fieldArray[i]->getName(), fieldArray[i]);
    loadfieldValue(fieldArray[i], clazz);
  }
}

void NLS::loadfieldValue(shared_ptr<Field> field, type_info clazz)
{
  int MOD_EXPECTED = Modifier::PUBLIC | Modifier::STATIC;
  int MOD_MASK = MOD_EXPECTED | Modifier::FINAL;
  if ((field->getModifiers() & MOD_MASK) != MOD_EXPECTED) {
    return;
  }

  // Set a value for this empty field.
  try {
    field->set(nullptr, field->getName());
    validateMessage(field->getName(), clazz);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (IllegalArgumentException | IllegalAccessException e) {
    // should not happen
  }
}

void NLS::validateMessage(const wstring &key, type_info clazz)
{
  // Test if the message is present in the resource bundle
  try {
    shared_ptr<ResourceBundle> resourceBundle =
        ResourceBundle::getBundle(clazz.getName(), Locale::getDefault());
    if (resourceBundle != nullptr) {
      any obj = resourceBundle->getObject(key);
      // if (obj == null)
      //  System.err.println("WARN: Message with key:" + key + " and locale: "
      //      + Locale.getDefault() + " not found.");
    }
  } catch (const MissingResourceException &e) {
    // System.err.println("WARN: Message with key:" + key + " and locale: "
    //    + Locale.getDefault() + " not found.");
  } catch (const runtime_error &e) {
    // ignore all other errors and exceptions
    // since this code is just a test to see if the message is present on the
    // system
  }
}
} // namespace org::apache::lucene::queryparser::flexible::messages