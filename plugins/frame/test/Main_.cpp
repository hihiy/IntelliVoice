// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/23
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

#include "ssi.h"
using namespace ssi;

#include "MinMaxFeat.h"
#include "AbsFilt.h"
#include "MyConsumer.h"
#include "RampFilt.h"
#include "MeanFeat.h"
#include "Delay.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define PI 3.14159

bool ex_buffer (void *arg);
bool ex_timebuffer(void *arg);
bool ex_framework(void *arg);
bool ex_sensor(void *arg);
bool ex_event(void *arg);
bool ex_trigger(void *arg);
bool ex_async(void *arg);
bool ex_msg(void *arg);
bool ex_sync(void *arg);
bool ex_timeserver(void *arg);
bool ex_xml(void *arg);
bool ex_export(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	ssimsg = new FileMessage("ssi.log");

	Factory::RegisterDLL ("ssievent", 0, ssimsg);
	Factory::RegisterDLL ("ssiframe", 0, ssimsg);
	//Factory::RegisterDLL ("ssigraphic", 0, ssimsg);
	Factory::RegisterDLL ("ssimouse", 0, ssimsg);
	Factory::RegisterDLL ("ssiioput", 0, ssimsg);

	ssi_random_seed ();

	Exsemble ex;
	ex.add(&ex_msg, 0, "MESSAGE", "How to output ssi formated messages.");
	ex.add(&ex_buffer, 0, "BUFFER", "How to use 'Buffer' class.");
	ex.add(&ex_timebuffer, 0, "TIMEBUFFER", "How to use 'TimeBuffer' class.");
	ex.add(&ex_framework, 0, "FRAMEWORK", "How to set up a pipeline using 'TheFramework'.");
	ex.add(&ex_sensor, 0, "SENSOR", "How to use input from a live sensor");
	ex.add(&ex_event, 0, "EVENT", "How to send/receive events in a pipeline.");
	ex.add(&ex_trigger, 0, "TRIGGER", "How to use a stream to trigger a consumer.");
	ex.add(&ex_async, 0, "ASYNC", "How to run a transformer asynchronously.");
	ex.add(&ex_sync, 0, "SYNC", "How to sync two separate pipelines.");
	ex.add(&ex_timeserver, 0, "TIMESERVER", "How to use pipeline as a timeserver.");
	ex.add(&ex_xml, 0, "XML", "How to run a xml pipeline from code.");
	ex.add(&ex_export, 0, "EXPORT", "How to export dlls used by a pipeline to a directory.");
	ex.show();

	Factory::Clear ();
	delete ssimsg; ssimsg = 0;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_buffer(void *arg) {

	Buffer buffer (50);

	{
		char data_in = '5';
		char data_out;
		buffer.put ((char *) &data_in, 0, sizeof (char));
		buffer.get ((char *) &data_out, 0, sizeof (char));
		ssi_print ("%c\n", data_out);
	}

	{
		int data_in = 5;
		int data_out;
		buffer.put ((char *) &data_in, 1, sizeof (int));
		buffer.get ((char *) &data_out, 1, sizeof (int));
		ssi_print ("%d\n", data_out);
	}

	{
		float data_in = 5.0f;
		float data_out;
		buffer.put ((char *) &data_in, 2, sizeof (float));
		buffer.get ((char *) &data_out, 2, sizeof (float));
		ssi_print ("%f\n", data_out);
	}

	{
		double data_in = 5.0;
		double data_out = 5.0;
		buffer.put ((char *) &data_in, 3, sizeof (double));
		buffer.get ((char *) &data_out, 3, sizeof (double));
		ssi_print ("%lf\n", data_out);
	}

	{
		int data_in[10] = {1,2,3,4,5,6,7,8,9,10};
		int data_out[10];
		buffer.put ((char *) data_in, 10, 10 * sizeof (int));
		buffer.get ((char *) data_out, 10, 10 * sizeof (int));
		for (int i = 0; i < 10; i++)
			ssi_print ("% d", data_out[i]);
		ssi_print ("\n");
	}

	return true;
}

bool ex_timebuffer(void *arg) {

	/// TIME BUFFER TESTS ///

	// creates a buffer for 1 second of data
	// sampled at 10Hz and int as sample type
	TimeBuffer tBuffer (1.0, 10.0, 1, sizeof (int), SSI_INT);

	ssi_print ("time: %lds\n", tBuffer.getCurrentSampleTime ());

	{
		int data[5] = {1,2,3,4,5};
		tBuffer.push ((char *) data, 5);
	}

	ssi_print ("time: %lds\n", tBuffer.getCurrentSampleTime ());

	{
		ssi_size_t elements = 0;
		ssi_size_t elements_out;
		int *data = 0;
		TimeBuffer::STATUS status = tBuffer.get (reinterpret_cast<char **> (&data), elements, elements_out, 0, 0.25);
		if (status == TimeBuffer::SUCCESS) {
			for (ssi_size_t i = 0; i < elements_out; i++)
				ssi_print ("%d ", data[i]);
			ssi_print ("\n");
		} else {
			ssi_print ("ERROR: %d\n", status);
		}
		delete[] data;
	}

	{
		int data[5] = {6,7,8,9,10};
		tBuffer.push ((char *) data, 5);
	}

	ssi_print ("time: %lds\n", tBuffer.getCurrentSampleTime ());

	{
		ssi_size_t elements = static_cast<ssi_size_t> (0.25 * tBuffer.sample_rate + 0.5);
		ssi_size_t elements_out;
		int *data = new int[elements];
		TimeBuffer::STATUS status = tBuffer.get (reinterpret_cast<char **> (&data), elements, elements_out, 0.25, 0.25);
		if (status == TimeBuffer::SUCCESS) {
			for (ssi_size_t i = 0; i < elements_out; i++)
				ssi_print ("%d ", data[i]);
			ssi_print ("\n");
		} else {
			ssi_print ("ERROR: %d\n", status);
		}
		delete[] data;
	}

	{
		int data[5] = {11,12,13,14,15};
		tBuffer.push ((char *) data, 5);
	}

	{
		ssi_size_t elements = static_cast<ssi_size_t> (0.25 * tBuffer.sample_rate + 0.5);
		ssi_size_t elements_out;
		int *data = new int[elements];
		TimeBuffer::STATUS status = tBuffer.get (reinterpret_cast<char **> (data), elements, elements_out, 0.25, 0.25);
		if (status == TimeBuffer::SUCCESS) {
			for (ssi_size_t i = 0; i < elements_out; i++)
				ssi_print ("%d ", data[i]);
			ssi_print ("\n");
		} else {
			ssi_print ("ERROR: %d\n", status);
		}
		delete[] data;
	}

	return true;
}

bool ex_framework (void *arg) {

	TheFramework *frame = ssi_pcast (TheFramework, Factory::GetFramework ());
	//frame->getOptions ()->monitor = true;
	//frame->getOptions ()->setMonitorPos (800, 0, 400, 200);
	//frame->getOptions ()->console = true;
	//frame->getOptions ()->setConsolePos (800, 200, 400, 200);
	//ThePainter *painter = ssi_pcast (ThePainter, Factory::GetPainter ());

	double sample_rate = 200.0;
	double frequency = 5.0;
	double sine_len = 60.0;
	double plot_dur = 11.1;
	const ssi_char_t *print_dur = "1.0s";
	double provide_size = 0.1;
	bool repeat = true;

	unsigned int len = static_cast<unsigned int> (sample_rate * sine_len);
	double *data = new double [len];
	data[0] = 0.0;
	double delta = 1/sample_rate;
	for (unsigned int i = 1; i < len; i++) {
		data[i] = data[i-1] + delta;
	}
	for (unsigned int i = 0; i < len; i++) {
		data[i] = sin (2.0 * PI * frequency * data[i]);
	}

	//Provider::SetLogLevel (SSI_LOG_LEVEL_DEBUG);
	//Transformer::SetLogLevel (SSI_LOG_LEVEL_DEBUG);
	//Consumer::SetLogLevel (SSI_LOG_LEVEL_DEBUG);
	frame->SetLogLevel (SSI_LOG_LEVEL_DEBUG);

	FileReader *simulator = ssi_create (FileReader, 0, true);
	simulator->getOptions ()->setPath ("input");
	simulator->getOptions ()->loop = true;
	simulator->getOptions ()->block = provide_size;
	ITransformable *provider = frame->AddProvider (simulator, SSI_FILEREADER_PROVIDER_NAME);
	//simulator.setLogLevel (SSI_LOG_LEVEL_DEBUG);
	frame->AddSensor (simulator);

	AbsFilt<float> abs;
	RampFilt<float> ramp;
	IFilter *filter[2] = {&abs, &ramp};
	MeanFeat<float> mean;
	MinMaxFeat<float> minmax;
	IFeature *feature[2] = {&mean, &minmax};

	//Chain chain (2, filter, 2, feature);
	Chain *chain = ssi_create (Chain, 0, true);
	chain->set (2, filter, 2, feature);
	ITransformable *chain_t = frame->AddTransformer (provider, chain, "0.05s", "0.05s");

	MyConsumer my_consumer_;
	//Chain chain_2 (2, filter, 2, feature);
	Chain *chain_2 = ssi_create (Chain, 0, true);
	chain_2->set (2, filter, 2, feature);
	frame->AddConsumer (provider, &my_consumer_, "0.05s", "0.05s", chain_2);

/*
	SignalPainter *signal_painter = ssi_create (SignalPainter, 0, true);
	signal_painter->getOptions ()->setName ("signal");
	signal_painter->getOptions ()->size = plot_dur;
	frame->AddConsumer (provider, signal_painter, "0.1s");

	SignalPainter *chain_painter = ssi_create (SignalPainter, 0, true);
	chain_painter->getOptions ()->setName ("chain");
	chain_painter->getOptions ()->size = plot_dur;
	frame->AddConsumer (chain_t, chain_painter, "0.1s");
*/
	FileWriter *signal_printer = ssi_create (FileWriter, 0, true);
	signal_printer->getOptions ()->type = File::ASCII;
	signal_printer->getOptions ()->setPath ("signal.txt");
	frame->AddConsumer (provider, signal_printer, print_dur);

	FileWriter *signal_bprinter = ssi_create (FileWriter, 0, true);
	signal_bprinter->getOptions ()->setPath ("signal.dat");
	frame->AddConsumer (provider, signal_bprinter, print_dur);

	FileWriter *chain_printer = ssi_create (FileWriter, 0, true);
	chain_printer->getOptions ()->type = File::ASCII;
	chain_printer->getOptions ()->setPath ("chain.txt");
	frame->AddConsumer (chain_t, chain_printer, print_dur);

	FileWriter *chain_bprinter = ssi_create (FileWriter, 0, true);
	chain_bprinter->getOptions ()->setPath ("chain.dat");
	frame->AddConsumer (chain_t, chain_bprinter, print_dur);

	frame->Start ();
	//painter->Arrange (1, 2, 0, 0, 800, 400);
	frame->Wait ();
	frame->Stop ();
	frame->Clear ();
	//painter->Clear ();

	delete[] data;

	return true;
}

bool ex_sensor (void *arg) {

	ITheFramework *frame = Factory::GetFramework ();
	//IThePainter *painter = Factory::GetPainter ();
	//ITheEventBoard *board = Factory::GetEventBoard ();

	Mouse *mouse = ssi_pcast (Mouse, Factory::Create (Mouse::GetCreateName (), "mouse"));
	mouse->getOptions ()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider (mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
    ITransformable *button_p = frame->AddProvider (mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor (mouse);

	ZeroEventSender *ezero = ssi_create (ZeroEventSender, "ezero", true);
	ezero->getOptions ()->mindur = 0.2;
    frame->AddConsumer (cursor_p, ezero, "0.25s");
/*
	board->RegisterSender (*ezero);

	SignalPainter *sigpaint = ssi_create (SignalPainter, 0, true);
	sigpaint->getOptions ()->setName ("mouse");
	sigpaint->getOptions ()->size = 2.0;
	frame->AddConsumer (cursor_p, sigpaint, "0.2s");

	sigpaint= ssi_create (SignalPainter, 0, true);
	sigpaint->getOptions ()->setName ("mouse (tr)");
	frame->AddEventConsumer (cursor_p, sigpaint, board, ezero->getEventAddress ());
*/
	frame->Start ();
	//board->Start ();
	//painter->Arrange (2, 1, 0, 0, 800, 400);
	frame->Wait ();
	//board->Stop ();
	frame->Stop ();
	frame->Clear ();
	//board->Clear ();
	//painter->Clear ();

	return true;
}

bool ex_event (void *arg) {

	ITheFramework *frame = Factory::GetFramework ();
	ssi_pcast(TheFramework, frame)->getOptions()->setConsolePos(800, 200, 400, 200);
	//IThePainter *painter = Factory::GetPainter ();
	//ITheEventBoard *board = Factory::GetEventBoard ();

	Mouse *mouse = ssi_create (Mouse, 0, false);
	mouse->getOptions ()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider (mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider (mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor (mouse);

	// output
/*
	SignalPainter *signal_painter = 0;

	signal_painter = ssi_create (SignalPainter, 0, true);
	signal_painter->getOptions ()->size = 2.0;
	signal_painter->getOptions ()->setName ("cursor");
	frame->AddConsumer (cursor_p, signal_painter, "0.2s");
*/
	// zero event

	ZeroEventSender *ezero = ssi_create (ZeroEventSender, 0, true);
	ezero->getOptions ()->setSender ("mouse");
	ezero->getOptions ()->setEvent ("click");
	ezero->getOptions ()->eager = true;
	ezero->getOptions ()->eall = true;
	ezero->getOptions ()->hard = true;
	ezero->getOptions ()->mindur = 0.5;
	ezero->getOptions ()->maxdur = 4.0;
	ezero->getOptions ()->hangin = 5;
	ezero->getOptions ()->hangout = 5;
	ezero->getOptions ()->empty = false;
	ezero->getOptions ()->setString("ezero");
	frame->AddConsumer (button_p, ezero, "0.2s");
	//board->RegisterSender (*ezero);

	FloatsEventSender *fsender = ssi_factory_create (FloatsEventSender, 0, true);
	fsender->getOptions ()->mean = true;
	fsender->getOptions ()->setEventName ("position");
	fsender->getOptions ()->setSenderName ("mouse");
	//frame->AddEventConsumer (cursor_p, fsender, board, ezero->getEventAddress ());
	//board->RegisterSender (*fsender);
/*
	signal_painter = ssi_create (SignalPainter, 0, true);
	signal_painter->getOptions ()->size = 0;
	signal_painter->getOptions ()->setName ("click event");
	frame->AddEventConsumer (cursor_p, signal_painter, board, ezero->getEventAddress ());
*/
	// thres event

	ThresEventSender *ethres = ssi_create (ThresEventSender, "ethres", true);
	ethres->getOptions ()->setSender ("tes");
	ethres->getOptions ()->setEvent ("te");
	ethres->getOptions ()->hard = false;
	ethres->getOptions ()->mindur = 0.5;
	ethres->getOptions ()->maxdur = 4.0;
	ethres->getOptions ()->hangin = 5;
	ethres->getOptions ()->hangout = 5;
	ethres->getOptions ()->thres = 0.5;
	ethres->getOptions ()->empty = false;
	ethres->getOptions ()->setString("ethres");
	frame->AddConsumer (cursor_p, ethres, "0.2s");
	//board->RegisterSender (*ethres);

	EventConsumer *ethres_c = ssi_create (EventConsumer, 0, true);
	ethres_c->getOptions ()->async = true;
	//board->RegisterListener (*ethres_c, ethres->getEventAddress ());
/*
	signal_painter = ssi_create (SignalPainter, 0, true);
	signal_painter->getOptions ()->size = 0;
	signal_painter->getOptions ()->setName ("thres event");
	ethres_c->AddConsumer (cursor_p, signal_painter);



	// event monitor

	EventMonitor *emonitor = ssi_create (EventMonitor, 0, true);
	emonitor->getOptions ()->setMonitorPos (0, 200, 800, 200);
	board->RegisterListener (*emonitor, 0, 50000);
*/
	// run

	frame->Start ();
	//board->Start ();
	//painter->Arrange (3,1,0,0,800,200);
	frame->Wait ();
	//board->Stop ();
	frame->Stop ();
	frame->Clear ();
	//board->Clear ();
	//painter->Clear ();

	return true;
}

bool ex_trigger (void *arg) {

	ITheFramework *frame = Factory::GetFramework ();
    //ssi_pcast(TheFramework, frame)->getOptions()->setConsolePos(800, 200, 400, 200);

	Mouse *mouse = ssi_create (Mouse, 0, false);
	mouse->getOptions ()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider (mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider (mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor (mouse);

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions ()->type = File::ASCII;
	frame->AddConsumer (cursor_p, writer, "0.2s", 0, 0, button_p);

	frame->Start ();
	frame->Wait ();
	frame->Stop ();
	frame->Clear ();

	return true;
}

bool ex_async (void *arg) {

	ITheFramework *frame = Factory::GetFramework ();
	ssi_pcast(TheFramework, frame)->getOptions()->setConsolePos(800, 200, 400, 200);
	//IThePainter *painter = Factory::GetPainter ();

	Mouse *mouse = ssi_create (Mouse, 0, false);
	ITransformable *cursor_p = frame->AddProvider (mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor (mouse);

	// trans

	Delay delay;
	ITransformable *filter_t = frame->AddTransformer (cursor_p, &delay, "0.2s");

	Asynchronous *delay_async = ssi_create (Asynchronous, 0, true);
	//Asynchronous *delay_async = ssi_pcast (Asynchronous, Asynchronous::Create (0));
	delay_async->setTransformer (&delay);
	ITransformable *filter_async_t = frame->AddTransformer (cursor_p, delay_async, "0.2s");

	// output
/*
	SignalPainter *signal_painter = 0;

	signal_painter = ssi_create (SignalPainter, 0, true);
	signal_painter->getOptions ()->size = 60.0;
	signal_painter->getOptions ()->setName ("cursor");
	frame->AddConsumer (cursor_p, signal_painter, "0.2s");

	signal_painter = ssi_create (SignalPainter, 0, true);
	signal_painter->getOptions ()->size = 60.0;
	signal_painter->getOptions ()->setName ("delay");
	frame->AddConsumer (filter_t, signal_painter, "0.2s");

	signal_painter = ssi_create (SignalPainter, 0, true);
	signal_painter->getOptions ()->size = 60.0;
	signal_painter->getOptions ()->setName ("delay async");
	frame->AddConsumer (filter_async_t, signal_painter, "0.2s");
*/
	// run

	frame->Start ();
	//painter->Arrange (1,3,0,0,800,400);
	frame->Wait ();
	frame->Stop ();
	frame->Clear ();
	//painter->Clear ();

	return true;
}

bool ex_msg(void *arg) {

	int id = 1;

	ssimsg = new FileMessage ();

	ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Debug Message #%d", id);
	id++;
	ssi_wrn("Debug Warning #%d", id);
	id++;
	//ssi_err("Debug Error #%d", id);

	delete ssimsg; ssimsg = 0;

	ssimsg = new SocketMessage (Socket::UDP, 1111, "localhost");

	ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Debug Message #%d", id);
	id++;
	ssi_wrn("Debug Warning #%d", id);
	id++;
	//ssi_err("Debug Error #%d", id);

	delete ssimsg; ssimsg = 0;

	return true;
}

void runSyncClient (void *ptr) {

	ssi_char_t *path = ssi_pcast (ssi_char_t, ptr);

#ifdef DEBUG
	ssi_execute ("..\\..\\..\\..\\bin\\Win32\\vc10\\xmlpiped.exe", path, -1);
#else
	ssi_execute ("..\\..\\..\\..\\bin\\Win32\\vc10\\xmlpipe.exe", path, -1);
#endif

}

bool ex_sync (void *arg) {

    char* name = "client.pipeline";
	RunAsThread client (runSyncClient, name, true);
	client.start ();

	TheFramework *frame = ssi_pcast (TheFramework, Factory::GetFramework ());
	frame->getOptions ()->sync = true;
	frame->getOptions ()->sport = 9999;
	frame->getOptions ()->slisten = false;
    /*
	frame->getOptions ()->setMonitorPos (0,0,400,200);
	frame->getOptions ()->setConsolePos (800, 200, 400, 200);
*/
	Mouse *mouse = ssi_create (Mouse, 0, false);
	mouse->getOptions ()->sr = 5.0;
	ITransformable *cursor_p = frame->AddProvider (mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor (mouse);

	SocketWriter *writer = ssi_create (SocketWriter, 0, true);
	writer->getOptions ()->type = Socket::UDP;
	writer->getOptions ()->port = 9998;
	frame->AddConsumer (cursor_p, writer, "1");

	frame->Start ();
	frame->Wait ();
	frame->Stop ();
	frame->Clear ();

	client.stop ();

	return true;
}

void runTimeClient (void *ptr) {

	int port = *ssi_pcast (int, ptr);

	/*TcpSocket sock;
	sock.create ();
	printf ("request framework time");
	ssi_size_t time;
	while (!sock.connect ("localhost", port)) {
		printf (".");
		sleep_ms (100);
	}
	int n_recv = sock.recv (&time, sizeof (time));
	if (n_recv > 0) {
		printf ("\nreceived time in ms: %u\n", time);
	}

	sock.shutdown (SD_BOTH);
	sock.close ();	*/

	UdpSocket sock;
	sock.Bind (IpEndpointName ("localhost", 459234));

	printf ("request framework time\n");
	char msg = 0;
	IpEndpointName server ("localhost", port);
	sock.SendTo (server, &msg, 1);
	ssi_size_t time;
	int recv = sock.ReceiveFrom (server, ssi_pcast (char, &time), sizeof (ssi_size_t));
	if (recv != -1) {
		printf ("\nreceived time in ms: %u\n", time);
	}

	Sleep (2000);
}

bool ex_timeserver (void *arg) {

	int port = 12345;
	RunAsThread mythread (&runTimeClient, &port);

	TheFramework *frame = ssi_pcast (TheFramework, Factory::GetFramework ());
	frame->getOptions ()->tserver = true;
	frame->getOptions ()->tport = port;

	frame->Start ();

	mythread.start ();

	frame->Wait ();

	mythread.stop ();

	frame->Stop ();
	frame->Clear ();

	return true;
}

bool ex_xml(void *arg) {

	XMLPipeline *xmlpipe = ssi_create(XMLPipeline, 0, false);
	xmlpipe->SetRegisterDllFptr(Factory::RegisterDLL);
	xmlpipe->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	ssi_size_t n_confs = 3;
	ssi_char_t *confs[] = { "global", "global2", "global3" };
	xmlpipe->parse("my", 3, confs, true);

	//IThePainter *painter = Factory::GetPainter();
	ITheFramework *frame = Factory::GetFramework();
    //ITheEventBoard *eboard = Factory::GetEventBoard();

	frame->Start();
    /*
	if (xmlpipe->startEventBoard()) {
		eboard->Start();
	}
   */
	ssi_print("\n\n\tpress enter to continue\n\n");
	getchar();
	/*
	if (xmlpipe->startEventBoard()) {
		eboard->Stop();
	}*/
	frame->Stop();
	frame->Clear();
	//painter->Clear();
	//eboard->Clear();

	delete xmlpipe;

	return true;
}

bool ex_export(void *arg) {

	ITheFramework *frame = Factory::GetFramework();
	XMLPipeline *xmlpipe = ssi_create(XMLPipeline, 0, false);
	ssi_size_t n_confs = 3;
	ssi_char_t *confs[] = { "global", "global2", "global3" };
	xmlpipe->parse("my", 3, confs, false);

	const ssi_char_t *dir = "dlls";
	ssi_mkdir(dir);
	Factory::ExportDlls(dir);

	delete xmlpipe;
	frame->Clear();

	return true;
}
