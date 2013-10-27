#include "stdafx.h"

#include <wt/Math.h>

#include "Minefield.h"

#define TD_TRACE_TAG "Minefield"

using namespace wt;

Minefield::Minefield() : mField(NULL){
}

Minefield::Minefield(Uint32 numRows, Uint32 numColumns, Uint32 numMines) : mNumRows(numRows), mNumColumns(numColumns), mNumMines(numMines){
	mField = new MineCell[numRows*numColumns];

	restart();
}

Minefield::~Minefield(){
	if(mField){
		delete[] mField;
	}
}

void Minefield::restart(){
	for(Uint32 x=0; x<mNumColumns; x++){
		for(Uint32 y=0; y<mNumRows; y++){
			MineCell& cell = getCell(x, y);

			cell.marked = false;
			cell.visible= false;
			cell.numNeighborMines = 0;
			cell.hasMine = false;
		}
	}

	Uint32 numMines = mNumMines;
	while(numMines){
		MineCell& cell = getCell((Uint32)(math::random()*(mNumColumns-1)), (Uint32)(math::random()*(mNumRows-1)));
		if(!cell.hasMine){
			cell.hasMine = true;
			--numMines;
		}

	}

	for(Uint32 x=0; x<mNumColumns; x++){
		for(Uint32 y=0; y<mNumRows; y++){
			MineCell& cell = getCell(x, y);


			cell.x = x;
			cell.y = y;

			cell.visible = false;

			for(int dx=-1; dx<2; dx++){
				for(int dy=-1; dy<2; dy++){
					if(dx==0 && dy==0){
						continue;
					}

					if(inBounds(x+dx, y+dy) && getCell(x+dx, y+dy).hasMine){
						++cell.numNeighborMines;
					}
				}
			}
		}
	}
}

const MineCell& Minefield::getCell(int32_t x, int32_t y) const{
	return getCell(x, y);
}

MineCell& Minefield::getCell(int32_t x, int32_t y){
	WT_ASSERT(inBounds(x, y), "Coord out of bounds (%d, %d)", x, y);

	int32_t idx =y*mNumColumns + x;
	WT_ASSERT(idx >= 0 && idx < (int32_t)(mNumRows*mNumColumns), "Invalid idx");

	return mField[idx];
}

bool Minefield::inBounds(int32_t x, int32_t y) const{
	return x >= 0 && y >= 0 &&
		x < (int32_t)mNumColumns && y < (int32_t)mNumRows;
}

	
Uint32 Minefield::getNumRows() const{
	return mNumRows;
}

Uint32 Minefield::getNumColumn() const{
	return mNumColumns;
}
