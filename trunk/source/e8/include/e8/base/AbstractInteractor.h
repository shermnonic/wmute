#ifndef ABSTRACTINTERACTOR_H
#define ABSTRACTINTERACTOR_H

class AbstractInteractor
{
public:
	enum MouseButtons // Chosen to be bit compatible with Qt4
	{
		NoButton    =0x0,
		LeftButton  =0x1,
		RightButton =0x2,
		MiddleButton=0x4,
		XButton1    =0x8,
		XButton2    =0x10
	};
	
	enum ModifierKeys
	{
		Shift   = 0x1,
		Control = 0x2,
		Alt     = 0x4,
		Super   = 0x8
	};

	struct MouseState
	{	
		int x, y;    ///< Window x,y position of mouse cursor
		int buttons; ///< Bitmask indicating pressed buttons, see MouseButtons
		int modifiers; ///< Special keys simultaneously pressed, see ModifierKeys
	};
	
	struct ScrollState
	{
		double xdelta, ydelta; ///< Distance of wheel rotation vertically / horizontally
	};
	
	struct KeyEvent
	{
		// TODO: Define special and function keys
		int scancode;
		int modifiers;
	};
	
	// Joystick support ?
	
	void onMousePress( const MouseState& );
	void onMouseMove ( const MouseState& );
	
	void onScroll( const ScrollState& );	

	void onKeyPress  ( const KeyEvent& );
	void onKeyRelease( const KeyEvent& );
};

#endif // ABSTRACTINTERACTOR_H
