#include "qt_ui_global.h"

#include <ccxx/cxcontainer.h>

using namespace std;

typedef vector<fn_event_callback_t > EventsVector;
typedef map<int, EventsVector > EventsMap;

static EventsMap _events;

void
UiGlobal::dispatchEvent(int eid, const std::string& msg, const std::vector<char>& data)
{
	EventsMap::iterator it = _events.find(eid);
	if (it != _events.end())
	{
		EventsVector & vs = it->second;
		for (int i = 0; i < vs.size(); ++i)
		{
			fn_event_callback_t e = vs[i];
			e(eid, msg, data);
		}
	}
}

void
UiGlobal::registEvent(int eid, fn_event_callback_t fn)
{
	EventsMap::iterator it = _events.find(eid);
	if (it != _events.end())
	{
		EventsVector & vs = it->second;
		if (! CxContainer::contain(vs, fn))
		{
			vs.push_back(fn);
		}
	} else {
		_events[eid] = EventsVector{fn};
	}
}

void
UiGlobal::unregistEvent(int eid, fn_event_callback_t fn)
{
	EventsMap::iterator it = _events.find(eid);
	if (it != _events.end())
	{
		EventsVector & vs = it->second;
		CxContainer::remove(vs, fn);
	}
}

bool
UiGlobal::isexistEvent(int eid, fn_event_callback_t fn)
{
	EventsMap::iterator it = _events.find(eid);
	if (it != _events.end())
	{
		EventsVector & vs = it->second;
		return CxContainer::contain(vs, fn);
	}
}

void
UiGlobal::dispatchEvent(int eid, const string& msg)
{
	dispatchEvent(eid, msg, vector<char>());
}

void
UiGlobal::dispatchEvent(int eid)
{
	dispatchEvent(eid, string(), vector<char>());
}
