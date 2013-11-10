#include "stdafx.h"

#include "GameFrontend.h"

int main(){
	AEngineFramework* engine = NULL;

	try{
		engine  = new GameFrontend;
	}catch(wt::Exception& e){
		LOGE("Game failed to initialize. \"%s\"", e.getFullDescription().c_str());
		return -1;
	}

	engine->startMainLoop();

	return 0;
}
