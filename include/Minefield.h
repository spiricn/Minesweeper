#ifndef MINEFIELD_H
#define MINEFIELD_H

#include "stdafx.h"

struct MineCell{
	bool visible;
	bool hasMine;
	unsigned char numNeighborMines;
	void* userData;
	bool marked;
	uint32_t x, y;

	MineCell() : visible(false), hasMine(false), userData(NULL), numNeighborMines(0), marked(false){
	}

}; // </MineCell>

class Minefield{
public:
	Minefield();

	Minefield(Uint32 numRows, Uint32 numColumns, Uint32 numMines);

	~Minefield();

	void restart();

	const MineCell& getCell(int32_t x, int32_t y) const;

	MineCell& getCell(int32_t x, int32_t y);

	bool inBounds(int32_t x, int32_t y) const;

	Uint32 getNumRows() const;

	Uint32 getNumColumn() const;

private:
	MineCell* mField;
	Uint32 mNumRows, mNumColumns, mNumMines;

}; // </Minefield>


#endif // </MINEFIELD_H>