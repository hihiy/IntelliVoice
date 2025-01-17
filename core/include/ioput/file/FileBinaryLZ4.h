// FileBinaryLZ4.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2015/04/06
// Copyright (C) 2007-15 University of Augsburg, Lab for Human Centered Multimedia
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

#ifndef SSI_IOPUT_FILEBINARYLZ4_H
#define SSI_IOPUT_FILEBINARYLZ4_H

#include "ioput/file/File.h"
#include "ioput/lz4/lz4frame.h"

namespace ssi {

class FileBinaryLZ4 : public File {

	friend class File;

private:
	LZ4F_compressionContext_t ctx;
	LZ4F_preferences_t prefs;
	int32_t blocksize;
	bool firstRun;

	char* in_buff, * out_buff;
	int32_t outBuffSize;

public:

	bool open();
	bool close();

	bool read (void *ptr, ssi_size_t size, ssi_size_t count);
	bool write (const void *ptr, ssi_size_t size, ssi_size_t count);
	bool readLine (ssi_size_t num, ssi_char_t *string);
	bool writeLine (const ssi_char_t *string);
	
};

}

#endif
