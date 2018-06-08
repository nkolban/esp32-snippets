/*
 * DMA.h
 *
 *  Created on: May 27, 2018
 *      Author: kolban
 */

#ifndef MAIN_DMA_H_
#define MAIN_DMA_H_

class DMA {
public:
	DMA();
	virtual ~DMA();
	void clearInterupts();
	void dumpBuffer();
	void dumpStatus();
	void reset();
	void setCameraMode();
	void start();
	void startRX();
	void stopRX();

};

#endif /* MAIN_DMA_H_ */
