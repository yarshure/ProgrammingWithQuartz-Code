/*
*  File:    DrawingBasics.c
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

#include <math.h>		// for M_PI
#include "DrawingBasics.h"

void doSimpleRect(CGContextRef context)
{
	CGRect ourRect;

	// Set the fill color to opaque red.
	CGContextSetRGBFillColor(context, 1.0, 0.0, 0.0, 1.0);
	// Set up the rectangle for drawing.
	ourRect.origin.x = ourRect.origin.y = 20.0;
	ourRect.size.width = 130.0;
	ourRect.size.height = 100.0;
	// Draw the filled rectangle.
	CGContextFillRect(context, ourRect);
}

void doStrokedRect(CGContextRef context)
{
	CGRect ourRect;
	// Set the stroke color to a light opaque blue.
	CGContextSetRGBStrokeColor(context, 0.482, 0.62, 0.871, 1.);
	// Set up the rectangle for drawing.
	ourRect.origin.x = ourRect.origin.y = 20.0;
	ourRect.size.width = 130.0;
	ourRect.size.height = 100.0;
	// Draw the stroked rectangle with a line width of 3.
	CGContextStrokeRectWithWidth(context, ourRect, 3.0);
}

void doStrokedAndFilledRect(CGContextRef context)
{
	// Define a rectangle to use for drawing.
	CGRect ourRect = {{20., 220.}, {130., 100.}};
	 
	// ***** Rectangle 1 *****
	// Set the fill color to a light opaque blue.
	CGContextSetRGBFillColor(context, 0.482, 0.62, 0.871, 1.);
	// Set the stroke color to an opaque green.
	CGContextSetRGBStrokeColor(context, 0.404, 0.808, 0.239, 1.);
	// Fill the rect.
	CGContextFillRect(context, ourRect);
	// ***** Rectangle 2 *****
	// Move the rectangle’s origin to the right by 200 units.
	ourRect.origin.x += 200.;
	// Stroke the rectangle with a line width of 10.
	CGContextStrokeRectWithWidth(context, ourRect, 10.);
	// ***** Rectangle 3 *****
	// Move the rectangle’s origin to the left by 200 units
	// and down by 200 units.
	ourRect.origin.x -= 200.;
	ourRect.origin.y -= 200.;
	// Fill then stroke the rect with a line width of 10.
	CGContextFillRect(context, ourRect);
	CGContextStrokeRectWithWidth(context, ourRect, 10.);
	// ***** Rectangle 4 *****
	// Move the rectangle’s origin to the right by 200 units.
	ourRect.origin.x += 200.;
	// Stroke then fill the rect.
	CGContextStrokeRectWithWidth(context, ourRect, 10.);
	CGContextFillRect(context, ourRect);
}

void createRectPath(CGContextRef context, CGRect rect)
{
	// Create a path using the coordinates of the rect passed in.
	CGContextBeginPath(context);
	CGContextMoveToPoint(context, rect.origin.x, rect.origin.y);
	// ***** Segment 1 *****
	CGContextAddLineToPoint(context, rect.origin.x + rect.size.width,
					 							rect.origin.y);
	// ***** Segment 2 *****
	CGContextAddLineToPoint(context, rect.origin.x + rect.size.width,
								rect.origin.y + rect.size.height);
	// ***** Segment 3 *****
	CGContextAddLineToPoint(context, rect.origin.x, 
									rect.origin.y + rect.size.height);
	// ***** Segment 4 is created by closing the path *****
	CGContextClosePath(context);
}

void doPathRects(CGContextRef context)
{
	// Define a rectangle to use for drawing.
	CGRect ourRect = {{20., 220.}, {130., 100.}};

	// ***** Rectangle 1 *****
	// Create the rect path.
	createRectPath (context, ourRect);
	// Set the fill color to a light opaque blue.
	CGContextSetRGBFillColor(context, 0.482, 0.62, 0.871, 1.);
	// Fill the path.
	CGContextDrawPath (context, kCGPathFill); // Clears the path.
	// ***** Rectangle 2 *****
	// Translate the coordinate system 200 units to the right.
	CGContextTranslateCTM(context, 200., 0.);
	// Set the stroke color to an opaque green.
	CGContextSetRGBStrokeColor(context, 0.404, 0.808, 0.239, 1.);
	createRectPath (context, ourRect);
	// Set the line width to 10 units.
	CGContextSetLineWidth (context, 10.);
	// Stroke the path.
	CGContextDrawPath (context, kCGPathStroke);  // Clears the path.
	// ***** Rectangle 3 *****
	// Translate the coordinate system 
	// 200 units to the left and 200 units down.
	CGContextTranslateCTM(context, -200., -200.);
	createRectPath (context, ourRect);
	//CGContextSetLineWidth(context, 10.);	// This is redundant.
	// Fill, then stroke the path.
	CGContextDrawPath (context, kCGPathFillStroke);  // Clears the path.
	// ***** Rectangle 4 *****
	// Translate the coordinate system 200 units to the right.
	CGContextTranslateCTM(context, 200., 0.);
	createRectPath (context, ourRect);
	// Stroke the path.
	CGContextDrawPath (context, kCGPathStroke);  // Clears the path.
	// Create the path again.
	createRectPath (context, ourRect);
	// Fill the path.
	CGContextDrawPath (context, kCGPathFill); // Clears the path.

}

void doAlphaRects(CGContextRef context)
{
	// ***** Part 1 *****
	CGRect ourRect = {{0.0, 0.0}, {130.0, 100.0}};
	int i, numRects = 6;
	float rotateAngle = 2*M_PI/numRects;
	float tint, tintAdjust = 1./numRects;
	
	// ***** Part 2 *****
	CGContextTranslateCTM (context, 2*ourRect.size.width, 
		 			2*ourRect.size.height);
	
	// ***** Part 3 *****
	for(i = 0, tint = 1.0; i < numRects ; i++, tint -= tintAdjust){
		CGContextSetRGBFillColor (context, tint, 0.0, 0.0, tint);
		CGContextFillRect(context, ourRect);
		// These transformations are cummulative.
		CGContextRotateCTM(context, rotateAngle);
	} 
	
}

static void drawStrokedLine(CGContextRef context, CGPoint start, CGPoint end)
{
	CGContextBeginPath(context);
	CGContextMoveToPoint(context, start.x, start.y);
	CGContextAddLineToPoint(context, end.x, end.y);
	CGContextDrawPath(context, kCGPathStroke);
}

void doDashedLines(CGContextRef context)
{
	CGPoint start, end;
	float lengths[6] = { 12.0, 6.0, 5.0, 6.0, 5.0, 6.0 };
	
	start.x = 20.; start.y = 270.;
	end.x = 300.; end.y = 270.;
 	// ***** Line 1 solid line *****
	CGContextSetLineWidth(context, 5.);
	drawStrokedLine(context, start, end);
 	// ***** Line 2 long dashes *****
	CGContextTranslateCTM(context, 0., -50.);
	CGContextSetLineDash(context, 0., lengths, 2);
	drawStrokedLine(context, start, end);
 	// ***** Line 3 long short pattern *****
	CGContextTranslateCTM(context, 0., -50.);
	CGContextSetLineDash(context, 0., lengths, 4);
	drawStrokedLine(context, start, end);
 	// ***** Line 4 long short short pattern *****
	CGContextTranslateCTM(context, 0., -50.);
	CGContextSetLineDash(context, 0., lengths, 6);
	drawStrokedLine(context, start, end);
 	// ***** Line 5 short short long pattern *****
	CGContextTranslateCTM(context, 0., -50.);
	CGContextSetLineDash(context, lengths[0]+lengths[1], lengths, 6);
	drawStrokedLine(context, start, end);
 	// ***** Line 6 solid line *****
	CGContextTranslateCTM(context, 0., -50.);
	// Reset dash to solid line.
	CGContextSetLineDash(context, 0, NULL, 0);
	drawStrokedLine(context, start, end);
}

void doClippedCircle(CGContextRef context)
{
	CGPoint circleCenter = {150., 150.};
	float   circleRadius = 100.0;
	float   startingAngle = 0.0, endingAngle = 2*M_PI;
	CGRect ourRect = { {65.,65.} , {170., 170.} };
	
	// ***** Filled Circle ***** 
	CGContextSetRGBFillColor (context, 0.663, 0., 0.031, 1.0);
	CGContextBeginPath(context);
	// Construct the circle path counterclockwise.
	CGContextAddArc(context, circleCenter.x, 
					circleCenter.y, circleRadius, 
					startingAngle, endingAngle, 0); 
	CGContextDrawPath(context, kCGPathFill);

	// ***** Stroked Square ***** 
	CGContextStrokeRect(context, ourRect);
	
	// Translate so that the next drawing doesn’t overlap what 
	// has already been drawn.
	CGContextTranslateCTM(context, ourRect.size.width + circleRadius + 5., 0);
	// Create a rectangular path and clip to that path.
	CGContextBeginPath(context);
	CGContextAddRect(context, ourRect);
	CGContextClip(context);
	
	// ***** Clipped Circle *****
	CGContextBeginPath(context);
	// Construct the circle path counterclockwise.
	CGContextAddArc (context, circleCenter.x, 
					circleCenter.y, circleRadius,
					startingAngle, endingAngle, 0);
	CGContextDrawPath(context, kCGPathFill);
}

void doPDFDocument(CGContextRef context, CFURLRef url)
{
	CGRect pdfRect;
	CGPDFDocumentRef pdfDoc = CGPDFDocumentCreateWithURL(url);
	if(pdfDoc != NULL){
		CGContextScaleCTM(context, .5, .5);
		// The media box is the bounding box of the PDF document.
		pdfRect = CGPDFDocumentGetMediaBox(pdfDoc, 1); // page 1
		// Set the destination rect origin to the Quartz origin.
		pdfRect.origin.x = pdfRect.origin.y = 0.;
		// Draw page 1 of the PDF document.
		CGContextDrawPDFDocument(context, pdfRect, pdfDoc, 1);
		
		CGContextTranslateCTM(context, pdfRect.size.width*1.2, 0);
		// Scale non-uniformly making the y coordinate scale 1.5 times
		// the x coordinate scale.
		CGContextScaleCTM(context, 1, 1.5);
		CGContextDrawPDFDocument(context, pdfRect, pdfDoc, 1);
		
		CGContextTranslateCTM(context, pdfRect.size.width*1.2, pdfRect.size.height);
		// Flip the y coordinate axis horizontally about the x axis.
		CGContextScaleCTM(context, 1, -1);
		CGContextDrawPDFDocument(context, pdfRect, pdfDoc, 1);
		CGPDFDocumentRelease(pdfDoc);
	}else
		fprintf(stderr, "Can't create PDF document for URL!\n");
}

