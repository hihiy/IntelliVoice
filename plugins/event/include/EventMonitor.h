// EventMonitor.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/14
// Copyright (C) 2007-14 University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_EVENT_EVENTMONITOR_H
#define SSI_EVENT_EVENTMONITOR_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Monitor;
class Window;

class EventMonitor : public IObject {

public:

	class Options : public OptionList {

	public:

		Options()
			: all(true), console(false), detail(true), chars(10000), update_ms(0), relative(false) {

			screen[0] = 0;
			screen[1] = 0;

			mpos[0] = 0;
			mpos[1] = 0;
			mpos[2] = 600;
			mpos[3] = 300;

			setMonitorName("EventBoard");

			addOption("all", &all, 1, SSI_BOOL, "output all events, otherwise only new events will be plotted");
			addOption("console", &console, 1, SSI_BOOL, "output on console instead of window");
			addOption("relative", &relative, 1, SSI_BOOL, "arrange windows relative to screen");
			addOption("screen", &screen, 2, SSI_REAL, "customize screen region [width,height], by default set to desktop size");
			addOption("mpos", &mpos, 4, SSI_REAL, "position of monitor on screen [posx,posy,width,height], either in pixels or relative to screen");
			addOption("mname", mname, SSI_MAX_CHAR, SSI_CHAR, "name of monitor (will be displayed in title)");
			addOption("chars", &chars, 1, SSI_SIZE, "maximum number of chars displayed");
			addOption("detail", &detail, 1, SSI_BOOL, "output detailed event content");
			addOption("update", &update_ms, 1, SSI_SIZE, "minimum update rate in ms");
		};

		void setMonitorPos(ssi_real_t x, ssi_real_t y, ssi_real_t width, ssi_real_t height) {
			mpos[0] = x;
			mpos[1] = y;
			mpos[2] = width;
			mpos[3] = height;
		}

		void setScreen(ssi_real_t width, ssi_real_t height) {
			screen[2] = width;
			screen[3] = height;
		}

		void setMonitorName(const ssi_char_t *name) {
			ssi_strcpy(mname, name);
		}

		bool all;
		bool console;
		ssi_real_t mpos[4];
		ssi_real_t screen[2];
		ssi_char_t mname[SSI_MAX_CHAR];
		bool relative;
		bool detail;
		ssi_size_t chars;
		ssi_size_t update_ms;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "ssi_listener_EventMonitor"; };
	static IObject *Create (const ssi_char_t *file) { return new EventMonitor (file); };
	~EventMonitor ();

	EventMonitor::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Generic visualization component to monitor events."; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	EventMonitor (const ssi_char_t *file = 0);
	EventMonitor::Options _options;
	ssi_char_t *_file;

	ssi_size_t _update_counter;

	Window *_window;
	Monitor *_monitor;
	ssi_char_t _string[SSI_MAX_CHAR];

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
};

}

#endif

