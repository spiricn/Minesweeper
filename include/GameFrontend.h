#ifndef GAMEFRONTEND_H
#define GAMEFRONTEND_H

#include "GameBackend.h"

#include "Minesweeper.h"

#include <wt/MusicPlayer.h>
#include <wt/FPSCameraControler.h>

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

class GameFrontend : public GameBackend{
public:
	enum Difficulty{
		eBEGGINER,
		eINTERMEDIATE,
		eEXPERT
	};

	void onUpdate(float dt);

	void onStart();

	void onMouseMotion(const MouseMotionEvent* evt);

	bool handleEvent(const Sp<Event> e);

	void onMouseDown(float x, float y, MouseButton btn);

	void restart(Difficulty difficulty);

	void onKeyDown(VirtualKey c);

	void changeDifficulty(Difficulty diff);

	void onStart(const LuaObject& config);

private:
	AResourceSystem* mAssets;
	Scene* mScene;
	Renderer* mRenderer;
	Physics* mPhysics;

	Minesweeper mGameState;
	MusicPlayer* mAmbientMusic;
	MusicPlayer* mDefeatMusic;
	ProcPtr mEndGameToast;
	ProcessManager* mProcessManager;
	gui::Window* mUi;
	math::FPSCameraControler mCamController;

	typedef std::vector<MineActor*> ActorList;
	ActorList mActors;

	const PointLight* mCursorPointLight;

	Difficulty mDifficulty;

	ParticleEffect* mCursor;
}; // </GameFrontend>

#endif // </GAMEFRONTEND_H>