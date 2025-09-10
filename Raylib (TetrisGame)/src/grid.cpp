#include <iostream>
#include "grid.h"
#include "colors.h"

using namespace std;

Grid::Grid()
{
    numRows = 20;
    numCols = 10;

    cellSize = 30;

    Initialize();

    colors = GetCellColors();
}
void Grid::Initialize()
{
    for(int rows = 0; rows < numRows; rows++){
        for(int cols = 0; cols < numCols; cols++){
            grid[rows][cols] = 0;
        }
    }
}

void Grid::print()
{
    for(int rows = 0; rows < numRows; rows++){
        for(int cols = 0; cols < numCols; cols++){
            cout << grid[rows][cols]<<" ";
        }
        cout << endl;
    }
}


void Grid::draw()
{
    for(int rows = 0; rows < numRows; rows++){
        for(int cols = 0; cols < numCols; cols++){
            int cellValue = grid[rows][cols];
            DrawRectangle(cols * cellSize + 11, rows * cellSize + 11, cellSize - 1, cellSize - 1, colors[cellValue]);
        }
    }
}

bool Grid::IsCellOutside(int row, int cols)
{
    if(row >= 0 && row < numRows && cols >= 0 && cols < numCols){
        return false;
    }
    return true;
}

bool Grid::IsCellEmpty(int row, int cols)
{
    if(grid[row][cols] == 0){
        return true;
    }
    return false;
}

int Grid::ClearFullRows()
{
    int completed = 0;
    for(int row = numRows - 1; row >= 0; row--)
    {
        if(IsRowFull(row)){
            ClearRow(row);
            completed++;
        }
        else if(completed > 0){
            MoveRowsDown(row, completed);
        }
    }
    return completed;
}

bool Grid::IsRowFull(int row)
{
    for(int cols = 0; cols < numCols; cols++)
    {
        if(grid[row][cols] == 0){
            return false;
        }
    }
    return true;
}

void Grid::ClearRow(int row)
{
    for(int cols = 0; cols < numCols; cols++){
        grid[row][cols] = 0;
    }
}

void Grid::MoveRowsDown(int row, int numRows)
{
    for(int cols = 0; cols < numCols; cols++){
        grid[row + numRows][cols] = grid[row][cols];
        grid[row][cols] = 0;
    }
    
}
