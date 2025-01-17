// SimpleFusion.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/02/26
// Copyright (C) 2007-14 University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_MODEL_SIMPLEFUSION_H
#define SSI_MODEL_SIMPLEFUSION_H

#include "base/IFusion.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class SimpleFusion : public IFusion {

public:

	enum FUSION_METHOD {		
		MAXIMUM,
		SUM,
		PRODUCT
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: method (MAXIMUM) {			
			addOption ("method", &method, 1, SSI_INT, "fusion method ( 0=MAXIMUM, 1=SUM, 2=PRODUCT )");
		};

		FUSION_METHOD method;
	};

public:

	static const ssi_char_t *GetCreateName () { return "ssi_fusion_SimpleFusion"; };
	static IObject *Create (const ssi_char_t *file) { return new SimpleFusion (file); };
	
	SimpleFusion::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Maximum / sum / product decision fusion."; };

	void setLogLevel (ssi_size_t log_level) {};

	bool train (ssi_size_t n_models,
		IModel **models,
		ISamples &samples);
	bool isTrained () { return _is_trained; };
	bool forward (ssi_size_t n_models,
		IModel **models,
		ssi_size_t n_streams,
		ssi_stream_t **streams,
		ssi_size_t n_probs,
		ssi_real_t *probs);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	ssi_size_t getModelNumber(ISamples &samples){ return samples.getStreamSize (); };

protected:

	SimpleFusion (const ssi_char_t *file = 0);
	virtual ~SimpleFusion ();
	SimpleFusion::Options _options;
	ssi_char_t *_file;

	bool _is_trained;

};

}

#endif
