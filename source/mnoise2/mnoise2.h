////////////////////////////////////////////////////////////////////////////////
// MNOISE2 with improved gui - Config (Wintermute version)
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MNOISE2_H
#define MNOISE2_H

// Written images CLI w/o GLUI
// #define WRITTEN_IMAGES_COMMAND_LINE_PROGRAM /* Set by CMake configuration. */
// Output PS instead of PNG
//#define WRITTEN_IMAGES_OUTPUT_POSTSCRIPT

// Automatically adjust linewidth according to image width
//#define SCREENSHOT_AUTO_ADJUST_LINEWIDTH

// Put GLUI user controls into separate windows
#define UI_SEPARATE_WINDOWS

// Override GLUI user interface and use plain GLUT (for debugging)
//#define USE_PLAIN_GLUT_INSTEAD_OF_GLUI       /* Set by CMake configuration. */

// Support for quality screenshot by rendering to highres framebuffer object
#define SUPPORT_OFFSCREEN_RENDERING

// Image resolution for off screen rendering
// 8096 x 8096
// 8096 x 5060
// 4080 x 2720
// 4096 x 4096
#define OFFSCREEN_WIDTH  8096
#define OFFSCREEN_HEIGHT 5060

// Debug octree code
//#define DEBUG_CUBE

// Enable WiiMote support
// #define USE_WIIMOTE                         /* Set by CMake configuration. */
// Use latest wiiuse library version 0.12 statically linked
#define ALT_WIIUSE

#endif
