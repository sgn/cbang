/******************************************************************************\

          This file is part of the C! library.  A.K.A the cbang library.

                Copyright (c) 2003-2019, Cauldron Development LLC
                   Copyright (c) 2003-2017, Stanford University
                               All rights reserved.

         The C! library is free software: you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public License
       as published by the Free Software Foundation, either version 2.1 of
               the License, or (at your option) any later version.

        The C! library is distributed in the hope that it will be useful,
          but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
                 Lesser General Public License for more details.

         You should have received a copy of the GNU Lesser General Public
                 License along with the C! library.  If not, see
                         <http://www.gnu.org/licenses/>.

        In addition, BSD licensing may be granted on a case by case basis
        by written permission from at least one of the copyright holders.
           You may request written permission by emailing the authors.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

#include "JSONWebsocket.h"

#include <cbang/Catch.h>
#include <cbang/iostream/VectorDevice.h>


using namespace cb;
using namespace cb::Event;
using namespace std;


namespace {
  struct JSONWriter : vector<char>, cb::VectorStream<>, public JSON::Writer {
    SmartPointer<Websocket> ws;

    JSONWriter(const SmartPointer<Websocket> &ws) :
      cb::VectorStream<>((vector<char> &)*this),
      JSON::Writer((ostream &)*this), ws(ws) {}

    ~JSONWriter() {TRY_CATCH_ERROR(close(););}

    void close() {
      JSON::Writer::close();
      ws->send(data(), size());
    }
  };
}


void JSONWebsocket::send(const JSON::Value &value) {send(value.toString());}


SmartPointer<JSON::Writer> JSONWebsocket::getJSONWriter() {
  return new JSONWriter(this);
}


void JSONWebsocket::onMessage(const JSON::ValuePtr &msg) {if (cb) cb(msg);}


void JSONWebsocket::onMessage(const char *data, uint64_t length) {
  onMessage(JSON::Reader::parse(InputSource(data, length)));
}
