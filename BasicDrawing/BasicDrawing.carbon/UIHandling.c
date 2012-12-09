/*
*  File:    UIHandling.c
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
#include "UIHandling.h"
#include "NavServicesHandling.h"
#include "AppDrawing.h"
#include "DoPrinting.h"

/* Constants */
#define kMyHIViewSignature 'blDG'
#define kMyHIViewFieldID    128


/* Private Prototypes */
static OSStatus MyDrawEventHandler(EventHandlerCallRef myHandler, EventRef event, void *userData);
pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
static void DoAboutBox();
static void SetupMenus(void);


/* Global Data */
static OSType gCurrentCommand = kHICommandSimpleRect;
static WindowRef gWindowRef = NULL;
static HIViewRef gMyHIView = NULL;
static PMPageFormat gPageFormat = NULL;
static Boolean gUseQTForExport = false;
static int  gDPI = 144;

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    
    OSStatus		err;
    static const HIViewID		kMyViewID = { kMyHIViewSignature,  kMyHIViewFieldID };      
    static const EventTypeSpec 	kMyViewEvents[] = { kEventClassControl, kEventControlDraw };	
    static const EventTypeSpec 	kMyCommandEvents[] = { kEventClassCommand, kEventCommandProcess };	

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
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gWindowRef);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    // Get the HIView associated with the window.
    HIViewFindByID( HIViewGetRoot( gWindowRef ), kMyViewID, &gMyHIView );
			     
    // Install the event handler for the HIView.				
    err = HIViewInstallEventHandler(gMyHIView, 
						    NewEventHandlerUPP (MyDrawEventHandler), 
						    GetEventTypeCount(kMyViewEvents), 
						    kMyViewEvents, 
						    (void *) gMyHIView, 
						    NULL); 


    // Install the handler for the menu commands.
    InstallApplicationEventHandler(NewEventHandlerUPP(DoAppCommandProcess), GetEventTypeCount(kMyCommandEvents), 
						kMyCommandEvents, NULL, NULL);

    // Initialize the current drawing command menu item
    SetMenuCommandMark(NULL, gCurrentCommand, checkMark);
    
    SetupMenus();
    
    // The window was created hidden so show it.
    ShowWindow( gWindowRef );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

static void UpdateExportImagesMethodMenu(void)
{
    if(gUseQTForExport){
		SetMenuCommandMark(NULL, kHICommandExportImagesWithCG, noMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWithQT, checkMark);
    }else{
		SetMenuCommandMark(NULL, kHICommandExportImagesWithQT, noMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWithCG, checkMark);
    }
}

static void UpdateExportImagesResolutionMenu(void)
{
    if(gDPI == 300){
		SetMenuCommandMark(NULL, kHICommandExportImagesWith72dpi, noMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWith144dpi, noMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWith300dpi, checkMark);
    }else if(gDPI == 72){
		SetMenuCommandMark(NULL, kHICommandExportImagesWith72dpi, checkMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWith144dpi, noMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWith300dpi, noMark);
    }else{ // gDPI == 144
		SetMenuCommandMark(NULL, kHICommandExportImagesWith72dpi, noMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWith144dpi, checkMark);
		SetMenuCommandMark(NULL, kHICommandExportImagesWith300dpi, noMark);
    }
}

static void SetupMenus(void)
{
    // If the running Mac OS version doesn't support CGImageDestination
    // export capability then disable the menu. Better than
    // this approach would be to be more selective since
    // this app can use QT to export images also.
    if(&CGImageDestinationCreateWithURL == NULL){
		DisableMenuCommand(NULL, kHICommandExportTIFF);
		DisableMenuCommand(NULL, kHICommandExportJPEG);
		DisableMenuCommand(NULL, kHICommandExportPNG);
    }
    
    // Disable the layer drawing choice if it isn't available.
    if(&CGLayerCreateWithContext == NULL)
		DisableMenuCommand(NULL, kHICommandDrawWithLayer);
    
    UpdateExportImagesMethodMenu();
    UpdateExportImagesResolutionMenu();
}


static OSStatus MyDrawEventHandler(EventHandlerCallRef myHandler, EventRef event, void *userData)
{
	OSStatus status = noErr;
	CGContextRef context;
	HIRect		bounds;

	// Get the CGContextRef.
	status = GetEventParameter (event, kEventParamCGContextRef, 
					typeCGContextRef, NULL, 
					sizeof (CGContextRef),
					NULL,
					&context);

	if(status != noErr){
		fprintf(stderr, "Got error %d getting the context!\n", status);
		return status;
	}		
						
	// Get the bounding rectangle.
	HIViewGetBounds ((HIViewRef) userData, &bounds);
	
	// Flip the coordinates by translating and scaling. This produces a
	// coordinate system that matches the Quartz default coordinate system
	// with the origin in the lower-left corner with the y axis pointing up.
	CGContextTranslateCTM(context, 0, bounds.size.height);
	CGContextScaleCTM(context, 1.0, -1.0);
	
	DispatchDrawing(context, gCurrentCommand);
					
	return status;
   
}

static OSType printableCommandFromCommand(OSType command)
{
    // Don't use pre-rendered drawing when printing.
	if(command == kHICommandDrawOffScreenImage || command == kHICommandDrawWithLayer)
		return kHICommandDrawNoOffScreenImage;
    
    return command;
}

pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)
    HICommand  aCommand;
    OSStatus   result = eventNotHandledErr;

    GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
    
	switch (aCommand.commandID){
		case kHICommandSimpleRect:
		case kHICommandStrokedRect:
		case kHICommandStrokedAndFilledRect:
		case kHICommandPathRects:
		case kHICommandAlphaRects:
		case kHICommandDashed:
		case kHICommandSimpleClip:
		case kHICommandPDFDoc:
		
		case kHICommandRotatedEllipses:
		case kHICommandDrawSkewCoordinates:

		case kHICommandBezierEgg:
		case kHICommandRoundedRects:
		case kHICommandStrokeWithCTM:
		case kHICommandRotatedEllipsesWithCGPath:
		case kHICommandPixelAligned:
		
		case kHICommandDeviceFillAndStrokeColor:
		case kHICommandDrawWithColorRefs:
		case kHICommandCLUTDrawGraphics:
		case kHICommandDrawWithGlobalAlpha:
		case kHICommandDrawWithBlendMode:
		case kHICommandFunctionsHaveOwnGSave:

		case kHICommandDrawJPEGImage:
		case kHICommandColorImageFromFile:
		case kHICommandColorImageFromData:
		case kHICommandColorImageFromCallbacks:
		case kHICommandGrayRamp:
		case kHICommandDrawWithCGImageSource:
		case kHICommandDrawWithCGImageSourceIncremental:
		case kHICommandDrawWithQuickTime:
		case kHICommandSubstituteImageProfile:
		case kHICommandDoSubImage:
		case kHICommandExportWithQuickTime:

		case kHICommandMaskTurkeyImage:
		case kHICommandImageMaskedWithMask:
		case kHICommandImageMaskedWithGrayImage:
		case kHICommandMaskImageWithColor:
		case kHICommandClipToMask:
		case kHICommandExportWithCGImageDestination:
		
		case kHICommandSimpleCGLayer:
		case kHICommandAlphaOnlyContext:
		case kHICommandDrawNoOffScreenImage:
		case kHICommandDrawOffScreenImage:
		case kHICommandDrawWithLayer:

		case kHICommandQuartzRomanText:
		case kHICommandQuartzTextModes:
		case kHICommandQuartzTextMatrix:

		case kHICommandThemeText:
		case kHICommandATSUText:
		
		case kHICommandSimplePattern:
		case kHICommandPatternPhase:
		case kHICommandPatternMatrix:
		case kHICommandUncoloredPattern:
		case kHICommandDrawWithPDFPattern:
		
		case kHICommandSimpleShadow:
		case kHICommandShadowScaling:
		case kHICommandShadowProblems:
		case kHICommandComplexShadow:
		
		case kHICommandMultipleShapeComposite:
		case kHICommandFillAndStrokeWithShadow:
		case kHICommandPDFDocumentShadow:
		
		case kHICommandSimpleAxialShading:
		case kHICommandExampleAxialShadings:
		case kHICommandSimpleRadialShading:
		case kHICommandExampleRadialShadings:
		case kHICommandEllipseShading:

		case kHICommandDoCompatibleEPS:
		
			SetMenuCommandMark(NULL, gCurrentCommand, noMark);
			gCurrentCommand = aCommand.commandID;
			SetMenuCommandMark(NULL, gCurrentCommand, checkMark);
			if(gMyHIView){
				HIViewSetNeedsDisplay(gMyHIView, true);
			}
			result = noErr;
			break;

		case kHICommandPageSetup:
			if(gPageFormat == NULL)
				gPageFormat = CreateDefaultPageFormat();
			
			if(gPageFormat)
				(void)DoPageSetup(gPageFormat);
			
			result = noErr;
			break;

		case kHICommandPrint:
			if(gPageFormat == NULL)
				gPageFormat = CreateDefaultPageFormat();
			
			if(gPageFormat)
				(void)DoPrint(gPageFormat, printableCommandFromCommand(gCurrentCommand));

			result = noErr;
			break;

		case kHICommandAbout:
			DoAboutBox();
			result = noErr; 
			break;

		// Treat copy and cut identically.
		case kHICommandCut:
		case kHICommandCopy:
			addPDFDataToPasteBoard(gCurrentCommand);
			break;

		case kHICommandExportPDF:
			if(gWindowRef) // gUseQTForExport and dpi are ignored for PDF export.
				(void)DoExport(gWindowRef, printableCommandFromCommand(gCurrentCommand), 
								exportTypePDF, gUseQTForExport, 0);
			break;

		case kHICommandExportTIFF:
			if(gWindowRef)
				(void)DoExport(gWindowRef, printableCommandFromCommand(gCurrentCommand), 
								exportTypeTIFF, gUseQTForExport, gDPI);
			break;

		case kHICommandExportPNG:
			if(gWindowRef)
				(void)DoExport(gWindowRef, printableCommandFromCommand(gCurrentCommand),
							exportTypePNG, gUseQTForExport, gDPI);
			break;

		case kHICommandExportJPEG:
			if(gWindowRef)
				(void)DoExport(gWindowRef, printableCommandFromCommand(gCurrentCommand),
								exportTypeJPEG, gUseQTForExport, gDPI);
			break;

		case kHICommandExportImagesWithCG:
			gUseQTForExport = false;
			UpdateExportImagesMethodMenu();
			break;

		case kHICommandExportImagesWithQT:
			gUseQTForExport = true;
			UpdateExportImagesMethodMenu();
			break;

		case kHICommandExportImagesWith72dpi:
			gDPI = 72;
			UpdateExportImagesResolutionMenu();
			break;

		case kHICommandExportImagesWith144dpi:
			gDPI = 144;
			UpdateExportImagesResolutionMenu();
			break;

		case kHICommandExportImagesWith300dpi:
			gDPI = 300;
			UpdateExportImagesResolutionMenu();
			break;

		case kHICommandPaste:
			// Uncheck the existing menu item.
			SetMenuCommandMark(NULL, gCurrentCommand, noMark);
			// Obtain new PDF document from pasteboard.
			(void)getPasteBoardPDFDoc(true);    
			gCurrentCommand = aCommand.commandID;
			if(gMyHIView){
				HIViewSetNeedsDisplay(gMyHIView, true);
			}
			break;

		case kHICommandQuit:
			QuitApplicationEventLoop();
			result = noErr;
			break;

		default:
			break;
	}
    HiliteMenu(0);
    return result;
}

static void DoStandardAlert(CFStringRef alertTitle, CFStringRef alertText)
{
	AlertStdCFStringAlertParamRec	param;
	DialogRef			dialog;
	OSStatus			err;
	DialogItemIndex			itemHit;
	
	GetStandardAlertDefaultParams( &param, kStdCFStringAlertVersionOne );
	
	param.movable = true;
	
	err = CreateStandardAlert( kAlertNoteAlert, alertText, NULL, &param, &dialog );
	if(err){
	    fprintf(stderr, "Can't create alert!\n");
		return;
	}

	if(alertTitle)
	    SetWindowTitleWithCFString( GetDialogWindow( dialog ), alertTitle);

	RunStandardAlert( dialog, NULL, &itemHit );
	
	return;
}


static void DoAboutBox()
{	
    CFStringRef alertMessage = CFCopyLocalizedString(kAboutBoxStringKey, NULL);
    CFStringRef alertTitle = CFCopyLocalizedString(kAboutBoxTitleKey, NULL);

    if (alertMessage != NULL && alertTitle != NULL)
    {
		DoStandardAlert(alertTitle, alertMessage);
    }
    if(alertMessage)
		CFRelease(alertMessage);
    if(alertTitle)
		CFRelease(alertTitle);
}

void DoErrorAlert(OSStatus status, CFStringRef errorFormatString)
{	
    if ((status != noErr) && (status != kPMCancel))           
    {
		CFStringRef formatStr = NULL;
		CFStringRef printErrorMsg = NULL;

        formatStr =  CFCopyLocalizedString(errorFormatString, NULL);	
		if (formatStr != NULL){
            printErrorMsg = CFStringCreateWithFormat(        
				NULL, NULL, 
				formatStr, status);
            if (printErrorMsg != NULL)
            {
				DoStandardAlert(NULL, printErrorMsg);
                CFRelease (printErrorMsg);                     
			}
			CFRelease (formatStr);                             
        }
    }
}
