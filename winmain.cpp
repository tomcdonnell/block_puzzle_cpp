/*************************************************************************************************\
*                                                                                                 *
* "winmain.cpp" - Main function of windows application "blockpuzzle.exe".                         *
*                                                                                                 *
*       Author  - Tom McDonnell                                                                   *
*                                                                                                 *
\*************************************************************************************************/

#include <windows.h>

#include "puzzle.h"

#define WIN32_LEAN_AND_MEAN
#define WINDOW_CLASS_NAME "WINCLASS1"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

OPENFILENAME openBox; // common dialog box structure

HWND main_window_handle = NULL;
puzzle puz;

int WINAPI WinMain(HINSTANCE hinstance,
                   HINSTANCE hprevinstance,
                   LPSTR     lpcmdline,
                   int       ncmdshow)
{
   WNDCLASS winclass;
   MSG      msg;      // generic message

   // first fill in the window class stucture
   winclass.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
   winclass.lpfnWndProc    = WindowProc;
   winclass.cbClsExtra     = 0;
   winclass.cbWndExtra     = 0;
   winclass.hInstance      = hinstance;
   winclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION); // PROBLEM: should have own icon
   winclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
   winclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
   winclass.lpszMenuName   = "BlockPuzzleMenu";
   winclass.lpszClassName  = WINDOW_CLASS_NAME;

   // register the window class
   if (!RegisterClass(&winclass))
     return(0);

   // create the window
   main_window_handle 
     = CreateWindow(WINDOW_CLASS_NAME,         // class
                    "Block Puzzle",            // title
                    WS_SYSMENU | WS_CAPTION | WS_VISIBLE,
                    CW_USEDEFAULT,             // x pos
                    CW_USEDEFAULT,             // y pos
                    SQUARE_SIZE * 8 + 6,       // width (+6 allows for border)
                    SQUARE_SIZE * 8 + 15 + 49, // height (+15 text area +49 title & menu)
                    NULL,                      // handle to parent 
                    NULL,                      // handle to menu (menu attached when class created)
                    hinstance,                 // instance
                    NULL);                     // creation parms
   if (main_window_handle == NULL)
     return(0);

   // create open dialog box window structure
   char szFile[260]; // buffer for filename

   // Initialize OPENFILENAME
   ZeroMemory(&openBox, sizeof(OPENFILENAME));
   openBox.lStructSize = sizeof(OPENFILENAME);
   openBox.hwndOwner = main_window_handle;
   openBox.lpstrFile = szFile;
   openBox.nMaxFile = sizeof(szFile);
   openBox.lpstrFilter = "Block Set\0*.BLK\0All\0*.*\0";
   openBox.nFilterIndex = 1;
   openBox.lpstrFileTitle = NULL;
   openBox.nMaxFileTitle = 0;
   openBox.lpstrInitialDir = NULL;
   openBox.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   // read default block set
   if (!puz.readBlockSet("Default Block Set.blk")) {
      MessageBox(main_window_handle,
                 "File \"Default Block Set.blk\" not found.",
                 "Block Puzzle", MB_OK);
      // kill the application (should maybe send WM_DESTROY message somehow)
      PostQuitMessage(0);
      return (0);
   }

   // enter main event loop (program is completely event driven)
   while (true) {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
         if (msg.message == WM_QUIT)
           break;
         TranslateMessage(&msg); // translate any accelerator keys
         DispatchMessage(&msg); // send the message to the window proc
      }
   }
   return (msg.wParam); // return to Windows
}