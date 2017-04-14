#define _CRT_SECURE_NO_WARNINGS

#include "RequestConnectionImpl.h"

#include <apiai/exceptions/Exception.h>
#include <apiai/exceptions/InvalidArgumentException.h>
#include <iostream>

using namespace std;
using namespace ai;

std::string UTF8ToANSI(std::string response)
{

	const char * c = response.c_str();
	char *pszCode = new char[response.length() + 1];
	strcpy(pszCode, response.c_str());


	BSTR    bstrWide;
	char*   pszAnsi;
	int     nLength;
	// Get nLength of the Wide Char buffer
	nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen(pszCode) + 1,
		NULL, NULL);
	bstrWide = SysAllocStringLen(NULL, nLength);
	// Change UTF-8 to Unicode (UTF-16)
	MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen(pszCode) + 1, bstrWide,
		nLength);
	// Get nLength of the multi byte buffer 
	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
	pszAnsi = new char[nLength];
	// Change from unicode to mult byte
	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
	SysFreeString(bstrWide);
	delete[] pszCode;

	std::string str(pszAnsi);
	return str;
}

RequestConnection::RequestConnectionImpl::RequestConnectionImpl(const string &URL)
{
    curl = curl_easy_init();
    if (!curl) {
        throw Exception("Cannot init CURL object.");
    }

    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
}

size_t RequestConnection::RequestConnectionImpl::read_callback(char *ptr, size_t size, size_t nmemb, io::StreamReader *reader)
{
    return reader->read(ptr, size * nmemb);
}

//unsigned int RequestConnection::RequestConnectionImpl::write_callback(char *in, unsigned int size, unsigned int nmemb, string *response)
unsigned int RequestConnection::RequestConnectionImpl::write_callback(void *ptr, size_t size, size_t count, void *stream)
{
  //response->append(in, size * nmemb);
  ((string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}

const string &RequestConnection::RequestConnectionImpl::getURL() const
{
    return URL;
}

void RequestConnection::RequestConnectionImpl::setURL(const string &value)
{
    URL = value;
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
}

string RequestConnection::RequestConnectionImpl::getBody()
{
    return this->bodyStream.str();
}

void RequestConnection::RequestConnectionImpl::setBody(const string &value)
{
    this->bodyStream.str(value);
    this->bodyStream.sealed(true);
}

io::StreamWriter RequestConnection::RequestConnectionImpl::getBodyStreamWriter() {
    return this->bodyStream;
}

const map<string, string> &RequestConnection::RequestConnectionImpl::getHeaders() const
{
    return headers;
}

void RequestConnection::RequestConnectionImpl::setHeaders(const map<string, string> &value)
{
    headers = value;
}

RequestConnection::RequestConnectionImpl& RequestConnection::RequestConnectionImpl::addHeader(const string &name, const string &value) {
    headers[name] = value;
    return *this;
}

string RequestConnection::RequestConnectionImpl::performConnection()
{
    //FIXME: bodyStreamReader should be used only when POST request type
    io::StreamReader bodyStreamReader(this->bodyStream);
	
//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"query\":\"\xec\x95\x88\xeb\x85\x95\",\n\"lang\" : \"ko\",\"sessionId\" : \"sessionId\",\n\"contexts\" : [],\"entities\" : []}");


	if (this->getBody().length() > 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &bodyStreamReader);
    }

    struct curl_slist *curl_headers = NULL;

    for (auto &key_value :headers) {
        ostringstream header;
		header << key_value.first << ": " << key_value.second;

        curl_headers = curl_slist_append(curl_headers, header.str().c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

	auto response = string();

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    CURLcode perform_result = curl_easy_perform(curl);
    curl_slist_free_all(curl_headers);

    long response_status_code = 0;
    if (CURLE_OK == curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_status_code)) {
        //cout << "response_status_code" << ": " << response_status_code << endl;
    }

    double download_size = 0;
    if (CURLE_OK == curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &download_size)) {
        //cout << "download_size" << ": " << download_size << endl;
    }
    if (CURLE_OK == perform_result) {
		//cout << "ASDF: " << UTF8ToANSI(response) << endl;
		return UTF8ToANSI(response);
		//return response;

    } else if (
               CURLE_SEND_ERROR == perform_result &&
               response_status_code >= 200 && response_status_code <= 299 &&
               download_size == response.size()
               ) {
        return response;
    } else {
        stringstream stream;
        stream << "Failure perform request: ";
        stream << curl_easy_strerror(perform_result);

        throw ai::InvalidArgumentException(stream.str());
    }
}

RequestConnection::RequestConnectionImpl::~RequestConnectionImpl()
{
    curl_easy_cleanup(curl);
}
