#include "game.h"
#include <random>

using namespace std;

Game::Game()
{
    grid = Grid();
    blocks = GetAllBlocks();
    CurrentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    GameOver = false;
    score = 0;

}

Block Game::GetRandomBlock()
{
    if(blocks.empty()){
        blocks = GetAllBlocks();
    }
    int RandIdx = rand() % blocks.size();
    Block block = blocks[RandIdx];
    blocks.erase(blocks.begin() + RandIdx);

    return block;
}

vector<Block> Game::GetAllBlocks()
{
    return {IBlock(), JBlock(), LBlock(), OBlock(), SBlock(), TBlock(), ZBlock()};
}

void Game::draw()
{
    grid.draw();
    CurrentBlock.draw(11, 11);
    switch(nextBlock.id){
        case 3:
            nextBlock.draw(255,290);
            break;
        case 4:
            nextBlock.draw(255,280);
            break;
        default:
            nextBlock.draw(270,270);
            break;
    }
}

void Game::HandleInput()
{
    int keyPressed = GetKeyPressed();
    if(GameOver && keyPressed != 0){
        GameOver = false;
        Reset();
    }

    switch(keyPressed)
    {
        case KEY_LEFT:
            MoveBlockLeft();
            break;

        case KEY_RIGHT:
            MoveBlockRight();
            break;

        case KEY_DOWN:
            MoveBlockDown();
            UpdateScore(0, 1);
            break;
        
        case KEY_UP:
            RotateBlock();
            break;
    }
}

void Game::MoveBlockLeft()
{   
    if(!GameOver){
        CurrentBlock.Move(0, -1);
        if(IsBlockOutside() || BlockFits() == false){
            CurrentBlock.Move(0, 1);
        }
    }
}

void Game::MoveBlockRight()
{
    if(!GameOver){
        CurrentBlock.Move(0, 1);
        if(IsBlockOutside() || BlockFits() == false){
            CurrentBlock.Move(0, -1);
        }
    }
    
}

void Game::MoveBlockDown()
{   
    if(!GameOver){
        CurrentBlock.Move(1, 0);
        if(IsBlockOutside() || BlockFits() == false){
            CurrentBlock.Move(-1, 0);
            LockBlock();
        }
    }
    
}

bool Game::IsBlockOutside()
{
    vector<Position> tiles = CurrentBlock.GetCellPosition();
    for(Position item: tiles){
        if(grid.IsCellOutside(item.row, item.cols)){
            return true;
        }
    }
    return false;
}

void Game::RotateBlock()
{
    if(!GameOver){
        CurrentBlock.Rotate();
        if(IsBlockOutside() || BlockFits() == false){
            CurrentBlock.UndoRotation();
        }
    }
    
}

void Game::LockBlock()
{
    vector<Position> tiles = CurrentBlock.GetCellPosition();
    for(Position item: tiles)
    {
        grid.grid[item.row][item.cols] = CurrentBlock.id;
    }
    CurrentBlock = nextBlock;
    if(BlockFits() == false)
    {
        GameOver = true;
    }
    nextBlock = GetRandomBlock();
    int rowsCleared = grid.ClearFullRows();
    UpdateScore(rowsCleared, 0);
}

bool Game::BlockFits()
{
    vector<Position> tiles = CurrentBlock.GetCellPosition();
    for(Position item : tiles)
    {
        if(grid.IsCellEmpty(item.row, item.cols) == false){
            return false;
        }
    }
    return true;
}

void Game::Reset()
{
    grid.Initialize();
    blocks = GetAllBlocks();
    CurrentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    score = 0;
}

void Game::UpdateScore(int LinesCleared, int moveDownPoints)
{
    switch (LinesCleared)
    {
    case 1:
        score += 100;
        break;
    case 2:
        score += 300;
        break;
    case 3:
        score += 500;
        break;
    default:
        break;
    }

    score += moveDownPoints;
}
