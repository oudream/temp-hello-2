#ifndef QT_UI_GLOBAL_H
#define QT_UI_GLOBAL_H


#include <string>
#include <vector>


static const int CI_EVENT_OPEN_WIN   = 0x01000001;
static const int CI_EVENT_SET_TITLE  = 0x01000002;
static const int CI_EVENT_DATA_FRESH = 0x01000003;
static const int CI_EVENT_INFO       = 0x01000004;
static const int CI_EVENT_QSS_FRESH  = 0x01000005;
static const int CI_EVENT_EXIT       = 0x01000006;
static const int CI_EVENT_SET_LOGIN  = 0x01000007;
static const int CI_EVENT_RESIZE     = 0x01000008;
static const int CI_EVENT_STA_FRESH  = 0x01000009;


typedef int (*fn_event_callback_t)(int, const std::string& , const std::vector<char>& );

class UiGlobal
{
public:
	static void
	dispatchEvent(int eid, const std::string& msg, const std::vector<char>& data);

	static void
	dispatchEvent(int eid, const std::string& msg);

	static void
	dispatchEvent(int eid);

	static void
	registEvent(int eid, fn_event_callback_t fn);

	static void
	unregistEvent(int eid, fn_event_callback_t fn);

	static bool
	isexistEvent(int eid, fn_event_callback_t fn);

};


#endif //QT_UI_GLOBAL_H
