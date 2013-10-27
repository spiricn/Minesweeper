#include "stdafx.h"

#include <wt/EventManager.h>
#include <wt/Lua.h>

#include "Minesweeper.h"

#define TD_TRACE_TAG "Minesweeper"

using namespace wt;

Minesweeper::Minesweeper() : mGameState(eINACTIVE), mField(NULL){
}

void Minesweeper::createField(Uint32 numRows, Uint32 numColumns, Uint32 numMines){
	if(mField){
		delete mField;
		mField = NULL;
	}

	mField = new Minefield(numRows, numColumns, numMines);

	mGameState = ePLAYING;
}

Minesweeper::GameState Minesweeper::getState() const{
	return mGameState;
}

Minesweeper::~Minesweeper(){
	delete mField;
}

void Minesweeper::hook(EventManager* em){
	mEventManager = em;

	mEventManager->registerInternalEvent(CellRevealedEvent::TYPE);
	mEventManager->registerInternalEvent(GameLostEvent::TYPE);
	mEventManager->registerInternalEvent(GameWonEvent::TYPE);
	mEventManager->registerInternalEvent(CellMarkedEvent::TYPE);

}

bool Minesweeper::victory(){
	for(Uint32 x=0; x<mField->getNumColumn(); x++){
		for(Uint32 y=0; y<mField->getNumRows(); y++){
			MineCell& cell = mField->getCell(x, y);
			if(!cell.marked && !cell.visible
				|| (cell.marked && !cell.hasMine)){
				return false;
			}
		}
	}

	mEventManager->queueEvent( new GameWonEvent );

	mGameState = eWON;

	return true;
}

void Minesweeper::setCellUserData(Uint32 x, Uint32 y, void* data){
	mField->getCell(x, y).userData = data;
}

const Minefield& Minesweeper::getField() const{
	return *mField;
}

void Minesweeper::restart(){
	mField->restart();
	mGameState = ePLAYING;
}

void Minesweeper::mark(MineCell& cell){
	if(mGameState != ePLAYING){
		return;
	}

	if(!cell.visible){
		cell.marked = !cell.marked;

		CellMarkedEvent* e = new CellMarkedEvent;
		e->cell = &cell;

		mEventManager->queueEvent(e);
	}

	victory();
}

void Minesweeper::mark(Uint32 x, Uint32 y){
	if(mGameState != ePLAYING){
		return;
	}

	if(mField->inBounds(x, y)){
		mark(mField->getCell(x, y));
	}
}

const  MineCell& Minesweeper::getCell(Uint32 x, Uint32 y) const{
	return mField->getCell(x, y);
}


void Minesweeper::reveal(std::set<MineCell*>& cells){
	if(cells.empty()){
		return;
	}

	CellRevealedEvent* e = new CellRevealedEvent;

	for(std::set<MineCell*>::iterator i=cells.begin(); i!=cells.end(); i++){
		(*i)->visible = true;
		e->cells.insert(*i);
	}

	mEventManager->queueEvent(e);
}

void Minesweeper::revealNeighbours(Uint32 x, Uint32 y){
	if(mGameState != ePLAYING){
		return;
	}

	MineCell& cell = mField->getCell(x, y);

	if(!cell.visible){
		return;
	}

	Uint32 numMarked=0;
	for(int dx=-1; dx<2; dx++){
		for(int dy=-1; dy<2; dy++){
			if(dx==0 && dy==0){
				continue;
			}

			if(mField->inBounds(x+dx, y+dy) && mField->getCell(x+dx, y+dy).marked){
				numMarked++;
			}
		}
	}

	if(numMarked == cell.numNeighborMines){
		for(int dx=-1; dx<2; dx++){
			for(int dy=-1; dy<2; dy++){
				if(dx==0 && dy==0){
					continue;
				}

				if(mField->inBounds(x+dx, y+dy) && !mField->getCell(x+dx, y+dy).marked && !mField->getCell(x+dx, y+dy).visible){
					reveal(x+dx, y+dy);
				}
			}
		}
	}
}

void Minesweeper::reveal(Uint32 x, Uint32 y){
	if(mGameState != ePLAYING){
		return;
	}

	MineCell& cell = mField->getCell(x, y);

	if(cell.marked){
		return;
	}

	if(cell.visible){
		return;
	}

	std::set<MineCell*> toReveal;

	if(!cell.hasMine){
		toReveal.insert(&cell);
		cell.visible = true;

		if(victory()){
			return;
		}

		if(cell.numNeighborMines != 0){
			reveal(toReveal);
			return;
		}

		std::vector<glm::vec2> unchecked;

		for(int dx=-1; dx<2; dx++){
			for(int dy=-1; dy<2; dy++){
				if(dx==0 && dy==0){
					continue;
				}

				if(mField->inBounds(x+dx, y+dy) && !mField->getCell(x+dx, y+dy).visible){
					unchecked.push_back(
						glm::vec2(x+dx, y+dy));
				}
			}
		}

		while(!unchecked.empty()){
			glm::vec2 cellCoords = unchecked.back();
			unchecked.pop_back();

			MineCell& neighbour = mField->getCell(cellCoords.x, cellCoords.y);

			neighbour.visible = true;
			toReveal.insert(&neighbour);

			if(victory()){
				return;
			}
				
			if(neighbour.numNeighborMines == 0){

				for(int dx=-1; dx<2; dx++){
					for(int dy=-1; dy<2; dy++){
						if(dx==0 && dy==0){
							continue;
						}

						if(mField->inBounds(cellCoords.x+dx, cellCoords.y+dy) && 
							!mField->getCell(cellCoords.x+dx, cellCoords.y+dy).visible){

							unchecked.push_back(
						glm::vec2(cellCoords.x+dx, cellCoords.y+dy));
						}
					}
				}
			}
		}
		reveal(toReveal);
	}
	else{
		LOGV("Emiting game lost event");
		mEventManager->queueEvent( new GameLostEvent );
		mGameState = eLOST;
	}		
}

const EvtType GameLostEvent::TYPE = "GameLost";

const EvtType CellRevealedEvent::TYPE = "CellRevealed";

const EvtType CellMarkedEvent::TYPE = "CellMarked";

const EvtType GameWonEvent::TYPE = "GameWon";