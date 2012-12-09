/*
*  File:    CoordinateSystem.c
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

#include "CoordinateSystem.h"
#include <math.h>		// for M_PI

void doRotatedEllipses(CGContextRef context)
{
	int i, totreps = 144;
	float  tint = 1.0, tintIncrement = 1.0/totreps;
	// Create a new transform consisting of a 45 degrees rotation.
	CGAffineTransform theTransform = CGAffineTransformMakeRotation(M_PI/4);
	// Apply a scale to the transform just created.
	theTransform = CGAffineTransformScale(theTransform, 1, 2);
	// Place the first ellipse at a good location.
	CGContextTranslateCTM(context, 100., 100.);
	
	for(i=0 ; i < totreps ; i++){
		// Make a snapshot the coordinate system.
		CGContextSaveGState(context);
			// Set up the coordinate system for the rotated ellipse.
			CGContextConcatCTM(context, theTransform);
			CGContextBeginPath(context);
			CGContextAddArc(context, 0., 0., 45., 0., 2*M_PI, 0); 
			// Set the fill color for this instance of the ellipse.
			CGContextSetRGBFillColor(context, tint, 0., 0., 1.0);
			CGContextDrawPath(context, kCGPathFill);
		// Restore the coordinate system to that of the snapshot.
		CGContextRestoreGState(context);
		// Compute the next tint color.
		tint -= tintIncrement;
		// Move over by 1 unit in x for the next ellipse.
		CGContextTranslateCTM(context, 1.0, 0.0);
	}
}

void drawSkewedCoordinateSystem(CGContextRef context)
{
    // alpha is 22.5 degrees and beta is 15 degrees.
    float alpha = M_PI/8, beta = M_PI/12;
    CGAffineTransform skew;
    // Create a rectangle that is 72 units on a side
    // with its origin at (0,0).
    CGRect r = CGRectMake(0, 0, 72, 72);
    
    CGContextTranslateCTM(context, 144, 144);
    // Draw the coordinate axes untransformed.
    drawCoordinateAxes(context);
    // Fill the rectangle.
    CGContextFillRect(context, r);

    // Create an affine transform that skews the coordinate system,
    // skewing the x-axis by alpha radians and the y-axis by beta radians. 
    skew = CGAffineTransformMake(1, tan(alpha), tan(beta), 1, 0, 0);
    // Apply that transform to the context coordinate system.
    CGContextConcatCTM(context, skew);

    // Set the fill and stroke color to a dark blue.
    CGContextSetRGBStrokeColor(context, 0.11, 0.208, 0.451, 1);
    CGContextSetRGBFillColor(context, 0.11, 0.208, 0.451, 1);
    
    // Draw the coordinate axes again, now transformed.
    drawCoordinateAxes(context);
    // Set the fill color again but with a partially transparent alpha.
    CGContextSetRGBFillColor(context, 0.11, 0.208, 0.451, 0.7);
    // Fill the rectangle in the transformed coordinate system.
    CGContextFillRect(context, r);
}
