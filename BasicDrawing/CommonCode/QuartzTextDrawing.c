/*
*  File:    QuartzTextDrawing.c
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

#include "QuartzTextDrawing.h"
#include "Utilities.h"

void drawQuartzRomanText(CGContextRef context)
{
    int i;
    static const char *text = "Quartz";
    size_t textlen = strlen(text);
    float fontSize = 60;
    
    float opaqueBlack[] = { 0., 0., 0., 1. };
    float opaqueRed[] = { 0.663, 0., 0.031, 1. };
    
    // Set the fill color space. This sets the 
    // fill painting color to opaque black.
    CGContextSetFillColorSpace(context, getTheCalibratedRGBColorSpace());
    
    // The Cocoa framework calls the draw method with an undefined
    // value of the text matrix. It's best to set it to what is needed by
    // this code: the identity transform.
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
       
    // Set the font with the PostScript name "Times-Roman", at
    // fontSize points, with the MacRoman encoding.
    CGContextSelectFont(context, "Times-Roman", fontSize, kCGEncodingMacRoman);

    // The default text drawing mode is fill. Draw the text at (70, 400).
    CGContextShowTextAtPoint(context, 70, 400, text, textlen);

    // Set the fill color to red.
    CGContextSetFillColor(context, opaqueRed);
    
    // Draw the next piece of text where the previous one left off.
    CGContextShowText(context, text, textlen);
    
    for(i = 0 ; i < 3 ; i++){
		// Get the current text pen position.
		CGPoint p = CGContextGetTextPosition(context);
		// Translate to the current text pen position.
		CGContextTranslateCTM(context, p.x, p.y);
		
		// Rotate clockwise by 90 degrees for the next
		// piece of text.
		CGContextRotateCTM(context, DEGREES_TO_RADIANS(-90));
		// Draw the next piece of text in blac at the origin.
		CGContextSetFillColor(context, opaqueBlack);
		CGContextShowTextAtPoint(context, 0, 0, text, textlen);
		// Draw the next piece of text where the previous piece
		// left off and paint it with red.
		CGContextSetFillColor(context, opaqueRed);
		CGContextShowText(context, text, textlen);
    }
}


static void myCGContextStrokeLineSegments(CGContextRef context, 
				const CGPoint s[], size_t count)
{
    // CGContextStrokeLineSegments is available only on Tiger and later
    // so if it isn't available, use an emulation of
    // CGContextStrokeLineSegments. It is better to use the
    // built-in CGContextStrokeLineSegments since it has significant
    // performance optimizations on some hardware.
    if(&CGContextStrokeLineSegments != NULL)
		CGContextStrokeLineSegments(context, s, count);
    else{
		size_t k;
		CGContextBeginPath(context);
		for (k = 0; k < count; k += 2) {
			CGContextMoveToPoint(context, s[k].x, s[k].y);
			CGContextAddLineToPoint(context, s[k+1].x, s[k+1].y);
		}
		CGContextStrokePath(context);
    }
    return;
}

void drawGridLines(CGContextRef context)
{
    static bool notInited = true;
    static CGPoint gridLines[240];
    int numlines = 60;
    
	// Only compute the grid lines the first time this routine is called.
	if(notInited){
		int i, stepsize = 4.;
		float val;
		for(i = 0, val=0 ; i < 2*numlines  ; i+= 2, val += stepsize){
			gridLines[i] = CGPointMake(val, -60);
			gridLines[i+1] = CGPointMake(val, 200);
		}
		
		for(i = 2*numlines, val = -20; i < 4*numlines  ; i+= 2, val += stepsize){
			gridLines[i] = CGPointMake(0, val);
			gridLines[i+1] = CGPointMake(400, val);
		}
		notInited = false;
	}

    size_t numPoints = sizeof(gridLines)/(sizeof(CGPoint));
    myCGContextStrokeLineSegments(context, gridLines,  numPoints);

}

void drawQuartzTextWithTextModes(CGContextRef context)
{
    int i,j;
    static const char *fillText = "Fill ";
    static const char *strokeText = "Stroke ";
    static const char *fillAndStrokeText = "FillStroke ";
    static const char *invisibleText = "Invisible ";
    static const char *clipText = "ClipText ";
    static const char *fillStrokeClipText = "FillStrokeClip ";
    float fontSize = 40, extraLeading = 5;
    float dash[2] = { 1, 1 };
    
    float opaqueRed[] = { 1., 0., 0., 1. };
    
    // Set the fill and stroke color space. This sets the 
    // fill and stroke painting color to opaque black.
    CGContextSetFillColorSpace(context, getTheCalibratedRGBColorSpace());
    CGContextSetStrokeColorSpace(context, getTheCalibratedRGBColorSpace());
    
    // The Cocoa framework calls the draw method with an undefined
    // value of the text matrix. It's best to set it to what is needed by
    // this code: the identity transform.
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
       
    // Set the font with the PostScript name "Times-Roman", at
    // fontSize points, with the MacRoman encoding.
    CGContextSelectFont(context, "Times-Roman", fontSize, kCGEncodingMacRoman);

    // ----  Text Line 1 ----

    // Default text drawing mode is fill. Draw the text at (10, 400).
    CGContextShowTextAtPoint(context, 10, 400, fillText, strlen(fillText));

    // Set the fill color to red.
    CGContextSetFillColor(context, opaqueRed);

    CGContextSetTextPosition(context, 180, 400);
    CGContextShowText(context, fillText, strlen(fillText));

    // Translate down for the next line of text.
    CGContextTranslateCTM(context, 0, -(fontSize + extraLeading));

    // ----  Text Line 2 ----
    
    // Now stroke the text by setting the text drawing mode
    // to kCGTextStroke. When stroking text, Quartz uses the stroke   
    // color in the graphics state. 
    CGContextSetTextDrawingMode(context, kCGTextStroke);
    CGContextShowTextAtPoint(context, 10, 400, strokeText, strlen(strokeText));
    
    // When stroking text, the line width and other gstate parameters
    // that affect stroking affect text stroking as well.
    CGContextSetLineWidth(context, 2);
    CGContextSetLineDash(context, 0, dash, 2);

    CGContextSetTextPosition(context, 180, 400);
    CGContextShowText(context, strokeText, strlen(strokeText));

    // Reset the line dash and line width to their defaults.
    CGContextSetLineDash(context, 0, NULL, 0);
    CGContextSetLineWidth(context, 1);

    // Translate down for the next line of text.
    CGContextTranslateCTM(context, 0, -(fontSize + extraLeading));

    // ----  Text Line 3 ----
    
    // Set the text drawing mode so that text is both filled and
    // stroked. This produces text that is filled with the fill
    // color and stroked with the stroke color.
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    CGContextShowTextAtPoint(context, 10, 400, fillAndStrokeText, strlen(fillAndStrokeText));

    // Now draw again with a thicker stroke width.
    CGContextSetLineWidth(context, 2);
    CGContextSetTextPosition(context, 180, 400);
    CGContextShowText(context, fillAndStrokeText, strlen(fillAndStrokeText));
    
    CGContextSetLineWidth(context, 1);
    CGContextTranslateCTM(context, 0, -(fontSize + extraLeading));

    // ----  Text Line 4 ----
    
    // Set the text drawing mode to invisible so that the next piece of
    // text does not appear. Quartz updates the text position as
    // if it had been drawn.
    CGContextSetTextDrawingMode(context, kCGTextInvisible);
    CGContextShowTextAtPoint(context, 10, 400, invisibleText, strlen(invisibleText));
    
    CGContextSetTextDrawingMode(context, kCGTextFill);

    CGContextSetTextPosition(context, 180, 400);
    CGContextShowText(context, fillText, strlen(fillText));

    CGContextTranslateCTM(context, 0, -(fontSize + extraLeading));

    // ----  Text Line 5 ----
    CGContextSaveGState(context);
		// Use the text as a clipping path.
		CGContextSetTextDrawingMode(context, kCGTextClip);
		CGContextShowTextAtPoint(context, 10, 400, clipText, strlen(clipText));
		
		// Position and draw a grid of lines.
		CGContextTranslateCTM(context, 10, 400);
		drawGridLines(context);
    CGContextRestoreGState(context);

    CGContextSaveGState(context);
		// The current text position is that after the last piece 
		// of text has been drawn. Since CGContextSaveGState/
		// CGContextRestoreGState do not affect the text position or 
		// the text matrix, the text position is that after the last 
		// text was "drawn", that drawn with the kCGTextClip mode 
		// above. This is where the next text drawn will go if it
		// isn't explicitly positioned.
		CGPoint nextTextPosition = CGContextGetTextPosition(context);
		
		// Draw so that the text is filled, stroked, and then used
		// the clip subsequent drawing.
		CGContextSetTextDrawingMode(context, kCGTextFillStrokeClip);

		// Explicitly set the text position.
		CGContextSetTextPosition(context, 180, 400);
		nextTextPosition = CGContextGetTextPosition(context);

		CGContextShowText(context, fillStrokeClipText, strlen(fillStrokeClipText));
		// Adjust the location of the grid lines so that they overlap the
		// text just drawn.
		CGContextTranslateCTM(context, nextTextPosition.x, nextTextPosition.y);
		// Draw the grid lines clipped by the text.
		drawGridLines(context);
    CGContextRestoreGState(context);
}

/*
	showFlippedTextAtPoint is a cover routine for CGContextShowText
	that is useful for drawing text in a coordinate system where the y axis 
	is flipped relative to the default Quartz coordinate system.

	This code assumes that the text matrix is only used to
	flip the text, not to perform scaling or any other
	possible use of the text matrix.

	This function preserves the a, b, c, and d components of
	the text matrix across its execution but updates the
	tx, ty components (the text position) to reflect the
	text just drawn. If all the text you draw is flipped, it 
	isn't necessary to continually set the text matrix. Instead
	you could simply call CGContextSetTextMatrix once with
	the flipped matrix each time your drawing  
	code is called.
 */
void showFlippedTextAtPoint(CGContextRef c, float x, float y, 
	const char *text, const size_t textLen)
{
    CGAffineTransform s;
    CGAffineTransform t = {1., 0., 0., -1., 0., 0.};
    CGPoint p;
    // Get the existing text matrix.
    s = CGContextGetTextMatrix(c);
    // Set the text matrix to the one that flips in y.
    CGContextSetTextMatrix(c, t);
    // Draw the text at the point.
    CGContextShowTextAtPoint(c, x, y, text, textLen);
    // Get the updated text position.
    p = CGContextGetTextPosition(c);
    // Update the saved text matrix to reflect the updated
    // text position.
    s.tx = p.x ; s.ty = p.y;
    // Reset to the text matrix in effect when this 
    // routine was called but with the text position updated.
    CGContextSetTextMatrix(c, s);
}


void drawQuartzTextWithTextMatrix(CGContextRef context)
{
    float fontSize = 60., extraLeading = 10.;
    static const char *text = "Quartz ";
    size_t textlen = strlen(text);
    CGPoint textPosition;
    CGAffineTransform t;

    // The Cocoa framework calls the draw method with an undefined
    // value of the text matrix. It's best to set it to what is needed by
    // this code. Initially that is the identity transform.
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
       
    // Set the font with the PostScript name "Times-Roman", at
    // fontSize points, with the MacRoman encoding.
    CGContextSelectFont(context, "Times-Roman", fontSize, kCGEncodingMacRoman);

    // ----  Text Line 1 ----

    // Draw the text at (10, 600).
    CGContextShowTextAtPoint(context, 10, 600, text, textlen);

    // Get the current text position. The text pen is at the trailing
    // point from the text just drawn. 
    textPosition = CGContextGetTextPosition(context);
    
    // Set the text matrix to one that flips text in y and sets
    // the text position to the user space coordinate (0,0).
    t = CGAffineTransformMake(1, 0, 0, -1, 0, 0);
    CGContextSetTextMatrix(context, t);
    
    // Set the text position to the point where the previous text ended.
    CGContextSetTextPosition(context, textPosition.x, textPosition.y);

    // Draw the text at the current text position. It will be drawn 
    // flipped in y, relative to the text drawn previously.
    CGContextShowText(context, text, textlen);

    // ----  Text Line 2 ----
    
    // Translate down for the next piece of text.
    CGContextTranslateCTM(context, 0, -(3*fontSize + extraLeading));

    CGContextSaveGState(context);
	// Change the text matrix to {1, 0, 0, 3, 0, 0}, which
	// scales text by a factor of 1 in x and 3 in y.
	// This scaling doesn't affect any drawing other than text
	// drawing since only text drawing is transformed by
	// the text matrix.
	t = CGAffineTransformMake(1, 0, 0, 3, 0, 0);
	CGContextSetTextMatrix(context, t);

	// This text is scaled relative to the previous text
	// because of the text matrix scaling.
	CGContextShowTextAtPoint(context, 10, 600, text, textlen);

    // This restores the graphics state to what it was at the time
    // of the last CGContextSaveGState, but since the text matrix
    // isn't part of the Quartz graphics state, it isn't affected.
    CGContextRestoreGState(context);

    // The text matrix isn't affected by CGContextSaveGState and
    // CGContextRestoreGState. You can see this by observing that
    // the next text piece appears immediately after the first piece
    // and with the same text scaling as that text drawn with the
    // text matrix established before we did CGContextRestoreGState.
    CGContextShowText(context, text, textlen);

    // ----  Text Line 3 ----
    // Translate down for the next piece of text.
    CGContextTranslateCTM(context, 0, -(fontSize + extraLeading));

    // Reset the text matrix to the identity matrix.
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
    
    // Now draw text in a flipped coordinate system. 
    CGContextSaveGState(context);
	// Flip the coordinate system to mimic a coordinate system with the origin
	// at the top-left corner of a window. The new origin is at 600 units in
	// +y from the old origin and the y axis now increases with positive y
	// going down the window.
	CGContextConcatCTM(context, CGAffineTransformMake(1, 0, 0, -1, 0, 600));
	// This text will be flipped along with the CTM.
 	CGContextShowTextAtPoint(context, 10, 10, text, textlen);
	// Obtain the user space coordinates of the current text position.
	textPosition = CGContextGetTextPosition(context);
	// Draw text at that point but flipped in y.
	showFlippedTextAtPoint(context, textPosition.x, textPosition.y, text, textlen);
    CGContextRestoreGState(context);
}

