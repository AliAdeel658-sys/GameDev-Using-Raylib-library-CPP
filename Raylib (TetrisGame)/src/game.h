#pragma once
#include "grid.h"
#include "blocks.cpp"

using namespace std;

class Game
{
public:
    Game();
    Block GetRandomBlock();
    vector<Block> GetAllBlocks();
    void draw();
    void HandleInput();
    void MoveBlockLeft();
    void MoveBlockRight();
    void MoveBlockDown();
    Grid grid;
    bool GameOver;
    int score;

private:
    bool IsBlockOutside();
    void RotateBlock();
    void LockBlock();
    bool BlockFits();
    void Reset();
    void UpdateScore(int LinesCleared, int moveDownPoints);
    vector<Block> blocks;
    Block CurrentBlock;
    Block nextBlock;
};