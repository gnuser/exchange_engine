
#include "common.h"
#include "server.h"
#include <sys/time.h>
#include <unistd.h>

std::vector<HTTPPathHandler> pathHandlers;

HTTPRequest::HTTPRequest(struct evhttp_request* _req) : req(_req){}
HTTPRequest::~HTTPRequest()
{
    LOG(INFO) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"  ;
}

HTTPRequest::RequestMethod HTTPRequest::GetRequestMethod()
{
    switch (evhttp_request_get_command(req)) {
    case EVHTTP_REQ_GET:
        return GET;
        break;
    case EVHTTP_REQ_POST:
        return POST;
        break;
    case EVHTTP_REQ_HEAD:
        return HEAD;
        break;
    case EVHTTP_REQ_PUT:
        return PUT;
        break;
	case EVHTTP_REQ_OPTIONS:
        return OPTIONS;
        break;
    default:
        return UNKNOWN;
        break;
    }
}

static std::string RequestMethodString(HTTPRequest::RequestMethod m)
{
    switch (m) {
    case HTTPRequest::GET:
        return "GET";
        break;
    case HTTPRequest::POST:
        return "POST";
        break;
    case HTTPRequest::HEAD:
        return "HEAD";
        break;
    case HTTPRequest::PUT:
        return "PUT";
        break;
    case HTTPRequest::OPTIONS:
        return "OPTIONS";
        break;
    default:
        return "unknown";
    }
}

void registerHTTPHandler(const std::string &prefix, const HTTPRequestHandler &handler)
{
    LOG(INFO) << "Registering HTTP handler for " << prefix;

    pathHandlers.push_back(HTTPPathHandler(prefix, handler));
}

std::string HTTPRequest::GetURI()
{
    return evhttp_request_get_uri(req);
}
std::string HTTPRequest::GetHeader()
{
    std::string urlheader;
    struct evkeyvalq *headers;
    struct evkeyval *header;
    headers = evhttp_request_get_input_headers(req);

    for (header = headers->tqh_first; header;header = header->next.tqe_next)
    {
        urlheader = urlheader + header->key + " : " + header->value + "\n";
    }

    return urlheader;
}

std::string HTTPRequest::                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              GetAuthorization()
{
    std::string auth_value;
    struct evkeyvalq *headers;
    struct evkeyval *header;
    headers = evhttp_request_get_input_headers(req);

    std::string header_key;
    for (header = headers->tqh_first; header;header = header->next.tqe_next)
    {
        header_key = header->key;
        if ( header_key.compare("Authorization") == 0)
        {
            auth_value = header->value;
        }
    }

    return auth_value;
}


void HTTPRequest::GetPeer()
{
    evhttp_connection* con = evhttp_request_get_connection(req);
    if (con)
    {
        const char* address = "";
        uint16_t port = 0;
        evhttp_connection_get_peer(con, (char**)&address, &port);
        LOG(INFO) << address << " : " << port;
        return;
    }

    LOG(INFO) << "GET_PEER_ERROR";
    return;
}


void HTTPRequest::WriteHeader(const std::string& hdr, const std::string& value)
{
    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    assert(headers);
    evhttp_add_header(headers, hdr.c_str(), value.c_str());
}

void HTTPRequest::WriteReply(int nStatus, const std::string& strReply)
{
    assert(req);
    struct evbuffer* evb = evhttp_request_get_output_buffer(req);
    assert(evb);
    evbuffer_add(evb, strReply.data(), strReply.size());
    auto req_copy = req;

    evhttp_send_reply(req_copy, nStatus, nullptr, nullptr);
    if (event_get_version_number() >= 0x02010600 && event_get_version_number() < 0x02020001)
    {
       evhttp_connection* conn = evhttp_request_get_connection(req_copy);
       if (conn)
       {
           bufferevent* bev = evhttp_connection_get_bufferevent(conn);
           if (bev)
           {
               bufferevent_enable(bev, EV_READ | EV_WRITE);
           }
       }
    }
}

std::string HTTPRequest::ReadBody()
{
    struct evbuffer* buf = evhttp_request_get_input_buffer(req);
    if (!buf)
    {
        LOG(INFO) << "READ_BODY ERROR 1";
        return "";
    }

    size_t size = evbuffer_get_length(buf);

    const char* data = (const char*)evbuffer_pullup(buf, size);
    if (!data)
    {
        LOG(INFO) << "READ_BODY ERROR 2   " << size;
        return "";
    }
    std::string rv(data, size);
    evbuffer_drain(buf, size);

    LOG(INFO) << "READ_BODY : " << rv;
    return rv;
}
bool checkHash(const std::string &txid)
{
    return isHex(txid) && HAHS_SIZE == txid.length();
}

void httpRequestCb(struct evhttp_request *req, void *arg)
{

    if (event_get_version_number() >= 0x02010600 && event_get_version_number() < 0x02020001)
    {
        evhttp_connection* conn = evhttp_request_get_connection(req);
        if (conn)
        {
            bufferevent* bev = evhttp_connection_get_bufferevent(conn);
            if (bev)
            {
                bufferevent_disable(bev, EV_READ);
            }
        }
    }

    std::unique_ptr<HTTPRequest> hreq(new HTTPRequest(req));

    hreq->GetPeer();
    LOG(INFO) << "Received a " <<  RequestMethodString(hreq->GetRequestMethod()) << " request for " <<  hreq->GetURI() << " from ";

    if (hreq->GetRequestMethod() == HTTPRequest::UNKNOWN)
    {
        hreq->WriteReply(HTTP_BADMETHOD);
        return;
    }

	if (hreq->GetRequestMethod() == HTTPRequest::OPTIONS)
    {

		hreq->WriteHeader("Access-Control-Allow-Origin", "*");
		hreq->WriteHeader("Access-Control-Allow-Credentials", "true");
		hreq->WriteHeader("Access-Control-Allow-Headers", "access-control-allow-origin,Origin, X-Requested-With, Content-Type, Accept, Authorization");
		hreq->WriteReply(HTTP_OK);
        return ;
	}

    hreq->WriteHeader("Access-Control-Allow-Origin", "*");
    hreq->WriteHeader("Access-Control-Allow-Credentials", "true");
    hreq->WriteHeader("Access-Control-Allow-Headers", "access-control-allow-origin,Origin, X-Requested-With, Content-Type, Accept, Authorization");

    if (hreq->GetRequestMethod() != HTTPRequest::GET && hreq->GetRequestMethod() != HTTPRequest::POST )
    {
        hreq->WriteReply(HTTP_BADMETHOD);
        return;
    }

    std::string strURI = hreq->GetURI();
    std::string path;
    std::vector<HTTPPathHandler>::const_iterator i = pathHandlers.begin();
    std::vector<HTTPPathHandler>::const_iterator iend = pathHandlers.end();

    bool sign_url =false;
    std::string sign_rpc = "/sign";
    if ( strURI.find(sign_rpc) != std::string::npos )
    {
        sign_url = true;
    }

    for (; i != iend; ++i)
    {
        if ( sign_url  && i->prefix == sign_rpc)
        {
            path = strURI;
            break;
        }
        else
        {
            bool match = (strURI == i->prefix);
            if (match)
            {
                path = strURI;
                break;
            }
        }
    }

    if(i != iend)
    {
        LOG(INFO) << "FOUND_PATH : " << path;
        i->handler(std::move(hreq));
    }
    else
    {
        LOG(INFO) << "NOT_FOUND_PATH : " <<  strURI;
        hreq->WriteReply(HTTP_NOTFOUND);
    }
}


void signalHandler(int sig)
{
    switch (sig)
    {
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGINT:
        {
            event_loopbreak();
        }
        break;
    }
}

void runDaemon(bool daemon)
{
    if (daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }
    }
}


bool contentToipfshash(const std::string &content, std::string &ipfsHash)
{
    char path[2048] = {0};
    strncpy(path, getenv("HOME"), strlen(getenv("HOME")));
    strcat(path, "/msg.txt");
    LOG(INFO) << "path is : "<< path;
    std::ofstream examplefile(path);
    examplefile << content;
    examplefile.close();

    std::string postUrlStr = "http://localhost:5001/api/v0/add";
    std::string postParams = "";
    std::string filePath(path);
    std::string postResponseStr;
    auto res = curl_post_req(postUrlStr, postParams, filePath, postResponseStr);
    if (res != CURLE_OK)
    {

        LOG(ERROR) << "curl post failed: " + std::string(curl_easy_strerror(res)) ;
    }
    else
    {
        int pos = postResponseStr.find("Hash");
        ipfsHash = postResponseStr.substr(pos + 7, pos + 27);
        LOG(INFO) << "createIpfsMsg is : "<< ipfsHash;
        return true;
    }
    return false;

}


size_t reqReply(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::string *str = (std::string*)stream;
    (*str).append((char*)ptr, size*nmemb);
    return size * nmemb;
}

bool curlBitcoinReq(const std::string &data,std::string &response)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;
	CURLcode res;

	const std::string url = "http://127.0.0.1:8332";

    if (curl)
    {
		headers = curl_slist_append(headers, "content-type: text/plain;");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)data.size());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, reqReply);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

		curl_easy_setopt(curl, CURLOPT_USERPWD, "hello:helloworld");
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
		res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "CURL_FAILED : " << curl_easy_strerror(res);
        return false;
    }
    LOG(INFO) << "CURL_RESULT : " << response;

    return true;
}

CURLcode curl_post_req(const std::string &url, const std::string &postParams, std::string &filepath, std::string &response)
{
    // init curl
    CURL *curl = curl_easy_init();
    // res code
    CURLcode res;
    if (curl)
    {
        // set params
        curl_easy_setopt(curl, CURLOPT_POST, 1); // post req
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str()); // params
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

        if (!filepath.empty()) {
            struct curl_httppost* post = NULL;
            struct curl_httppost* last = NULL;
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "uploadfile", CURLFORM_FILE, filepath.c_str(), CURLFORM_END);
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        }

        // start req
        res = curl_easy_perform(curl);
    }
    // release curl
    curl_easy_cleanup(curl);
    return res;
}

// reply of the requery
size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::string *str = (std::string*)stream;
    (*str).append((char*)ptr, size*nmemb);
    return size * nmemb;
}

struct UserInfo
{
    std::string user_name;
    std::string user_password;
    int user_id;
};

std::vector<UserInfo*> g_user_info;
std::map<std::string,std::string> g_user_password;
std::map<std::string,int> g_authorization_userid;
static int g_user_id =0;

static void ErrorReturn(int ret_code,const std::string&response,std::unique_ptr<HTTPRequest> req)
{
    std::string result = makeReplyMsg(ret_code,response);
    req->WriteHeader("Content-Type", "application/json");
    req->WriteReply(HTTP_OK,result);
}

void RegisterUser(std::unique_ptr<HTTPRequest> req)
{
    try
    {

        int ret_code =0;
        std::string post_data = req->ReadBody();
        std::cout << "RegisterUser receive :" <<  post_data << std::endl;
        auto json_data = json::parse(post_data);
        if(!json_data.is_object())
        {
             LOG(ERROR) << " RegisterUser params error ";
             throw;
        }

        std::string user_name = json_data["name"].get<std::string>();
        std::string user_password = json_data["password"].get<std::string>();
        int user_id = json_data["user_id"].get<int>();

        std::string  response = "OK!";
        std::string  result = "";
        if ( g_user_password.find(user_name) !=  g_user_password.end() )
        {
            ret_code = 1;
            response = "The user name has registed!";
            result = makeReplyMsg(ret_code,response);
            req->WriteHeader("Content-Type", "application/json");
            req->WriteReply(HTTP_OK,result);
            return;
        }

        g_user_password[user_name] = user_password;
        if (user_id <= 0)
        {
           g_user_id++;
           user_id = g_user_id;
        }
        std::string  user_name_password = user_name +":"+user_password;
        g_authorization_userid[user_name_password] = user_id;

        UserInfo* user_info_add = new UserInfo;
        user_info_add->user_id = user_id;
        user_info_add->user_name = user_name;
        user_info_add->user_password = user_password;
        g_user_info.push_back(user_info_add);


        result = makeReplyMsg(ret_code,response);
        req->WriteHeader("Content-Type", "application/json");
        req->WriteReply(HTTP_OK,result);
        return;
    }
    catch(...)
    {
        LOG(ERROR) << "  RegisterUser error: \n ";
    }
    req->WriteReply(HTTP_INTERNAL,ERROR_REQUEST);
}
void auth(std::unique_ptr<HTTPRequest> req)
{
    try
    {
        std::string head_data = req->GetHeader();
        std::cout << "auth receive head:" <<  head_data << std::endl;

        std::string auth_value = req->GetAuthorization();
        int ret_code = 0;
        std::string response ;

        if ( g_authorization_userid.find(auth_value) == g_authorization_userid.end() )
        {
            ret_code = 1;
            response = "Authorization  failed ,user name or password error!";
            std::string error_info = makeReplyMsg(ret_code,response);
            req->WriteHeader("Content-Type", "application/json");
            req->WriteReply(HTTP_OK,error_info);
            return;
        }

        std::cout << "auth value is : " << auth_value << std::endl;
        json user_id_json = json::object();

        user_id_json["user_id"] = g_authorization_userid[auth_value];
        std::string result = makeReplyMsg(ret_code,user_id_json);
        req->WriteHeader("Content-Type", "application/json");
        req->WriteReply(HTTP_OK,result);
        return;
    }
    catch(...)
    {
        LOG(ERROR) << "  auth error: \n ";
    }
    req->WriteReply(HTTP_INTERNAL,ERROR_REQUEST);
}

static  bool CheckAccessId(const std::string&url_data,int user_id)
{
    return true;
}

void sign(std::unique_ptr<HTTPRequest> req)
{
    try
    {

        std::string  url_data = req->GetURI();
        std::cout << "sign url: " << url_data << std::endl;
        std::string auth_value = req->GetAuthorization();

        int ret_code = 0;
         std::string response;

        if ( g_authorization_userid.find(auth_value) == g_authorization_userid.end() )
        {
            ret_code = 1;
            response = "Authorization  failed ,user name or password error!";
            std::string error_info = makeReplyMsg(ret_code,response);
            req->WriteHeader("Content-Type", "application/json");
            req->WriteReply(HTTP_OK,error_info);
            return;
        }



        int user_id = g_authorization_userid[auth_value];
        if ( !CheckAccessId(url_data,user_id) )
        {
            ret_code = 2;
            response = "sign  failed ,user don't match access_id !";
            std::string error_info = makeReplyMsg(ret_code,response);
            req->WriteHeader("Content-Type", "application/json");
            req->WriteReply(HTTP_OK,error_info);
            return;
        }

        json user_id_json = json::object();
        user_id_json["user_id"] = user_id;
        std::string result = makeReplyMsg(ret_code,user_id_json);
        req->WriteHeader("Content-Type", "application/json");
        req->WriteReply(HTTP_OK,result);
        return;
    }
    catch(...)
    {
        LOG(ERROR) << "  sign error: \n ";
    }
    req->WriteReply(HTTP_INTERNAL,ERROR_REQUEST);
}
