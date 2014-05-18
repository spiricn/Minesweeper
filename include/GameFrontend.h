#ifndef GAMEFRONTEND_H
#define GAMEFRONTEND_H

#include "Minesweeper.h"

#include <wt/MusicPlayer.h>
#include <wt/CameraController.h>
#include <wt/gui/WindowManager.h>
#include <wt/AEngineFramework.h>

using namespace wt;

// Forward declarations
namespace wt{
	class ParticleEffect;
	class AResourceSystem;
	class ModelledActor;
	class Scene;
	class Renderer;
	class Physics;
	class PointLight;

	namespace gui{
		class Window;
	}; // </Gui>

}; // </wt>


class MineActor{
public:
	const MineCell* cell;
	ModelledActor* sceneActor;

	void update(float dt){
	}
};

class GameFrontend : public AEngineFramework, public IEventListener{
public:
	enum Difficulty{
		eBEGGINER,
		eINTERMEDIATE,
		eEXPERT
	};

	GameFrontend();

	void onUpdate(float dt);

	void onMouseMotion(const MouseMotionEvent* evt);

	bool handleEvent(const EventPtr e);

	void onMouseDown(float x, float y, MouseButton btn);

	void restart(Difficulty difficulty);

	void onKeyDown(VirtualKey c);

	void changeDifficulty(Difficulty diff);

	void onStart(const LuaObject& config);

	void onBtnNewGameClicked();

	void onBtnQuitClicked();

	void startGame();
private:

	void toggleMenu();

	Minesweeper mGameState;
	MusicPlayer* mAmbientMusic;
	MusicPlayer* mDefeatMusic;
	ProcPtr mEndGameToast;
	ProcessManager* mProcessManager;
	gui::Window* mUi;
	CameraController mCamController;

	typedef std::vector<MineActor*> ActorList;
	ActorList mActors;

	const PointLight* mCursorPointLight;

	Difficulty mDifficulty;

	ParticleEffect* mCursor;

	bool mMenuEnabled;
	bool mGameStarted;
	gui::WindowManager mWindowManager;
}; // </GameFrontend>

#endif // </GAMEFRONTEND_H>
