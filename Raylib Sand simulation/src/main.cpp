#include "raylib.h"
#include <vector>
#include <cstring>

using namespace std;

const int numRows = 39;
const int numCols = 39;

const Color sand = { 194, 178, 128, 255 };
const Color darkGrey = {26, 31, 40, 255};
const Color lightBlue = {59, 85, 162, 255};


void HandleMouse(vector<vector<int>>& grid, int cellSize) {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        int col = (mousePos.x - 9) / cellSize;
        int row = (mousePos.y - 9) / cellSize;
        if (row >= 0 && row < numRows && col >= 0 && col < numCols) {
            grid[col][row] = 1;
        }
    }
}

int main(){

    int cellSize = 15;

    vector<vector<int>> grid(numCols, vector<int>(numRows, 0));
    
    InitWindow(600, 600, "Sand Simulation");
    SetTargetFPS(20);

    while(WindowShouldClose() == false)
    {
        BeginDrawing();
        ClearBackground(darkGrey);

        HandleMouse(grid, cellSize);

        for(int i = 0; i < numCols; i++){
            for(int j = 0; j < numRows; j++){
                int x = i * cellSize;
                int y = j * cellSize;

                if(grid[i][j] == 1){
                    DrawRectangle(x + 9, y + 9, cellSize - 1, cellSize - 1, sand);
                }
                else{
                    DrawRectangle(x + 9, y + 9, cellSize - 1, cellSize - 1, lightBlue);
                }   
            }
        }
        
        vector<vector<int>> nextGrid(numCols, vector<int>(numRows, 0));

        for(int i = 0; i < numCols; i++){
            for(int j = numRows - 1; j >= 0; j--){
                int state = grid[i][j];
                if(state == 1){
                    // Check if we are not at the bottom row
                    if(j + 1 < numRows){
                        int below = grid[i][j + 1];

                        int belowA = (i - 1 >= 0) ? grid[i - 1][j + 1] : 1;
                        int belowB = (i + 1 < numCols) ? grid[i + 1][j + 1] : 1;

                        if(below == 0){
                            nextGrid[i][j + 1] = 1;
                        }
                        else if(belowA == 0 && belowB == 0){
                            int rand = GetRandomValue(0, 1);
                            if(rand == 0 && i - 1 >= 0){
                                nextGrid[i - 1][j + 1] = 1;
                            }
                            else if(i + 1 < numCols){
                                nextGrid[i + 1][j + 1] = 1;
                            }
                            else{
                                nextGrid[i][j] = 1;
                            }
                        }
                        else if(belowA == 0 && i - 1 >= 0){
                            nextGrid[i - 1][j + 1] = 1;
                        }
                        else if(belowB == 0 && i + 1 < numCols){
                            nextGrid[i + 1][j + 1] = 1;
                        }
                        else{
                            nextGrid[i][j] = 1;
                        }
                    }
                    else{
                        nextGrid[i][j] = 1;
                    }
                }
            }
        }
        grid = nextGrid;

        EndDrawing();
    }

    return 0;
}