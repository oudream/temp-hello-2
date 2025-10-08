#include "mqtt_client.h"

// async_subscribe.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT subscriber using the C++ asynchronous client
// interface, employing callbacks to receive messages and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Subscribing to a topic
//  - Receiving messages through the ClientCallback API
//  - Receiving network disconnect updates and attempting manual reconnects.
//  - Using a "clean session" and manually re-subscribing to topics on
//    reconnect.
//

#include <string>
#include <cstring>

#include "mqtt/async_client.h"

#include <ccxx/cxstring.h>
#include <ccxx/cxinterinfo_group_define.h>
#include <ccxx/cxinterinfo.h>
#include <ccxx/cxtime.h>

using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "MQTT-CLIENT")


static string f_server_ip("localhost");
static int f_server_port = 1883;
static string f_server_address("tcp://localhost:1883");
//static string f_client_id("Tk_mqtt_jiexi_b812345b");
static string f_client_id("mqtt_client_b812345b");
//static string f_app_name("Tk_mqtt_jiexi");
static string f_user_name;
static string f_password;

static const int	f_QOS = 1;
static const int	N_RETRY_ATTEMPTS = 5;

static vector<string> f_topics;

static fn_mqtt_connected_t f_fn_mqtt_connected = NULL;
static fn_mqtt_connected_t f_fn_mqtt_connection_lost = NULL;
static fn_mqtt_received_payload_t f_fn_mqtt_received_payload = NULL;
static fn_mqtt_received_token_t f_fn_mqtt_received_token = NULL;

static msepoch_t f_dtInit = 0;


// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.
class ActionListener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token& tok)     override {
        fnError().out(".action_listener.on_failure [ %s ]!", name_.c_str());
        if (tok.get_message_id() != 0)
        {
            fnError().out(".action_listener.on_failure for token: [%d]", tok.get_message_id());
        }
    }

    void on_success(const mqtt::token& tok) override {
        fnInfo().out(".action_listener.on_success [ %s ]!", name_.c_str());
        int iToken = tok.get_message_id();
        if (iToken != 0)
        {
            fnInfo().out(".action_listener.on_success for token: [%d]", iToken);
        }
        string sTopic;
        auto top = tok.get_topics();
        if (top && !top->empty())
        {
            stringstream ss;
            ss << (*top)[0];
            sTopic = ss.str();
            fnInfo().out(".action_listener.on_success for token topics: [%s], ...", sTopic.c_str());
        }

        if (f_fn_mqtt_received_token != NULL)
        {
            f_fn_mqtt_received_token(name_, iToken, sTopic);
        }
    }

public:
    ActionListener(const std::string& name) : name_(name) {}

};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local ClientCallback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class ClientCallback : public virtual mqtt::callback,
                 public virtual mqtt::iaction_listener

{
    // The MQTT client
    mqtt::async_client& cli_;
    // Options to use if we need to reconnect
    mqtt::connect_options& connOpts_;

    // Re-connection failure
    void on_failure(const mqtt::token& tok) override {
        connected_ = false;
        fnError().out(".ClientCallback.on_failure Connection attempt failed");
    }

    // (Re)connection success
    // Either this or connected() can be used for callbacks.
    void on_success(const mqtt::token& tok) override {
        connected_ = true;
        fnInfo().out(".ClientCallback.on_success.");
    }

    // (Re)connection success
    void connected(const std::string& cause) override {
        connected_ = true;
        fnInfo().out(".ClientCallback.connected Connection success. %s", cause.c_str());
        if (f_fn_mqtt_connected != NULL)
        {
            f_fn_mqtt_connected(cause);
        }
    }

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void connection_lost(const std::string& cause) override {
        connected_ = false;
        fnError().out(".ClientCallback.connection_lost Connection lost");
        if (!cause.empty())
            fnError().out(".ClientCallback.connection_lost \tcause:");
        fnInfo().out(".ClientCallback.connection_lost Reconnecting...");
        if (f_fn_mqtt_connection_lost != NULL)
        {
            f_fn_mqtt_connection_lost(cause);
        }
    }

    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override {
        string topic = msg->get_topic();
        string payload = msg->to_string();
        fnDebug().out(".ClientCallback.message_arrived Message arrived \ntopic: %s \npayload: %s", topic.c_str(), payload.c_str());
        if (f_fn_mqtt_received_payload != NULL)
        {
            f_fn_mqtt_received_payload(topic, payload);
        }
    }

    void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
    volatile bool connected_;

    ClientCallback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
            : cli_(cli), connOpts_(connOpts), connected_(false) {}

};

static mqtt::async_client *f_cli = NULL;
static mqtt::connect_options *f_connOpts = NULL;
static ClientCallback *f_cb = NULL;
static ActionListener *f_subListener = NULL;
static ActionListener *f_pubListener = NULL;

void MqttClient::init(const char *ip, int port, const char *clientId, const char *username, const char *password)
{
    if (f_cb != NULL)
    {
        return;
    }
    // A subscriber often wants the server to remember its messages when its
    // disconnected. In that case, it needs a unique ClientID and a
    // non-clean session.
    f_server_ip = ip;
    f_server_port = port;
    f_server_address = CxString::format("tcp://%s:%d", ip, port);
    f_client_id = clientId;

    f_cli = new mqtt::async_client(f_server_address, f_client_id);
    f_connOpts = new mqtt::connect_options();
    f_connOpts->set_clean_session(false);
    setUsername(username);
    setPassword(password);
    // Install the ClientCallback(s) before connecting.
    f_cb = new ClientCallback(*f_cli, *f_connOpts);
    f_cli->set_callback(*f_cb);
    f_subListener = new ActionListener("Subscription");
    f_pubListener = new ActionListener("Publication");
    f_dtInit = CxTime::currentMsepoch();
}

void MqttClient::unInit()
{
    if (f_cb == NULL)
    {
        return;
    }
    try {
        if (f_cli->is_connected())
        {
            fnInfo().out("::unInit disconnecting from the MQTT server...");
            f_cli->disconnect()->wait();
            fnInfo().out("::unInit disconnecting from the MQTT server, OK!");
        }
    }
    catch (const mqtt::exception& exc) {
        fnError().out("::unInit disconnecting, exception:%s", exc.what());
    }
    delete f_cli;
    f_cli = NULL;
    delete f_connOpts;
    delete f_cb;
    f_cb = NULL;
    delete f_subListener;
    delete f_pubListener;
}

void MqttClient::registConnected(fn_mqtt_connected_t fn)
{
    f_fn_mqtt_connected = fn;
}

void MqttClient::registReceivedPayload(fn_mqtt_received_payload_t fn)
{
    f_fn_mqtt_received_payload = fn;
}

bool MqttClient::isConnected()
{
    return f_cb != NULL && f_cb->connected_;
}

void MqttClient::subscribe(const string &topic)
{
    assert(f_cli != NULL);
    try {
        f_cli->subscribe(topic, f_QOS, nullptr, *f_subListener);
        fnDebug().out("::subscribe success!  topic: %s", topic.c_str());
    }
    catch (const mqtt::exception& exc) {
        fnError().out("::subscribe exception: %s.  topic: %s", exc.what(), topic.c_str());
    }
}

int MqttClient::publish(const string &topic, const string &payload, int timeout)
{
    int r = 0;
    assert(f_cli != NULL);
    try
    {
        mqtt::delivery_token_ptr pubtok = f_cli->publish(topic.c_str(), payload.c_str(), payload.size(), f_QOS, mqtt::message::DFLT_RETAINED, NULL, *f_pubListener);
        if (pubtok)
        {
            r = pubtok->get_message_id();
            if (timeout > 0)
            {
                pubtok->wait_for(timeout);
            }
        }
        if (payload.size() < 1024)
        {
            fnDebug().out("::publish %s ; topic -> %s ; payload.size=%d -> %s",  r > 0 ? "SUCCESS" : "FAIL", topic.c_str(), payload.size(), payload.c_str());
        }
        else
        {
            fnDebug().out("::publish %s ; topic -> %s ; payload.size=%d",  r > 0 ? "SUCCESS" : "FAIL", topic.c_str(), payload.size());
        }
    }
    catch (const mqtt::exception& exc)
    {
        fnError().out("::publish exception: %s ; topic -> %s ; payload -> %s", exc.what(), topic.c_str(), payload.c_str());
    }
    return r;
}

void MqttClient::registConnectionLost(fn_mqtt_connected_t fn)
{
    f_fn_mqtt_connection_lost = fn;
}

void MqttClient::registReceivedToken(fn_mqtt_received_token_t fn)
{
    f_fn_mqtt_received_token = fn;
}

void MqttClient::setUsername(const char *username)
{
    f_user_name = username == NULL ? "" : username;
    if (f_connOpts != NULL)
    {
        f_connOpts->set_user_name(f_user_name);
    }
}

void MqttClient::setPassword(const char *password)
{
    f_password = password == NULL ? "" : password;
    if (f_connOpts != NULL)
    {
        f_connOpts->set_password(f_password);
    }
}

void MqttClient::checkConnect(msepoch_t dtNow, const char *ip, int port, const char *clientId, const char *username, const char *password)
{
    static msepoch_t dtCheck = 0;
    if (dtNow - dtCheck < 5000)
    {
        return;
    }
    dtCheck = dtNow;
    if (f_cb != NULL && f_cb->connected_)
    {
        return;
    }
    if (dtNow - f_dtInit < 1000 * 60 * 10)
    {
        unInit();
    }
    if (f_cb == NULL)
    {
        init(ip, port, clientId, username, password);
    }
    try {
        fnInfo().out("::start Connecting to the MQTT server...");
        f_cli->connect(*f_connOpts, nullptr, *f_cb);
    }
    catch (const mqtt::exception& exc) {
        fnError().out("::start Unable to connect to MQTT server[%s], exception:%s", f_server_address.c_str(), exc.what());
    }
}

