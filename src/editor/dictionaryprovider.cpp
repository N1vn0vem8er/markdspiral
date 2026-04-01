#include "dictionaryprovider.h"
#include <nuspell/finder.hxx>
#include <QFileInfo>
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
        dict.load_aff_dic(path);
        auto sharedDict = std::make_shared<nuspell::Dictionary>(std::move(dict));
        cache[langCode] = sharedDict;
        return sharedDict;
    }
    catch (...)
    {
        return nullptr;
    }
}

QList<QString> DictionaryProvider::getLanguages()
{
    if(!languages.isEmpty())
        return languages;
    const auto dicts = nuspell::search_default_dirs_for_dicts();
    for(const auto& i : dicts)
    {
        languages.append(QFileInfo(i).baseName());
    }
    return languages;
}
