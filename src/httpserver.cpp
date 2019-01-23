#include"workers.hpp"
#include"httpserver.hpp"
#include"targets.hpp"

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;
using namespace http;

void MyRequestHandler::handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp){
	resp.setStatus(HTTPResponse::HTTP_OK);
	resp.setContentType("text/html");

	auto uri = req.getURI();
	http_resp_struct erg;
	erg.kwh = workers::NeXt::measurekWh();
	if (erg.kwh == 0) erg.error_occured = true;
	else erg.error_occured = false;

	ostream& out = resp.send();
	out << "<h1>Results</h1>"
		<< "<p>kWh: " << erg.kwh << "</p>";
	out.flush();
}

int MyRequestHandler::count = 0;

HTTPRequestHandler* MyRequestHandlerFactory::createRequestHandler(const HTTPServerRequest &) {
	return new MyRequestHandler;
}

int MyServerApp::main(const vector<string> &) {
	HTTPServer s(new MyRequestHandlerFactory, ServerSocket(9090), new HTTPServerParams);
	s.start();
	waitForTerminationRequest();
	s.stop();
	return Application::EXIT_OK;
	
}