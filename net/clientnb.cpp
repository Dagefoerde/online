/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <assert.h>

#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/StreamCopier.h>
#include <Poco/Thread.h>
#include <Poco/URI.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Runnable;
using Poco::Thread;
using Poco::URI;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionSet;

const char *HostName = "127.0.0.1";
constexpr int PortNumber = 9191;

struct Session
{
    std::string _session_name;
    Poco::Net::HTTPClientSession *_session;

    Session(const char *session_name, bool https = false)
        : _session_name(session_name)
    {
        if (https)
            _session = new Poco::Net::HTTPSClientSession(HostName, PortNumber);
        else
            _session = new Poco::Net::HTTPClientSession(HostName, PortNumber);
    }
    void sendPing(int i)
    {
        Poco::Net::HTTPRequest request(
            Poco::Net::HTTPRequest::HTTP_POST,
            "/ping/" + _session_name + "/" + std::to_string(i));
        try {
            Poco::Net::HTMLForm form;
            form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
            form.prepareSubmit(request);
            form.write(_session->sendRequest(request));
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Failed to write data: " << e.name() <<
                  " " << e.message() << "\n";
            throw;
        }
    }
    int getResponse()
    {
        int number = 42;
        Poco::Net::HTTPResponse response;

        try {
            std::cerr << "try to get response\n";
            std::istream& responseStream = _session->receiveResponse(response);

            std::string result(std::istreambuf_iterator<char>(responseStream), {});
            std::cerr << "Got response '" << result << "'\n";
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Exception converting: " << e.name() <<
                  " " << e.message() << "\n";
            throw;
        }
        return number;
    }
};

struct Client : public Poco::Util::Application
{
public:
    int main(const std::vector<std::string>& /* args */) override
    {
        Session first("init");
        Session second("init");

        int count = 42, back;
        first.sendPing(count);
        second.sendPing(count + 1);

        back = first.getResponse();
        assert (back == count + 1);

        back = second.getResponse();
        assert (back == count + 1);

        return 0;
    }
};

POCO_APP_MAIN(Client)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
