/*************************************************************************************************\
*                                                                                                 *
* "puzzle.h" - Class "puzzle" definition.                                                         *
*                                                                                                 *
*     Author - Tom McDonnell                                                                      *
*                                                                                                 *
\*************************************************************************************************/

#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "block.h"
#include "stack.h"
#include "queue.h"

#define MAX_PUZZLE_HEIGHT 25
#define MAX_PUZZLE_WIDTH  25
#define MAX_NUMBER_BLOCKS 15
#define SQUARE_SIZE       25

extern HWND main_window_handle;

class puzzle {
 public:
   puzzle(void);
   ~puzzle(void);

   int getWidth(void)         {return width;        }
   int getHeight(void)        {return height;       }
   int getSolutionCount(void) {return solutionCount;}

   void stopSolving(void)     {solving = false;}

   bool holdingBlock(void)    {return currentBlockPtr != NULL;}
   
   /*
    * Test whether puzzle is currently solved.
    * Return true if is, false otherwise.
    */
   bool solved(void) {return foundSolution;}

   /*
    * Serve block from 'Q' and set as "currentBlock".
    * Return true if successful, false otherwise.
    */
   void pickUpBlock(void);

   /*
    * Append "currentBlock" to 'Q' and set "currentBlockPtr" to NULL.
    * Return true if successful, false otherwise.
    */
   void putDownBlock(void);

   /*
    * Rotate "currentBlock" 90 degrees clockwise.
    */
   void rotateBlock(void) {currentBlockPtr->rotate();}

   /*
    * Flip "currentBlock" vertically.
    */
   void flipBlock(void) {currentBlockPtr->flip();}

   /*
    * Change "currentBlock" orientation.
    * (orientation is an int (0-7) descibing state of block.
    *  0 = initial state
    *  0, 1, 2, 3 = rotated (0, 1, 2, or 3) *90 degrees clockwise
    *  4, 5, 6, 7 = flipped verically then rotated
    *               (0, 1, 2, 3) *90 degrees clockwise)
    */
   void reorientBlock(int o);

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
   bool blockOrientUnique(int o);

   /*
    * Draw "currentBlock" at position 'p' in puzzle.
    * For use when holding block. ie. block not in puzzle.
    * 'p' here is position in grid of "holdPos" of block.
    */
   void drawBlock(pos p);

   /*
    * Erase "currentBlock" from position 'p' in puzzle.
    * for use when holding block. ie. block not in puzzle.
    * 'p' here is position in grid of "holdPos" of block.
    */
   void eraseBlock(pos);

   /*
    * Attempt to add "currentBlock" to puzzle at first available
    * position (looking at grid from left->right & top->bottom).
    * Return true if successful, else false.
    */
   bool addBlock();

   /*
    * Attempt to add "currentBlock" to puzzle at position 'p'.
    * Return true if successful, else false.
    */
   bool addBlock(pos p);
   
   /*
    * Remove last block added to puzzle (of those remaining) and
    * set it as "currentBlock".
    * Do nothing if already holding block.
    * Return true if block removed, else false.
    */
   void removeBlock(void);

   /*
    * Remove block occupying position 'p' from puzzle
    * and set as "currentBlock".
    * Do nothing if already holding block or 'p' unoccupied.
    */
   bool removeBlock(pos p);

   /*
    * Print puzzle grid to screen as text.
    */
   void print(void);

   /*
    * Print current block to screen as text.
    */
   void printBlock(void) {currentBlockPtr->print();}

   /*
    * Draw puzzle.
    */
   void draw(void);

   /*
    * Draw text in text area of main window.
    * If empty string is supplied will redraw most recent message.
    */
   void drawText(char *);

   /*
    * Find all solutions of the puzzle from the puzzles current
    * state.  Return the number of solutions found.
    */
   int solve(void);

   /*
    * Read the "solutionNo"th solution from the file "solution.dat",
    * add blocks to the puzzle in the way described,
    * then draw the solved puzzle.
    */
   void viewSolution(int);

   /*
    * Remove all blocks from the puzzle, delete all the blocks,
    * then read a new block set from file "fileName".
    */
   bool readBlockSet(char *filename);

 private:
   bool blockFits(const pos);
   void drawBlock(pos p, COLORREF colour);
   void updateGrid(void);
   pos  findNextEmptyPos(void);
   bool solveRecursively(ofstream);
   void drawSquare(const COLORREF, int, int);

   COLORREF grid[MAX_PUZZLE_HEIGHT][MAX_PUZZLE_WIDTH];
   block *currentBlockPtr;
   int height,
       width,
       solutionCount;
   bool solving,
        foundSolution;
   double percentSolved;
   pos nextEmptyPos;
   stack<block *> S; // blocks in puzzle grid
   queue<block *> Q; // blocks waiting to be added to puzzle grid
   // PROBLEM: for Q & S, should be able to set max size using constructor
   char textBuffer[30]; // holds most recent text message drawn
};

#endif