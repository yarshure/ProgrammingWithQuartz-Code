/*
*  File:    PDFHandling.c
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

#include "AppDrawing.h"
#include "FrameworkUtilities.h"
#include "Utilities.h"
#include "DataProvidersAndConsumers.h"

CGPDFDocumentRef createNewPDFRefFromPasteBoard(void)
{
	CGDataProviderRef dataProvider = NULL;
	CGPDFDocumentRef pasteBoardPDFDocument = NULL;
	/*	Create a reference to the PDF data on the pasteboard.
		The implementation of myCreatePDFDataFromPasteBoard depends
		on the application framework you are using for your application.
	
		myCreatePDFDataFromPasteBoard creates a reference that is owned
		by the calling application.
    */
	CFDataRef pasteBoardData = myCreatePDFDataFromPasteBoard();
	
	if(pasteBoardData == NULL){
		fprintf(stderr, "There is no PDF data on pasteboard!\n");
		return NULL;
	}
    
	// Create a data provider from the pasteboard data.
	dataProvider = myCGDataProviderCreateWithCFData(pasteBoardData);
	// Release the pasteboard data since the data provider retains
	// it and this code owns a reference but no longer requires it.
	CFRelease(pasteBoardData);
	
	if(dataProvider == NULL){
		fprintf(stderr, "Couldn't create data provider.\n");
		return NULL;
    }
    pasteBoardPDFDocument = CGPDFDocumentCreateWithProvider(dataProvider);
	// Release the data provider now that the code is done with it.
	CGDataProviderRelease(dataProvider);
	if(pasteBoardPDFDocument == NULL){
		fprintf(stderr, "Couldn't create PDF document from pasteboard data provider.\n");
		return NULL;
	}
	return pasteBoardPDFDocument;
}

CGPDFDocumentRef getPasteBoardPDFDoc(Boolean reset)
{
    static CGPDFDocumentRef pdfDoc = NULL;
    if(reset){
		// Release any existing document.
		CGPDFDocumentRelease(pdfDoc);
		pdfDoc = createNewPDFRefFromPasteBoard();
    }else{
		// If there isn't already one, create it fresh.
		if(pdfDoc == NULL){
			pdfDoc = createNewPDFRefFromPasteBoard();
		}
    }
    return pdfDoc;
}


void drawPasteBoardPDF(CGContextRef context)
{
    CGRect  pdfRect;
    CGPDFDocumentRef pdfDoc = getPasteBoardPDFDoc(false);   // Obtain the existing one.
    if(pdfDoc == NULL){
		fprintf(stderr, "Quartz couldn't create CGPDFDocumentRef from pasteboard.\n");
		return;
    }
    // The media box is the bounding box of the PDF document. 
    pdfRect = CGPDFDocumentGetMediaBox(pdfDoc , 1);   // page 1
    // Make the destination rect origin at the Quartz origin. 
    pdfRect.origin.x = pdfRect.origin.y = 0.;	
    CGContextDrawPDFDocument(context, pdfRect, pdfDoc, 1);  // page 1
}

CFDataRef cfDataCreatePDFDocumentFromCommand(OSType command)
{
	// Media rect for the drawing. In a real application this
	// should be the bounding rectangle of the graphics
	// that will be the PDF content. 
    const CGRect mediaRect = { {0, 0}, {612, 792} }; 
    CFMutableDictionaryRef dict = NULL;
    CGContextRef pdfContext = NULL;
    CGDataConsumerRef consumer = NULL;
    CFMutableDataRef data = NULL;
    
    // Create a dictionary to hold the optional information describing the PDF data.
    dict = CFDictionaryCreateMutable(NULL, 0,
						&kCFTypeDictionaryKeyCallBacks, 
						&kCFTypeDictionaryValueCallBacks);
    if(dict == NULL){
		fprintf(stderr, "Couldn't make dict!\n");
		return NULL;
    }
    
    // Add the creator and title information to the PDF content.
    CFDictionarySetValue(dict, kCGPDFContextTitle, CFSTR("Pasted From Sample Quartz Application"));
    CFDictionarySetValue(dict, kCGPDFContextCreator, CFSTR("Sample Quartz Application"));

    // Create a mutable CFData object with unlimited capacity.
    data = CFDataCreateMutable(NULL, 0);
    if(data == NULL){
		CFRelease(dict);
		fprintf(stderr, "Couldn't make CFData!\n");
		return NULL;
    }

    // Create the data consumer to capture the PDF data.
    consumer = myCGDataConsumerCreateWithCFData(data);
    if(consumer == NULL){
		CFRelease(dict);
		CFRelease(data);
		fprintf(stderr, "Couldn't create data consumer!\n");
		return NULL;
    }

    pdfContext = CGPDFContextCreate(consumer, &mediaRect, dict);
    CGDataConsumerRelease(consumer);
    CFRelease(dict);
    
    if(pdfContext == NULL){
		CFRelease(data);
		fprintf(stderr, "Couldn't create pdf context!\n");
		return NULL;
    }
    
    CGContextBeginPage(pdfContext, &mediaRect);
		CGContextSaveGState(pdfContext);
			CGContextClipToRect(pdfContext, mediaRect);
			DispatchDrawing(pdfContext, command);
		CGContextRestoreGState(pdfContext);
    CGContextEndPage(pdfContext);
    CGContextRelease(pdfContext);
    
    return data;
}	


OSStatus MakePDFDocument(CFURLRef url, const ExportInfo *exportInfo)	
{
    OSStatus err = noErr;
    // Create an auxiliaryInfo dictionary.
    CFMutableDictionaryRef info = CFDictionaryCreateMutable(NULL, 0,
						&kCFTypeDictionaryKeyCallBacks, 
						&kCFTypeDictionaryValueCallBacks);
    // Use this as the media box for the document.
	// In a real application this should be the bounding
	// rectangle of the graphics that will be the PDF content. 
    const CGRect mediaRect = { {0, 0}, {612, 792} };
    if(info){
		// Add the title information for this document.
		CFDictionarySetValue(info, kCGPDFContextTitle, 
			CFSTR("BasicDrawing Sample Graphics"));
		// Add the author information for this document. This is typically  
		// the user creating the document.
		CFDictionarySetValue(info, kCGPDFContextAuthor, 
			CFSTR("David Gelphman and Bunny Laden"));
		// The creator is the application creating the document.
		CFDictionarySetValue(info, kCGPDFContextCreator, 
			CFSTR("BasicDrawing Application"));
#if 0
		// Before using the kCGPDFContextCropBox key, check to ensure that it
		// is available.
		if(&kCGPDFContextCropBox != NULL){
			// Prepare the crop box entry. Use this rectangle as the crop box for
			// this example.
			CGRect cropBox = CGRectMake(100, 100, 200, 200);
			// Create a CFData object from the crop box rectangle.
			CFDataRef cropBoxData = CFDataCreate(NULL, (UInt8 *)&cropBox, sizeof(cropBox));
			if(cropBoxData){
				// Add the crop box entry to the auxiliaryInfo dictionary.
				CFDictionarySetValue(info, kCGPDFContextCropBox, cropBoxData);
				// Release the CFData created to add the crop box entry.
				CFRelease(cropBoxData);
			}
		}
#endif
    }else{
		fprintf(stderr, "Couldn't create info dictionary!\n");
    }
    
    if(url){
		CGContextRef pdfContext = CGPDFContextCreateWithURL(url, &mediaRect, info);
		if(pdfContext){
			CGContextBeginPage(pdfContext, &mediaRect);
			CGContextSaveGState(pdfContext);
			CGContextClipToRect(pdfContext, mediaRect);
			DispatchDrawing(pdfContext, exportInfo->command);
			CGContextRestoreGState(pdfContext);
			CGContextEndPage(pdfContext);
			CGContextRelease(pdfContext);
		}else{
			fprintf(stderr, "Can't create PDF document!\n");
		}
    }
    if(info) CFRelease(info);
    
    return err;
}

