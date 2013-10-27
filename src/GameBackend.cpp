#include "stdafx.h"

#include <wt/Singleton.h>
#include <wt/AGameWindow.h>
#include <wt/AGameInput.h>
#include <wt/EventManager.h>
#include <wt/Utils.h>
#include <wt/lua/State.h>
#include <wt/SDLGameInput.h>
#include <wt/SDLGameWindow.h>
#include <wt/Timer.h>

#include "GameBackend.h"

using namespace wt;

#define LOG_FILE_PATH "game.log"

GameBackend::GameBackend() : mWindow(NULL), mInput(NULL), mEventManager(NULL), mLogFile(NULL){
	mLogFile = fopen(LOG_FILE_PATH, "wb");

	utils::setHomeDir("d:/Documents/prog/c++/workspace/Minesweeper/assets");

	if(mLogFile == NULL){
		LOGW("Error openning log file");
	}
	else{
		td_setFileOutput(mLogFile);
	}

	LOG("New log session %s", wt::utils::getCurrentTime("%H:%M:%S %d/%b/%Y").c_str());

	// Main scripting state
	mLuaState = new wt::lua::State;
	
	// Main event manager
	mEventManager = new EventManager(mLuaState);

	// Create game input & hook it to the event manager
	mInput = new SDLGameInput;
	mInput->hook(mEventManager);

	// Create window & hook it to the event manager
	mWindow = new SDLGameWindow;
	mWindow->create( AGameWindow::VideoMode(1280, 720, "Minesweeper", true, false) );
	mWindow->hook(mEventManager);

	// Initialize glew (must be done AFTER creating a valid OpenGL context i.e. rendering window)
	GLenum r = glewInit();
	if(r != GLEW_OK){
		WT_THROW("Error initializing glew \"%s\"",
			(const char*)glewGetErrorString(r));
	}

	mEventManager->registerGlobalListener(this);
}

bool GameBackend::handleEvent(const Sp<Event> evt){
	const EvtType& type = evt->getType();

	if(type == KeyPressEvent::TYPE){
		const KeyPressEvent* e = static_cast<const KeyPressEvent*>(evt.get());
		if(e->mAction == KeyPressEvent::DOWN){
			onKeyDown(e->mCode);
		}
		else{
			onKeyUp(e->mCode);
		}
	}
	else if(type == AppQuitEvent::TYPE){
		onAppQuit();
	}
	else if(type == WindowSizeChange::TYPE){
		const WindowSizeChange* e = static_cast<const WindowSizeChange*>(evt.get());
		onWindowSizeChanged(e->newWidth, e->newHeight);
	}

	else if(type == MouseMotionEvent::TYPE){
		const MouseMotionEvent* e = static_cast<const MouseMotionEvent*>(evt.get());
		onMouseMotion(e);
	}

	else if(type == MousePressEvent::TYPE){
		const MousePressEvent* e = static_cast<const MousePressEvent*>(evt.get());

		if(e->mAction == MousePressEvent::eBUTTON_DOWN){
			onMouseDown(e->mX, e->mY, e->mButton);
		}
		else{
			onMouseUp(e->mX, e->mY, e->mButton);
		}
	}

	return true;
}

void GameBackend::run(){
	Timer time;

	time.reset();
	float dt=0.0f;

	LOGD("Main loop running.");

	onStart();

	while(!mInput->isKeyDown(KEY_ESC)){
		mInput->pollAndDispatch();
		mEventManager->tick();

		dt = time.getSeconds();

		onUpdate(dt);

		mWindow->swapBuffers();
	}
}

wt::AGameWindow* GameBackend::getWindow(){
	return mWindow;
}

wt::AGameInput* GameBackend::getInput(){
	return mInput;
}

wt::EventManager* GameBackend::getEventManager(){
	return mEventManager;
}

wt::lua::State* GameBackend::getLuaState(){
	return mLuaState;
}