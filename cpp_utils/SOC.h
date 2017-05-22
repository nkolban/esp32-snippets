/*
 * SOC.h
 *
 *  Created on: May 16, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SOC_H_
#define COMPONENTS_CPP_UTILS_SOC_H_

class SOC {
public:
	SOC();
	virtual ~SOC();
	class I2S {
	public:
		static void dump();
	};
};

#endif /* COMPONENTS_CPP_UTILS_SOC_H_ */
