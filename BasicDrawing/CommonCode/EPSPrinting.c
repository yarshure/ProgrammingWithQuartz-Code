/*
*  File:    EPSPrinting.c
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


#include "EPSPrinting.h"
#include "BitmapContext.h"
#include "Utilities.h"

static CGRect getEPSBBox(char *epspath)
{
    CGRect bbox = CGRectZero;
    FILE *fp = fopen(epspath, "r");
	if(fp){
		int llx, lly, urx, ury, numScanned = 0;
		char ch;
		/*  This is a VERY poor man's EPS DSC parser, here just so that 
			this sample code can handle simple EPS files. It is
			simple but very inefficient. In addition it does not ensure 
			that the DSC comments are at the beginning of a line, 
			nor does it handle (atend) style comments at all.
			It will simply find the first occurance of a
			%%BoundingBox comment and if it is of the typical 
			form, it will obtain the bounding box data. 
		*/
		while(!feof(fp)){
			numScanned = fscanf(fp, 
					"%%%%BoundingBox: %d %d %d %d", 
					&llx, &lly, &urx, &ury);
			if(numScanned == 4){
				bbox.origin.x = llx;
				bbox.origin.y = lly;
				bbox.size.width = urx - llx;
				bbox.size.height = ury - lly;
				break;
			}
			// Haven't found the BoundingBox comment yet so
			// read a character, advancing the file position,
			// and scan again.
			if(fread(&ch, 1, 1, fp) != 1)
				break;
		}
		fclose(fp);
	}
    return bbox;
}

static CGImageRef createEPSPreviewImage(CFURLRef url)
{   
    /*	The CGImage used as the preview needs to have the
		same width and height as the EPS data it will
		be associated with. This sample code doesn't attempt
		to use any preview image associated with the EPS 
		data but instead simply draws a box of an appropriate
		size. Your code would most likely create an image
		that reflects a PICT or TIFF preview present in the
		EPS data. 
	*/
    char path[PATH_MAX + 1];
    if(!CFURLGetFileSystemRepresentation(url, true, path, sizeof(path))){
		fprintf(stderr, "Couldn't get the path for EPS file!\n");
		return NULL;
    }
    
    CGRect epsRect = getEPSBBox(path);
    // Check whether the EPS bounding box is empty.
    if( CGRectEqualToRect(epsRect, CGRectZero) ){
		fprintf(stderr, "Couldn't find BoundingBox comment!\n");
		return NULL;
    }
    Boolean wantDisplayColorSpace = false;
    Boolean needsTransparentBitmap = true;
    // Create a bitmap context to draw to in order to
    // create the preview image. Use the routine
    // createRGBBitmapContext from the earlier chapter.
    CGContextRef bitmapContext = createRGBBitmapContext(
				    epsRect.size.width, 
				    epsRect.size.height, 
				    wantDisplayColorSpace,
				    needsTransparentBitmap);
    if(bitmapContext == NULL){
		fprintf(stderr, "Couldn't create bitmap context\n");
		return NULL;
    }
    
    epsRect.origin.x = epsRect.origin.y = 0;
    /*	Draw the contents of the preview. The preview consists
		of two lines and a stroke around the bounding box. One
		of the two lines is drawn from the lower-left corner to 
		the upper-right corner of the bounding box and the other
		line is from the lower-right corner to the upper-left
		corner of the bounding box.
	*/
    CGContextBeginPath(bitmapContext);
    CGContextMoveToPoint(bitmapContext, 0, 0);
    CGContextAddLineToPoint(bitmapContext, epsRect.size.width, epsRect.size.height);
    CGContextMoveToPoint(bitmapContext, epsRect.size.width, 0);
    CGContextAddLineToPoint(bitmapContext, 0, epsRect.size.height);
    CGContextStrokePath(bitmapContext);
    // Stroke the bounding rectangle, inset so that the stroke is
    // completely contained in the EPS bounding rect.
    CGContextStrokeRect(bitmapContext, CGRectInset(epsRect, 0.5, 0.5));

    // Now create an image from the bitmap raster data. This image
    // has a data provider that releases the image raster data when
    // the image is released. Use the createImageFromBitmapContext
    // from Chapter 12. Calling createImageFromBitmapContext
    // gives up ownership of the raster data used by the context.
    CGImageRef epsPreviewImage = createImageFromBitmapContext(bitmapContext);
    
    // Release the bitmap context.
    CGContextRelease(bitmapContext);
    
    if(epsPreviewImage == NULL){
		fprintf(stderr, "Couldn't create preview image!\n");
		return NULL;
    }
    
    return epsPreviewImage;
}

/*  This technique of handling EPS data is available in 
    Mac OS X v10.1 and later and is one alternative method
	of supporting EPS data during printing as compared to 
	converting EPS data to PDF data using CGPSConverter which 
	is only available in Panther and later. 
*/
static CGImageRef createCGEPSImage(CFURLRef url)
{
    CGImageRef previewImage = NULL;
    CGImageRef epsImage = NULL;
    CGDataProviderRef epsDataProvider = NULL;
    
    previewImage = createEPSPreviewImage(url);
    if(previewImage == NULL){
		fprintf(stderr, "Couldn't create EPS preview!\n");
		return NULL;
    }
    
    // It is important that the data provider supplying the
    // EPS data conform to the Quartz guidelines for data providers
    // and is able to provide the data until the data releaser function
    // is called. If you have a custom data provider, you need
    // to follow these guidelines since your data provider
    // is not necessarily called before you release the image
    // that uses the provider.
    epsDataProvider = CGDataProviderCreateWithURL(url);
    if(epsDataProvider == NULL){
		CGImageRelease(previewImage);
		fprintf(stderr, "Couldn't create EPS data provider!\n");
		return NULL;
    }
    
    // Create the hybrid CGImage that contains the preview image
    // and the EPS data. Note that the data provider isn't
    // called during image creation but at some later point in time.
    epsImage = PMCGImageCreateWithEPSDataProvider(epsDataProvider, previewImage);
    // The preview image and data provider are no longer needed
    // because Quartz retains them and this code doesn't
    // require them further.
    CGImageRelease(previewImage);
    CGDataProviderRelease(epsDataProvider);
    
    if(epsImage == NULL){
		fprintf(stderr, "Couldn't create EPS hybrid image!\n");
		return NULL;
    }
    return epsImage;
}

void drawEPSDataImage(CGContextRef context, CFURLRef url)
{
    // Create the a CGImage that has EPS data associated with it.
    CGImageRef epsDataImage = createCGEPSImage(url);
    if(epsDataImage == NULL){
		return;
    }
    // Create a destination rectangle at the location
    // to draw the EPS document. The size of the rect is scaled
    // down to 1/2 the size of the EPS graphic.
    CGRect destinationRect = CGRectMake(100, 100, 
			CGImageGetWidth(epsDataImage), 
			CGImageGetHeight(epsDataImage));
    // Draw the image to the destination. When the EPS
    // data associated with the image is sent to a PostScript 
    // printer, the EPS bounding box is mapped to this 
    // destination rectangle, translated and scaled as necessary.
    CGContextDrawImage(context, destinationRect, epsDataImage);
    
    // Draw the image a second time. This time the image is
    // rotated by 45 degrees and scaled by an additional scaling factor
    // of 0.5 in the x dimension. The center point of this image coincides
    // with the center point of the earlier drawing.
    CGContextTranslateCTM(context, 
	    destinationRect.origin.x + destinationRect.size.width/2, 
	    destinationRect.origin.y + destinationRect.size.height/2);
    CGContextRotateCTM(context, DEGREES_TO_RADIANS(45));
    CGContextScaleCTM(context, 0.5, 1);
    CGContextTranslateCTM(context, 
	    -(destinationRect.origin.x + destinationRect.size.width/2), 
	    -(destinationRect.origin.y + destinationRect.size.height/2) );
    CGContextDrawImage(context, destinationRect, epsDataImage);
    
    // Release the image when done.
    CGImageRelease(epsDataImage);
}