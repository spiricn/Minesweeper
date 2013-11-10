#include "stdafx.h"

#include <wt/Assets.h>
#include <wt/Scene.h>
#include <wt/Renderer.h>
#include <wt/SceneLoader.h>
#include <wt/Physics.h>
#include <wt/Toast.h>
#include <wt/gui/Window.h>
#include <wt/gui/TextView.h>

#include "GameFrontend.h"

#define TD_TRACE_TAG "GameFrontend"

#define MUSIC_AMBIENT "rsrc/music/primavera.ogg"

#define MUSIC_VICTORY "rsrc/music/canzone.ogg"

#define MUSIC_DEFEAT "rsrc/music/zone.ogg"

using namespace wt;
using namespace physx;

class GodrayProcess : public AProcess{
public:
	GodrayProcess(const IEngineServiceLocator* serviceLocator) : mInterpolator(0, 1, 10, true), mServiceLocator(serviceLocator){
	}

	void onProcUpdate(float dt){
		float pos = 0.0f;
		mInterpolator.update(dt, pos);

		
		glm::vec2 point;

		float theta = pos*2*math::PI;

		math::pointOnCircle(5, theta , point);

		Scene::GodRayParams params;
		mServiceLocator->getScene()->getGodRayParams(params);
		params.enabled = true;

		params.sourcePosition = glm::vec3(point.x, -100.0f, point.y);

		params.sourceColor = Color::fromHSV(pos, 0.5, 0.5);

		mServiceLocator->getScene()->setGodRayParams(params);
	}

private:
	const IEngineServiceLocator* mServiceLocator;
	Interpolator<float> mInterpolator;
};

class LightCycler : public AProcess{
public:
	LightCycler(const IEngineServiceLocator* serviceLocator, float cycleDuration) : mCycleDuration(cycleDuration), mServiceLocator(serviceLocator), mCyclePosition(0.0f){
	}

	void onProcUpdate(float dt){
		mCyclePosition += dt;

		if(mCyclePosition >= mCycleDuration){
			mCyclePosition = 0.0f;
		}

		glm::vec2 point;

		float theta = (mCyclePosition/mCycleDuration)*2*math::PI;

		math::pointOnCircle(30, theta , point);

		glm::vec3 pos(point.x, 100, point.y);

		DirectionalLight::Desc desc = mServiceLocator->getScene()->getDirectionalLight()->getDesc();

		desc.direction = glm::normalize(-pos);
		mServiceLocator->getScene()->setDirectionalLightDesc(desc);
	}

private:
	const IEngineServiceLocator* mServiceLocator;
	const float mCycleDuration;
	float mCyclePosition;
}; // </LightCycler>


class MineDetacher : public AProcess{
private:
	struct DetachCell{
		const MineCell* cell;
		float waitTime;

	}; // </DetachCell>

	std::vector<DetachCell> mCells;
	Interpolator<float> mInterpolator;
	float mBaseHeight;
public:
	MineDetacher(EvtPtr evt) : mInterpolator(0, -0.3, .1, false, Interpolator<float>::eEASE_IN_QUAD){
		CellRevealedEvent* e = (CellRevealedEvent*)evt.get();

		for(std::set<const MineCell*>::iterator i=e->cells.begin(); i!=e->cells.end(); i++){
			const MineCell* cell = *i;

			ModelledActor* actor = (ModelledActor*)cell->userData;

			glm::vec3 pos;
			actor->getTransformable()->getTranslation(pos);

			mBaseHeight = pos.y;

			String skin;

			switch(cell->numNeighborMines){
				case 0:skin = "visible"; break;
				case 1:skin = "1"; break;
				case 2:skin = "2"; break;
				case 3:skin = "3"; break;
				case 4:skin = "4"; break;
				case 5:skin = "5"; break;
				case 6:skin = "6"; break;
				case 7:skin = "7"; break;
				case 8:skin = "8"; break;
			}

			actor->setSkin(skin);

			DetachCell detachCell;
			detachCell.cell = cell;

				
					

			if(cell->numNeighborMines > 0){
			}
			else{
				detachCell.waitTime = math::random();
			}

			mCells.push_back(detachCell);
		}
	}
		
	void onProcUpdate(float dt){
		mInterpolator.update(dt);

		for(std::vector<DetachCell>::iterator i=mCells.begin(); i!=mCells.end(); i++){
			i->waitTime -= dt;

			const MineCell* cell = i->cell;

			ModelledActor* actor = (ModelledActor*)cell->userData;

			if(i->cell->numNeighborMines == 0){
				if(i->waitTime <= 0.0f){
					// drop
					((PxRigidDynamic*)actor->getPhysicsActor()->getPxActor())->wakeUp();


					// remove from list
					i = mCells.erase(i);
					if(i == mCells.end()){
						break;
					}
				}
			}
			else{
				glm::vec3 pos;
				actor->getTransformable()->getTranslation(pos);

				pos.y = mBaseHeight + mInterpolator.getValue();

				actor->getController()->setTranslation(pos);

				// The above setting of the translation wakes up  the actor so it needs to be put back to sleep
				static_cast<PxRigidDynamic*>(actor->getPhysicsActor()->getPxActor())->putToSleep();

				if(mInterpolator.isFinished()){
					// remove from list
					i = mCells.erase(i);
					if(i == mCells.end()){
						break;
					}
				}
			}
		}

		if(mCells.empty()){
			killProcess();
		}
	}
private:
};

GameFrontend::GameFrontend() : mGameStarted(false){
	AEngineFramework::Desc engineDesc;

	engineDesc.homeDir = "d:/Documents/prog/c++/workspace/Minesweeper/assets";

	engineDesc.window.screenWidth = 1280;
	engineDesc.window.screenHeight = 720;

	initializeFramework(engineDesc);

	mMenuEnabled = true;

	mProcessManager = getProcessManager();
		
	mGameState.hook(getEventManager());

	// GUI
	mUi = new gui::Window;
	
	mUi->create(
		getWindow()->getWidth(),
		getWindow()->getHeight()
	);

	mUi->setInput(getInput());

	Font* font = getAssets()->getFontManager()->create("_demo_font");
	font->load("./rsrc/fonts/cour.ttf", 20);

	mUi->hook(getEventManager() );
	mUi->setDefaultFont( font );

	mUi->setDefaultScaleMode( gui::View::eGRID );
	mUi->setGridSize(10, 3);

	gui::Button* btnNewGame = mUi->createView<gui::Button>("btn_newGame");
	btnNewGame->setText("New game");
	btnNewGame->setGridLocation(2, 1, 2, 1);

	getEventManager()->registerCallback(
		new MemberCallback<GameFrontend>(this, &GameFrontend::onBtnNewGameClicked), gui::ButtonClickedEvent::TYPE, true, btnNewGame->getId());

	gui::Button* btnQuit = mUi->createView<gui::Button>("btn_quit");
	btnQuit->setText("Quit");
	btnQuit->setGridLocation(5, 1, 2, 1);

	getEventManager()->registerCallback(
		new MemberCallback<GameFrontend>(this, &GameFrontend::onBtnQuitClicked), gui::ButtonClickedEvent::TYPE, true, btnQuit->getId());

	getScene()->setUIWindow(mUi);

	getInput()->setMouseGrabbed(false);

	getAssets()->load("assets.wtr");

	SceneLoader loader(getScene(), getAssets());
	loader.load("scene.wts");

	PointLight::Desc desc;
	desc.enabled = true;
	desc.position = glm::vec3(210, 10, 210);

	desc.color =  Color::Green();
	desc.ambientIntensity = 0;
	desc.diffuseIntensity = 0.1;

	desc.attenuation.constant = 0;
	desc.attenuation.linear = 0.3;
	desc.attenuation.quadratic = 0.01;

	//mCursorPointLight = getScene()->createPointLight(desc);

	mCursorPointLight = (PointLight*)getScene()->findActorByName("cursor_light");

	
	mProcessManager->attach(mAmbientMusic = new MusicPlayer(getAssets()->getSoundSystem()));

	getScene()->getFog().color = Color(1, 201/255.0f, 14/255.0f);
	getScene()->getFog().density += 0.007f;
	
	getEventManager()->registerGlobalListener(this);
}

void GameFrontend::startGame(){
	
	getAssets()->getSoundSystem()->setGlobalVolume(1.0f);

	ParticleEffect* effect = getScene()->createParticleEffect();
	effect->create(getAssets()->getParticleResourceManager()->find("field"));
	effect->getController()->setTranslation(glm::vec3(0, -200, 0));

	mCursor = getScene()->createParticleEffect();
	mCursor->create(getAssets()->getParticleResourceManager()->find("cursor"));

	mProcessManager->attach( new LightCycler(this, 6.0f) );

	mProcessManager->attach( new GodrayProcess(this) );

	{
		// Create cursor plane
		PhysicsActor::Desc desc;
		desc.collisionMask = 0x00;
		desc.group = 0x2;

		desc.type = PhysicsActor::eTYPE_STATIC;
		desc.geometryType = PhysicsActor::eGEOMETRY_BOX;

		desc.geometryDesc.boxGeometry.hx = 30;
		desc.geometryDesc.boxGeometry.hy = 0.001;
		desc.geometryDesc.boxGeometry.hz = 30;

		math::Transform t;
		t.setTranslation(glm::vec3(0, 1, 0));
		t.getMatrix(desc.pose);

		desc.controlMode = PhysicsActor::eCTRL_MODE_PHYSICS;

		getPhysics()->createActor(NULL, desc);
	}

	mGameStarted = true;

	getScene()->setSkyBox( getAssets()->getSkyBoxManager()->find("main") );
}

void GameFrontend::toggleMenu(){
	gui::View* v;

	v = mUi->findView("btn_quit");
	v->setVisible(!v->isVisible());

	v = mUi->findView("btn_newGame");
	v->setVisible(!v->isVisible());

	mMenuEnabled = !mMenuEnabled;

	TRACED();
}


void GameFrontend::onBtnNewGameClicked(){
	if(!mGameStarted){
		startGame();
	}

	restart(eBEGGINER);
	toggleMenu();
}

void GameFrontend::onBtnQuitClicked(){
	stopMainLoop();
}

void GameFrontend::onUpdate(float dt){
	if(mGameStarted){
		getScene()->getSkyBox()->getTransform().rotate(1, 1, 1, 3*dt);
	}
	else{
		ModelledActor* actor = (ModelledActor*)getScene()->findActorByName("menu_cube");
		actor->getController()->rotate(glm::vec3(1, 1, 0), 30*dt);
	}
}

void GameFrontend::onMouseMotion(const MouseMotionEvent* evt){
	if(mMenuEnabled || !mGameStarted){
		return;
	}

	RaycastHitEvent hit;

	if(getScene()->getPhysics()->pick(getScene()->getCamera(),
		glm::vec2(evt->mX, evt->mY),
		glm::vec2(getWindow()->getWidth(), getWindow()->getHeight()), hit, 
		// Pick only cursor plane
		0x2, Physics::ePICK_ACTORS
		)){

		PointLight::Desc desc = mCursorPointLight->getDesc();

		desc.position = hit.mImpact + glm::vec3(0, 5, 0);

		mCursorPointLight->setDesc(desc);

		mCursor->getController()->setTranslation( hit.mImpact + glm::vec3(0, 3, 0) );
	}
}

bool GameFrontend::handleEvent(const Sp<Event> e){
	if(e->getType() == CellRevealedEvent::TYPE){
		CellRevealedEvent* evt = (CellRevealedEvent*)e.get();

		if(evt->cells.size() > 1){
			/*getAssets()->getSoundSystem()->playSound("$ROOT/misc/magicclick");*/
		}
		mProcessManager->attach( new MineDetacher(e) );

	}
	else if(e->getType() == CellMarkedEvent::TYPE){
		CellMarkedEvent* evt = (CellMarkedEvent*)e.get();

		ModelledActor* actor = (ModelledActor*)evt->cell->userData;

		actor->setSkin(
			evt->cell->marked ? "marked" : "hidden");

	}
	else if(e->getType() == GameLostEvent::TYPE){
		Toast* toast = new Toast(mUi,
			glm::vec2(0, 0), 
			glm::vec2(getWindow()->getWidth(), getWindow()->getHeight()),
			getAssets()->getTextureManager()->find("defeat"));
		toast->setLinger(true)->setFadeOutValue(0.3f);

		mProcessManager->attach(
			mEndGameToast = toast
		);			

		LOG("Game lost!");
		for(Uint32 i=0; i<mGameState.getField().getNumColumn(); i++){
			for(Uint32 j=0; j<mGameState.getField().getNumRows(); j++){
				ModelledActor* a = (ModelledActor*)mGameState.getCell(i, j).userData;

				PxVec3 force;
				pxConvert(glm::vec3(-5 + math::random()*10, math::random()*20, -5 + math::random()*10), force);
					
				((PxRigidDynamic*)a->getPhysicsActor()->getPxActor())->setLinearVelocity(force);
			}
		}

		mAmbientMusic->play(MUSIC_DEFEAT);
			
		getPhysics()->getScene()->setGravity(PxVec3(0, -9.81f, 0));
	}
	else if(e->getType() == GameWonEvent::TYPE){
		LOG("Game won!");

		Toast* toast = new Toast(mUi,
				glm::vec2(0, 0), 
				glm::vec2(getWindow()->getWidth(), getWindow()->getHeight()),
				getAssets()->getTextureManager()->find("victory"));
		toast->setLinger(true)->setFadeOutValue(0.3);

		mProcessManager->attach( mEndGameToast = toast );


		mAmbientMusic->play(MUSIC_VICTORY);

		getPhysics()->getScene()->setGravity(PxVec3(0, 3.81f, 0));
		for(Uint32 i=0; i<mGameState.getField().getNumColumn(); i++){
			for(Uint32 j=0; j<mGameState.getField().getNumRows(); j++){
				ModelledActor* a = (ModelledActor*)mGameState.getCell(i, j).userData;

				PxVec3 force;
				pxConvert(glm::vec3(-5 + math::random()*10, math::random()*20, -5 + math::random()*10), force);
					
				((PxRigidDynamic*)a->getPhysicsActor()->getPxActor())->setLinearVelocity(force);
			}
		}
	}
	else{
		processEvent(e);
	}

	return true;
}

void GameFrontend::onMouseDown(float x, float y, MouseButton btn){
	if(mMenuEnabled || !mGameStarted){
		return;
	}

	RaycastHitEvent hit;

	if(mGameState.getState() != Minesweeper::ePLAYING){
		restart(mDifficulty);
	}
	else{
		if(getScene()->getPhysics()->pick(getScene()->getCamera(),
			glm::vec2(x, y),
			glm::vec2(getWindow()->getWidth(), getWindow()->getHeight()), hit, 
			// Pick only mines
			0x1, Physics::ePICK_ACTORS
			)){
				MineActor* ma = (MineActor*)hit.mPickedActor->getSceneActor()->getUserData();

				WT_ASSERT(ma != NULL, "Picked invalid scene actor");

				if(getInput()->isMouseButtonDown(BTN_LEFT) && getInput()->isMouseButtonDown(BTN_RIGHT)){
					mGameState.revealNeighbours(ma->cell->x, ma->cell->y);
				}
				else if(btn == BTN_LEFT){
					mGameState.reveal(ma->cell->x, ma->cell->y);
					
				}
				else if(btn == BTN_RIGHT){
					mGameState.mark(ma->cell->x, ma->cell->y);
				}

		}
	}
}

void GameFrontend::restart(Difficulty difficulty){
	LOG("restart");

	Uint32 numRows, numColumns, numMines;
	glm::quat cameraRotation;
	glm::vec3 cameraPosition;

	if(difficulty == eEXPERT){
		numRows = 16;
		numColumns = 30;
		numMines = 99;
		cameraRotation =  glm::quat(0.524339, 0.472943, 0.525832, -0.474297);
		cameraPosition  = glm::vec3(-5.062535, 26.684690, -1.298899);
	}
	else if(difficulty == eINTERMEDIATE){
		numRows = 16;
		numColumns = 16;
		numMines = 40;
		cameraRotation = glm::quat(0.554675, 0.442495, 0.550975, -0.439430);
		cameraPosition = glm::vec3(-8.682729, 22.547667, -1.188196);
	}
	else{
		cameraPosition = glm::vec3(-2.447727, 22.372480, -1.832187);

		cameraRotation.x = 0.489420;
		cameraRotation.y = 0.510378;
		cameraRotation.z = 0.489429;
		cameraRotation.w = -0.510336;

		numRows = 9;
		numColumns = 9;
		numMines = 10;
	}

	mDifficulty = difficulty;

	// Create new field
	mGameState.createField( numColumns, numRows, numMines );

	getScene()->getCamera().setTranslation(cameraPosition);
	getScene()->getCamera().setRotation(cameraRotation);

	// Remove all the actors from the scene
	for(ActorList::iterator i=mActors.begin(); i!=mActors.end(); i++){
		getScene()->deleteActor((*i)->sceneActor);
		delete *i;
	}

	mActors.clear();
		
	// Handle toast left from the previous game defeat/victory
	if(mEndGameToast){
		Toast* toast = (Toast*)mEndGameToast.get();
		if(toast->isAlive()){
			toast->setFadeOutValue(0)->setFadeOutTime(0.3)->setLinger(false)->fadeOutNow();
			
			mEndGameToast = NULL;
		}
	}

	mAmbientMusic->play(MUSIC_AMBIENT);

	getPhysics()->getScene()->setGravity(PxVec3(0, -9.81f, 0));


	for(Uint32 x=0; x<mGameState.getField().getNumColumn(); x++){
		for(Uint32 y=0; y<mGameState.getField().getNumRows(); y++){
			// Create scene actor
			ModelledActor* actor = getScene()->createModelledActor();
			actor->setModel(
				getAssets()->getModelManager()->find("cube"), "hidden");

			mGameState.setCellUserData(x, y, actor);

			// Initial actor position
			actor->getController()->setTranslation(glm::vec3(
				(- (mGameState.getField().getNumColumn() * 2.3f)/2) + x * (2.3),
				0,
				(- (mGameState.getField().getNumRows() * 2.3f)/2) + y * (2.3))
			);

			actor->getController()->setRotation(
				glm::vec3(0, 1, 0), -90
			);

			// Create game actor
			MineActor* ma = new MineActor;
			ma->sceneActor = actor;
			ma->cell = &mGameState.getCell(x, y);

			mActors.push_back(ma);
			actor->setUserData(ma);

			// Create physics actor
			PhysicsActor::Desc desc;

			desc.geometryType = PhysicsActor::eGEOMETRY_BOX;
			desc.controlMode = PhysicsActor::eCTRL_MODE_PHYSICS;
			desc.type = PhysicsActor::eTYPE_DYNAMIC;
			desc.geometryDesc.boxGeometry.hx = 1.0f;
			desc.geometryDesc.boxGeometry.hy = 1.0f;
			desc.geometryDesc.boxGeometry.hz = 1.0f;

			// Collide only with one another
			desc.group = 0x1;
			desc.collisionMask = 0x1;
			actor->getTransformable()->getTransformMatrix(desc.pose);

			PhysicsActor* pActor = getPhysics()->createActor(actor, desc);
			((PxRigidDynamic*)pActor->getPxActor())->putToSleep();
		}
	}

}

void GameFrontend::onKeyDown(VirtualKey c){
	if(c == KEY_r){
		restart(mDifficulty);
	}
	else if(c == KEY_ESC){
		toggleMenu();
	}
	else if(c == KEY_d){
		changeDifficulty(mDifficulty == eBEGGINER ? eINTERMEDIATE : mDifficulty == eINTERMEDIATE ? eEXPERT : eBEGGINER);
	}
	else if(c == KEY_F5){
		glm::vec3 pos;
		getScene()->getCamera().getTranslation(pos);
		LOGI("Camera position: = {%f, %f, %f}", pos.x, pos.y, pos.z);

		glm::quat rot;
		getScene()->getCamera().getRotation(rot);
		LOGI("Camera rotation = {%f, %f, %f, %f}", rot.x, rot.y, rot.z, rot.w);

		glm::vec3 fw;
		getScene()->getCamera().getForwardVector(fw);
		LOGI("Camera facing = {%f, %f, %f}", fw.x, fw.y, fw.z);


		float h = getPhysics()->getTerrainHeightAt(glm::vec2(pos.x, pos.z));

		LOGI("Terrain position = {%f, %f, %f}", pos.x, h, pos.z);
	}
}

void GameFrontend::changeDifficulty(Difficulty diff){
	if(mDifficulty == diff){
		return;
	}

	Texture2D* texture = getAssets()->getTextureManager()->find(
		diff == eBEGGINER ? "begginer" : diff == eINTERMEDIATE ? "intermediate" : "expert");

#if 0
	// TODO
	Toast* toast = new Toast(&getUi(),
		glm::vec2(0, 0), getUi().getCanvas().getSize()/7.0f, texture);

	toast->setDuration(0.5f)->setFadeInTime(0.1f)->setFadeOutTime(0.1f);

	mProcessManager->attach(toast);
#endif

	restart(diff);
}
