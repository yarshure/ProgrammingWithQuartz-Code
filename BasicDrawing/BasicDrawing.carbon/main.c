//
//  main.c
//  BasicDrawing
//
//  Created by David Gelphman on 9/9/04.
//  Copyright __MyCompanyName__ 2004. All rights reserved.
//

#include <Carbon/Carbon.h>

#define kMyHIViewSignature 'blDG'
#define kMyHIViewFieldID    128

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    WindowRef 		window;
    
    OSStatus		err;
    ControlID		myControlID = {kMyHIViewSignature,  kMyHIViewFieldID };      
    EventTypeSpec 	myHIViewSpec = {kEventClassControl, kEventControlDraw };	
    EventTargetRef	myEventTarget;
    ControlRef		myHIControl;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    // Install HIView event handler
    GetControlByID (window, &myControlID, &myHIControl);								
    myEventTarget = GetControlEventTarget (myHIControl);								
    err = InstallEventHandler (myEventTarget, 
					NewEventHandlerUPP (MyDrawEventHandler), 
					1, 
					&myHIViewSpec, 
					(void *) myHIControl, 
					NULL); 

    
    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

