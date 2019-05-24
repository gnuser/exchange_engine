This is just for testing  accessws sever from viabtc_exchange_server.
Because there is conflict from  libev(viabtc_exchange_server)  and  libevent(auth_sign_server),so you should first do "install_libevent.sh" .Then libevent maybe install local path not system path.
How to use it ?
First you  should  start services from  viabtc_exchange_server,
then edit the config.json of accessws ,like this
...
"auth_url": "http://127.0.0.1:9000/auth",
"sign_url": "http://127.0.0.1:9000/sigh",
...

then you can use  curl to register you account,like this
curl -H "Content-Type:application/json" -X POST  -d '{"name":"[yourname]","password":"[yourpassword]","user_id":[can specify or set 0(system auto create) ]}' 127.0.0.1:9000/RegisterUser


then you can transmit {"id":1000, "method": "server.auth", "params":["(name):(password)",""] } for websocket online test.


