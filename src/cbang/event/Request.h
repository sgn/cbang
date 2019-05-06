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

#pragma once

#include "Enum.h"
#include "Headers.h"
#include "Buffer.h"

#include <cbang/SmartPointer.h>
#include <cbang/util/Version.h>
#include <cbang/util/Base.h>
#include <cbang/net/IPAddress.h>
#include <cbang/net/URI.h>
#include <cbang/net/Session.h>
#include <cbang/json/JSON.h>

#include <string>
#include <iostream>
#include <typeinfo>


namespace cb {
  class URI;
  class IPAddress;
  class SSL;

  namespace Event {
    class Connection;

    class Request : protected SmartPointer<Request>::SelfRef, public Enum {
      friend class SelfRefCounter;

      Headers inputHeaders;
      Headers outputHeaders;

      Buffer inputBuffer;
      Buffer outputBuffer;

      RequestMethod method;
      URI originalURI;
      URI uri;
      Version version;
      HTTPStatus responseCode;
      std::string responseCodeLine;

      SmartPointer<Connection> connection;
      ConnectionError connError;

      SmartPointer<Session> session;
      std::string user = "anonymous";

      bool chunked = false;
      bool replying = false;

      uint64_t bytesRead = 0;
      uint64_t bytesWritten = 0;

      JSON::Dict args;

    public:
      Request(RequestMethod method = RequestMethod(), const URI &uri = URI(),
              const Version &version = Version(1, 1));
      virtual ~Request();

      template <class T>
      T &cast() {
        T *ptr = dynamic_cast<T *>(this);
        if (!ptr) THROW("Cannot cast Request to " << typeid(T).name());
        return *ptr;
      }

      uint64_t getID() const;

      const Headers &getInputHeaders() const {return inputHeaders;}
      const Headers &getOutputHeaders() const {return outputHeaders;}
      Headers &getInputHeaders() {return inputHeaders;}
      Headers &getOutputHeaders() {return outputHeaders;}

      const Buffer &getInputBuffer() const {return inputBuffer;}
      const Buffer &getOutputBuffer() const {return outputBuffer;}
      Buffer &getInputBuffer() {return inputBuffer;}
      Buffer &getOutputBuffer() {return outputBuffer;}

      RequestMethod getMethod() const {return method;}
      void setMethod(RequestMethod method) {this->method = method;}

      const URI &getOriginalURI() const {return originalURI;}
      const URI &getURI() const {return uri;}
      URI &getURI() {return uri;}
      std::string getHost() const {return uri.getHost();}

      const Version &getVersion() const {return version;}
      void setVersion(const Version &version) {this->version = version;}

      bool isOk() const {return responseCode == HTTP_OK && !connError;}
      HTTPStatus getResponseCode() const {return responseCode;}
      void setResponseCodeLine(const std::string &s) {responseCodeLine = s;}
      const std::string &getResponseCodeLine() const {return responseCodeLine;}
      std::string getResponseLine() const;
      std::string getRequestLine() const;

      bool hasConnection() const {return connection.isSet();}
      Connection &getConnection() const {return *connection;}
      void setConnection(const SmartPointer<Connection> &con)
        {connection = con;}
      bool isConnected() const;
      ConnectionError getConnectionError() const {return connError;}
      void setConnectionError(ConnectionError err) {connError = err;}

      const SmartPointer<Session> &getSession() const {return session;}
      void setSession(const SmartPointer<Session> &session)
      {this->session = session;}
      std::string
      getSessionID(const std::string &cookie = "sid",
                   const std::string &header = "Authorization") const;

      const std::string &getUser() const;
      void setUser(const std::string &user);

      bool isChunked() const {return chunked;}
      bool isReplying() const {return replying;}

      uint64_t getBytesRead() const {return bytesRead;}
      uint64_t getBytesWritten() const {return bytesWritten;}

      bool isSecure() const;
      SSL getSSL() const;

      virtual bool isWebsocket() const {return false;}
      virtual void resetOutput();

      virtual void insertArg(const std::string &arg)
      {args.insert(String(args.size()), arg);}
      virtual void insertArg(const std::string &key, const std::string &arg)
      {args.insert(key, arg);}
      virtual const JSON::Value &getArgs() const {return args;}
      virtual JSON::Value &getArgs() {return args;}
      virtual const std::string &getArg(unsigned i) const
      {return args.getString(i);}
      virtual const std::string &getArg(const std::string &key) const
      {return args.getString(key);}
      virtual std::string getArg(const std::string &key,
                                 const std::string &defaultVal) const
      {return args.getString(key, defaultVal);}
      virtual JSON::Value &parseJSONArgs();
      virtual JSON::Value &parseQueryArgs();
      virtual JSON::Value &parseArgs();

      const IPAddress &getClientIP() const;

      bool inHas(const std::string &name) const;
      std::string inFind(const std::string &name) const;
      std::string inGet(const std::string &name) const;
      void inSet(const std::string &name, const std::string &value);
      void inRemove(const std::string &name);

      bool outHas(const std::string &name) const;
      std::string outFind(const std::string &name) const;
      std::string outGet(const std::string &name) const;
      void outSet(const std::string &name, const std::string &value);
      void outRemove(const std::string &name);

      void setPersistent(bool x);
      virtual void setCache(uint32_t age);

      bool hasContentType() const;
      std::string getContentType() const;
      void setContentType(const std::string &contentType);
      void guessContentType();

      typedef enum {
        COMPRESS_NONE, COMPRESS_AUTO, COMPRESS_ZLIB, COMPRESS_GZIP,
        COMPRESS_BZIP2
      } compression_t;

      void outSetContentEncoding(compression_t compression);
      compression_t getRequestedCompression() const;

      bool hasCookie(const std::string &name) const;
      std::string findCookie(const std::string &name) const;
      std::string getCookie(const std::string &name) const;
      void setCookie(const std::string &name, const std::string &value,
                     const std::string &domain = std::string(),
                     const std::string &path = std::string(),
                     uint64_t expires = 0, uint64_t maxAge = 0,
                     bool httpOnly = false, bool secure = false);

      std::string getInput() const;
      std::string getOutput() const;

      SmartPointer<JSON::Value> getInputJSON() const;
      SmartPointer<JSON::Value> getJSONMessage() const;
      SmartPointer<JSON::Writer>
      getJSONWriter(unsigned indent, bool compact,
                    compression_t compression = COMPRESS_AUTO);
      SmartPointer<JSON::Writer>
      getJSONWriter(compression_t compression = COMPRESS_AUTO);
      SmartPointer<JSON::Writer> getJSONPWriter(const std::string &callback);

      SmartPointer<std::istream> getInputStream() const;
      SmartPointer<std::ostream>
      getOutputStream(compression_t compression = COMPRESS_AUTO);

      virtual void sendError(HTTPStatus code);
      virtual void sendError(HTTPStatus code, const std::string &message);
      virtual void sendJSONError(HTTPStatus code, const std::string &message);
      virtual void sendError(const Exception &e);
      virtual void sendError(const std::exception &e);

      virtual void send(const Buffer &buf);
      virtual void send(const char *data, unsigned length);
      virtual void send(const char *s);
      virtual void send(const std::string &s);
      virtual void sendFile(const std::string &path);

      virtual void reply(HTTPStatus code = HTTP_OK);
      virtual void reply(const Buffer &buf);
      virtual void reply(const char *data, unsigned length);
      virtual void reply(HTTPStatus code, const char *data, unsigned length);

      template <typename T>
      void reply(HTTPStatus code, const T &data) {send(data); reply(code);}

      virtual void startChunked(HTTPStatus code = HTTP_OK);
      virtual void sendChunk(const Buffer &buf);
      virtual void sendChunk(const char *data, unsigned length);
      virtual void sendChunk(const std::string &s);
      virtual SmartPointer<JSON::Writer> getJSONChunkWriter();
      virtual void endChunked();

      virtual void redirect(const URI &uri,
                            HTTPStatus code = HTTP_TEMPORARY_REDIRECT);
      virtual void cancel();

      // Callbacks
      virtual void onHeaders() {}
      virtual void onRequest();
      virtual bool onContinue() {return true;}
      virtual void onResponse(ConnectionError code) {}

      // Used by Connection
      bool mustHaveBody() const;
      bool mayHaveBody() const;
      bool needsClose() const;

      static Version parseHTTPVersion(const std::string &s);
      void parseResponseLine(const std::string &line);

      void write();

    protected:
      void writeResponse(Buffer &buf);
      void writeRequest(Buffer &buf);
      void writeHeaders(Buffer &buf);
    };
  }
}
