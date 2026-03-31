#ifndef DICTIONARYPROVIDER_H
#define DICTIONARYPROVIDER_H

#include <QMap>
#include <QString>
#include <mutex>
#include <nuspell/dictionary.hxx>

class DictionaryProvider
{
public:
    static DictionaryProvider& instance();
    std::shared_ptr<nuspell::Dictionary> getDictionary(const QString& langCode);

private:
    DictionaryProvider() = default;
    QMap<QString, std::shared_ptr<nuspell::Dictionary>> cache;
    std::mutex mutex;
};

#endif // DICTIONARYPROVIDER_H
