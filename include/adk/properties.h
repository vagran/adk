/* /ADK/include/adk/properties.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file properties.h
 * Properties are helper layer for managing small structured data in form of
 * key, value and some additional attributes. They are suitable for storing and
 * accessing configuration, some program objects properties exposed to the user
 * interface etc. They can be loaded/stored from XML, dynamically displayed in
 * GTK+ windows with a possibility to be edited by user, easily accessed via
 * simple API. Each represented entry can have validator attached. Default
 * validators check the correspondence of the value to one of several built-in
 * types. It is possible to subscribe for value change notifications via signal
 * associated with each entry. Transactions are supported which allow to
 * validate one atomic batch of changes.
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

namespace adk {

class Properties {

};

} /* namespace adk */

#endif /* PROPERTIES_H_ */
