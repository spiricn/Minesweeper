#include "stdafx.h"

#include "GameFrontend.h"
#include "GameBackend.h"

int main(){
	
	GameBackend* backend;

	try{
		backend = new GameFrontend;
	}catch(wt::Exception& e){
		LOGE("Game failed to initialize. \"%s\"", e.getFullDescription().c_str());
		return -1;
	}

	backend->run();

	return 0;
}