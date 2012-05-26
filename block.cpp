/*************************************************************************************************\
*                                                                                                 *
* "block.cpp" - Member functions of class "block" (defined in "block.h").                         *
*                                                                                                 *
*     Author  - Tom McDonnell                                                                     *
*                                                                                                 *
\*************************************************************************************************/

#include "block.h"

int block::blockCount = 0;

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

/*
 * Constructor.
 */
block::block(void) {
   colour        = RGB(255, 255, 255); // default colour white
   origHoldPos.r = -1;
   origHoldPos.c = -1;
   TLcol         = -1;
   ++blockCount;
}

/*
 * Destructor.
 */
block::~block(void) {
   --blockCount;
}

/*
 * Rotate block 90 degrees clockwise.
 */
void block::rotate(void) {
   bool tempGrid[MAX_BLOCK_SIZE][MAX_BLOCK_SIZE];
   
   // copy grid to tempGrid
   int r, c;
   for (r = 0; r < height; ++r)
     for (c = 0; c < width; ++c)
       tempGrid[r][c] = grid[r][c];

   // exchange rows with reversed columns
   for (r = 0; r < height; ++r)
     for (c = 0; c < width; ++c)
       grid[c][height - 1 - r] = tempGrid[r][c];

   // update holdPos
   int temp  = holdPos.r;
   holdPos.r = holdPos.c;
   holdPos.c = height - 1 - temp;

   // swap height and width
   temp   = height;
   height = width;
   width  = temp;

   // update orientation
   if (orientation < 4)
     orientation = (orientation + 1) % 4;
   else 
     orientation = (orientation - 3) % 4 + 4;

   findTLcol(); // update blocks top-leftmost square position.
}

/*
 * Flip block vertically.
 */
void block::flip(void) {
   bool temp;
   int rFinish = height / 2,
       r, c;

   // flip grid vertically
   for (r = 0; r < rFinish; ++r)
     for (c = 0; c < width; ++c) {
        temp = grid[height - 1 - r][c];
	     grid[height - 1 - r][c] = grid[r][c];
	     grid[r][c] = temp;
     }

   // update orientation
   if (!(orientation % 2)) // 0, 2, 4, 6
      orientation = (orientation + 4) % 8;
   else if (orientation == 1 || orientation == 5) // 1, 5
      orientation = (orientation + 6) % 8;
   else // 3, 7
      orientation = (orientation + 2) % 8;

   findTLcol(); // update blocks top-leftmost square position.   

   holdPos.r = height - 1 - holdPos.r; // update holdPos
}

/*
 * Change block orientation to "newOrientation" by rotating / flipping.
 */
void block::changeOrientation(const int newOrientation) {
   assert(newOrientation >= 0 && newOrientation < 8);

   while (orientation != newOrientation)
     if (abs(newOrientation - orientation) == 4)
       flip();
     else
       rotate();
}

/*
 * Prints block information to screen in text format.
 */
void block::print(void) {
   // textcolor(colour); PROBLEM: need to change text colour then use (char)219
   int r, c;
   for (r = 0; r < height; ++r) {
      for (c = 0; c < width; ++c) {
         if (grid[r][c])
           cout << (char)219 << (char)219;
         else
           cout << "  ";
      }
      cout << endl;
   }
}

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////

/*
 * For each possible orientation (0-7), tests whether block orientated
 * this way is identical to same block with other orientation.  If the
 * orientation tested is found to be unique, the relevant position in
 * 'uniqueOrientations[8]' is set to 'true' else 'false'.
 */
void block::findUniqueOrientations(void) {
   bool testGrid[8][MAX_BLOCK_SIZE][MAX_BLOCK_SIZE];
   int testGridH[8], testGridW[8], // height and width of block in testGrid
       i, j, // counters
       r, c;

   // initialize 'testGrid[8][size]' & 'uniqueOrient[8]'
   for (i = 0; i < 8; ++i) {
      uniqueOrient[i] = true;
      changeOrientation(i);

      // copy 'grid' to 'testGrid'
      for (r = 0; r < height; ++r)
        for (c = 0; c < width; ++c)
          testGrid[i][r][c] = grid[r][c];

      testGridH[i] = height;
      testGridW[i] = width;
   }

   // eliminate orientations that are not unique
   for (i = 0; i < 7; ++i) {
      if (uniqueOrient[i])
        for (j = i + 1; j < 8; ++j) {
           changeOrientation(i);
           if (gridEqual(testGridH[j], testGridW[j], testGrid[j]))
             uniqueOrient[j] = false;
        }
   }

   // reset block orientation
   changeOrientation(0);
}

/*
 * Test whether block stored in 'g' is equivalent to that in 'grid'
 * return true if equivalent, false otherwise.
 */
bool block::gridEqual(const int h, const int w, const bool g[][MAX_BLOCK_SIZE]) {
   if (h != height || w != width)
     return false;
   else {
      int r, c;
      for (r = 0; r < height; ++r)
        for (c = 0; c < width; ++c)
          if (g[r][c] != grid[r][c])
            return false;
   }
   return true;
}

/*
 * Return the position of the first square encountered when moving through
 * the grid from left->right & top->bottom (ie. top left).
 */
inline void block::findTLcol(void) {
   for (TLcol = 0; TLcol < width; ++TLcol)
     if (grid[0][TLcol])
	    break;
}

// FRIEND FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

ostream &operator<<(ostream &output, block *bPtr) {
   output << bPtr->getColour() << " " << bPtr->getOrientation();
   return output;
}

istream &operator>>(istream &input, block &b) {
   // input colour
   int red, green, blue;
   input >> red >> green >> blue;
   b.colour = RGB(red, green, blue);
     
   input.eatwhite();

   // input grid, height, width
   b.height = b.width = 0;
   int r = 0, c = 0;
   char ch;
   bool finished = false, newLine = false;
   while (!finished) {
      ch = (char)input.get();
      switch (ch) {
       case '0':
         b.grid[r][c] = false;
         newLine = false;
         ++c;
         break;
       case '1':
         b.grid[r][c] = true;
         newLine = false;
         ++c;
         break;
       case '2':
         // set origHoldPos
         b.origHoldPos.r = r;
         b.origHoldPos.c = c;
         b.grid[r][c] = true;
         newLine = false;
         ++c;
         break;
       case '\n':
         if (newLine) // test for two consecutive newLines
           finished = true;
         else {
            newLine = true;
            b.width = c;
            c = 0;
            ++r;
         } break;
       case EOF:
         finished = true;
         ++r;
         break;
       default:
         cerr << "Block read error (1).";
      }
   }
   b.height = r;
   b.findTLcol();

   // error check;
   if (b.TLcol == -1 || b.origHoldPos.r == -1 || b.origHoldPos.c == -1)
     cerr << "Block read error (2).";

   b.findUniqueOrientations();

   return input;
}