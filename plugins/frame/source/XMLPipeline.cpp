// XMLPipeline.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/29
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

#include "XMLPipeline.h"
#include "ioput/file/FilePath.h"
#include "ioput/option/OptionList.h"
#include "Asynchronous.h"
#include "ioput/file/FileTools.h"
#include "FrameLibCons.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *XMLPipeline::ssi_log_name = "framexml__";

XMLPipeline::VERSION XMLPipeline::DEFAULT_VERSION = XMLPipeline::V1;
const ssi_char_t *XMLPipeline::DATE_VARIABLE_NAME = "$(date)";

XMLPipeline::XMLPipeline ()
: _register_fptr (Factory::RegisterDLL),
	_start_painter (false),
	_start_eboard (false),
	_eboard (0),
	_n_global_confpaths (0),
	_global_confpaths (0),
	_savepipe (false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	_frame = Factory::GetFramework ();
	_eboard = Factory::GetEventBoard ();
}

XMLPipeline::~XMLPipeline () {
	clear ();
}

bool XMLPipeline::compare (const ssi_char_t *s1, const ssi_char_t *s2) {
	if (ssi_strlen (s1) != ssi_strlen (s2)) {
		return false;
	}

	for (ssi_size_t i = 0; i < ssi_strlen (s1); i++) {
		if (tolower (s1[i]) != tolower (s2[i])) {
			return false;
		}
	}

	return true;
}

ssi_char_t *XMLPipeline::applyConfig (ssi_char_t *pipestr, const ssi_char_t *confpath) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "apply config from '%s'", confpath);

	ssi_size_t n_confstr = 0;
	ssi_char_t *confstr = FileTools::ReadAsciiFile (confpath, n_confstr);
	if (confstr) {
		ssi_size_t n_lines = ssi_split_string_count (confstr, '\n');
		if (n_lines > 0) {
			ssi_char_t comment = '#';
			ssi_char_t equal = '=';
			ssi_char_t *key = 0;
			ssi_char_t *value = 0;
			ssi_size_t len = 0;
			ssi_size_t pos = 0;
			ssi_char_t **lines = new ssi_char_t *[n_lines];
			ssi_split_string (n_lines, lines, confstr, '\n');
			for (ssi_size_t i = 0; i < n_lines; i++) {
				ssi_char_t *line = lines[i];
				ssi_strtrim (line);
				len = ssi_strlen (line);
				// empty or comment
				if (len > 2 && line[0] != comment) {
					key = line;
					pos = 0;
					while (pos < len && key[pos] != equal) {
						pos++;
					}
					// no valid key / value pair
					if (pos > 0 && pos < len) {
						key[pos] = '\0';
						value = line + pos + 1;
						pos = 0;
						len = ssi_strlen (value);
						while (pos < len && value[pos] != comment) {
							pos++;
						}
						value[pos] = '\0';
						ssi_strtrim (key);
						ssi_strtrim (value);
						if (ssi_log_level >= SSI_LOG_LEVEL_BASIC) {
							ssi_print ("             %s -> %s\n", key, value);
						}
						// found a valid key / value pair
						ssi_char_t *old = pipestr;
						ssi_char_t *search = ssi_strcat ("$(", key, ")");
						pipestr = ssi_strrepl (pipestr, search, value);
						delete[] search;
						delete[] old;
					}
				}
				delete[] lines[i]; lines[i] = 0;
			}
			delete[] lines;
		}
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s", pipestr);

	delete[] confstr; confstr = 0;
	return pipestr;
}

bool XMLPipeline::parse (const ssi_char_t *filepath, ssi_size_t n_confs, ssi_char_t **confpaths, bool savepipe) {
	printf("wejoäfj %s\n",filepath);

	FilePath fp (filepath);
	ssi_char_t *filepath_with_ext = 0;
	ssi_char_t *local_confpath_with_ext = 0;
	_savepipe = savepipe;

	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_PIPELINE) != 0) {
		filepath_with_ext = ssi_strcat (filepath, SSI_FILE_TYPE_PIPELINE);
	} else {
		filepath_with_ext = ssi_strcpy (filepath);
	}

	ssi_char_t workdir_old[SSI_MAX_CHAR];

#if _WIN32||_WIN64
	::GetCurrentDirectory (SSI_MAX_CHAR, workdir_old);
	::SetCurrentDirectory (fp.getDir ());
#else
	getcwd(workdir_old,SSI_MAX_CHAR);
	chdir(fp.getDir ());
#endif
	ssi_now_friendly(_date);

	if (!_global_confpaths && n_confs > 0) {
		_n_global_confpaths = n_confs;
		_global_confpaths = new ssi_char_t *[_n_global_confpaths];
		ssi_char_t *global_confpath_with_ext = 0;
		for (ssi_size_t i = 0; i < _n_global_confpaths; i++) {
			if (confpaths[i] != 0) {
				FilePath fpc (confpaths[i]);
				if (strcmp (fpc.getExtension (), SSI_FILE_TYPE_PCONFIG) != 0) {
					global_confpath_with_ext = ssi_strcat (confpaths[i], SSI_FILE_TYPE_PCONFIG);
				} else {
					global_confpath_with_ext = ssi_strcpy (confpaths[i]);
				}
				if (ssi_exists (global_confpath_with_ext)) {
					_global_confpaths[i] = global_confpath_with_ext;
				} else {
					ssi_wrn ("could not find config file '%s'", confpaths[i]);
					_global_confpaths[i] = 0;
					delete[] global_confpath_with_ext;
				}
			}
		}
	}

	local_confpath_with_ext = ssi_strcat (fp.getPath (), SSI_FILE_TYPE_PCONFIG);
	bool has_local_config = ssi_exists (local_confpath_with_ext);

	ssi_msg (SSI_LOG_LEVEL_BASIC, "load '%s' (local config=%s, global config=%s)", filepath_with_ext, has_local_config ? "yes" : "no", _n_global_confpaths > 0 ? "yes" : "no");

	TiXmlDocument doc;
	//if (_n_global_confpaths > 0 || has_local_config) {

		ssi_size_t n_pipestr = 0;
		ssi_char_t *pipestr = FileTools::ReadAsciiFile (filepath_with_ext, n_pipestr);
		if (!pipestr) {
			ssi_wrn("failed reading pipeline '%s'", filepath_with_ext);
			return false;
		}

		// apply global config files
		if (_n_global_confpaths > 0) {
			for (ssi_size_t i = 0; i < _n_global_confpaths; i++) {
				if (_global_confpaths[i]) {
					pipestr = applyConfig (pipestr, _global_confpaths[i]);
					if (!pipestr) {
						ssi_wrn ("failed applying config '%s' to pipeline '%s'", filepath_with_ext, _global_confpaths[i]);
						return false;
					}
				}
			}
		}

		// apply local config file
		if (has_local_config) {
			pipestr = applyConfig (pipestr, local_confpath_with_ext);
			if (!pipestr) {
				ssi_wrn ("failed applying config '%s' to pipeline '%s'", filepath_with_ext, local_confpath_with_ext);
				return false;
			}
		}

		// apply timestamp
		ssi_char_t *old = pipestr;
		pipestr = ssi_strrepl(pipestr, DATE_VARIABLE_NAME, _date);
		delete[] old;

		if (_savepipe) {
			const ssi_char_t *savepath = ssi_strcat (filepath_with_ext, "~");
			FileTools::WriteAsciiFile (savepath, pipestr);
			delete[] savepath;
		}

		doc.Parse (pipestr, 0, TIXML_ENCODING_UTF8);

		if (doc.Error ()) {
			ssi_wrn ("failed parsing pipeline from file '%s' (r:%d,c:%d)", filepath_with_ext, doc.ErrorRow (), doc.ErrorCol ());
			return false;
		}

		delete[] pipestr;

	/*} else {
		if (!doc.LoadFile (filepath_with_ext)) {
			ssi_wrn ("failed loading pipeline from file '%s' (r:%d,c:%d)", filepath_with_ext, doc.ErrorRow (), doc.ErrorCol ());
			return false;
		}
	}*/

	TiXmlElement *body = doc.FirstChildElement();
	if (!body || strcmp (body->Value (), "pipeline") != 0) {
		ssi_wrn ("tag <pipeline> missing (r:%d,c:%d)", body->Row (), body->Column ());
		delete[] filepath_with_ext;
		return false;
	}

	TiXmlElement *element = body->FirstChildElement ();
	do {
		if (!parseElement (element)) {
			ssi_wrn ("could not load pipeline from file %s (r:%d,c:%d)", filepath_with_ext, element->Row (), element->Column ());
			delete[] filepath_with_ext;
			return false;
		}
	} while (element = element->NextSiblingElement ());
	#if _WIN32||_WIN64
	::SetCurrentDirectory (workdir_old);
	#else
	chdir(workdir_old);
	#endif
	delete[] filepath_with_ext;
	delete[] local_confpath_with_ext;

	return true;
}

bool XMLPipeline::parseElement (TiXmlElement *element) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "parse element '%s'", element->Value ());

	if (strcmp (element->Value (), "include") == 0) {
		return parseInclude (element);
	} else if (strcmp (element->Value (), "register") == 0) {
		return parseRegister (element);
	} else if (strcmp (element->Value (), "sensor") == 0) {
		return parseSensor (element);
	} else if (strcmp (element->Value (), "transformer") == 0) {
		return parseTransformer (element);
	} else if (strcmp (element->Value (), "consumer") == 0) {
		return parseConsumer (element);
	} else if (strcmp (element->Value (), "option") == 0) {
		return parseOption (element);
	} else if (strcmp (element->Value (), "framework") == 0) {
		return parseFramework (element);
	} else if (strcmp (element->Value (), "eventboard") == 0) {
		return parseEventBoard (element);
	} else if (strcmp (element->Value (), "painter") == 0) {
		return parsePainter (element);
	} else if (strcmp (element->Value (), "listener") == 0) {
		ssi_wrn ("found deprecated 'listener' element, use 'object'");
		return parseListener (element);
	} else if (strcmp (element->Value (), "thread") == 0) {
		return parseThread (element);
	} else if (strcmp (element->Value (), "execute") == 0) {
		return parseExecute (element);
	} else if (strcmp (element->Value (), "gate") == 0) {
		return parseGate (element);
	} else if (strcmp (element->Value (), "object") == 0) {
		IObject *object = parseObject (element, true);
		return object != 0;
	} else {
		ssi_wrn ("unkown element '%s'", element->Value ());
		return false;
	}
}

bool XMLPipeline::parseFramework (TiXmlElement *element) {

	IObject *object = Factory::GetFramework ();

	TiXmlAttribute *attribute = element->FirstAttribute ();
	while (attribute)
	{
		if (strcmp (attribute->Name(), "create") != 0 && strcmp (attribute->Name(), "option") != 0 ) {
			if (object->getOptions ()->setOptionValueFromString (attribute->Name (), attribute->Value ())) {
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: set option <'%s'='%s'>", element->Value (), attribute->Name(), attribute->Value ());
			}
		}
		attribute = attribute->Next();
	}

	return true;
}

bool XMLPipeline::parseEventBoard (TiXmlElement *element) {

	IObject *object = Factory::GetEventBoard ();

	TiXmlAttribute *attribute = element->FirstAttribute ();
	while (attribute)
	{
		if (strcmp (attribute->Name(), "create") != 0 && strcmp (attribute->Name(), "option") != 0 ) {
			if (object->getOptions ()->setOptionValueFromString (attribute->Name (), attribute->Value ())) {
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: set option <'%s'='%s'>", element->Value (), attribute->Name(), attribute->Value ());
			}
		}
		attribute = attribute->Next();
	}

	return true;
}

bool XMLPipeline::parsePainter (TiXmlElement *element) {

	IObject *object = Factory::GetPainter ();

	TiXmlAttribute *attribute = element->FirstAttribute ();
	while (attribute)
	{
		if (strcmp (attribute->Name(), "create") != 0 && strcmp (attribute->Name(), "option") != 0 ) {
			if (object->getOptions ()->setOptionValueFromString (attribute->Name (), attribute->Value ())) {
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: set option <'%s'='%s'>", element->Value (), attribute->Name(), attribute->Value ());
			}
		}
		attribute = attribute->Next();
	}

	return true;
}

bool XMLPipeline::parseExecute (TiXmlElement *element) {

	const ssi_char_t *name = 0;
	name = element->Attribute ("name");
	if (!name) {
		ssi_wrn ("execute: attribute 'name' is missing");
		return false;
	}

	const ssi_char_t *args = element->Attribute ("args");

	int wait = -1;
	const ssi_char_t *wait_s = element->Attribute ("wait");
	if (wait_s) {
		if (sscanf (wait_s, "%d", &wait) != 1) {
			ssi_wrn ("could not parse 'wait' argument (='%s'), using default value", wait_s);
			wait = -1;
		}
	}

	ITheFramework::EXECUTE::list type = ITheFramework::EXECUTE::NOW;
	const ssi_char_t *type_s = element->Attribute ("type");
	if (type_s) {
		if (compare (type_s, "pre")) {
			type = ITheFramework::EXECUTE::PRE;
		} else if (compare (type_s, "post")) {
			type = ITheFramework::EXECUTE::POST;
		} else if (compare (type_s, "now")) {
			type = ITheFramework::EXECUTE::NOW;
		} else {
			ssi_wrn ("could not parse 'type' argument (='%s'), using default value", type_s);
		}
	}

	_frame->AddExeJob (name, args, type, wait);

	return true;
}

bool XMLPipeline::parseInclude (TiXmlElement *element) {

	const ssi_char_t *name = 0;
	name = element->Attribute ("name");
	if (!name) {
		ssi_wrn ("insert: attribute 'name' is missing");
		return false;
	}
	bool skip = false;
	const ssi_char_t *skip_s = element->Attribute ("skip");
	if (skip_s) {
		if (compare (skip_s, "true")) {
			skip = true;
		}
	}

	if (skip) {
		ssi_msg (SSI_LOG_LEVEL_BASIC, "skip include '%s'", name);
		return true;
	} else {
		ssi_msg (SSI_LOG_LEVEL_BASIC, "include '%s'", name);

		// executable directory
		FilePath fp (name);
		ssi_char_t fullpath[SSI_MAX_CHAR];
		if (fp.isRelative ()) {
			ssi_char_t workdir[SSI_MAX_CHAR];
			#if _WIN32||_WIN64
			::GetCurrentDirectory (SSI_MAX_CHAR, workdir);
			ssi_sprint (fullpath, "%s\\%s", workdir, fp.getPath ());
			#else
				getcwd(workdir,SSI_MAX_CHAR);
				ssi_sprint (fullpath, "%s/%s", workdir, fp.getPath ());
			#endif
			
		} else {
			strcpy (fullpath, name);
		}

		return parse (fullpath, _n_global_confpaths, _global_confpaths, _savepipe);
	}
}

bool XMLPipeline::parseGate (TiXmlElement *element) {

	bool open = true;

	const ssi_char_t *close_s = element->Attribute ("close");
	if (close_s) {
		if (compare (close_s, "true")) {
			open = false;
		} else if (compare (close_s, "yes")) {
			open = false;
		} else if (strcmp (close_s, "1") == 0) {
			open = false;
		} else if (compare (close_s, "false")) {
			open = true;
		} else if (compare (close_s, "no")) {
			open = true;
		} else if (strcmp (close_s, "0") == 0) {
			open = true;
		} else {
			ssi_wrn ("gate: could not parse 'close' argument (=%s), using default value", close_s);
		}
	}

	const ssi_char_t *open_s = element->Attribute ("open");
	if (open_s) {
		if (compare (open_s, "true")) {
			open = true;
		} else if (compare (open_s, "yes")) {
			open = true;
		} else if (strcmp (open_s, "1") == 0) {
			open = true;
		} else if (compare (open_s, "false")) {
			open = false;
		} else if (compare (open_s, "no")) {
			open = false;
		} else if (strcmp (open_s, "0") == 0) {
			open = false;
		} else {
			ssi_wrn ("gate: could not parse 'open' argument (=%s), using default value", open_s);
		}
	}

	if (!open) {
		return true;
	}

	TiXmlElement *child = element->FirstChildElement ();
	do {
		if (!parseElement (child)) {
			return false;
		}
	} while (child = child->NextSiblingElement ());

	return true;
}

IObject *XMLPipeline::parseObject (TiXmlElement *element, bool auto_free) {

	const ssi_char_t *create = element->Attribute ("create");
	if (!create) {
		ssi_wrn ("%s: attribute 'create' is missing", element->Value ());
		return 0;
	}
	const ssi_char_t *option = element->Attribute ("option");
	if (!option || option[0] == '\0') {
		option = 0;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: <create='%s' option='%s'>", element->Value (), create, option);

	IObject *object = Factory::Create (create, option, auto_free);
	if (!object) {
		ssi_wrn ("%s: could not create object '%s'", element->Value (), create);
		return 0;
	}

	TiXmlAttribute *attribute = element->FirstAttribute ();
	while (attribute)
	{
		if (strcmp (attribute->Name(), "create") != 0 && strcmp (attribute->Name(), "option") != 0) {
			if (object->getOptions ()->setOptionValueFromString (attribute->Name (), attribute->Value ())) {
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: set option <'%s'='%s'>", element->Value (), attribute->Name(), attribute->Value ());
			}
		}
		attribute = attribute->Next();
	}

	if (_eboard && _eboard->RegisterSender (*object)) {
		_start_eboard = true;
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: register as sender <create='%s'>", element->Value (), create);
	}

	if (_eboard) {
		TiXmlElement *listen = element->FirstChildElement ("listen");
		while (listen) {
			const char *address = listen->Attribute ("address");
			if (address) {

				int span = 0;
				const char *span_s = listen->Attribute ("span", &span);

				IEvents::EVENT_STATE_FILTER::List state_filter = IEvents::EVENT_STATE_FILTER::ALL;
				const ssi_char_t *state = listen->Attribute ("state");
				if (state != 0) {
					SSI_DBG (SSI_LOG_LEVEL_DEBUG, "listener->input: <state='%s'>", state);
					if (compare (state, "all")) {
						state_filter = IEvents::EVENT_STATE_FILTER::ALL;
					} else if (compare (state, "continued")) {
						state_filter = IEvents::EVENT_STATE_FILTER::CONTINUED;
					} else if (compare (state, "completed")) {
						state_filter = IEvents::EVENT_STATE_FILTER::COMPLETED;
					} else if (compare (state, "zerodur")) {
						state_filter = IEvents::EVENT_STATE_FILTER::ZERODUR;
					} else {
						ssi_wrn ("unknown state filter '%s'", state);
					}
				}

				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%s: listen <'%s',%d,%s>", element->Value (), address, span, state);
				_eboard->RegisterListener (*object, address, span, state_filter);
			} else {
				ssi_wrn ("%s: attribute 'address' is missing", element->Value ());
			}
			listen = listen->NextSiblingElement ("listen");
		}
	}

	return object;
}


bool XMLPipeline::parseThread (TiXmlElement *element) {

	IObject *thread = ssi_pcast (IObject, parseObject (element));
	_frame->AddRunnable (ssi_pcast (SSI_IRunnableObject, thread));

	return true;
}

bool XMLPipeline::parseRegister (TiXmlElement *element) {

	TiXmlElement *load = element->FirstChildElement ("load");
	const ssi_char_t *name = 0;
	do {
		name = load->Attribute ("name");
		if (!name) {
			ssi_wrn ("register->load: attribute 'name' is missing");
			return false;
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "register->load: <name='%s'>", name);

		_register_fptr (name, ssiout, ssimsg);

		if (strcmp (name, "ssigraphic.dll") == 0
			|| strcmp (name, "ssigraphicd.dll") == 0) {
			_start_painter = true;
		}

	} while (load = load->NextSiblingElement ("load"));

	return true;
}

bool XMLPipeline::parseListener (TiXmlElement *element) {

	IObject *listener = parseObject (element);
	if (!listener) {
		return false;
	}

	ssi_char_t *address = 0;
	ssi_size_t span_ms = 0;
	IEvents::EVENT_STATE_FILTER::List state_filter = IEvents::EVENT_STATE_FILTER::ALL;
	bool is_runnable = false;

	TiXmlElement *input = element->FirstChildElement ("input");
	if (input) {

		const ssi_char_t *listen = input->Attribute ("listen");
		if (listen) {
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "listener->input: <listen='%s'>", listen);
			address = ssi_strcpy (listen);
		}

		int value = 0;
		if (input->Attribute ("span", &value)) {
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "listener->input: <span='%d'>", value);
			span_ms = ssi_cast (ssi_size_t, value);
		}

		const ssi_char_t *state = input->Attribute ("state");
		if (state != 0) {
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "listener->input: <state='%s'>", state);
			if (compare (state, "all")) {
				state_filter = IEvents::EVENT_STATE_FILTER::ALL;
			} else if (compare (state, "continued")) {
				state_filter = IEvents::EVENT_STATE_FILTER::CONTINUED;
			} else if (compare (state, "completed")) {
				state_filter = IEvents::EVENT_STATE_FILTER::COMPLETED;
			} else if (compare (state, "zerodur")) {
				state_filter = IEvents::EVENT_STATE_FILTER::ZERODUR;
			} else {
				ssi_wrn ("unknown state filter '%s'", state);
			}
		}
	}

	if (_eboard) {
		_eboard->RegisterListener (*listener, address, span_ms, state_filter);
	}

	return true;
}

bool XMLPipeline::parseSensor (TiXmlElement *element) {

	ISensor *sensor = ssi_pcast (ISensor, parseObject (element));
	if (!sensor) {
		return false;
	}

	TiXmlElement *provider = element->FirstChildElement ("provider");
	if (provider) {
		do {

			const ssi_char_t *provider_pin = provider->Attribute ("pin");
			if (!provider_pin) {
				ssi_wrn ("sensor->provider: attribute 'to' is missing");
				return false;
			}

			const ssi_char_t *provider_set = provider->Attribute ("channel");
			if (!provider_set) {
				ssi_wrn ("sensor->provider: attribute 'channel' is missing");
				return false;
			}

			const ssi_char_t *provider_size = provider->Attribute ("size");
			ssi_time_t size = THEFRAMEWORK_DEFAULT_BUFFER_CAP;
			if (provider_size) {
				size = atof (provider_size);
			}

			const ssi_char_t *provider_check = provider->Attribute ("check");
			ssi_time_t check = 1.0;
			if (provider_check) {
				check = atof (provider_check);
			}

			const ssi_char_t *provider_sync = provider->Attribute ("sync");
			ssi_time_t sync = 1.0;
			if (provider_sync) {
				sync = atof (provider_sync);
			}


			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sensor->provider: <pin='%s', channel='%s', size='%.2lfs', check='%.2lfs', sync='%.2lfs'>", provider_pin, provider_set, size, check, sync);

			IFilter *provider_transformer = 0;
			TiXmlElement *provider_transformer_element = provider->FirstChildElement ("transformer");
			if (provider_transformer_element) {
				provider_transformer = ssi_pcast (IFilter, parseObject (provider_transformer_element));
				if (!provider_transformer) {
					return false;
				}
			}

			ITransformable *provider = _frame->AddProvider (sensor, provider_set, provider_transformer, size, check, sync);
			if (!provider) {
				ssi_wrn ("sensor: could not set provider '%s'", provider_set);
				return false;
			}

			_transformable_map.insert (transformable_pair_t (String (provider_pin), provider));

		} while (provider = provider->NextSiblingElement ("provider"));
	} else {
		ssi_wrn ("sensor: element 'provider' is missing");
		return false;
	}

	_frame->AddSensor (sensor);

	return true;
}

bool XMLPipeline::parseTransformer (TiXmlElement *element) {

	ITransformer *transformer = ssi_pcast (ITransformer, parseObject (element));
	if (!transformer) {
		return false;
	}

	ssi_size_t xinput_size = 0;
	const ssi_char_t **xinput_pins = 0;

	TiXmlElement *xinput = element->FirstChildElement ("xinput");
	if (xinput) {

		getChildSize(xinput, "input", xinput_size);

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "transformer->xinput: <size='%u'>", xinput_size);

		if (xinput_size > 0) {

			xinput_pins = new const ssi_char_t *[xinput_size];

			TiXmlElement *input = 0;
			for (ssi_size_t i = 0; i < xinput_size; i++) {
				input = input ? input->NextSiblingElement ("input") : xinput->FirstChildElement ("input");
				if (input) {
					xinput_pins[i] = input->Attribute ("pin");
					if (!xinput_pins[i]) {
						ssi_wrn ("transformer->xinput->input#%d: attribute 'pin' is missing", i);
						return false;
					}
				} else {
					ssi_wrn ("transformer: element#%d 'xinput->input' is missing", i);
					return false;
				}

				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "transformer->xinput->input#%d: <pin='%s'>", i, xinput_pins[i]);

			}
		}

	}

	TiXmlElement *input = element->FirstChildElement ("input");
	const ssi_char_t *input_pin = 0;
	const ssi_char_t *input_frame_size = 0;
	const ssi_char_t *input_delta_size = 0;
	bool input_async = 0;

	if (input) {

		input_pin = input->Attribute ("pin");
		if (!input_pin) {
			ssi_wrn ("transformer->input: attribute 'pin' is missing");
			return false;
		}

		input_frame_size = input->Attribute ("frame");
		if (!input_frame_size) {
			ssi_wrn ("transformer->input: attribute 'frame' is missing");
			return false;
		}

		input_delta_size = input->Attribute ("delta");
		if (!input_delta_size || input_delta_size[0] == '\0') {
			input_delta_size = 0;
		}

		const ssi_char_t *input_async_s = input->Attribute ("async");
		if (!input_async_s || input_async_s[0] == '\0') {
			input_async = false;
		} else {
			if (compare ("true", input_async_s)) {
				input_async = true;
			} else {
				input_async = false;
			}
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "transformer->input: <pin='%s' frame='%s' delta='%s' async='%s'>", input_pin, input_frame_size, input_delta_size, input_async ? "true" : "false");

	} else {
		ssi_wrn ("transformer: element 'input' is missing");
		return false;
	}

	ITransformable *input_transformable = getTransformable (input_pin);
	if (!input_transformable) {
		ssi_wrn ("transformer: could not find input");
		return false;
	}

	TiXmlElement *output = element->FirstChildElement ("output");
	ssi_time_t size = THEFRAMEWORK_DEFAULT_BUFFER_CAP;
	const ssi_char_t *output_pin;
	if (output) {
		output_pin = output->Attribute ("pin");
		if (!output_pin) {
			ssi_wrn ("transformer->output: attribute 'pin' is missing");
			return false;
		}

		const ssi_char_t *output_size = output->Attribute ("size");
		if (output_size) {
			size = atof (output_size);
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "transformer->output: <pin='%s', size='%.2lf'>", output_pin, size);

	} else {
		ssi_wrn ("transformer: input '%s' is missing", input_pin);
		return false;
	}

	ITransformable **xinput_transformable = 0;
	if (xinput_size > 0) {
		xinput_transformable = new ITransformable *[xinput_size];
		for (ssi_size_t i = 0; i < xinput_size; i++) {
			xinput_transformable[i] = getTransformable (xinput_pins[i]);
			if (!xinput_transformable[i]) {
				ssi_wrn ("consumer: xinput#%d '%s' is missing", i, xinput_pins[i]);
				return false;
			}
		}
	}
	if (input_async) {
		Asynchronous *asynchronous = ssi_pcast (Asynchronous, Factory::Create (Asynchronous::GetCreateName ()));
		asynchronous->setTransformer (transformer);
		ITransformable *output_transformable = output_transformable = _frame->AddTransformer (input_transformable, xinput_size, xinput_transformable, asynchronous, input_frame_size, input_delta_size, size);
		_transformable_map.insert (transformable_pair_t (String (output_pin), output_transformable));
	} else {
		ITransformable *output_transformable = output_transformable = _frame->AddTransformer (input_transformable, xinput_size, xinput_transformable, transformer, input_frame_size, input_delta_size, size);
		_transformable_map.insert (transformable_pair_t (String (output_pin), output_transformable));
	}

	delete[] xinput_pins;
	delete[] xinput_transformable;

	return true;
}

bool XMLPipeline::parseConsumer (TiXmlElement *element) {

	IConsumer *consumer = ssi_pcast (IConsumer, parseObject (element));
	if (!consumer) {
		return false;
	}
	_consumer_map.insert (consumer_pair_t (String (consumer->getName ()), consumer));

	ssi_size_t input_size = 0;
	const ssi_char_t **input_pins = 0;
	ITransformer **input_transformers = 0;
	const ssi_char_t *trigger_pin = 0;

	TiXmlElement *xinput = element->FirstChildElement ("xinput");
	if (xinput) {

		ssi_size_t xinput_size;
		getChildSize(xinput, "input", xinput_size);

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "consumer->xinput: <size='%u'>", xinput_size);

		if (xinput_size > 0) {

			input_size = xinput_size + 1;
			input_pins = new const ssi_char_t *[input_size];
			input_transformers = new ITransformer *[input_size];

			TiXmlElement *input = 0;
			for (ssi_size_t i = 0; i < xinput_size; i++) {
				input = input ? input->NextSiblingElement ("input") : xinput->FirstChildElement ("input");
				if (input) {
					input_pins[i+1] = input->Attribute ("pin");
					if (!input_pins[i+1]) {
						ssi_wrn ("consumer->xinput->input#%d: attribute 'pin' is missing", i+1);
						return false;
					}
				} else {
					ssi_wrn ("consumer: element#%d 'xinput->input' is missing", i+1);
					return false;
				}

				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "consumer->xinput->input#%d: <pin='%s'>", i+1, input_pins[i+1]);

				TiXmlElement *input_transformer_element = input->FirstChildElement ("transformer");
				if (input_transformer_element) {
					input_transformers[i+1] = ssi_pcast (ITransformer, parseObject (input_transformer_element));
					if (!input_transformers[i+1]) {
						return false;
					}
				} else {
					input_transformers[i+1] = 0;
				}

			}
		}

	} else {
		input_size = 1;
		input_pins = new const ssi_char_t *[input_size];
		input_transformers = new ITransformer *[input_size];
	}

	TiXmlElement *input = element->FirstChildElement ("input");
	const ssi_char_t *input_event = 0;
	const ssi_char_t *input_frame_size = 0;
	const ssi_char_t *input_delta_size = 0;
	if (input) {

		input_pins[0] = input->Attribute ("pin");
		if (!input_pins[0]) {
			ssi_wrn ("consumer->input: attribute 'pin' is missing");
			return false;
		}

		trigger_pin = input->Attribute ("trigger");

		input_event = input->Attribute ("listen");
		if (input_event) {

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "consumer->input: <pin='%s' listen='%s'>", input_pins[0], input_event);

		} else {
			input_frame_size = input->Attribute ("frame");
			if (!input_frame_size) {
				ssi_wrn ("consumer->input: attribute 'frame' is missing");
				return false;
			}

			input_delta_size = input->Attribute ("delta");
			if (!input_delta_size || input_delta_size[0] == '\0') {
				input_delta_size = 0;
			}

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "consumer->input: <pin='%s' frame='%s' delta='%s' trigger='%s'>", input_pins[0], input_frame_size, input_delta_size, trigger_pin);
		}

		TiXmlElement *input_transformer_element = input->FirstChildElement ("transformer");
		if (input_transformer_element) {
			input_transformers[0] = ssi_pcast (ITransformer, parseObject (input_transformer_element));
			if (!input_transformers[0]) {
				return false;
			}
		} else {
			input_transformers[0] = 0;
		}

	} else {
		ssi_wrn ("consumer: element 'input' is missing");
		return false;
	}

	ITransformable **input_transformable = new ITransformable *[input_size];
	for (ssi_size_t i = 0; i < input_size; i++) {
		input_transformable[i] = getTransformable (input_pins[i]);
		if (!input_transformable[i]) {
			ssi_wrn ("consumer: input#%d '%s' is missing", i, input_pins[i]);
			return false;
		}
	}

	if (input_event) {
		if (_eboard) {
			_frame->AddEventConsumer (input_size, input_transformable, consumer, _eboard, input_event, input_transformers);
		}
	} else {
		ITransformable *input_trigger = 0;
		if (trigger_pin) {
			input_trigger = getTransformable (trigger_pin);
			if (!input_trigger) {
				ssi_wrn ("consumer: trigger input '%s' is missing", trigger_pin);
				return false;
			}
		}
		_frame->AddConsumer (input_size, input_transformable, consumer, input_frame_size, input_delta_size, input_transformers, input_trigger);
	}

	delete[] input_pins;
	delete[] input_transformable;
	delete[] input_transformers;

	return true;
}

bool XMLPipeline::parseOption (TiXmlElement *element) {

	const ssi_char_t *create = 0;
	create = element->Attribute ("create");
	if (!create) {
		ssi_wrn ("option: attribute 'create' is missing");
		return false;
	}

	const ssi_char_t *option = 0;
	option = element->Attribute ("option");
	if (!option) {
		ssi_wrn ("option: attribute 'option' is missing");
		return false;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "option: <create='%s', option='%s'>", create, option);

	IObject *object = Factory::Create (create, option, false);
	if (!object) {
		ssi_wrn ("consumer: could not create object '%s'", create);
		return false;
	}

	delete object;

	TiXmlElement *set = element->FirstChildElement ("set");
	if (!set) {
		return true;
	}

	const ssi_char_t *option_name = 0;
	const ssi_char_t *option_value = 0;
	do {
		option_name = set->Attribute ("name");
		if (!option_name) {
			ssi_wrn ("option->set: attribute 'name' is missing");
			return false;
		}

		option_value = set->Attribute ("value");
		if (!option_value) {
			ssi_wrn ("option->set: attribute 'value' is missing");
			return false;
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "option->set: <name='%s', value='%s'>", option_name, option_value);

		if (!OptionList::SetOptionValueInPlace (option, option_name, option_value)) {
			ssi_wrn ("option->set: could no set option '%s'", option_name);
			return false;
		}

	} while (set = set->NextSiblingElement ("set"));

	return true;
}

ITransformable *XMLPipeline::getTransformable (const ssi_char_t *name) {

	String string (name);
	transformable_map_t::iterator it;

	it = _transformable_map.find (name);

	if (it == _transformable_map.end ()) {
		ssi_wrn ("transformable '%s' not found", name);
		return 0;
	}

	return it->second;
}

IConsumer *XMLPipeline::getConsumer (const ssi_char_t *name) {

	String string (name);
	consumer_map_t::iterator it;

	it = _consumer_map.find (name);

	if (it == _consumer_map.end ()) {
		ssi_wrn ("consumer '%s' not found", name);
		return 0;
	}

	return it->second;
}

void XMLPipeline::clear () {

	_transformable_map.clear ();
	_start_painter = false;
	_start_eboard = false;
	for (ssi_size_t i = 0; i < _n_global_confpaths; i++) {
		delete[] _global_confpaths[i];
	}
	delete[] _global_confpaths; _global_confpaths = 0;
	_n_global_confpaths = 0;
	_savepipe = false;
}

bool XMLPipeline::getChildSize(TiXmlElement *mama, const ssi_char_t *value, ssi_size_t &n_childs) {

	n_childs = 0;

	TiXmlElement *element = mama->FirstChildElement(value);
	if (!element || strcmp(element->Value(), value) != 0) {
		ssi_wrn("tag <%s> missing", value);
		return false;
	}

	do {
		n_childs++;
		element = element->NextSiblingElement(value);
	} while (element);

	return true;
}

}
