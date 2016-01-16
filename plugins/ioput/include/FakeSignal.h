// FakeSignal.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/01/12
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

#ifndef SSI_FILE_FAKESIGNAL_H
#define SSI_FILE_FAKESIGNAL_H

#include "base/ISensor.h"
#include "ioput/option/OptionList.h"
#include "thread/ClockThread.h"

namespace ssi {

class FakeSignal : public ISensor, public ClockThread {
public:

	struct SIGNAL {
		enum LIST {
			SINE = 0,
			RANDOM,
			IMAGE
		};
	};

	class Options : public OptionList {
	public:
		
		Options()
			: sr(50.0), type(SIGNAL::SINE) {
			addOption("sr", &sr, 1, SSI_DOUBLE, "sample rate of sine wave");			
			addOption("signal", &type, 1, SSI_INT, "signal (0=sine, 1=random)");
		}

		double sr;		
		SIGNAL::LIST type;
	};

	class SignalChannel : public IChannel {
	public:
		SignalChannel () {
			ssi_stream_init(stream, 0, 0, 0, SSI_UNDEF, 0);
		}
		~SignalChannel () {
			ssi_stream_destroy (stream);
		}
		const ssi_char_t *getName() { return "signal"; };
		const ssi_char_t *getInfo () { return "signal (sine wave | random | image)"; };
		ssi_stream_t getStream () { return stream; };
		ssi_stream_t stream;
	};

	~FakeSignal();
	static const ssi_char_t *GetCreateName () { return "ssi_sensor_FakeSignal"; };
	static IObject *Create (const ssi_char_t *file) { return new FakeSignal (file); };
	
	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "signal generator"; };
	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);

	ssi_video_params_t getFormat() { return _video_params; };
	const void *getMetaData(ssi_size_t &size) { 
		if (_options.type == SIGNAL::IMAGE) {
			size = sizeof(_video_params); 
			return &_video_params;
		}
		return 0;
	};

	bool connect ();
	bool start ();
	bool stop ();
	void clock ();
	bool disconnect ();

protected:

	FakeSignal(const ssi_char_t *file);

	ssi_char_t *_file;
	Options _options;
	SignalChannel _channel;
	ssi_video_params_t _video_params;
	unsigned char *_video_image;
	IProvider *_provider;
};
}
#endif