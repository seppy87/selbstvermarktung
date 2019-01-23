#pragma once
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>
#include <iostream>
#include <string>
#include <vector>
#include"events.hpp"

namespace http {

	struct http_resp_struct {
		float kwh;
		float VAh;
		float currentDraw;
		unsigned short rate; // in Percent: 100,75,50,25 or 0
		bool error_occured;
		char* error_code;

	};

	class MyRequestHandler : public Poco::Net::HTTPRequestHandler {
	public:
		void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
	private:
		static int count;
	};

	class MyRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	public:
		virtual Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &);

	};

	class MyServerApp : public Poco::Util::ServerApplication {
	protected:
		int main(const std::vector<std::string> &);

		events::PowerMeassureResultEvent theEvent;
	};

}