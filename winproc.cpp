/*************************************************************************************************\
*                                                                                                 *
* "winproc.cpp" - Event handler for main window of windows application "blockpuzzle.exe".         *
*                                                                                                 *
*       Author  - Tom McDonnell                                                                   *
*                                                                                                 *
\*************************************************************************************************/

#include <windows.h>

#include "menu.h"
#include "puzzle.h"

enum gameStates {HOLDING_BLOCK, NOT_HOLDING_BLOCK, SOLVING, VIEWING_SOLUTIONS};

extern OPENFILENAME openBox; // defined in winmain.cpp
extern puzzle       puz;     // defined in winmain.cpp

static enum gameStates gameState;
static pos             mousePos;  // mouse position in row, column format

static int solutionNo = 0, solutionCount = 0; // used when viewing solutions

/*
 * Event handler of main window.
 */
LRESULT CALLBACK WindowProc(HWND   hwnd, 
						          UINT   msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
   PAINTSTRUCT	ps;  // used in WM_PAINT
   HDC hdc;

   switch(msg) {
    case WM_CREATE:
		// do initialization stuff here
      gameState = NOT_HOLDING_BLOCK;
		return(0);
      break;
    case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);
      if (gameState != SOLVING)
        puz.draw();
      puz.drawText(""); // redraw last text message drawn
		EndPaint(hwnd, &ps);
		return(0);
      break;
    case WM_MOUSEMOVE:
      if (gameState == HOLDING_BLOCK) {
         // if necessary, erase block and redraw in its new position
         assert(puz.holdingBlock());
         pos newMousePos;
         newMousePos.r = (int)HIWORD(lparam) / SQUARE_SIZE;
         newMousePos.c = (int)LOWORD(lparam) / SQUARE_SIZE;
         if (mousePos != newMousePos                               &&
             newMousePos.r >= 0 && newMousePos.r < puz.getHeight() &&
             newMousePos.c >= 0 && newMousePos.c < puz.getWidth()) {
            // mousePos has changed AND newMousePos is inside puzzle grid
            puz.eraseBlock(mousePos);
            // update mouse pos
            mousePos = newMousePos;
            puz.drawBlock(mousePos);
         }
      }
      break;
    case WM_LBUTTONDOWN:
       if ((int)HIWORD(lparam) / SQUARE_SIZE == puz.getHeight())
         break; // mouse pointer is over status bar at bottom of window - do nothing
       mousePos.r = (int)HIWORD(lparam) / SQUARE_SIZE;
       mousePos.c = (int)LOWORD(lparam) / SQUARE_SIZE;
       switch (gameState) {
        case NOT_HOLDING_BLOCK:
          // remove block from puzzle or pick up block
          assert(!puz.holdingBlock());
          puz.removeBlock(mousePos);
          if (!puz.holdingBlock())
            puz.pickUpBlock();
          puz.drawBlock(mousePos);
          gameState = HOLDING_BLOCK;
          break;
        case HOLDING_BLOCK:
          // add block to puzzle or put down block
          assert(puz.holdingBlock());
          if (puz.addBlock(mousePos)) {
             if (puz.solved())
               MessageBox(main_window_handle, 
                          "Congratulations - puzzle solved!",
                          "Block Puzzle", MB_OK);
          }
          else {
             puz.eraseBlock(mousePos);
             puz.putDownBlock();
          }
          gameState = NOT_HOLDING_BLOCK;
          break;
        case SOLVING:
          // display number of solutions found so far
          char buffer[30];
          sprintf(buffer, "%d solutions found so far.", puz.getSolutionCount());
          puz.drawText(buffer);
          break;
        case VIEWING_SOLUTIONS:
          ++solutionNo;
          if (solutionNo <= solutionCount)
            puz.viewSolution(solutionNo);
          else {
             puz.draw();
             puz.drawText(" ");
             gameState = NOT_HOLDING_BLOCK;
          }
          break;
       }
       break; // end WM_LBUTTON_DOWN
    case WM_MBUTTONDOWN:
      if (gameState == HOLDING_BLOCK) {
         // rotate block
         assert(puz.holdingBlock());
         puz.eraseBlock(mousePos);
         puz.rotateBlock();
         puz.drawBlock(mousePos);
      }
      break;
    case WM_RBUTTONDOWN:
      switch (gameState) {
       case HOLDING_BLOCK:
         // flip block
         assert(puz.holdingBlock());
         puz.eraseBlock(mousePos);
         puz.flipBlock();
         puz.drawBlock(mousePos);
         break;
       case SOLVING:
         // halt solution process
         puz.stopSolving();
         gameState = VIEWING_SOLUTIONS;
         break;
       case VIEWING_SOLUTIONS:
         puz.draw();
         puz.drawText(" ");
         gameState = NOT_HOLDING_BLOCK;
      }
      break;
    case WM_COMMAND: { // pull down menu item selected
       switch (wparam) {
/* features not yet fully implemented
        case MENU_FILE_LOAD_NEW_BLOCK_SET:
          // load new block set

          // PROBLEM: when "tetris block set" is loaded, program crashes when
          //          attempting to pick up last block
          
          if (gameState == HOLDING_BLOCK) {
             assert(puz.holdingBlock());
             puz.eraseBlock(mousePos);
             puz.putDownBlock();
          }
          GetOpenFileName(&openBox); // display the open dialog box
          puz.readBlockSet(openBox.lpstrFile);
          puz.drawText("New Block Set Loaded.");
          gameState = NOT_HOLDING_BLOCK;
          break;
        case MENU_FILE_LOAD_NEW_PUZZLE_GRID:
          // load new puzzle grid
          break;
*/
        case MENU_FILE_EXIT:
          // kill the application
	 	    PostQuitMessage(0);
 		    return(0);
          break;
        case MENU_OPTIONS_SOLVE:
          // solve puzzle
          if (gameState == HOLDING_BLOCK) {
             assert(puz.holdingBlock());
             puz.eraseBlock(mousePos);
             puz.putDownBlock();
          }
          puz.draw();
          gameState = SOLVING;
          solutionCount = puz.solve();
          if (solutionCount > 0) {
             solutionNo = 1;
             puz.viewSolution(solutionNo);
             gameState = VIEWING_SOLUTIONS;
          }
          else
            gameState = NOT_HOLDING_BLOCK;
          break;
        case MENU_HELP_INSTRUCTIONS:
          MessageBox(main_window_handle,
                     "Use the left mouse button to pick up a block or\n"
                     "to place the block held in the puzzle.\n"
                     "While holding a block, use the middle mouse button\n"
                     "to rotate or the right mouse button to flip the\n"
                     "block.\n\n"
                     "To put down a block, attempt to place it where it\n"
                     "will not fit.\n\n"
                     "To solve the puzzle, select solve from the 'Options'\n"
                     "menu.\n"
                     "While the computer solves the puzzle, use the left\n"
                     "mouse button to recive an update on the number of\n"
                     "solutions found or the right mouse button to halt\n"
                     "the solution process.\n\n"
                     "While viewing solutions found by the computer, use\n"
                     "the left mouse button to view the next solution or\n"
                     "the right mouse button to stop viewing solutions.\n\n"
                     "The options 'Load New Block Set' and 'Load new Puzzle\n"
                     "grid' in the 'File' menu are unavailable.\n",
                     "Block Puzzle - Instructions", MB_OK);
          break;
        case MENU_HELP_ABOUT:
          MessageBox(main_window_handle,
                     "Version 1.0\n"
                     "by Tom McDonnell, 2001.\n\n"
                     "Based on 'I.Q. Block', Petoy.", 
                     "Block Puzzle", MB_OK);
          break;
       }
    } break;
    case WM_DESTROY:
		// kill the application
	 	PostQuitMessage(0);
 		return(0);
      break;
   } // end main switch

   // use default winProc to process any messages not taken care of
   return (DefWindowProc(hwnd, msg, wparam, lparam));
}