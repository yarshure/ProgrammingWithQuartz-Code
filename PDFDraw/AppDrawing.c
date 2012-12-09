/*
*  File:    AppDrawing.c
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
#include "UIHandling.h"

/**** Macros and Defines ****/

/**** our private prototypes ***/

CGRect myCGPDFDocumentPageGetBoxRect(CGPDFDocumentRef pdfDoc, size_t pageNumber, CGPDFBox boxType)
{
    switch(boxType){
	default:
	case kCGPDFMediaBox:
	    return CGPDFDocumentGetMediaBox(pdfDoc, pageNumber);
	    
	case kCGPDFCropBox:
	    return CGPDFDocumentGetCropBox(pdfDoc, pageNumber);

	case kCGPDFBleedBox:
	    return CGPDFDocumentGetBleedBox(pdfDoc, pageNumber);

	case kCGPDFTrimBox:
	    return CGPDFDocumentGetTrimBox(pdfDoc, pageNumber);

	case kCGPDFArtBox:
	    return CGPDFDocumentGetArtBox(pdfDoc, pageNumber);
    }
}

void drawWithoutRotation(CGContextRef context, CGPDFDocumentRef pdfDoc, size_t pageNumber, CGPDFBox boxType)
{
    CGRect rect = myCGPDFDocumentPageGetBoxRect(pdfDoc, pageNumber, boxType);
    rect.origin.x = rect.origin.y = 0.;
    CGContextDrawPDFDocument(context, rect, pdfDoc, pageNumber);
}

#define MIN(a,b)  ((a) < (b) ? (a) : (b))

static inline float DEGREES_TO_RADIANS(float degrees){
	return degrees * M_PI/180;
}

void getRotatedPageDimensions(CGPDFDocumentRef pdfDoc, size_t pageNumber, CGPDFBox boxType,
					int rotationAngle,
					float *widthP, float *heightP)
{
    float width, height;
    CGRect rect = myCGPDFDocumentPageGetBoxRect(pdfDoc, pageNumber, boxType);
    if(boxType != kCGPDFMediaBox){
		CGRect mediaBox = CGPDFDocumentGetMediaBox(pdfDoc, pageNumber);
		rect = CGRectIntersection(rect, mediaBox);
    }
    width = CGRectGetWidth(rect);
    height = CGRectGetHeight(rect);
    // Obtain the page rotation angle and ensure that it is within 0-360 range.
    rotationAngle += CGPDFDocumentGetRotationAngle(pdfDoc, pageNumber);
    rotationAngle %= 360;
    if (rotationAngle < 0)
	rotationAngle += 360;

    if(rotationAngle == 90 || rotationAngle == 270){
	// Interchange the width and height if rotation angle is 90 or 270 degrees.
	float tmp = width;
	width = height;
	height = tmp;
    }

    *widthP = width;
    *heightP = height;
    return;
}


CGAffineTransform myPageGetDrawingTransform(CGPDFDocumentRef pdfDoc, size_t pageNumber, 
					CGPDFBox boxType, CGRect destRect,
					int rotate, bool  preserveAspectRatio)
{
    CGAffineTransform fullTransform, rTransform, sTransform, t1Transform, t2Transform;
    float boxOriginX, boxOriginY, boxWidth, boxHeight;
    float destOriginX, destOriginY, destWidth, destHeight;
    float scaleX, scaleY;

    // First intersect the boundary rectangle of boxType with the media box. This is
    // to conform with the meaning of a given boundary rectangle in the PDF spec.
    CGRect boxRect = myCGPDFDocumentPageGetBoxRect(pdfDoc, pageNumber, boxType);
    if(boxType != kCGPDFMediaBox){
		CGRect mediaBox = CGPDFDocumentGetMediaBox(pdfDoc, pageNumber);
		boxRect = CGRectIntersection(boxRect, mediaBox);
    }
    
    // Obtain the origin, width and height of the PDF box to transform.
    boxOriginX = CGRectGetMinX(boxRect);
    boxOriginY = CGRectGetMinY(boxRect);
    boxWidth = CGRectGetWidth(boxRect);
    boxHeight = CGRectGetHeight(boxRect);

    // Construct a transformation that translates the center of the box
    // to the origin.
    t1Transform = CGAffineTransformMakeTranslation( -(boxOriginX + boxWidth/2), 
						-(boxOriginY + boxHeight/2) );

    // Add the intrinsic page rotation to the rotation requested.
    rotate += CGPDFDocumentGetRotationAngle(pdfDoc, pageNumber);
    // Adjust the page rotation angle to ensure that it is between 0-360 degrees.
    rotate %= 360;
    if (rotate < 0)
	rotate += 360;
    
    // Construct a transformation that rotates by the rotation angle. Since a positive
    // requested rotation is clockwise and Quartz considers clockwise rotations to be
    // negative angles, this code negates the rotation angle when creating the rotation
    // matrix.
    rTransform = CGAffineTransformMakeRotation(DEGREES_TO_RADIANS(-rotate));
        
    // If the rotation is +90 or -90 degrees then the rotation 
    // interchanges the width and height.
    if(rotate == 90 || rotate == 270){
	float tmp = boxWidth;
	boxWidth = boxHeight;
	boxHeight = tmp;
    }

    // Obtain the origin, width and height of the destination rect.
    destOriginX = CGRectGetMinX(destRect);
    destOriginY = CGRectGetMinY(destRect);
    destWidth = CGRectGetWidth(destRect);
    destHeight = CGRectGetHeight(destRect);

    // This computes x and y scaling factors to scale the box dimensions
    // into the destination dimensions. Using the MIN function in
    // this manner ensures that the minimum scaling will be 1. This
    // makes sure that the box is never scaled up to fit, only down.

#if ALLOWSCALINGUP
    scaleX = destWidth/boxWidth;
    scaleY = destHeight/boxHeight;
#else    
    scaleX = MIN(1, destWidth/boxWidth);
    scaleY = MIN(1, destHeight/boxHeight);
#endif
    
    // If there is a request to preserve the aspect ratio then the scale factors
    // must be the same and in order to ensure that both dimensions fit 
    // in the destination, the minimum scaling must be used.
    if(preserveAspectRatio){
		scaleX = scaleY = MIN(scaleX, scaleY);
    }

    // Construct an affine transform that represents this scaling.
    sTransform = CGAffineTransformMakeScale(scaleX, scaleY);

    // Now construct a transform that transforms the origin to the center
    // of destRect.
    t2Transform = CGAffineTransformMakeTranslation( destOriginX + destWidth/2, 
							destOriginY + destHeight/2);

    // Concatenate translation with the rotation. This is 
    // (t1Transform x rTransform). 
    fullTransform = CGAffineTransformConcat(t1Transform, rTransform);

    // Concatenate the previous result with the scaling, that is 
    // (t x sTransform). In this case t is the result of the previous 
    // calculations and sTransform is the scaling matrix just created.

    fullTransform = CGAffineTransformConcat(fullTransform, sTransform);

    // Concatenate the previous result with translation 2, that is 
    // (t x t2Transform). In this case t is the result of the previous 
    // calculations and t2Transform is the translation matrix just created.
    fullTransform = CGAffineTransformConcat(fullTransform, t2Transform);
    
    return fullTransform;
}

void drawPageInRect(CGContextRef context, CGPDFDocumentRef pdfDoc, size_t pageNumber, 
					CGPDFBox boxType, CGRect destRect,
					int additionalPageRotation)
{
    bool preserveAspectRatio = true;
    CGRect clipRect;
    // Calculate the drawing transform to center the specified box into the
    // destRect with the additional rotation beyond that in the rotate value for the PDF page,
    // and require the aspect ratio to be preserved.
    CGAffineTransform t = myPageGetDrawingTransform(pdfDoc, pageNumber, boxType, destRect, 
					additionalPageRotation, preserveAspectRatio);
    CGContextSaveGState(context);
	CGContextConcatCTM(context, t);
	clipRect = myCGPDFDocumentPageGetBoxRect(pdfDoc, pageNumber, boxType);
	// Intersect this rect with the media box if necessary.
	if(boxType != kCGPDFMediaBox){
	    CGRect mediaBox = CGPDFDocumentGetMediaBox(pdfDoc, pageNumber);
	    clipRect = CGRectIntersection(clipRect, mediaBox);
	}
	CGContextClipToRect(context, clipRect);
	// Drawing the PDF document into the media box results in no translation or scaling so
	// the result is that the only transformations done when drawing the PDF document are
	// those applied by the transform t above.
	CGContextDrawPDFDocument(context, CGPDFDocumentGetMediaBox(pdfDoc, pageNumber), pdfDoc, pageNumber);
    CGContextRestoreGState(context);
}

void drawPage(CGContextRef context, CGPDFDocumentRef pdfDoc, size_t pageNumber, 
		    CGPDFBox boxType, int extraRotation)
{
    float width, height;
    CGRect destRect;
    // Obtain the page dimensions of the page of interest 
    getRotatedPageDimensions(pdfDoc, pageNumber, boxType, 
				extraRotation, &width, &height);

    destRect = CGRectMake(0, 0, width, height);
    // The rect that is supplied does preserve the aspect ratio since the 
    // width and height are that of the PDF box being drawn.
    drawPageInRect(context, pdfDoc, pageNumber, boxType, destRect, extraRotation);
}

void getRotatedPDFPageDimensions(CGPDFPageRef page, CGPDFBox boxType, int rotation,
						float *widthP, float *heightP)
{
	float width, height;
	CGRect boxRect = CGPDFPageGetBoxRect(page, boxType);
	// Intersect the boundary rect with the media box if necessary.
	if(boxType != kCGPDFMediaBox){
		CGRect mediaBox = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
		boxRect = CGRectIntersection(boxRect, mediaBox);
	}
	width = CGRectGetWidth(boxRect);
	height = CGRectGetHeight(boxRect);
	// Obtain the page rotation angle, add it to the  
	// requested additional rotation and ensure  
	// that it is within the range of 0-360 degrees.
	rotation += CGPDFPageGetRotationAngle(page);
	rotation %= 360;
	if (rotation < 0)
		rotation += 360;

	if(rotation == 90 || rotation == 270){
	    // Interchange the width and height if rotation angle is 90 or 270 degrees.
	    float tmp = width;
	    width = height;
	    height = tmp;
	}

	*widthP = width;
	*heightP = height;
	return;
}

void drawPDFPageInRect(CGContextRef context, CGPDFPageRef pdfPage, 
							CGPDFBox boxType, CGRect destRect,
							int additionalPageRotation)
{
	bool preserveAspectRatio = true;
	CGRect clipRect;

	// Calculate the drawing transform to center the specified box 
	// into the destRect with the additional rotation specified, 
	// and require the aspect ratio to be preserved.
	CGAffineTransform t = CGPDFPageGetDrawingTransform(pdfPage, 
					boxType, destRect, 
					additionalPageRotation, preserveAspectRatio);
	CGContextSaveGState(context);
		CGContextConcatCTM(context, t);
		clipRect = CGPDFPageGetBoxRect(pdfPage, boxType);
		// Intersect this rect with the media box if necessary.
		if(boxType != kCGPDFMediaBox){
		    CGRect mediaBox = CGPDFPageGetBoxRect(pdfPage, kCGPDFMediaBox);
		    clipRect = CGRectIntersection(clipRect, mediaBox);
		}
		CGContextClipToRect(context, clipRect);
		CGContextDrawPDFPage(context, pdfPage);
	CGContextRestoreGState(context);
}

void drawWithPDFPage(CGContextRef context, CGPDFDocumentRef pdfDoc,
			size_t pageNumber, CGPDFBox boxType, int extraRotation)
{
	float width, height;
	CGRect destRect;
	CGPDFPageRef pdfPage = CGPDFDocumentGetPage(pdfDoc, pageNumber);
	if(!pdfPage){
		fprintf(stderr, "Couldn't get page number %zd !\n", pageNumber);
		return;
	}
	getRotatedPDFPageDimensions(pdfPage, boxType, 
				    extraRotation, &width, &height);
	destRect = CGRectMake(0, 0, width, height);
	// The rect that is supplied preserves the aspect ratio since the 
	// width and height are that of the PDF box being drawn.
	drawPDFPageInRect(context, pdfPage, boxType, destRect, extraRotation);
}


OSStatus MyDrawProc(CGrafPtr port, const Rect *drawingRectP, CGPDFDocumentRef pdfDoc, int pageNumber, CGPDFBox box, 
			    APIVersion apiSet, int scaleFactor, int extraPageRotation)
{
    OSStatus err = noErr;
    CGContextRef context = NULL;
	// this assumes a 1-1 mapping of QD to CG coordinates.
	CGRect rect = CGRectMake(0,0, drawingRectP->right - drawingRectP->left, drawingRectP->bottom - drawingRectP->top);
    /*
		QDBeginCGContext/QDEndCGContext are only available on MacOS X v10.1 and later. QDBeginCGContext 
		returns a CGContextRef corresponding to the port passed in. The graphics state is initialized 
		to default values. The initial coordinate system of the context is at the lower left
		corner of the port passed in.
			
		Calling QDEndCGContext gives up the CGContextRef and restores the QuickDraw 
		bottlenecks on the port to their previous state. 
		
		Note that QDBeginCGContext cannot be called a second time without first
		calling QDEndCGContext.
    */
    err = QDBeginCGContext(port, &context);
    if(!err){
		CGContextSaveGState(context);
			CGContextSetRGBFillColor(context, 1, 1, 1, 1);  // fill is white
			CGContextFillRect(context, rect);
		CGContextRestoreGState(context);
		if(pdfDoc){
			// CGContextSaveGState(context);
			if(scaleFactor != 100){
				float scale = ((float)scaleFactor)/100.;
				CGContextScaleCTM(context, scale, scale);
			}
			if(apiSet == kPantherAPI){
				drawWithPDFPage(context, pdfDoc, pageNumber, box, extraPageRotation);
			}else{
				if(apiSet == kEmulatedPantherAPI)
					drawPage(context, pdfDoc, pageNumber, box, extraPageRotation);
				else
					drawWithoutRotation(context, pdfDoc, pageNumber, box);
			}
			// CGContextRestoreGState(context);
		}

			/*	after QDEndCGContext, the context parameter is NULL and
				we can't do any further imaging with Quartz into the context
				corresponding to the port until we again call QDBeginCGContext.
			*/
		CGContextSynchronize(context);
        err = QDEndCGContext(port, &context);	
        if(err || context != NULL){
            printf("got an error from QDEndCGContext\n");
        }
    }else
        printf("got an error from QDBeginCGContext\n");

	return err;	
} //	MyDrawProc

