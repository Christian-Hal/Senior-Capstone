
#include "App.h"

/*
Entry point for the program.
*/
int main(){

	App app; 

	if (!app.init()) {
		return -1;
	}

	app.run();
	app.shutdown();
	return 0;
}