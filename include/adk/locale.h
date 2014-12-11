/* This file is a part of ADK library.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file locale.h
 * Locale-related utilities.
 */

#ifndef LOCALE_H_
#define LOCALE_H_

namespace adk {

/** Helper class for temporal locale change. */
class LocaleGuard {
public:
    LocaleGuard(int category = LC_ALL, const char *locale = "C"):
        category(category)
    {
        saved_locale = std::setlocale(category, locale);
    }

    ~LocaleGuard()
    {
        std::setlocale(category, saved_locale);
    }

private:
    int category;
    const char *saved_locale;
};

} /* namespace adk */

#endif /* LOCALE_H_ */
