#ifndef GAMEBACKEND_H
#define GAMEBACKEND_H

#include <wt/EventListener.h>
#include <wt/AGameInput.h>

// Forward declarations
namespace wt{
	class AGameWindow;
	class EventManager;

	namespace lua{
		class State;
	}; // </lua>
}; // </wt>

class GameBackend : public wt::EventListener{
public:
	GameBackend();

	void run();

	virtual bool handleEvent(const wt::Sp<wt::Event> evt);

	virtual void onWindowSizeChanged(uint32_t w, uint32_t h){}

	virtual void onKeyUp(wt::VirtualKey code){}

	virtual void onKeyDown(wt::VirtualKey code){}
	
	virtual void onMouseDown(float x, float y, wt::MouseButton btn){}

	virtual void onMouseUp(float x, float y, wt::MouseButton btn){}

	virtual void onUpdate(float dt){}

	virtual void onMouseMotion(const wt::MouseMotionEvent* evt){}

	virtual void onAppQuit(){}

	virtual void onStart(){}

protected:
	wt::AGameWindow* getWindow();

	wt::AGameInput* getInput();

	wt::EventManager* getEventManager();

	wt::lua::State* getLuaState();

private:
	wt::AGameWindow* mWindow;
	wt::AGameInput* mInput;
	wt::EventManager* mEventManager;
	wt::lua::State* mLuaState;
	FILE* mLogFile;

	GameBackend(const GameBackend&);
}; // </GAMEBACKEND_H>

#endif // </GAMEBACKEND_H>
