/*
*  File:    FrameworkUtilities.c
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

#include "FrameworkUtilities.h"
#include "AppDrawing.h"

static PasteboardRef getThePasteBoard(void)
{
	static PasteboardRef pasteBoard = NULL;
 	if (pasteBoard == NULL){
		OSStatus err = PasteboardCreate(kPasteboardClipboard,&pasteBoard);
		if(err || pasteBoard == NULL){
			fprintf(stderr, "Got error %d trying to get pasteboard!\n", (int)err);
		}
	}
	return pasteBoard;
}

CFDataRef myCreatePDFDataFromPasteBoard(void)
{
	OSStatus err = noErr;
	// This is the extern string kUTTypePDF in UTCoreTypes.h in 
	// Mac OS X Tiger and later.
	static CFStringRef kPasteBoardTypePDF = CFSTR("com.adobe.pdf");
	CFDataRef pasteBoardData = NULL;
	PasteboardRef pasteBoard = getThePasteBoard();
	ItemCount   numPasteBoardItems;
	UInt32      i;
	(void)PasteboardSynchronize(pasteBoard);
	err = PasteboardGetItemCount(pasteBoard, &numPasteBoardItems);
	if(err || numPasteBoardItems == 0){
		fprintf(stderr, "There is NO data on pasteboard!\n");
		return NULL;
	}
	// Iterate over the items on the pasteboard.
	for(i = 1; i <= numPasteBoardItems && pasteBoardData == NULL; i++)
	{
		PasteboardItemID	id;
		CFArrayRef		flavorTypeArray;
		CFIndex			flavorCount;
		CFIndex			flavorIndex;
	
		// Every item on the pasteboard has a unique identifier.
		err = PasteboardGetItemIdentifier( pasteBoard, i, &id);
		if(err)
			return NULL;

		// Each pasteboard item has a flavor array associated with it.
		err = PasteboardCopyItemFlavors(pasteBoard, id, &flavorTypeArray);
		if(err || flavorTypeArray == NULL)
			return NULL;
	    
		flavorCount = CFArrayGetCount( flavorTypeArray );
		
		// Iterate over the flavors for this pasteboard item and see 
		// if one of them is type PDF.
		for(flavorIndex = 0; flavorIndex < flavorCount; flavorIndex++)
		{
			CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex( 
										flavorTypeArray, flavorIndex );
			if( flavorType &&
				(CFStringCompare(flavorType, kPasteBoardTypePDF, 0)
					== kCFCompareEqualTo)
			){
				err = PasteboardCopyItemFlavorData(pasteBoard, id,
					    					flavorType, &pasteBoardData);
				if(err || pasteBoardData == NULL){
		    			fprintf(stderr, "Couldn't get pdf data from pasteboard!\n");
				}
				// Got the PDF data so break out of the loop.
				break;
			}
		}
		// Release each flavor type array this code creates or copies.
		CFRelease(flavorTypeArray);
	}
	return pasteBoardData;
}

void addPDFDataToPasteBoard(OSType command)
{
    OSStatus err = noErr;
    PasteboardRef pasteBoard;
    // Create the content to add to the pasteboard.
    CFDataRef pdfData = cfDataCreatePDFDocumentFromCommand(command);
    if(!pdfData){
		fprintf(stderr, "Couldn't create data to put on pasteboard.\n");
		return;
    }
    
    pasteBoard = getThePasteBoard();

    // First clear the pasteboard of its current contents.
    err = PasteboardClear(pasteBoard);
    if(err){
		CFRelease(pdfData);
		fprintf(stderr, "couldn't clear the pasteboard due to err = %d.\n", (int)err);
		return;
    }

    // Put the PDF data on the pasteboard.
    err = PasteboardPutItemFlavor( pasteBoard, (PasteboardItemID)1,
					CFSTR("com.adobe.pdf"), 
					pdfData, 
					kPasteboardFlavorNoFlags);
    // Release the data since the code is done with it,
    // regardless of whether there is an error putting it on
    // the pasteboard.
    CFRelease(pdfData);
    if(err){
		fprintf(stderr, "Got err = %d putting data on pasteboard.\n", (int)err);
    }
}

