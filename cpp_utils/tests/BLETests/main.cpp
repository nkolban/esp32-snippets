/**
 * Main file for running the BLE samples.
 */
extern "C" {
	void app_main(void);
}


// The list of sample entry points.
void SampleServer(void);
void Sample1(void);
void SampleRead(void);
void SampleWrite(void);
void SampleScan(void);
void SampleNotify(void);
void SampleClient_Notify(void);
void SampleClient(void);
void Sample_MLE_15(void);


//
// Un-comment ONE of the following
//            ---
void app_main(void) {
	//SampleServer();
	//Sample1();
	//SampleRead();
	//SampleWrite();
	//SampleScan();
	//SampleNotify();
	//SampleClient();
	SampleClient_Notify();
	//Sample_MLE_15();
} // app_main
