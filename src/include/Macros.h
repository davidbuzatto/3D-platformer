#pragma once

#define trace( ... ) TraceLog( LOG_INFO, __VA_ARGS__ );

// Toggle for the raygui-based editable property panel (position/rotation/
// scale value boxes). Lives here, not in a single .c file, because more
// than one translation unit (GameWorld.c, MapPieceEditor.c) needs to agree
// on whether it's defined.
//#define USE_GUI