
#pragma once 

/*
Initializes the application. 

Handles the render loop 

Closes the application 

<Example>: 

	App app; 
	app.run()
	app.shutdown(); 

*/
class App {

public: 
	bool init();
	void run();
	void shutdown();
};


