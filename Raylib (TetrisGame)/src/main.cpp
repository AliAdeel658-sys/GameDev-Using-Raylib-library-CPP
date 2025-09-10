#include <raylib.h>
#include "game.h"
#include "colors.h"
#include <iostream>

using namespace std;

double lastUpdateTime = 0;

bool EventTriggered(double interval)
{
  double currentTime = GetTime();
  if(currentTime - lastUpdateTime >= interval){
    lastUpdateTime = currentTime;
    return true;
  }
  return false;
}

int main() 
{   
    
    InitWindow(500,620,"The Game");
    SetTargetFPS(60);

    Font font = LoadFontEx("Font/monogram.ttf", 64, 0, 0);
    
    Game game = Game();

    while (WindowShouldClose() == false)
    {   
      game.HandleInput();
      if(EventTriggered(0.5)){
        game.MoveBlockDown();
      }
      BeginDrawing();
      ClearBackground(darkBlue);

      DrawTextEx(font, "Score", {365,15}, 38 ,2 ,WHITE);
      DrawTextEx(font, "Next", {370,175}, 38 ,2 ,WHITE);

      if(game.GameOver)
      {
        DrawTextEx(font, "Game Over", {320,450}, 38 ,2 ,WHITE);
      }
      
      DrawRectangleRounded({320,55,170,60}, 0.3, 6, lightBlue);

      char scoreText[10];
      sprintf(scoreText,"%d", game.score);
      Vector2 textSize = MeasureTextEx(font ,scoreText, 38, 2);

      DrawTextEx(font, scoreText, {320 + (170 - textSize.x) / 2, 65}, 38 ,2 ,WHITE);
      
      DrawRectangleRounded({320,215,170,180}, 0.3, 6, lightBlue);

      game.draw();

      EndDrawing();
    }
    
    CloseWindow();
}