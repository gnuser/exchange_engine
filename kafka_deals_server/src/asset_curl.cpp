#include "asset_curl.h"
#include "easylogging++.h"
#include <curl/curl.h>

static size_t ReplyCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	std::string *str = (std::string*)stream;
	(*str).append((char*)ptr, size*nmemb);
	return size * nmemb;
}




bool CurlRequest(const std::string& request_url, const std::string&request_auth,  const std::string& request_data,  std::string& response)
{
	CURL *curl = curl_easy_init();
	struct curl_slist *headers = NULL;
	CURLcode res;
	response.clear();

	std::string error_str ;
	//error_str.clear();
	if (curl)
	{
		headers = curl_slist_append(headers, "content-type:application/json;");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request_data.size());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReplyCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

		curl_easy_setopt(curl, CURLOPT_USERPWD, request_auth.c_str());
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
		res = curl_easy_perform(curl);
	}
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		error_str = curl_easy_strerror(res);
		LOG(ERROR) << error_str ;
		return false;
	}
	return true;	

}

bool CurlPost(const std::string& post_url, const std::string& post_data,std::string& response)
{
	CURL *curl = curl_easy_init();
	struct curl_slist *headers = NULL;
	CURLcode res;
	response.clear();
	std::string error_str;
	error_str.clear();
	if (curl)
	{
		headers = curl_slist_append(headers, "content-type:application/json;");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, post_url.c_str());

		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)post_data.size());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReplyCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
		res = curl_easy_perform(curl);
	}
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		error_str = curl_easy_strerror(res);
		LOG(ERROR) << error_str;
		return false;
	}
	return true;

}



bool CurlGet(const std::string& get_url,std::string& response)
{
	CURL *curl = curl_easy_init();
	struct curl_slist *headers = NULL;
	CURLcode res;
	response.clear();
	std::string error_str;
	error_str.clear();
	if (curl)
	{
		headers = curl_slist_append(headers, "content-type:application/json;");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());

		//curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)post_data.size());
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReplyCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
		res = curl_easy_perform(curl);

	}
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		error_str = curl_easy_strerror(res);
		LOG(ERROR) << error_str;
		return false;
	}
	return true;

}

