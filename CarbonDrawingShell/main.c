/*
*  File:    main.c
*  
*  Copyright:  Copyright © 2005 Apple Computer, Inc., All Rights Reserved
* 
*  Disclaimer:  IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*        consideration of your agreement to the following terms, and your use, installation, modification 
*        or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*        not agree with these terms, please do not use, install, modify or redistribute this Apple 
*        software.
*
*        In consideration of your agreement to abide by the following terms, and subject to these terms, 
*        Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*        original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*        Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*        redistribute the Apple Software in its entirety and without modifications, you must retain this 
*        notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*        Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*        endorse or promote products derived from the Apple Software without specific prior written 
*        permission from Apple.  Except as expressly stated in this notice, no other rights or 
*        licenses, express or implied, are granted by Apple herein, including but not limited to any 
*        patent rights that may be infringed by your derivative works or by other works in which the 
*        Apple Software may be incorporated.
*
*        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*        IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*        AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*        OR IN COMBINATION WITH YOUR PRODUCTS.
*
*        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*        OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*        REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*        UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*        IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <Carbon/Carbon.h>

#include <math.h>
void doAlphaRects(CGContextRef context)
{
	// ***** Part 1 *****
	CGRect ourRect = {{0.0, 0.0}, {130.0, 100.0}};
	int i, numRects = 6;
	float rotateAngle = 2*M_PI/numRects;
	float tint, tintAdjust = 1.0/numRects;
	
	// ***** Part 2 *****
	CGContextTranslateCTM (context, 2*ourRect.size.width, 
		 			2*ourRect.size.height);
	
	// ***** Part 3 *****
	for(i = 0, tint = 1.0; i < numRects ; i++, tint -= tintAdjust){
		CGContextSetRGBFillColor (context, tint, 0.0, 0.0, tint);
		CGContextFillRect(context, ourRect);
		CGContextRotateCTM(context, rotateAngle);   // cumulative
	} 
}

OSStatus MyDrawEventHandler (EventHandlerCallRef myHandlerRef, 
					EventRef event, void *userData)
{
	OSStatus status = noErr;
	CGContextRef myContext;
	
	// Get the CGContextRef.
	status = GetEventParameter (event, 
							kEventParamCGContextRef, 
							typeCGContextRef, 
							NULL, 
							sizeof (CGContextRef),
							NULL,
							&myContext);												
	if(status != noErr){
		fprintf(stderr, "Got error %d getting the context!\n", status);
		return status;
	}

// Set to 1 to transform from the HIView default orientation to
// the CG default orientation.
#if 0		
	HIRect			bounds;
	// Get the bounding rectangle
	HIViewGetBounds ((HIViewRef) userData, &bounds);
	
	// Flip the coordinates by translating and scaling. This produces a
	// coordinate system that matches the Quartz default coordinate system
	// with the origin in the lower-left corner with the y axis pointing up.
	CGContextTranslateCTM(myContext, 0, bounds.size.height);
	CGContextScaleCTM(myContext, 1.0, -1.0);
#endif
	
	// Your drawing code can replace what follows.

	// Draw the coordinate axes.
	CGContextSetLineWidth(myContext, 5.0);
	CGContextBeginPath(myContext);
	// First draw the x axis.
	CGContextMoveToPoint(myContext, -2000.0, 0.0);
	CGContextAddLineToPoint(myContext, 2000.0, 0.0);
	CGContextDrawPath(myContext, kCGPathStroke); 
	// Next draw the y axis.
	CGContextMoveToPoint(myContext, 0.0, -2000.0);
	CGContextAddLineToPoint(myContext, 0.0, 2000.0);
	CGContextDrawPath(myContext, kCGPathStroke); 
	// Now draw the alpha rects.
	doAlphaRects (myContext);											
	return status;   
}

// Declare the signature and field ID for the HIView. These must
// match what you assigned to the HIView in Interface Builder.
#define kMyHIViewSignature  'vFpd'
#define kMyHIViewFieldID     135

int main (int argc, char* argv[])
{
 	IBNibRef 							nibRef;    
	OSStatus							err;
	WindowRef							window;
	HIViewRef							myView;
	// Declare the event class and kind for the Carbon event of interest.
	static const EventTypeSpec	kMyEvents[] = { kEventClassControl, kEventControlDraw };					
	static const HIViewID		myViewID = { kMyHIViewSignature, kMyHIViewFieldID };

	// Create a reference for the main nib.
	err = CreateNibReference (CFSTR("main"), &nibRef);
	require_noerr (err, CantGetNibRef);
    
	// Set the menu bar to the be the MenuBar from the main nib.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr (err, CantSetMenuBar);
    
	// Create the MainWindow from the main nib.
	err = CreateWindowFromNib (nibRef, CFSTR("MainWindow"), &window);
	require_noerr (err, CantCreateWindow ); 
	DisposeNibReference(nibRef);
	
	// Get the HIView of the requested ID associated with the window.
	HIViewFindByID( HIViewGetRoot( window ), myViewID, &myView );
	
	// Install the event handler for the HIView.															
	err = HIViewInstallEventHandler (myView, 
							NewEventHandlerUPP (MyDrawEventHandler), 
							GetEventTypeCount(kMyEvents), 
							kMyEvents, 
							(void *) myView, 
							NULL); 
	ShowWindow (window);    
	RunApplicationEventLoop();  
   
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:

	return err;
}
