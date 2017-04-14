#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <string>
#include <locale>
#include <codecvt>

#include <future>
#include <thread>
#include <functional>

#include <apiai/io/StreamReader.h>

#include <apiai/AI.h>
#include <apiai/query/TextQueryRequest.h>
#include <apiai/query/request/QueryText.h>
#include <apiai/exceptions/Exception.h>
#include <apiai/exceptions/JSONException.h>
#include <apiai/query/response/Fulfillment.h>


#include <atlcore.h>

using namespace std;
using namespace ai::query::request;


extern "C" __declspec(dllexport) const char* API_request(const char* input)
{
	std::string result;
	ai::AI::global_init();

	string s(input);
	auto credentials = ai::Credentials("1ea86ba26bf34a7197b1e25c2daee0f3");

	auto params = Parameters("sessionId").setResetContexts(true);
	USES_CONVERSION;
	wstring w_input = A2W(s.c_str());
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	std::string utf8_input = convert.to_bytes(w_input);


	auto request = std::shared_ptr<TextQueryRequest>(new TextQueryRequest(QueryText::One(utf8_input), "ko", credentials, params));

	try {
		result = request->perform().result.getFulfillment()->getSpeech();
	}
	catch (std::exception &e) {
		result = "error";
	}


	return result.c_str();
}

//extern "C" __declspec(dllexport) string API_request(string input)
//{
//	std::string result;
//	ai::AI::global_init();
//
//	auto credentials = ai::Credentials("1ea86ba26bf34a7197b1e25c2daee0f3");
//
//	auto params = Parameters("sessionId").setResetContexts(true);
//	USES_CONVERSION;
//	wstring w_input = A2W(input.c_str());
//	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
//	std::string utf8_input = convert.to_bytes(w_input);
//
//
//	auto request = std::shared_ptr<TextQueryRequest>(new TextQueryRequest(QueryText::One(utf8_input), "ko", credentials, params));
//
//	try {
//		result = request->perform().result.getFulfillment()->getSpeech();
//	}
//	catch (std::exception &e) {
//		result = "error";
//	}
//
//
//	return result;
//}

