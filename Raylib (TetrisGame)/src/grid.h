#pragma once
#include <vector>
#include <raylib.h>

using namespace std;

class Grid{
    public:

        Grid();
        void Initialize();
        void print();
        void draw();
        bool IsCellOutside(int row, int cols);
        bool IsCellEmpty(int row, int cols);
        int ClearFullRows();

        int grid[20][10];
    private:
        bool IsRowFull(int row);
        void ClearRow(int row);
        void MoveRowsDown(int row, int numRows);
        int numRows;
        int numCols;
        int cellSize;

        vector<Color> colors;
};