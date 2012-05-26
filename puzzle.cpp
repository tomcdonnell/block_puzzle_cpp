/*************************************************************************************************\
*                                                                                                 *
* "puzzle.cpp" - Member functions of class "puzzle" (defined in "puzzle.h").                      *
*                                                                                                 *
*      Author  - Tom McDonnell                                                                    *
*                                                                                                 *
\*************************************************************************************************/

#include "puzzle.h"

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

/*
 * Constructor.
 */
puzzle::puzzle(void) {
   currentBlockPtr = NULL; // initialise Q
   
   height = 8; // initailize grid height
   width  = 8; // initialize grid width

   for (int r = 0; r < height; ++r)
     for (int c = 0; c < width; ++c)
       grid[r][c] = RGB(0, 0, 0);

   nextEmptyPos.r = nextEmptyPos.c = 0;
   strcpy(textBuffer, "");
   solving = foundSolution = false;
}

/*
 * Destructor.
 */
puzzle::~puzzle(void) {
}

/*
 * Serve block from 'Q' and set as "currentBlock".
 * Return true if successful, false otherwise.
 */
void puzzle::pickUpBlock(void) {
   assert(currentBlockPtr == NULL);
   Q.serve(currentBlockPtr);
   if (!solving) {
      // reset orientation
      currentBlockPtr->changeOrientation(0);
      // set holdPos to original
      currentBlockPtr->resetHoldPos();
   }
}

/*
 * Append "currentBlock" to 'Q' and set "currentBlockPtr" to NULL.
 * Return true if successful, false otherwise.
 */
void puzzle::putDownBlock(void) {
   assert(currentBlockPtr != NULL);
   Q.append(currentBlockPtr);
   currentBlockPtr = NULL;
}

/*
 * Change "currentBlock" orientation.
 * (orientation is an int (0-7) descibing state of block.
 *  0 = initial state
 *  0, 1, 2, 3 = rotated (0, 1, 2, or 3) *90 degrees clockwise
 *  4, 5, 6, 7 = flipped verically then rotated
 *               (0, 1, 2, 3) *90 degrees clockwise)
 */
inline void puzzle::reorientBlock(int o) {
   currentBlockPtr->changeOrientation(o);
}

/*
 * Test whether orientation of "currentBlock" is unique.
 * (a unique orientation is one that has no identical orientations
 *  less than itself.
 *  eg. A square block's only unique orientation is 0 as for each
 *      other orientation (1-7) there is an orientation that is
 *      identical and lesser in value (0).
 *      A rectangular block has two unique orientations, 0 and 1.
 *      A block with no symmetry has 8 unique orientations (0-7).
 */
inline bool puzzle::blockOrientUnique(int o) {
   return currentBlockPtr->uniqueOrientation(o);
}

/*
 * Draw "currentBlock" at position 'p' in puzzle.
 * For use when holding block. ie. block not in puzzle.
 * 'p' here is position in grid of "holdPos" of block.
 */
void puzzle::drawBlock(pos p) {
   assert(p.r >= 0 && p.r < height && p.c >= 0 && p.c < width);
   // correct for holdPos of block
   p.r -= currentBlockPtr->getHoldPos().r; 
   p.c -= currentBlockPtr->getHoldPos().c;
   drawBlock(p, currentBlockPtr->getColour());
}

/*
 * Erase "currentBlock" from position 'p' in puzzle.
 * for use when holding block. ie. block not in puzzle.
 * 'p' here is position in grid of "holdPos" of block.
 */
void puzzle::eraseBlock(pos p) {
   assert(p.r >= 0 && p.r < height && p.c >= 0 && p.c < width);
   // correct for holdPos of block
   p.r -= currentBlockPtr->getHoldPos().r;
   p.c -= currentBlockPtr->getHoldPos().c;
   drawBlock(p, RGB(0, 0, 0));
}

/*
 * Attempt to add "currentBlock" to puzzle at first available
 * position (looking at grid from left->right & top->bottom).
 * Return true if successful, else false.
 */
inline bool puzzle::addBlock(void) {
   return addBlock(nextEmptyPos);
}

/*
 * Attempt to add "currentBlock" to puzzle at position 'p'.
 * Return true if successful, else false.
 */
bool puzzle::addBlock(pos p) {
   assert(p.r >= 0 && p.r < height && p.c >= 0 && p.c < width);

   if (!solving) {
      // correct for holdPos and TL of block
      p.r -= currentBlockPtr->getHoldPos().r; 
      p.c -= currentBlockPtr->getHoldPos().c - currentBlockPtr->getTLcol();

      if (p.r < 0 || p.r >= height || p.c < 0 || p.c >= width)
        return false;
   }

   if (blockFits(p)) {
      // update grid array
      int blockH = currentBlockPtr->getHeight(),
          blockW = currentBlockPtr->getWidth(),
          TLcol  = currentBlockPtr->getTLcol(),
          colour = currentBlockPtr->getColour(),
          r, c;
      for (r = 0; r < blockH; ++r)
	     for (c = 0; c < blockW; ++c)
          if (currentBlockPtr->getGrid(r, c))
          grid[p.r + r][p.c + c - TLcol] = colour;

      if (grid[nextEmptyPos.r][nextEmptyPos.c] != RGB(0, 0, 0))
        // nextEmptyPos is no longer empty
        nextEmptyPos = findNextEmptyPos();

      currentBlockPtr->setPuzPos(p); // set puzPos
      S.push(currentBlockPtr);
      currentBlockPtr = NULL;
      return true; // block added to puzzle
   }
   return false; // block doesn't fit
}

/*
 * Remove last block added to puzzle (of those remaining) and
 * set it as "currentBlock".
 * Do nothing if already holding block.
 * Return true if block removed, else false.
 */
inline void puzzle::removeBlock(void) {
   assert(currentBlockPtr == NULL);
   S.pop(currentBlockPtr);
   updateGrid();
}

/*
 * Remove block occupying position 'p' from puzzle
 * and set as "currentBlock".
 * Do nothing if already holding block or 'p' unoccupied.
 */
bool puzzle::removeBlock(pos p) {
   assert(p.r >= 0 && p.r < height && p.c >= 0 && p.c < width);
   assert(currentBlockPtr == NULL);
   if (grid[p.r][p.c] != RGB(0, 0, 0)) {
      // find block in stack
      stack<block *> tempS(S.getCount());
      while (!S.empty()) {
         S.pop(currentBlockPtr);
         if (currentBlockPtr->getColour() == grid[p.r][p.c])
           break;
         else
           tempS.push(currentBlockPtr); // wrong block
      }
      // restore stack
      block *bPtr;
      while (!tempS.empty()) {
         tempS.pop(bPtr);
         S.push(bPtr);
      }
      // set holdPos of block
      p.r -= currentBlockPtr->getPuzPos().r;
      p.c -= currentBlockPtr->getPuzPos().c - currentBlockPtr->getTLcol();
      currentBlockPtr->setHoldPos(p); 
      updateGrid();
      return true;
   }
   return false;
}

/*
 * Print puzzle grid to screen as text.
 */
void puzzle::print(void) {
   int r, c;
   for (r = 0; r < height; ++r) {
      for (c = 0; c < width; ++c)
        if (grid[r][c] != RGB(0, 0, 0)) {
           // PROBLEM: need to set text colour then use (char)219
           cout << (char)grid[r][c] << (char)grid[r][c];
        }
        else cout << "--";
      cout << endl;
   }
}

/*
 * Draw puzzle.
 */
void puzzle::draw(void) {
   int r, c;
   for (r = 0; r < height; ++r)
     for (c = 0; c < width; ++c)
       drawSquare(grid[r][c], r, c);
}

/*
 * Draw text in text area of main window.
 * If empty string is supplied will redraw most recent message.
 */
void puzzle::drawText(char *stringPtr) {
   HDC hdc = GetDC(main_window_handle);
   
   // clear text area
   HPEN pen = CreatePen(PS_SOLID, 0, RGB(200, 200, 200));
   HPEN old_pen = (HPEN)SelectObject(hdc, pen);   
   HBRUSH brush = CreateSolidBrush(RGB(200, 200, 200));
   HBRUSH old_brush = (HBRUSH)SelectObject(hdc, brush);
   Rectangle(hdc, 0, height * SQUARE_SIZE,
             width * SQUARE_SIZE, height * SQUARE_SIZE + 16);
   SelectObject(hdc, old_pen);
   SelectObject(hdc, old_brush);
   DeleteObject(pen);
   DeleteObject(brush);

   // draw text
   COLORREF oldTextColour = SetTextColor(hdc, RGB(0, 0, 0)),
            oldBkColour   = SetBkColor(hdc, RGB(200, 200, 200));
   if (strcmp(stringPtr, "") != 0) // if not empty string
     strcpy(textBuffer, stringPtr);
   TextOut (hdc, 0, height * SQUARE_SIZE, textBuffer, strlen(textBuffer));
   SetTextColor(hdc, oldTextColour);
   SetBkColor  (hdc, oldBkColour);

   ReleaseDC(main_window_handle, hdc);
}

/*
 * Find all solutions of the puzzle from the puzzles current
 * state.  Return the number of solutions found.
 */
int puzzle::solve(void) {
   assert(currentBlockPtr == NULL);
   solutionCount = 0;
   percentSolved = 0;
   ofstream file("solution.dat");
   stack<block *> tempS(S.getCount());

   file << S; // save initial state of puzzle to first line of solution file

   // empty stack to enable solve of partially complete puzzle
   while (S.pop(currentBlockPtr))
     tempS.push(currentBlockPtr);
   currentBlockPtr = NULL;

   int startTime = GetTickCount();  // start timing
   solving = true;
   bool foundAllSolutions = solveRecursively(file);
   solving = false;
   int finishTime = GetTickCount(); // stop  timing
   float timeTaken = float(finishTime - startTime) / 1000; // calculate time taken in seconds

   drawText(" "); // clear text area of percentage complete message
   char buffer[100]; // NOTE: problems can occur if message below greater in size than buffer

   if (foundAllSolutions)
     sprintf(buffer, "%d solutions were found.\n"
                     "Time taken: %.2f seconds",
             solutionCount, timeTaken);
   else {
      // remove blocks placed during solve
      while (!S.empty()) {
         removeBlock();
         putDownBlock();
      }
      sprintf(buffer, "%2.1f%% through solution process,\n"
                      "%d solutions were found.\n"
                      "Time taken: %.2f seconds",
              percentSolved, solutionCount, timeTaken);
   }


   MessageBox(main_window_handle, buffer, "BlockPuzzle", MB_OK);

   // restore stack
   while (tempS.pop(currentBlockPtr))
     S.push(currentBlockPtr);

   currentBlockPtr = NULL;
   return solutionCount;
}

/*
 * Read the "solutionNo"th solution from the file "solution.dat",
 * add blocks to the puzzle in the way described,
 * then draw the solved puzzle.
 */
void puzzle::viewSolution(int solutionNo) {
   assert(currentBlockPtr == NULL);
   assert(solutionCount > 0);
   assert(solutionNo > 0 && solutionNo <= solutionCount);

   ifstream inputSolnFile("solution.dat");
   int i, bOrientation, bCount;
   COLORREF bColour;

   for (i = 0; i < solutionNo; ++i)
     while (inputSolnFile.get() != '\n');

   bCount = 0;
   solving = true; // so that add/removeBlock() do not draw
   while (inputSolnFile.peek() != '\n') {
      inputSolnFile >> bColour >> bOrientation;
      // find block in queue
      for (i = 0; i < Q.getCount(); ++i) {
         pickUpBlock();
         if (currentBlockPtr->getColour() == bColour)
           break;
         putDownBlock();
      }
      currentBlockPtr->changeOrientation(bOrientation);
      addBlock();
      ++bCount;
      // ignore spaces following block data in file
      inputSolnFile.ignore(2);
   }
   inputSolnFile.get(); // remove '\n' from input stream
   draw();

   sprintf(textBuffer, "Solution %d.", solutionNo);
   drawText(textBuffer);

   for (int j = 0; j < bCount; ++j) {
      removeBlock();
      putDownBlock();
   }
   solving = false; // allowing add/removeBlock() to draw again
}

/*
 * Remove all blocks from the puzzle, delete all the blocks,
 * then read a new block set from file "fileName".
 */
bool puzzle::readBlockSet(char *fileName) {
   assert(currentBlockPtr == NULL);
   ifstream file(fileName, ios::nocreate);

   if (file == NULL)
     return false;

   // remove all blocks from puzzle
   pos p;
   for (p.r = 0; p.r < height; ++p.r)
     for (p.c = 0; p.c < width; ++p.c)
       if (grid[p.r][p.c] != RGB(0, 0, 0)) {
          removeBlock(p);
          putDownBlock();
       }

   // delete all blocks
   while (Q.serve(currentBlockPtr))
     delete currentBlockPtr;

   // fill Q with blocks read from file
   block tempBlock;
   while (file >> tempBlock) {
      currentBlockPtr = new block(tempBlock);
      Q.append(currentBlockPtr);
   }

   currentBlockPtr = NULL;
   return true;
}

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////

/*
 * Test whether "currentBlock" fits in puzzle grid with blocks TL
 * square on 'p'.  Return true if does, false otherwise.
 */
bool puzzle::blockFits(const pos p) {
   assert(p.r >= 0 && p.r < height && p.c >= 0 && p.c < width);
   // superimpose block over grid with TL on p.  Check whether any
   // part of block now lies outside grid boundary or is in position
   // already occupied.
   int blockH    = currentBlockPtr->getHeight(),
       blockW    = currentBlockPtr->getWidth(),
       TLcol     = currentBlockPtr->getTLcol(),
       rowOffset = p.r,
       colOffset = p.c - TLcol,
       r, c;
   for (r = 0; r < blockH; ++r)
     for (c = 0; c < blockW; ++c)
       if (currentBlockPtr->getGrid(r, c)
           && (//    left overlaps
                  (c < TLcol && (c + colOffset) < 0)
               // OR right overlaps
               || (c > TLcol && (c + colOffset) > width - 1)
	            // OR bottom overlaps (no need to check top overlap)
               || (r > 0 && (r + rowOffset) > height - 1)
	            // OR position already occupied
               || (grid[r + rowOffset][c + colOffset] != RGB(0, 0, 0))))
	      return false;
   return true;
}

/*
 * 'p' here is the puzzle grid position of the top left square of
 * block grid, occupied or unoccupied. ie not TL.
 */
void puzzle::drawBlock(pos p, COLORREF colour) {
   int blockH = currentBlockPtr->getHeight(),
       blockW = currentBlockPtr->getWidth(),
       r, c;
   for (r = 0; r < blockH; ++r)
     for (c = 0; c < blockW; ++c) {
        if (currentBlockPtr->getGrid(r, c)
            && p.r + r < height && p.c + c < width)
          if (colour != RGB(0, 0, 0))
            drawSquare(colour, p.r + r, p.c + c);
          else // colour = black (erase)
            // draw square with colour stored in grid
            drawSquare(grid[p.r + r][p.c + c], p.r + r, p.c + c);
     }
}

/*
 * To be used after removing a block from the stack.
 * Remove "currentBlock" from puzzle grid using "pusPos" and also
 * erase "currentBlock" from screen and update "nextEmptyPos".
 */
void puzzle::updateGrid(void) {
   COLORREF colour  = currentBlockPtr->getColour();
   int startR  = currentBlockPtr->getPuzPos().r,
       startC  = currentBlockPtr->getPuzPos().c - currentBlockPtr->getTLcol(),
       finishR = startR + currentBlockPtr->getHeight(),
       finishC = startC + currentBlockPtr->getWidth(),
       r, c;

   // remove block of specific colour from grid
   for (r = startR; r < finishR; ++r)
     for (c = startC; c < finishC; ++c)
       if (grid[r][c] == colour) {
          grid[r][c] = RGB(0, 0, 0);
          if (!solving)
            drawSquare(grid[r][c], r, c);
       }

   // update nextEmptyPos
   if (solving) {
      nextEmptyPos.r = currentBlockPtr->getPuzPos().r;
      nextEmptyPos.c = currentBlockPtr->getPuzPos().c;
   }
   else {
      // reset so searches from r = c = 0
      nextEmptyPos.r = nextEmptyPos.c = 0;
      nextEmptyPos = findNextEmptyPos();
   }

   foundSolution = false; // just removed block so cannot be solved
}

/*
 * Find next empty position in puzzle grid searching
 * left->right & top->bottom starting at the previous
 * "nextEmptyPos".
 */
pos puzzle::findNextEmptyPos(void) {
   pos p;
   p.c = nextEmptyPos.c;
   for (p.r = nextEmptyPos.r; p.r < height; ++p.r) {
      for (; p.c < width; ++p.c)
        if (grid[p.r][p.c] == RGB(0, 0, 0))
          return p;
      p.c = 0;
   }
   foundSolution = true;
   return p;
}

/*
 * Slave function for "int solve(void)".
 */
bool puzzle::solveRecursively(ofstream file) {
   assert(currentBlockPtr == NULL);

   // handle Windows OS messages so that mouse movement etc. is not
   // halted while solving and so that user may stop solution process
   MSG msg;
   if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
         solving = false;
   	TranslateMessage(&msg); // translate any accelerator keys
		DispatchMessage(&msg); // send the message to the window proc
   }
   if (!solving)
     return false;

   int QCount = Q.getCount(),
       i, j;
   for (i = 0; i < QCount; ++i) { // for each block left in queue
      pickUpBlock();
      for (j = 0; j < 8; ++j) { // for each possible block orientation

         // update message each time the stack is emptied (puzzle cleared)
         if (S.empty()) {
            sprintf(textBuffer, "%2.1f%% complete.",
                    percentSolved);
            drawText(textBuffer);
            percentSolved += 100 / (float)(QCount * 8);
         }

         if (blockOrientUnique(j)) {
            reorientBlock(j);
            if (addBlock()) {
               // NOTE: must add this feature to pull down menu
               //if (animateSolnProcess) {
               //   draw();
               //   delay(100);
               //}

               if (foundSolution) {
                  ++solutionCount;
                  file << S;
               }
               if (!solveRecursively(file)) // recursive call
                 // solution process has been halted early
                 return false;
               removeBlock();
            }
         }
      }
      putDownBlock();
   }
   return true;
}

/*
 * Draw square to screen at 'p' in colour 'c'.
 */
void puzzle::drawSquare(const COLORREF colour, int r, int c) {
   if (r >= 0 && r < height && c >= 0 && c < width) {
      HDC hdc = GetDC(main_window_handle);

      HPEN pen = CreatePen(PS_SOLID, 0, colour);
      HPEN old_pen = (HPEN)SelectObject(hdc, pen);

      HBRUSH brush = CreateSolidBrush(colour);
      HBRUSH old_brush = (HBRUSH)SelectObject(hdc, brush);

      Rectangle(hdc, c * SQUARE_SIZE, r * SQUARE_SIZE,
               (c + 1) * SQUARE_SIZE, (r + 1) * SQUARE_SIZE);
   
      SelectObject(hdc, old_pen);
      SelectObject(hdc, old_brush);
      DeleteObject(pen);
      DeleteObject(brush);
      ReleaseDC(main_window_handle, hdc);
   }
}