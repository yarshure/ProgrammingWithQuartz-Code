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

#include <ApplicationServices/ApplicationServices.h>

/*
    Return the generic RGB color space. This is a 'get' function and the caller should
    not release the returned value unless the caller retains it first. Usually callers
    of this routine will immediately use the returned colorspace with CoreGraphics
    so they typically do not need to retain it themselves.
    
    This function creates the generic RGB color space once and hangs onto it so it can
    return it whenever this function is called.
*/
// The full path to the generic RGB ICC profile.
#define 	kGenericRGBProfilePathStr \
	"/System/Library/ColorSync/Profiles/Generic RGB Profile.icc"

CGColorSpaceRef getTheCalibratedRGBColorSpace(void)
{
	static CGColorSpaceRef genericRGBColorSpace = NULL;
	if (genericRGBColorSpace == NULL)
	{
		CMProfileRef genericRGBProfile = NULL;
		OSStatus err = noErr;
		CMProfileLocation loc;
		// Build up a profile location for ColorSync.
		loc.locType = cmPathBasedProfile;
		strcpy (loc.u.pathLoc.path, kGenericRGBProfilePathStr);
		// Open the profile with ColorSync.
		err = CMOpenProfile(&genericRGBProfile, &loc);
		if(err == noErr){
			genericRGBColorSpace =
				CGColorSpaceCreateWithPlatformColorSpace(
					genericRGBProfile);
			if(genericRGBColorSpace == NULL)
				fprintf(stderr, 
					"couldn't create the generic RGB color space\n");
			// This code opened the profile so it is 
			// up to it to close it.
			CMCloseProfile(genericRGBProfile); 
		}else{
			// ColorSync could not open the profile so log a message 
			// to the console.
			fprintf(stderr, 
				"couldn't open generic profile due to error %d\n",
				(int)err);
		}
	}
	return genericRGBColorSpace;
}

void addURLAnnotationToPDFPage(CGContextRef c, CGRect rect)
{
    char *link = 
	"http://developer.apple.com/documentation/GraphicsImaging/";
    CFURLRef linkURL = CFURLCreateWithBytes(NULL, link, strlen(link), 
					    kCFStringEncodingUTF8, NULL);
    if(!linkURL){
		fprintf(stderr, "Couldn't create url for link!\n");
		return;
    }
    CGPDFContextSetURLForRect(c, linkURL, rect);
    CFRelease(linkURL);
}

void myCreatePDFDocumentAtURL(CFURLRef url)
{
    float red[] = { 1., 0., 0., 1. };
    // Make the media box the same size as the graphics this code draws.
    CGRect mediaBox = CGRectMake(0, 0, 200, 200);
    
    // Create a PDF context using mediaBox as the default media box for the
    // document. The document will be created at the location described by
    // the supplied URL. This example passes NULL for the auxiliaryInfo
    // dictionary.
    CGContextRef pdfContext = CGPDFContextCreateWithURL(url, &mediaBox, NULL);
    if(!pdfContext){
		fprintf(stderr, "Couldn't create PDF context!\n");
		return;
    }

    // Calling CGContextBeginPage indicates the following content is
    // to appear on the first page. The rect passed to this function
    // specifies the media box for this page.
    CGContextBeginPage(pdfContext, &mediaBox);
    // It is good programming practice to bracket the drawing
    // you do on a page with saving and restoring the graphics
    // state.
    CGContextSaveGState(pdfContext);
		// Clip to the media box.
		CGContextClipToRect(pdfContext, mediaBox);
		// Set the fill color and color space.
		CGContextSetFillColorSpace(pdfContext, 
			    		getTheCalibratedRGBColorSpace());
		CGContextSetFillColor(pdfContext, red);
		// Fill the rectangle of the media box with red.
		CGContextFillRect(pdfContext, mediaBox);

		// Add a link to a URL so that if you click
		// on the rect you will go to that URL. 
		addURLAnnotationToPDFPage(pdfContext, mediaBox);
    CGContextRestoreGState(pdfContext);

    // Calling CGContextEndPage denotes the end of the first page. 
    // You MUST call CGContextEndPage after each time you call
    // CGContextBeginPage and they should not be nested.
    CGContextEndPage(pdfContext);
    
    // You MUST release the PDF context when done with it. When the
    // retain count on the context reaches zero, Quartz flushes the
    // drawing content to the PDF file being created and closes it.
    CGContextRelease(pdfContext);
}

/* This function returns a NULL CFStringRef if the password
    is greater than 32 bytes long or the string contains
    characters outside the range 32-127 inclusive. */
CFStringRef createPasswordString(const unsigned char *password)
{
    int i, len = strlen(password);
    // Check the length.
    if(len > 32){
		return NULL;
    }
    // Check that the byte codes are in the printable ASCII range.
    for(i = 0; i < len ; i++){
		if(password[i] < 32 || password[i] > 127)
			return NULL;
    }
    
    return CFStringCreateWithCString(NULL, password, 
					kCFStringEncodingASCII);
}

void addEncryptionKeys(CFMutableDictionaryRef dict)
{
    const unsigned char *ownerPassword = "test";
    CFStringRef ownerPasswordRef = NULL;
    if(dict == NULL){
		return;
    }
    
    ownerPasswordRef = createPasswordString(ownerPassword);
    if(!ownerPasswordRef){
		fprintf(stderr, "Invalid owner password %s!\n", ownerPassword);
		return;
    }
    
    if(&kCGPDFContextOwnerPassword != NULL){
		// Add the owner password.
		CFDictionarySetValue(dict, kCGPDFContextOwnerPassword, ownerPasswordRef);
		CFRelease(ownerPasswordRef);
		// No user password supplied so Quartz will use the empty string.

		// Mark that printing is disallowed.
		CFDictionarySetValue(dict, kCGPDFContextAllowsPrinting, kCFBooleanFalse);
    }else{
		CFRelease(ownerPasswordRef);
		fprintf(stderr, 
			"Encrypted PDF not available in this version of Mac OS X!\n");
    }
}

void myCreate2PagePDFDocumentAtURL(CFURLRef url)
{
    float red[] = { 1., 0., 0., 1. };
    float blue[] = { 0., 0., 1., 1. };
    CGRect annotRect;
    CFStringRef redPageName = CFSTR("com.mycompany.links.dg.redpage");
    CFStringRef bluePageName = CFSTR("com.mycompany.links.dg.bluepage");
    // Make the media box the same size as a US Letter size page.
    CGRect mediaBox = CGRectMake(0, 0, 612, 792);
    CGRect rectBox = CGRectMake(55, 55, 500, 680);
    // Create a point whose center is the center of rectBox.
    CGPoint centerPoint = { rectBox.origin.x + rectBox.size.width/2 ,
				rectBox.origin.y + rectBox.size.height/2 };
    CGContextRef pdfContext = NULL;
    CFMutableDictionaryRef auxiliaryInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
						&kCFTypeDictionaryKeyCallBacks, 
						&kCFTypeDictionaryValueCallBacks);
    addEncryptionKeys(auxiliaryInfo);
    pdfContext = CGPDFContextCreateWithURL(url, &mediaBox, auxiliaryInfo);
    if(auxiliaryInfo)
	CFRelease(auxiliaryInfo);
    
    if(!pdfContext){
		fprintf(stderr, "Couldn't create PDF context!\n");
		return;
    }

    // Start the first page
    CGContextBeginPage(pdfContext, &mediaBox);
    CGContextSaveGState(pdfContext);
		// Clip to the media box.
		CGContextClipToRect(pdfContext, mediaBox);
		// Set the fill color and color space.
		CGContextSetFillColorSpace(pdfContext, 
			    		getTheCalibratedRGBColorSpace());
		CGContextSetFillColor(pdfContext, red);
		// Fill the rectangle of the media box with red.
		CGContextFillRect(pdfContext, rectBox);
		// Make a new named destination at the center of the rect being
		// painted. Here the code uses the name redPageName since
		// this is the "red" page that is being named.
		CGPDFContextAddDestinationAtPoint(pdfContext, 
				redPageName, centerPoint);
		// Make a link to a destination not yet created, that for
		// the "blue" page. Making this link is independent from
		// the creation of the destination above. Clicking
		// on this link in the generated PDF document navigates to
		// the destination referenced by bluePageName.
		CGPDFContextSetDestinationForRect(pdfContext, bluePageName,
			rectBox);
    CGContextRestoreGState(pdfContext);
    CGContextEndPage(pdfContext);

    // Start the second page.
    CGContextBeginPage(pdfContext, &mediaBox);
    CGContextSaveGState(pdfContext);
		// Clip to the media box.
		CGContextClipToRect(pdfContext, mediaBox);
		// Set the fill color and color space.
		CGContextSetFillColorSpace(pdfContext, 
			    		getTheCalibratedRGBColorSpace());
		CGContextSetFillColor(pdfContext, blue);
		// Fill the rectangle of the media box with blue.
		CGContextFillRect(pdfContext, rectBox);
		// Make a new named destination at the center of the rect 
		// being painted. Here the code uses the name bluePageName 
		// since this is the "blue" page that is being named. The link 
		// on page 1 refers to this destination.
		CGPDFContextAddDestinationAtPoint(pdfContext, 
				bluePageName, centerPoint);
		// Make a link to a destination already created
		// for page 1, the red page. Clicking on this link
		// in the generated PDF document navigates to
		// the destination referenced by redPageName.
		CGPDFContextSetDestinationForRect(pdfContext, redPageName,
			rectBox);
    CGContextRestoreGState(pdfContext);
    CGContextEndPage(pdfContext);
    
    // You MUST release the PDF context when done with it. When the
    // retain count on the context reaches zero, Quartz flushes the
    // drawing content to the PDF file being created and closes it.
    CGContextRelease(pdfContext);
}

int main (int argc, const char * argv[]) {
    static char *path = "output.pdf";
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, path, 
			    strlen(path), false);
    if(!url){
		fprintf(stderr, "Couldn't create URL!\n");
		return 1;
    }
    myCreate2PagePDFDocumentAtURL(url);
    CFRelease(url);
    return 0;
}
