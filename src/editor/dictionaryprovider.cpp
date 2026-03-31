#include "dictionaryprovider.h"
#include <nuspell/finder.hxx>
#include <qdebug.h>

DictionaryProvider &DictionaryProvider::instance()
{
    static DictionaryProvider provider;
    return provider;
}

std::shared_ptr<nuspell::Dictionary> DictionaryProvider::getDictionary(const QString &langCode)
{
    std::lock_guard<std::mutex> lock(mutex);

    if(cache.contains(langCode))
        return cache[langCode];

    std::string path = nuspell::search_dirs_for_one_dict({"/usr/share/hunspell"}, langCode.toStdString());

    if(path.empty())
        return nullptr;

    try
    {
        nuspell::Dictionary dict;
        auto sharedDict = std::make_shared<nuspell::Dictionary>(std::move(dict));
        cache[langCode] = sharedDict;
        return sharedDict;
    }
    catch (...)
    {
        return nullptr;
    }
}
