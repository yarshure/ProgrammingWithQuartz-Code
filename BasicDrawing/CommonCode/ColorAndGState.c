/*
*  File:    ColorAndGState.c
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

#include "ColorAndGState.h"
#include "Utilities.h"

void doColorSpaceFillAndStroke(CGContextRef context)
{
	CGColorSpaceRef theColorSpace = getTheCalibratedRGBColorSpace();
	float opaqueRed[] = { 0.663, 0.0, 0.031, 1.0 }; // red,green,blue,alpha
	float aBlue[] = { 0.482, 0.62, 0.871, 1.0 };	// red,green,blue,alpha
	
	// Set the fill color space to be the generic calibrated RGB color space.
	CGContextSetFillColorSpace(context, theColorSpace);
	// Set the fill color to opaque red. The number of elements in the
	// array passed to this function must be the number of color
	// components in the current fill color space plus 1 for alpha.
	CGContextSetFillColor(context, opaqueRed);
	
	// Set the stroke color space to be the generic calibrated RGB color space.
	CGContextSetStrokeColorSpace(context, theColorSpace);
	// Set the stroke color to opaque blue. The number of elements
	// in the array passed to this function must be the number of color
	// components in the current stroke color space plus 1 for alpha.
	CGContextSetStrokeColor(context, aBlue);
	
	CGContextSetLineWidth(context, 8.);
 	// Rectangle 1.
	CGContextBeginPath(context);
	CGContextAddRect(context, CGRectMake(20., 20., 100., 100.));
	CGContextDrawPath(context, kCGPathFillStroke);
    
	// Continue to use the stroke colorspace already set
	// but change the stroke alpha value to a semitransparent blue.
	aBlue[3] = 0.5;
	CGContextSetStrokeColor(context, aBlue);
 	// Rectangle 2.
	CGContextBeginPath(context);
	CGContextAddRect(context, CGRectMake(140., 20., 100., 100.));
	CGContextDrawPath(context, kCGPathFillStroke);
    
	// Don't release the color space since this routine 
	// didn't create it.
}

void drawWithColorRefs(CGContextRef context)
{
	static CGColorRef opaqueRedColor = NULL, opaqueBlueColor = NULL, transparentBlueColor = NULL;

	// Initialize the CGColorRefs if necessary
	if(opaqueRedColor == NULL){
		// Initialize the color array to an opaque red 
		// in the generic calibrated RGB color space.
		float color[4] = { 0.663, 0.0, 0.031, 1.0 };
		CGColorSpaceRef theColorSpace = getTheCalibratedRGBColorSpace();
		// Create a CGColorRef for opaque red.
		opaqueRedColor = CGColorCreate(theColorSpace, color);
		// Make the color array correspond to an opaque blue color.
		color[0] = 0.482; color[1] = 0.62; color[2] = 0.871;
		// Create another CGColorRef for opaque blue.
		opaqueBlueColor = CGColorCreate(theColorSpace, color);
		// Create a new CGColorRef from the opaqueBlue CGColorRef 
		// but with a different alpha value.
		transparentBlueColor = CGColorCreateCopyWithAlpha(opaqueBlueColor, 0.5);
		if(!(opaqueRedColor && opaqueBlueColor && transparentBlueColor)){
		    fprintf(stderr, "Couldn't create one of the CGColorRefs!!!\n");
		    return;
		}
	}

	// Set the fill color to the opaque red CGColor object.
	CGContextSetFillColorWithColor(context, opaqueRedColor);
	// Set the stroke color to the opaque blue CGColor object.
	CGContextSetStrokeColorWithColor(context, opaqueBlueColor);
	
	CGContextSetLineWidth(context, 8.);
	// Draw the first rectangle.
	CGContextBeginPath(context);
	CGContextAddRect(context, CGRectMake(20., 20., 100., 100.));
	CGContextDrawPath(context, kCGPathFillStroke);
    
	// Set the stroke color to be that of the transparent blue 
	// CGColor object.
	CGContextSetStrokeColorWithColor(context, transparentBlueColor);	
	// Draw a second rectangle to the right of the first one.
	CGContextBeginPath(context);
 	CGContextAddRect(context, CGRectMake(140., 20., 100., 100.));
	CGContextDrawPath(context, kCGPathFillStroke);
}

void doIndexedColorDrawGraphics(CGContextRef context)
{
	CGColorSpaceRef theBaseRGBSpace = getTheCalibratedRGBColorSpace();
	CGColorSpaceRef theIndexedSpace = NULL;
	unsigned char lookupTable[6];
	float opaqueRed[] = { 0, 1 }; // index, alpha
	float aBlue[] = { 1, 1 };   // index, alpha
	
	// Set the first 3 values in the lookup table to a red of
	// 169/255 = 0.663, no green, and blue = 8/255 = 0.031. This makes  
	// the first entry in the lookup table a shade of red.
	lookupTable[0] = 169; lookupTable[1] = 0; lookupTable[2] = 8;
	
	// Set the second 3 values in the lookup table to a red value
	// of 123/255 = 0.482, a green value of 158/255 = 0.62, and
	// a blue value of 222/255 = 0.871. This makes the second entry
	// in the lookup table a shade of blue.
	lookupTable[3] = 123; lookupTable[4] = 158; lookupTable[5] = 222;
	
	// Create the indexed color space with this color lookup table,
	// using the RGB color space as the base color space and a 2 element
	// color lookup table to characterize the indexed color space.
	theIndexedSpace = CGColorSpaceCreateIndexed(theBaseRGBSpace, 1, lookupTable);
	if(theIndexedSpace != NULL){
	    CGContextSetStrokeColorSpace(context, theIndexedSpace);
	    CGContextSetFillColorSpace(context, theIndexedSpace);
	    // Release the color space this code created since it is no
		// longer needed in this routine.
	    CGColorSpaceRelease(theIndexedSpace);

	    // Set the stroke color to an opaque blue.
	    CGContextSetStrokeColor(context, aBlue);
	    // Set the fill color to an opaque red.
	    CGContextSetFillColor(context, opaqueRed);

	    CGContextSetLineWidth(context, 8.);
	 	// Draw the first rectangle.
	    CGContextBeginPath(context);
	    CGContextAddRect(context, CGRectMake(20., 20., 100., 100.));
	    CGContextDrawPath(context, kCGPathFillStroke);

	    // Continue to use the stroke colorspace already set
	    // but change the stroke alpha value to a semitransparent value
	    // while leaving the index value unchanged.
	    aBlue[1] = 0.5;
	    CGContextSetStrokeColor(context, aBlue);
	    // Draw another rectangle to the right of the first one.
	    CGContextBeginPath(context);
	    CGContextAddRect(context, CGRectMake(140., 20., 100., 100.));
	    CGContextDrawPath(context, kCGPathFillStroke);
	}else
	    fprintf(stderr, "Couldn't make the indexed color space!\n");
}

void drawWithGlobalAlpha(CGContextRef context)
{
	int i;
	CGRect rect = CGRectMake(40., 210., 100., 100.);
	float color[4] = { 1.0, 0.0, 0.0, 1.0 }; // opaque red
	// Set the fill color space to that returned by getTheCalibratedRGBColorSpace.
	CGContextSetFillColorSpace(context, getTheCalibratedRGBColorSpace());
	
	CGContextSetFillColor(context, color);
	for(i = 0; i < 2 ; i++){
		CGContextSaveGState(context);
			// Paint the leftmost rect on this row with 100% opaque red.
			CGContextFillRect(context, rect);
	    
			CGContextTranslateCTM(context, rect.size.width + 70., 0.);
			// Set the alpha value of this rgba color to 0.5.
			color[3] = 0.5;
			// Use the new color as the fill color in the graphics state.
			CGContextSetFillColor(context, color);
			// Paint the center rect on this row with 50% opaque red.
			CGContextFillRect(context, rect);

			CGContextTranslateCTM(context, rect.size.width + 70., 0.);
			// Set the alpha value of this rgba color to 0.25.
			color[3] = 0.25;
			// Use the new color as the fill color in the graphics state.
			CGContextSetFillColor(context, color);
			// Paint the rightmost rect on this row with 25% opaque red.
			CGContextFillRect(context, rect);
		CGContextRestoreGState(context);
		// After restoring the graphics state, the fill color is set to
		// that prior to calling CGContextSaveGState, that is, opaque
		// red. The coordinate system is also restored.
		
		// Now set the context global alpha value to 50% opaque.
		CGContextSetAlpha(context, 0.5);
		// Translate down for a second row of rectangles.
		CGContextTranslateCTM(context, 0., -(rect.size.height + 70.));
		// Reset the alpha value of the color array to fully opaque.
		color[3] = 1.0;
	}
}

void drawWithColorBlendMode(CGContextRef context, CFURLRef url)
{
    // A pleasant green color.
    float green[4] = { 0.584, 0.871, 0.318, 1.0 }; 
    CGRect insetRect, pdfRect;
    
    // Create a CGPDFDocument object from the URL.
    CGPDFDocumentRef pdfDoc = CGPDFDocumentCreateWithURL(url);
    if(pdfDoc == NULL){
		fprintf(stderr, "Couldn't create CGPDFDocument from URL!\n");
		return;
    }
    // Obtain the media box for page 1 of the PDF document.
    pdfRect = CGPDFDocumentGetMediaBox(pdfDoc, 1);
    // Set the origin of the rectangle to (0,0).
    pdfRect.origin.x = pdfRect.origin.y = 0;
    
    // Graphic 1, the left portion of the figure.
    CGContextTranslateCTM(context, 20, 10 + CGRectGetHeight(pdfRect)/2);
    
    // Draw the PDF document.
    CGContextDrawPDFDocument(context, pdfRect, pdfDoc, 1);
    
    // Set the fill color space to that returned by getTheCalibratedRGBColorSpace.
    CGContextSetFillColorSpace(context, getTheCalibratedRGBColorSpace());
    // Set the fill color to green.
    CGContextSetFillColor(context, green);
    
    // Graphic 2, the top-right portion of the figure.
    CGContextTranslateCTM(context, CGRectGetWidth(pdfRect) + 10, 
				    CGRectGetHeight(pdfRect)/2 + 10);

    // Draw the PDF document again.
    CGContextDrawPDFDocument(context, pdfRect, pdfDoc, 1);

    // Make a fill rectangle that is the same size as the PDF document
    // but inset each side by 80 units in x and 20 units in y.
    insetRect = CGRectInset(pdfRect, 80, 20);
    // Fill the rectangle with green. Because the fill color is opaque and
    // the blend mode is Normal, this obscures the drawing underneath. 
    CGContextFillRect(context, insetRect);
    
    // Graphic 3, the bottom-right portion of the figure.
    CGContextTranslateCTM(context, 0, -(10 + CGRectGetHeight(pdfRect)));

    // Draw the PDF document again.
    CGContextDrawPDFDocument(context, pdfRect, pdfDoc, 1);
    
    // Set the blend mode to kCGBlendModeColor which will
    // colorize the destination with subsequent drawing.
    CGContextSetBlendMode(context, kCGBlendModeColor);
    // Draw the rectangle on top of the PDF document. The portion of the
    // background that is covered by the rectangle is colorized
    // with the fill color.
    CGContextFillRect(context, insetRect);

    // Release the CGPDFDocumentRef the code created.
    CGPDFDocumentRelease(pdfDoc);
}


static void createEllipsePath(CGContextRef context, CGPoint center, 
											CGSize ellipseSize)
{
	CGContextSaveGState(context);
		// Translate the coordinate origin to the center point.
		CGContextTranslateCTM(context, center.x, center.y);
		// Scale the coordinate system to half the width and height
		// of the ellipse.
		CGContextScaleCTM(context, 
					ellipseSize.width/2, ellipseSize.height/2);
		CGContextBeginPath(context);
		// Add a circular arc to the path, centered at the origin and
		// with a radius of 1.0. This radius, together with the
		// scaling above for the width and height, produces an ellipse
		// of the correct size.
		CGContextAddArc(context, 0.0, 0.0, 1.0, 0.0, DEGREES_TO_RADIANS(360.0), 0.0);
		// Close the path so that this path is suitable for stroking,
		// should that be desired.
		CGContextClosePath(context);
	CGContextRestoreGState(context);
}

void doClippedEllipse(CGContextRef context)
{
    CGPoint theCenterPoint = { 120., 120. };
    CGSize theEllipseSize = { 100., 200. };
    float dash[1] = { 2 };
    static CGColorRef opaqueBrownColor = NULL, opaqueOrangeColor = NULL;
    // Initialize the CGColorRefs if necessary.
    if(opaqueBrownColor == NULL){
		// The initial value of the color array is an 
		// opaque brown in an RGB color space.
		float color[4] = { 0.325, 0.208, 0.157, 1.0 };
		CGColorSpaceRef theColorSpace = getTheCalibratedRGBColorSpace();
		// Create a CGColorRef for opaque brown.
		opaqueBrownColor = CGColorCreate(theColorSpace, color);
		// Make the color array correspond to an opaque orange.
		color[0] = 0.965 ; color[1] = 0.584; color[2] = 0.059;
		// Create another CGColorRef for opaque orange.
		opaqueOrangeColor = CGColorCreate(theColorSpace, color);
	}
	// Draw two ellipses centered about the same point, one
	// rotated 45 degrees from the other.
	CGContextSaveGState(context);
		// Ellipse 1
		createEllipsePath(context, theCenterPoint, theEllipseSize);
		CGContextSetFillColorWithColor(context, opaqueBrownColor);
		CGContextFillPath(context);
		// Translate and rotate about the center point of the ellipse.
		CGContextTranslateCTM(context, theCenterPoint.x, theCenterPoint.y);
		// Rotate by 45 degrees.
		CGContextRotateCTM(context, DEGREES_TO_RADIANS(45));
		// Ellipse 2
		// CGPointZero is a pre-defined Quartz point corresponding to 
		// the coordinate (0,0).
		createEllipsePath(context, CGPointZero, theEllipseSize);
		CGContextSetFillColorWithColor(context, opaqueOrangeColor);
		CGContextFillPath(context);
	CGContextRestoreGState(context);
	
	CGContextTranslateCTM(context, 170., 0.);
	// Now use the first ellipse as a clipping area prior to
	// painting the second ellipse.
	CGContextSaveGState(context);
		// Ellipse 3
		createEllipsePath(context, theCenterPoint, theEllipseSize);
		CGContextSetStrokeColorWithColor(context, opaqueBrownColor);
		CGContextSetLineDash(context, 0, dash, 1);
		// Stroke the path with a dash.
		CGContextStrokePath(context);
		// Ellipse 4
		createEllipsePath(context, theCenterPoint, theEllipseSize);
		// Clip to the elliptical path.
		CGContextClip(context);
		CGContextTranslateCTM(context, theCenterPoint.x, theCenterPoint.y);
		// Rotate by 45 degrees.
		CGContextRotateCTM(context, DEGREES_TO_RADIANS(45));
		// Ellipse 5
		createEllipsePath(context, CGPointZero, theEllipseSize);
		CGContextSetFillColorWithColor(context, opaqueOrangeColor);
		CGContextFillPath(context);
	CGContextRestoreGState(context);
}
