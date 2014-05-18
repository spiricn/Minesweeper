#ifndef MINES_H
#define MINES_H

#include "stdafx.h"

#include <wt/EventManager.h>
#include <wt/Lua.h>

#include "Minefield.h"

using namespace wt;

class CellRevealedEvent : public AEvent{
protected:
	void serialize(LuaObject& dst){
	}

	void deserialize(LuaObject& src){
	}

public:
	std::set<const MineCell*> cells;

	static const EventType TYPE;

	CellRevealedEvent(){
	}

	const EventType& getType() const{
		return TYPE;
	}

}; // </CellRevealedEvent>

class GameLostEvent : public AEvent{
protected:
	void serialize(LuaObject& dst){
	}

	void deserialize(LuaObject& src){
	}

public:
	static const EventType TYPE;

	GameLostEvent(){
	}

	const EventType& getType() const{
		return TYPE;
	}

}; // </GameLostEvent>
	
class CellMarkedEvent : public AEvent{
protected:
	void serialize(LuaObject& dst){
	}

	void deserialize(LuaObject& src){
	}

public:
	static const EventType TYPE;

	MineCell* cell;
	CellMarkedEvent(){
	}

	const EventType& getType() const{
		return TYPE;
	}

}; // </CellMarkedEvent>


class GameWonEvent : public AEvent{
protected:
	void serialize(LuaObject& dst){
	}

	void deserialize(LuaObject& src){
	}

public:
	static const EventType TYPE;

	GameWonEvent(){
	}

	const EventType& getType() const{
		return TYPE;
	}

}; // </GameWonEvent>

class Minesweeper{
public:
	enum GameState{
		eINACTIVE,
		ePLAYING,
		eWON,
		eLOST
	};

public:
	Minesweeper();

	void createField(Uint32 numRows, Uint32 numColumns, Uint32 numMines);

	GameState getState() const;

	~Minesweeper();

	void hook(EventManager* em);

	bool victory();

	void setCellUserData(Uint32 x, Uint32 y, void* data);

	const Minefield& getField() const;

	void restart();

	void mark(MineCell& cell);

	void mark(Uint32 x, Uint32 y);

	const  MineCell& getCell(Uint32 x, Uint32 y) const;

	void reveal(std::set<MineCell*>& cells);

	void revealNeighbours(Uint32 x, Uint32 y);

	void reveal(Uint32 x, Uint32 y);


private:
	Minefield* mField;
	EventManager* mEventManager;	
	GameState mGameState;

}; // </Minesweeper>

#endif // </MINES_H>
