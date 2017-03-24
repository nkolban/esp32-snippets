/*
 * 1. Open up the project properties
 * 2. Visit C/C++ General > Preprocessor Include Paths, Macros, etc
 * 3. Select the Providers tab
 * 4. Check the box for "CDT GCC Built-in Compiler Settings"
 * 5. Set the compiler spec command to "xtensa-esp32-elf-gcc ${FLAGS} -E -P -v -dD "${INPUTS}""
 * 6. Rebuild the index
*/

#include <esp_log.h>
#include <string>
#include "sdkconfig.h"


static char tag[]="cpp_helloworld";

extern "C" {
	void app_main(void);
}

class Greeting {
public:
	void helloEnglish() {
		ESP_LOGD(tag, "Hello %s", name.c_str());
	}

	void helloFrench() {
		ESP_LOGD(tag, "Bonjour %s", name.c_str());
	}

	void setName(std::string name) {
		this->name = name;
	}
private:
	std::string name = "";

};

void app_main(void)
{
	Greeting myGreeting;
	myGreeting.setName("Neil");
	myGreeting.helloEnglish();
	myGreeting.helloFrench();
}

