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
#include "AppDrawing.h"
#include "NavServicesHandling.h"
#include "UIHandling.h"
#include "PSToPDF.h"

static OSStatus Initialize();

static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);

static pascal OSStatus myWinEvtHndlr(EventHandlerCallRef myHandler, 
                                            EventRef event, void* userData);

static pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, 
                                            EventRef theEvent, void* userData);

static OSStatus InstallAppEvents(void);

static void HandleWindowUpdate(WindowRef window);

static void DoAboutBox(void);
static void WarnAboutEncryptedPDFDocument(void);
static void WarnAboutNoPrintingOrCopying(bool noCopy, bool noPrint);
static void WarnAboutNoNewPDFDocument(void);

static WindowRef MakeWindow(void);
static void EnableDisableMenus(Boolean enable);
static MenuCommand getCommandForBox(CGPDFBox boxType);
static OSStatus DoTheOpenCommand(void);


/* Global Data */
static IBNibRef gNibRef;
static WindowRef gWindowRef;
static CGPDFDocumentRef gThePDFDocument;
static int gTotalPages;
static int gCurrentPage;
static CGPDFBox gBoxType;
static APIVersion gAPISet;
static int gScaleFactor;
static int gExtraPageRotation;

int main(int argc, char* argv[])
{
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &gNibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(gNibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );

    err = Initialize();
    require_noerr( err, CantInitialize );

    // open the window
    gWindowRef = MakeWindow();
    if(gWindowRef){
	Rect bounds;
	SetPortWindowPort(gWindowRef);
	GetWindowPortBounds(gWindowRef, &bounds);
	ClipRect(&bounds);
	InvalWindowRect(gWindowRef, &bounds); 
	DoTheOpenCommand();
	//ShowWindow(gWindowRef);
    }else{
	err = kCantCreateWindow;
    }

    require_noerr( err, CantCreateWindow );

    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
CantInitialize:
    DoErrorAlert(err, kMyCantInitializeAppKey);

    return err;
}


static void UpdateAPICommandSetMenu()
{
    MenuCommand currentSelection;
    SetMenuCommandMark(NULL, kHICommandUsePantherAPI, noMark);
    SetMenuCommandMark(NULL, kHICommandUseEmulatedPantherAPI, noMark);
    SetMenuCommandMark(NULL, kHICommandUseJaguarAPI, noMark);
    
    switch(gAPISet){
	case kPantherAPI:
	    currentSelection = kHICommandUsePantherAPI;
	    break;
	    
	default:
	case kEmulatedPantherAPI:
	    currentSelection = kHICommandUseEmulatedPantherAPI;
	    break;
    
	case kJaguarAPI:
	    currentSelection = kHICommandUseJaguarAPI;
	    break;
    }
    
    SetMenuCommandMark(NULL, currentSelection, checkMark);
}

static void UpdateScalingCommandSetMenu()
{
    MenuCommand currentSelection;
    SetMenuCommandMark(NULL, kHICommandScale100, noMark);
    SetMenuCommandMark(NULL, kHICommandScale200, noMark);
    SetMenuCommandMark(NULL, kHICommandScale50, noMark);
    
    switch(gScaleFactor){
	case 50:
	    currentSelection = kHICommandScale50;
	    break;
	    
	default:
	case 100:
	    currentSelection = kHICommandScale100;
	    break;
    
	case 200:
	    currentSelection = kHICommandScale200;
	    break;
    }
    
    SetMenuCommandMark(NULL, currentSelection, checkMark);

}

static OSStatus Initialize()
{
    OSErr err = noErr;

    InitCursor();
        
    gThePDFDocument = NULL;
    gTotalPages = 0;
    gCurrentPage = 0;
    gBoxType = kCGPDFCropBox;
    gAPISet = kEmulatedPantherAPI;
    gScaleFactor = 100;
    gExtraPageRotation = 0;
    
    EnableDisableMenus(false);

    UpdateAPICommandSetMenu();
    UpdateScalingCommandSetMenu();
    
    SetMenuCommandMark(NULL, getCommandForBox(gBoxType), checkMark);

    err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, 
                            NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);
    if (!err)
        err = InstallAppEvents();

    return err;
}

static WindowRef MakeWindow(void)
{
	WindowRef	window = NULL;	// initialize to NULL
	OSStatus err = noErr;
	EventTypeSpec	list[] = {
                                    {kEventClassWindow, kEventWindowClose },
                                    { kEventClassWindow, kEventWindowDrawContent },
                                    { kEventClassWindow, kEventWindowBoundsChanged }
                                };
	static EventHandlerUPP winEvtHandler = NULL;		// window event handler

	// if we haven't already made our window event handler UPP then do so now
	if(!winEvtHandler){
	    winEvtHandler = NewEventHandlerUPP(myWinEvtHndlr);
	    if(!winEvtHandler)
		err = memFullErr;
	}

	if(!err)
	    err = CreateWindowFromNib(gNibRef, CFSTR("MainWindow"), &window);

	if(!err){
	    EventHandlerRef	ref;
	    err = InstallWindowEventHandler(window, winEvtHandler, 
			sizeof(list)/sizeof(EventTypeSpec), list, NULL, &ref);

	    if(err){
		DisposeWindow(window);
		window = NULL;
	    }
	}
	return window;
}

static OSStatus InstallAppEvents(void)
{
	EventTypeSpec  eventType;

	EventHandlerUPP gAppCommandProcess = NewEventHandlerUPP(DoAppCommandProcess);
	eventType.eventClass = kEventClassCommand;
	eventType.eventKind = kEventCommandProcess;
	InstallApplicationEventHandler(gAppCommandProcess, 1, &eventType, NULL, NULL);
        return noErr;
}


static void CloseDocumentWindow(WindowRef window)
{
    if(gThePDFDocument){
	CGPDFDocumentRelease(gThePDFDocument);
	gThePDFDocument = NULL;
    }
    EnableDisableMenus(false);
    HideWindow(window);
}

static pascal OSStatus myWinEvtHndlr(EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler, userData)
    WindowRef			window = NULL;
    OSStatus			result = eventNotHandledErr;
    
    GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);
    switch(GetEventKind(event)){
	case kEventWindowDrawContent:
	    HandleWindowUpdate(window);
	    result = noErr;
	    break;

	case kEventWindowBoundsChanged:
	    {
		Rect bounds;
		InvalWindowRect(window, GetWindowPortBounds(window, &bounds));
	    }
	    break;
		
	case kEventWindowClose:
	    CloseDocumentWindow(window);
	    result = noErr;
	    break;
		
	default:
	    break;
    }
    
    return result;
}

typedef struct MyPDFDocumentInfo
{
    CGPDFDocumentRef pdfDoc;
    bool documentIsUnlocked;
    bool documentForbidsNewDocument;
    bool documentForbidsPrinting;
    bool documentForbidsCopying;
}MyPDFDocumentInfo;

// This code handles a document that is either not encrypted or 
// is encrypted but has an empty password.
void checkPDFDocumentPermissions(MyPDFDocumentInfo *infoP)
{
    CGPDFDocumentRef pdfDoc = infoP->pdfDoc;
    infoP->documentIsUnlocked = false;
    infoP->documentForbidsNewDocument =
    infoP->documentForbidsPrinting = 
    infoP->documentForbidsCopying = true;
    if(CGPDFDocumentIsUnlocked(pdfDoc)){
	    // Mark that the docuement is unlocked.
	    infoP->documentIsUnlocked = true;
	    // Check the permissions that are allowed.
	    infoP->documentForbidsPrinting =
			    !CGPDFDocumentAllowsPrinting(pdfDoc);
	    infoP->documentForbidsCopying = 
			    !CGPDFDocumentAllowsCopying(pdfDoc);
	    /*  If printing isn't allowed or copying isn't allowed
 	    with these permissions then this application needs to
 	    ensure that these permissions are respected.
	    */
    
	    /* If the document was encrypted, this 
	    code will mark its private structure so that 
	    later it can ensure that it wonÕt 
	    create a new document from this one, both at print 
	    time or if this application draws to a PDF context. */
	    infoP->documentForbidsNewDocument = 
			    CGPDFDocumentIsEncrypted(pdfDoc);
	}
	return;
}

void createMyPDFDocumentInfo(CGPDFDocumentRef pdfDoc, 
				    MyPDFDocumentInfo *pdfDocInfoP)
{
	pdfDocInfoP->pdfDoc = pdfDoc;
	if(pdfDocInfoP->pdfDoc){
		// Check whether the document is encrypted and obtain
		// the appropriate permissions if so. This check
		// only unlocks documents with an empty password.
		checkPDFDocumentPermissions(pdfDocInfoP);
		// If the document can't be unlocked with
		// an empty password, warn the user.
		if(!pdfDocInfoP->documentIsUnlocked){
	    		WarnAboutEncryptedPDFDocument();
	    		CGPDFDocumentRelease(pdfDocInfoP->pdfDoc);
	    		pdfDocInfoP->pdfDoc = NULL;
		}else{
	    		if( pdfDocInfoP->documentForbidsPrinting ||
				pdfDocInfoP->documentForbidsCopying
	    		){
				/* The document is unlocked but there
				are restrictions so the user should be
				warned. This warning could potentially
				come when the user tries to do one
				of the forbidden operations (as Preview does)
				but this code simplifies it for demonstration
				purposes */
				WarnAboutNoPrintingOrCopying(
			    		pdfDocInfoP->documentForbidsCopying,
			    		pdfDocInfoP->documentForbidsPrinting
				);
	    		}else{
				/* Note also that the documentForbidsNewDocument
		    		property set by checkPDFDocumentPassword must
		    		be respected at print time or new PDF document
		    		creation.
				*/
				if(pdfDocInfoP->documentForbidsNewDocument){
		    			WarnAboutNoNewPDFDocument();
				}
	    		}
		}
	}
}

static OSStatus CreatePDFDocument(FSRef fileRef)
{
    OSStatus err = noErr;
    CFURLRef url = NULL;
    Boolean isPS = false;
    url = CFURLCreateFromFSRef(NULL, &fileRef);
    // Test for EPS/PS file
    if(url){
	UInt8 filepath[PATH_MAX];
	if(CFURLGetFileSystemRepresentation(url, true, filepath, sizeof(filepath))){
	    FILE *theFile = fopen(filepath, "r");
	    if(theFile != NULL){
		// Read the first four bytes.
		char bytes[4];
		fread(bytes, 1, sizeof(bytes), theFile);
		// First two bytes are %! or the first four bytes are
		// the windows EPS header style data.
		if(0 == memcmp(bytes, "%!", 2) ||
		    strncmp(bytes, "\xC5\xD0\xD3\xC6", 4) == 0
		) isPS = true;

		fclose(theFile);
	    }
	}
    }
    if(url){
	MyPDFDocumentInfo pdfDocInfo;
	CGPDFDocumentRef pdfDoc;
	if(isPS){
	    pdfDoc = createCGPDFDocFromPSDoc(url);
	}else{
	    pdfDoc = CGPDFDocumentCreateWithURL(url);
	}
	createMyPDFDocumentInfo(pdfDoc, &pdfDocInfo);
	CFRelease(url);
	gThePDFDocument = pdfDocInfo.pdfDoc;
    }else
	err = memFullErr;
    
    if(!err && gThePDFDocument){
	gTotalPages = CGPDFDocumentGetNumberOfPages(gThePDFDocument);
	gCurrentPage = 1;
    }
    
    
    // Reset the extra page rotation back to 0
    gExtraPageRotation = 0;
    
    if(!err && gThePDFDocument){
	EnableDisableMenus(true);
    }

    return err;
}

#define MAX(a, b)       ( (a) > (b) ? (a) : (b) )
#define kXOffset 20
#define kYOffset 20

OSStatus doTheFile(FSRef theFileRef)
{
    OSStatus err = noErr;

    EnableMenuCommand(NULL, kHICommandOpen);

    if(!err)
	err = CreatePDFDocument(theFileRef);
    if(!err && gThePDFDocument){
	Rect bounds;
	CGRect rect = CGPDFDocumentGetMediaBox(gThePDFDocument, gCurrentPage);
	short size = MAX(CGRectGetWidth(rect) + 40, CGRectGetHeight(rect) + 40);
	MoveWindow(gWindowRef, kXOffset, GetMBarHeight() + kYOffset, false);
	SizeWindow(gWindowRef, size, size, false);
	InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
	ShowWindow(gWindowRef);
    }
    
    return err;
}


static void EnableDisableMenus(Boolean enable)
{
    if(enable){
	//EnableMenuCommand(NULL, kPrintCommand);	
	if(gTotalPages > 1)
	    EnableMenuCommand(NULL, kHICommandNextPage);
	if(gCurrentPage != 1)
	    EnableMenuCommand(NULL, kHICommandPreviousPage);
	else
	    DisableMenuCommand(NULL, kHICommandPreviousPage);
    }else{
	//DisableMenuCommand(NULL, kPrintCommand);	
	DisableMenuCommand(NULL, kHICommandNextPage);
	DisableMenuCommand(NULL, kHICommandPreviousPage);
    }
}


static OSStatus DoTheOpenCommand(void)
{
	OSStatus theErr = noErr;

	// check to see if we already have data. If so, we'll dispose of it since we are going to open a new file.
        if( gThePDFDocument != NULL){
            CGPDFDocumentRelease(gThePDFDocument);
            gThePDFDocument = NULL;
        }

	EnableDisableMenus(false);

	// Hide the existing windows, if they exist.
	if(gWindowRef != NULL)HideWindow(gWindowRef);

	// Get a PDF file and read the data.
	if(theErr == noErr){
	    DoOpenDocument();
	};

	return theErr;
}

static MenuCommand getCommandForBox(CGPDFBox boxType)
{
    switch(boxType){
	default:
	case kCGPDFMediaBox:
	    return kHICommandUseMediaBox;
	    
	case kCGPDFCropBox:
	    return kHICommandUseCropBox;

	case kCGPDFBleedBox:
	    return kHICommandUseBleedBox;

	case kCGPDFTrimBox:
	    return kHICommandUseTrimBox;

	case kCGPDFArtBox:
	    return kHICommandUseArtBox;
    }
}

// Handle command-process events at the application level
pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)
    HICommand  aCommand;
    OSStatus   result = eventNotHandledErr;
    Rect	bounds;

    GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
    
    switch (aCommand.commandID){
		case kHICommandAbout:
		    DoAboutBox();
		    result = noErr; 
		    break;

		case kHICommandOpen:
		    DoTheOpenCommand();
		    break;

		case kHICommandNextPage:
		    gCurrentPage++;
		    if(gCurrentPage == gTotalPages)
			DisableMenuCommand(NULL, kHICommandNextPage);
			
		    if(gCurrentPage != 1)
			EnableMenuCommand(NULL, kHICommandPreviousPage);
		    
		    if(gWindowRef)
			InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));

		    break;

		case kHICommandPreviousPage:
		    gCurrentPage--;
		    if(gCurrentPage == 1)
			DisableMenuCommand(NULL, kHICommandPreviousPage);
		    if(gCurrentPage != gTotalPages)
			EnableMenuCommand(NULL, kHICommandNextPage);
		    else
			DisableMenuCommand(NULL, kHICommandNextPage);
			
		    if(gWindowRef)
			InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    break;

		case kHICommandFirstPage:
		    if(gCurrentPage != 1){
			gCurrentPage = 1;
			DisableMenuCommand(NULL, kHICommandPreviousPage);
			if(gCurrentPage != gTotalPages)
			    EnableMenuCommand(NULL, kHICommandNextPage);
			else
			    DisableMenuCommand(NULL, kHICommandNextPage);

			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandLastPage:
		    if(gCurrentPage != gTotalPages){
			gCurrentPage = gTotalPages;
			DisableMenuCommand(NULL, kHICommandNextPage);
			if(gCurrentPage != 1)
			    EnableMenuCommand(NULL, kHICommandPreviousPage);
			else
			    DisableMenuCommand(NULL, kHICommandPreviousPage);


			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;
		    
		case kHICommandUseMediaBox:
		    if(gBoxType != kCGPDFMediaBox){
			SetMenuCommandMark(NULL, getCommandForBox(gBoxType), noMark);
			SetMenuCommandMark(NULL, kHICommandUseMediaBox, checkMark);
			gBoxType = kCGPDFMediaBox;
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandUseCropBox:
		    if(gBoxType != kCGPDFCropBox){
			SetMenuCommandMark(NULL, getCommandForBox(gBoxType), noMark);
			SetMenuCommandMark(NULL, kHICommandUseCropBox, checkMark);
			gBoxType = kCGPDFCropBox;
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;
		case kHICommandUseArtBox:
		    if(gBoxType != kCGPDFArtBox){
			SetMenuCommandMark(NULL, getCommandForBox(gBoxType), noMark);
			SetMenuCommandMark(NULL, kHICommandUseArtBox, checkMark);
			gBoxType = kCGPDFArtBox;
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;
		case kHICommandUseTrimBox:
		    if(gBoxType != kCGPDFTrimBox){
			SetMenuCommandMark(NULL, getCommandForBox(gBoxType), noMark);
			SetMenuCommandMark(NULL, kHICommandUseTrimBox, checkMark);
			gBoxType = kCGPDFTrimBox;
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;
		case kHICommandUseBleedBox:
		    if(gBoxType != kCGPDFBleedBox){
			SetMenuCommandMark(NULL, getCommandForBox(gBoxType), noMark);
			SetMenuCommandMark(NULL, kHICommandUseBleedBox, checkMark);
			gBoxType = kCGPDFBleedBox;
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandUsePantherAPI:
		    if(gAPISet != kPantherAPI){
			gAPISet = kPantherAPI;
			UpdateAPICommandSetMenu();
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandUseEmulatedPantherAPI:
		    if(gAPISet != kEmulatedPantherAPI){
			gAPISet = kEmulatedPantherAPI;
			UpdateAPICommandSetMenu();
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandUseJaguarAPI:
		    if(gAPISet != kJaguarAPI){
			gAPISet = kJaguarAPI;
			UpdateAPICommandSetMenu();
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandScale100:
		    if(gScaleFactor != 100){
			gScaleFactor = 100;
			UpdateScalingCommandSetMenu();
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandScale50:
		    if(gScaleFactor != 50){
			gScaleFactor = 50;
			UpdateScalingCommandSetMenu();
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandScale200:
		    if(gScaleFactor != 200){
			gScaleFactor = 200;
			UpdateScalingCommandSetMenu();
			if(gWindowRef)
			    InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    }
		    break;

		case kHICommandRotatePageClockwise:
		    gExtraPageRotation += 90;
		    if(gWindowRef)
			InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    break;

		case kHICommandRotatePageCounterclockwise:
		    gExtraPageRotation -= 90;
		    if(gWindowRef)
			InvalWindowRect(gWindowRef, GetWindowPortBounds(gWindowRef, &bounds));
		    break;

		default:
			break;

		case kHICommandQuit:
			QuitApplicationEventLoop();
			result = noErr;
			break;
    
    }
    HiliteMenu(0);
    return result;
}

static void HandleWindowUpdate(WindowRef window)
{
	OSStatus err = noErr;
	Rect bounds;
	err = MyDrawProc(GetWindowPort(window), GetWindowPortBounds(window, &bounds), gThePDFDocument,
					gCurrentPage, gBoxType, gAPISet, gScaleFactor, gExtraPageRotation);
}

static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
#pragma unused (appleEvt, reply, refcon)
	QuitApplicationEventLoop();
//#pragma noreturn (QuitAppleEventHandler)
        return 128;
}

static void DoAlertWithText(CFStringRef key)
{
    CFStringRef ourString = NULL;
    SInt16      alertItemHit = 0;
    Str255      stringBuf;

    ourString =  CFCopyLocalizedString(key, NULL);
    if (ourString != NULL)
    {
	if (CFStringGetPascalString (ourString, stringBuf, sizeof(stringBuf), GetApplicationTextEncoding()))
	{
	    StandardAlert(kAlertStopAlert, stringBuf, NULL, NULL, &alertItemHit);
	}
	CFRelease (ourString);                             
    }
}

static void WarnAboutNoPrintingOrCopying(bool noCopy, bool noPrint)
{
    CFStringRef theKey = NULL;
    if(noCopy && noPrint){
	theKey = kPDFDocumentCantCopyOrPrintKey;
    }else{
	if(noCopy)
	    theKey = kPDFDocumentCantCopyKey;
	else
	    if(noPrint)
		theKey = kPDFDocumentCantPrintKey;
    }
    
    if(theKey)
	DoAlertWithText(theKey);
}

static void WarnAboutNoNewPDFDocument(void)
{
	DoAlertWithText(kPDFDocumentCantCreateNewDocumentKey);
}


static void WarnAboutEncryptedPDFDocument(void)
{	
	DoAlertWithText(kPDFDocumentIsProtectedKey);
}

static void DoAboutBox()
{	
	DoAlertWithText(kAboutBoxStringKey);
}


void DoErrorAlert(OSStatus status, CFStringRef errorFormatString)
{	
    CFStringRef formatStr = NULL, printErrorMsg = NULL;
    SInt16      alertItemHit = 0;
    Str255      stringBuf;

    if ((status != noErr) && (status != kPMCancel))           
    {
        formatStr =  CFCopyLocalizedString (errorFormatString, NULL);	
	if (formatStr != NULL){
			printErrorMsg = CFStringCreateWithFormat(        
                       NULL, NULL, 
                       formatStr, status);
            if (printErrorMsg != NULL)
            {
                if (CFStringGetPascalString (printErrorMsg,    
                              stringBuf, sizeof(stringBuf), 
                              GetApplicationTextEncoding()))
				{
                    StandardAlert(kAlertStopAlert, stringBuf, 
                                NULL, NULL, &alertItemHit);
				}
                CFRelease (printErrorMsg);                     
            }
           CFRelease (formatStr);                             
        }
    }
}

