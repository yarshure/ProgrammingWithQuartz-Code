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

#include <stdio.h>
#include <ApplicationServices/ApplicationServices.h>

static void usage(const char *name){
    fprintf(stderr, "Usage: %s [inputfile] \n", name);
}

/* This is a data type useful for passing around a PDF document
    and its media box. */
typedef struct MyPDFData
{
    CGPDFDocumentRef pdfDoc;
    CGRect	    mediaRect;
}MyPDFData;

/* This is a simple function to create a CFURLRef from
    a path to a file. The path can be relative to the
    current directory or an absolute path. */
static CFURLRef createURL(const char *path)
{
    return CFURLCreateFromFileSystemRepresentation(NULL, path, 
			    strlen(path), false);
}


/* For the supplied URL and media box, create a PDF context
that creates a PDF file at that URL and uses supplied rect
as its document media box. */
CGContextRef myCreatePDFContext(CFURLRef url, CGRect mediaBox)
{
    CGContextRef pdfContext = NULL;
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
				kCFAllocatorDefault, 0,
				&kCFTypeDictionaryKeyCallBacks, 
				&kCFTypeDictionaryValueCallBacks);
    // Add some creator information to the generated PDF file.
    if(dict){
		CFDictionarySetValue(dict, kCGPDFContextCreator, 
					CFSTR("PDF Stamper Application"));
		pdfContext = CGPDFContextCreateWithURL(url, &mediaBox, dict);
		CFRelease(dict);
    }
    return pdfContext;
}

/* For a URL corresponding to an existing PDF document on disk,
create a CGPDFDocumentRef and obtain the media box of the first
page. */
MyPDFData myCreatePDFSourceDocument(CFURLRef url)
{
    MyPDFData myPDFData;
    myPDFData.pdfDoc = CGPDFDocumentCreateWithURL(url);
    if(myPDFData.pdfDoc){
		myPDFData.mediaRect = CGPDFPageGetBoxRect(myPDFData.pdfDoc, kCGPDFMediaBox);
		// Make the media rect origin at 0,0. 
		myPDFData.mediaRect.origin.x = 
		myPDFData.mediaRect.origin.y = 0.;
    }
    return myPDFData;
}

/* Draw the source PDF document into the context and then draw the stamp PDF document
on top of it. When drawing the stamp on top, place it along the diagonal from the lower
left corner to the upper right corner and center its media rect to the center of that
diagonal. */
void StampWithPDFDocument(CGContextRef context, 
			CGPDFDocumentRef sourcePDFDoc, 
			CGPDFDocumentRef stampFileDoc, CGRect stampMediaRect)
{
    CGRect pageRect;
    float angle;
    size_t i, numPages = CGPDFDocumentGetNumberOfPages(sourcePDFDoc);
    
    // Loop over document pages and stamp each one appropriately.
    for(i = 1 ; i <= numPages ; i++)
    {
		// Use the page rectangle of each page from the source to compute
		// the destination media box for each page and the location of
		// the stamp.
		CGRect pageRect = CGPDFDocumentGetMediaBox(sourcePDFDoc, i);
		CGContextBeginPage(context, &pageRect);
		CGContextSaveGState(context);
			// Clip to the media box of the page.
			CGContextClipToRect(context, pageRect);	    
			// First draw the content of the source document.
			CGContextDrawPDFDocument(context, pageRect, sourcePDFDoc, i);
			// Translate to center of destination rect, that is the center of 
			// the media box of content to draw on top of.
			CGContextTranslateCTM(context, 
				pageRect.size.width/2, pageRect.size.height/2);
			// Compute angle of the diagonal across the destination page.
			angle = atan(pageRect.size.height/pageRect.size.width);
			// Rotate by an amount so that drawn content goes along a diagonal
			// axis across the page.
			CGContextRotateCTM(context, angle);
			// Move the origin so that the media box of the PDF to stamp
			// is centered around center point of destination.
			CGContextTranslateCTM(context, 
			-stampMediaRect.size.width/2, 
			-stampMediaRect.size.height/2);
			// Now draw the document to stamp with on top of original content.
			CGContextDrawPDFDocument(context, stampMediaRect, 
					stampFileDoc, 1);
		CGContextRestoreGState(context);
		CGContextEndPage(context);
    }
}

/*	From an input PDF document and a PDF document whose contents you
    want to draw on top of the other, create a new PDF document
    containing all the pages of the input document with the first page
    of the "stamping" overlayed. 
*/
void createStampedFileWithFile(CFURLRef inURL, 
			CFURLRef stampURL, CFURLRef outURL)
{
    CGContextRef pdfContext = NULL;
    MyPDFData stampFileData, sourceFileData;
    sourceFileData = myCreatePDFSourceDocument(inURL);
    if(!sourceFileData.pdfDoc){
		fprintf(stderr, 
			"Can't create PDFDocumentRef for source input file!\n");
		return;
    }

    stampFileData = myCreatePDFSourceDocument(stampURL);
    if(!stampFileData.pdfDoc){
		CGPDFDocumentRelease(sourceFileData.pdfDoc);
		fprintf(stderr, 
			"Can't create PDFDocumentRef for file to stamp with!\n");
		return;
    }
    
    pdfContext = myCreatePDFContext(outURL, sourceFileData.mediaRect);
    if(!pdfContext){
		CGPDFDocumentRelease(sourceFileData.pdfDoc);
		CGPDFDocumentRelease(stampFileData.pdfDoc);
		fprintf(stderr, 
			"Can't create PDFContext for output file!\n");
		return;
    }
    
    StampWithPDFDocument(pdfContext, sourceFileData.pdfDoc, 
	    stampFileData.pdfDoc, stampFileData.mediaRect);
    
    CGContextRelease(pdfContext);
    CGPDFDocumentRelease(sourceFileData.pdfDoc);
    CGPDFDocumentRelease(stampFileData.pdfDoc);
}


int main (int argc, const char * argv[]) {
    static char *suffix = ".watermarked.pdf";
    static char *stampFileName = "confidential.pdf";
    const char *inputFileName = NULL;
    char *outputFileName = NULL;
    CFURLRef inURL = NULL, outURL = NULL, stampURL = NULL;
    int outputnamelength;
    
    if(argc != 2){
		usage(argv[0]);
        return 1;
    }

    inputFileName = argv[1];
    outputnamelength = strlen(inputFileName) + strlen(suffix) + 1;
    outputFileName = (char *)malloc(outputnamelength);
    strncpy(outputFileName, inputFileName, outputnamelength);
    strncat(outputFileName, suffix, outputnamelength);
    
    inURL = createURL(inputFileName);
    if(!inURL){
		fprintf(stderr, "Couldn't create URL for input file!\n");
		return 1;
    }
    
    outURL = createURL(outputFileName);
    if(!outURL){
		CFRelease(inURL);
		fprintf(stderr, "Couldn't create URL for output file!\n");
		return 1;
    }

    stampURL = createURL(stampFileName);
    if(!stampURL){
		CFRelease(inURL);
		CFRelease(outURL);
		fprintf(stderr, "Couldn't create URL for stamping file!\n");
		return 1;
    }
    
    createStampedFileWithFile(inURL, stampURL, outURL);
    
    CFRelease(stampURL);
    CFRelease(outURL);
    CFRelease(inURL);
    return 0;
}

