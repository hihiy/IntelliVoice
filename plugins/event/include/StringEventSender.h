// StringEventSender.h
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

#ifndef SSI_EVENT_STRINGEVENTSENDER_H
#define SSI_EVENT_STRINGEVENTSENDER_H

#include "base/IConsumer.h"
#include "base/IEvents.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class StringEventSender : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options()
			: mean(true), n_buffer(1024), delim(';') {

			setSenderName("string");
			setEventName("value");
			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");		
			addOption ("mean", &mean, 1, SSI_BOOL, "send mean values");		
			addOption("buffer", &n_buffer, 1, SSI_SIZE, "size of string buffer");
			addOption("delim", &delim, 1, SSI_CHAR, "row delimiter");
		};

		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEventName (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}

		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];		
		bool mean;
		ssi_size_t n_buffer;
		ssi_char_t delim;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "ssi_consumer_StringEventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new StringEventSender (file); };
	~StringEventSender ();

	StringEventSender::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Sends the current stream as an string to the board."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	StringEventSender (const ssi_char_t *file = 0);
	StringEventSender::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _event;
	ssi_size_t _n_buffer;
	ssi_char_t *_buffer;
	
};

}

#endif