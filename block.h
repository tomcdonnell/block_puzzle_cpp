/*************************************************************************************************\
*                                                                                                 *
* "block.h" - Class "block" definition.                                                           *
*                                                                                                 *
*   Author  - Tom McDonnell                                                                       *
*                                                                                                 *
\*************************************************************************************************/

#ifndef BLOCK_H
#define BLOCK_H

#include <iostream.h>
#include <windows.h>
#include <assert.h>

#include "misc.h"

#define MAX_BLOCK_SIZE 6

class block {
   friend ostream &operator<<(ostream &, block *);
   friend istream &operator>>(istream &, block &);

 public:
   block(void);
   ~block(void);

   void setColour(const COLORREF c) {colour = c;           }
   void setPuzPos(pos p)            {puzPos = p;           }
   void setOrigHoldPos(pos p)       {origHoldPos = p;      }
   void setHoldPos(pos p)           {holdPos = p;          }
   void resetHoldPos(void)          {holdPos = origHoldPos;}
   
   int      getHeight(void)       {return height;     }
   int      getWidth(void)        {return width;      }
   int      getOrientation(void)  {return orientation;}
   pos      getPuzPos(void)       {return puzPos;     }
   pos      getHoldPos(void)      {return holdPos;    }
   int      getTLcol(void)        {return TLcol;      }
   bool     getGrid(int r, int c) {return grid[r][c]; }
   COLORREF getColour(void)       {return colour;     }

   /*
    * Return total number of block objects instantiated.
    */
   int getBlockCount(void) {return blockCount;}

   /*
    * Rotate block 90 degrees clockwise.
    */
   void rotate(void);

   /*
    * Flip block vertically.
    */
   void flip(void);
   
   /*
    * Change block orientation.
    * (orientation is an int (0-7) descibing state of block.
    *  0 = initial state
    *  0, 1, 2, 3 = rotated (0, 1, 2, or 3) *90 degrees clockwise
    *  4, 5, 6, 7 = flipped verically then rotated (0, 1, 2, 3) *90 degrees clockwise)
    */
   void changeOrientation(int newOrientation);
    
   /*
    * Print block info (grid, unique orientations, current 
    * orientation, colour) to screen as text.
    */
   void print(void);
  
   /*
    * Test whether orientation of block is unique.
    * (a unique orientation is one that has no identical orientations
    *  less than itself.
    *  eg. A square block's only unique orientation is 0 as for each
    *      other orientation (1-7) there is an orientation that is
    *      identical and lesser in value (ie. 0).
    *      A rectangular block has two unique orientations, 0 and 1.
    *      A block with no symmetry has 8 unique orientations (0-7).)
    */
   bool uniqueOrientation(int o) {return uniqueOrient[o];}
   
 private:
   void findUniqueOrientations(void);
   bool gridEqual(const int, const int, const bool [][MAX_BLOCK_SIZE]);
   void findTLcol(void);
   
   int  height, width, orientation,
        TLcol; // column of blocks TL square (row is always 0)
   bool grid[MAX_BLOCK_SIZE][MAX_BLOCK_SIZE],
        uniqueOrient[8];
   pos  puzPos,      // position in puzzle of blocks TL square
        origHoldPos, // position of block mouse will hold when block is picked up from queue
        holdPos;     // position of block held by mouse pointer
   COLORREF colour;
   static int blockCount; // total no. of blocks instantiated
};

#endif