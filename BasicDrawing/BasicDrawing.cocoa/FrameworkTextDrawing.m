/*
*  File:    FrameworkTextDrawing.m
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


#import "FrameworkTextDrawing.h"
#import "Utilities.h"

static NSString *getTextString()
{
    static NSString *textString = nil;
    if(textString == nil){
		// These unicode values are the characters: Q, u, a, r, t, z, 
		// eighthnote, floral heart, black chess queen, and two CJK characters.
		const unichar chars[] = {0x0051, 0x0075, 0x0061, 0x0072, 0x0074,  
				0x007A, 0x266A, 0x2766, 0x265B, 0x3042, 0x304E };   
		textString = [NSString stringWithCharacters:chars 
					length: sizeof(chars)/sizeof(unichar)];
		[textString retain];			
    }
    return textString;
}

#define doPointDrawing 1

void drawNSStringWithAttributes(void)
{
    NSString *textString = getTextString();
    NSPoint p;
    NSFont *font;
    NSColor *redColor;
    NSMutableDictionary *stringAttributes;
#if doPointDrawing
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif
    
    // Text Line 1. Draw with default attributes.
    p = NSMakePoint(20., 400.);
        
    // Draw text with default text attributes. The point supplied is
    // not the text baseline but rather the lower-left corner of the box 
    // which bounds the text.
    [ textString drawAtPoint:p withAttributes:nil];

#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif

    // Text Line 2. Draw with a specific font and color.
    
    // Position the text 50 units below the previous text.
    p.y -= 50;

    // Set attributes to use when drawing the string.
    stringAttributes = [NSMutableDictionary dictionaryWithCapacity:4];
    
    // Use the font with the PostScript name "Times-Roman" at 40 point.
    font = [ NSFont fontWithName:@"Times-Roman" size: 40];
    [stringAttributes setObject:font forKey:NSFontAttributeName];

    // Set the color attribute to an opaque red.
    redColor = [NSColor colorWithCalibratedRed:0.663 green:0 blue:0.031 alpha:1.0];
    
    [stringAttributes setObject:redColor forKey:NSForegroundColorAttributeName];

    // Draw the text.
    [ textString drawAtPoint:p withAttributes:stringAttributes];

#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif

    // Text Line 3. Draw stroked text.
    
    // Position the text 50 units below the previous text.
    p.y -= 50;

    // Panther and later support stroke attributes. A positive value 
    // of the stroke width attribute produces text that is stroked rather
    // than filled.
    [ stringAttributes setObject:[NSNumber numberWithFloat: 3.0] 
				forKey:NSStrokeWidthAttributeName];
    [ textString drawAtPoint:p withAttributes:stringAttributes];

#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif

    // Text Line 4. Draw with fill and stroke.
    
    p.y -= 50;

    // Panther and later support stroke attributes. A negative value 
    // of the stroke width attribute results in text that is both filled
    // and stroked.
    [stringAttributes setObject:  [NSNumber numberWithFloat: -3.0] 
	    forKey:NSStrokeWidthAttributeName];

    // Set the stroke color attribute to black.
    [stringAttributes setObject:
	    [NSColor colorWithCalibratedRed:0 green:0 blue:0 alpha:1.0]
	    forKey:NSStrokeColorAttributeName];
	
    [textString drawAtPoint:p withAttributes:stringAttributes];

#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif

    // Text Line 5. Draw at baseline.
    // Tiger and later support the drawWithRect method which allows 
    // string text drawing from a point on the text baseline. 
    NSRect rect;
    p.y -= 50;
    rect.origin = p;
    rect.size = NSMakeSize(0,0);
    [ textString drawWithRect:rect options:
		NSStringDrawingDisableScreenFontSubstitution 
		attributes:stringAttributes ];
#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif


}

void drawWithNSLayout(void)
{
    static NSLayoutManager *myLayout = nil;
    static NSTextStorage *textStorage = nil;
    static NSRange myTextRange;
    NSPoint p;
	if(myLayout == nil){
		NSTextContainer *textContainer;
		NSMutableDictionary *stringAttributes;
		NSFont *font;
		NSColor *redColor;
		
		// Initialize the text storage with the string to draw.
		textStorage = [ [NSTextStorage alloc] initWithString: getTextString()];
		// Initialize the layout manager to use with the text storage.
		myLayout = [ [NSLayoutManager alloc] init];
		// Allocate and initialize a text container object.
		textContainer = [ [NSTextContainer alloc] init];
		// Add the text container to the layout.
		[myLayout addTextContainer:textContainer];
		// Release the text container since the layout retains it and
		// this code no longer needs it.
		[textContainer release];
		// Add the layout to the text storage.
		[textStorage addLayoutManager:myLayout];
		// The text storage retains the layout so this code can release it.
		[myLayout release];

		// Set attributes to use when drawing the string.
		stringAttributes = [NSMutableDictionary dictionaryWithCapacity:2];

		// Use the font with the PostScript name "Times-Roman" at 40 point.
		font = [ NSFont fontWithName:@"Times-Roman" size: 40];
		[stringAttributes setObject:font forKey:NSFontAttributeName];

		// Set the text color attribute to an opaque red.
		redColor = [NSColor colorWithCalibratedRed:0.663 green:0 blue:0.031 alpha:1.0];
		[stringAttributes setObject:redColor forKey:NSForegroundColorAttributeName];
		
		// Create the range of text for the entire length of text
		// in the textStorage object.
		myTextRange = NSMakeRange(0, [textStorage length]);
		// Set the attributes on the entire range of text. 
		[textStorage setAttributes:stringAttributes range:myTextRange];
	}

    // Set the point for drawing the layout. 
    p = NSMakePoint(20., 400.);

    // Draw the text range at the point.
    [myLayout drawGlyphsForGlyphRange:myTextRange atPoint:p];

#if doPointDrawing
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    drawPoint(context, *(CGPoint *)&p);
#endif

}

// A clipping procedure that will be used by the
// subclass of NSLayoutManager defined by the code that follows.
typedef void (ClippingDrawProc)(CGContextRef c, float x, 
					float y, void *info);

// The interface to the NSLayoutManager subclass.
@interface MyNSLayoutManager : NSLayoutManager
{
    // The extra instance variables for this subclass. 
    CGTextDrawingMode _textMode;
    CGColorRef _fColor;
    CGColorRef _sColor;
    float _yStartPosition;
    float _lineWidth;
    ClippingDrawProc *_clippingDrawProc;
    void *_clippingInfo;
}
- (void)setTextMode:(CGTextDrawingMode)textMode;
- (void)setFillColor:(CGColorRef)color;
- (void)setStrokeColor:(CGColorRef)color;
- (void)setTextLineWidth:(float)width;
- (void)setClippingDrawProc:(ClippingDrawProc *)clippingDrawProc 
					    withInfo:(void *)info;
@end

// The implementation of the MyNSLayoutManager custom subclass.
@implementation MyNSLayoutManager

// Public methods to set the special attributes
// of the MyNSLayoutManager instance.
- (void)setTextMode:(CGTextDrawingMode)textMode{
	_textMode = textMode;
}

- (void)setFillColor:(CGColorRef)color{ 
		CGColorRetain(color);
		CGColorRelease(_fColor);
		_fColor = color;
}

- (void)setStrokeColor:(CGColorRef)color{
		CGColorRetain(color);
		CGColorRelease(_sColor);
		_sColor = color;
}

- (void)setTextLineWidth:(float)width{
	_lineWidth = width;
}

- (void)setClippingDrawProc:(ClippingDrawProc *)clippingDrawProc withInfo:(void *)info{
	_clippingDrawProc = clippingDrawProc;
	_clippingInfo = info;
}

- (id)init
{
    if (self = [super init]) {
		// Initialize the custom instance variables.
		_textMode = kCGTextFill;
		_fColor = nil;
		_sColor = nil;
		_yStartPosition = 0;
		_lineWidth = 1;
		_clippingDrawProc = NULL;
		_clippingInfo = NULL;
    }
    return self;
}

// This class needs a dealloc method to ensure that 
// the retained fill and stroke colors are released.
- (void)dealloc {
    CGColorRelease(_sColor);
    CGColorRelease(_fColor);
    [super dealloc];
}

// This code overrides this method to record the y coordinate
// to use as the true baseline for the text drawing. 
- (void)drawGlyphsForGlyphRange:(NSRange)glyphsToShow 
				atPoint:(NSPoint)origin
{
    _yStartPosition = origin.y;
    [ super drawGlyphsForGlyphRange:glyphsToShow atPoint:origin];
}

// This is the rendering method of NSLayoutManager that the
// code overrides to perform its custom rendering.
- (void)showPackedGlyphs:(char *)glyphs 
			length:(unsigned)glyphLen 
			glyphRange:(NSRange)glyphRange 
			atPoint:(NSPoint)point 
			font:(NSFont *)font 
			color:(NSColor *)color 
			printingAdjustment:(NSSize)printingAdjustment 
{
   // Obtain the destination drawing context.
    CGContextRef context = (CGContextRef)[
		[NSGraphicsContext currentContext] graphicsPort];

    // Adjust start position y value based on the adjusted y coordinate.
    // This ensures the text baseline is at the starting position
    // passed to drawGlyphsForGlyphRange. This technique won't work
    // for super, subscripts, or underlines but that's OK for this example.
    point.y = _yStartPosition;

    // The Quartz graphics state should be preserved by showPackedGlyphs.
    CGContextSaveGState(context);
    
    // Set the desired text drawing mode.
    CGContextSetTextDrawingMode(context, _textMode);
    
	// Set the fill color if needed.
    if( _textMode == kCGTextFill || _textMode == kCGTextFillStroke || 
		_textMode == kCGTextFillClip || _textMode == kCGTextFillStrokeClip)
    {
	    if(_fColor) CGContextSetFillColorWithColor(context, _fColor);
    }    
    
	// Set the  line width and the stroke color if needed.
    if( _textMode == kCGTextStroke || _textMode == kCGTextFillStroke || 
		_textMode == kCGTextStrokeClip || _textMode == kCGTextFillStrokeClip)
    {
		CGContextSetLineWidth(context, _lineWidth);
		if(_sColor) CGContextSetStrokeColorWithColor(context, _sColor);
    }    
    
	// Now draw the text. Check whether to adjust for printing widths
    // and if needed adjust extra character spacing accordingly.
    if(printingAdjustment.width != 0.0){
		// If printingAdjustment width is non-zero then the text 
		// needs to be adjusted. printingAdjustment is the per character
		// adjustment required for this piece of text. Because
		// the Quartz text character spacing set is transformed by
		// the text matrix, this code needs to factor out that effect
		// prior to setting it. Cocoa sets the text matrix to account
		// for the point size of the font so we factor that out of the
		// per character width supplied here.
		float charAdjust = printingAdjustment.width / [ font pointSize ];
		CGContextSetCharacterSpacing(context, charAdjust); 
    } else {
		CGContextSetCharacterSpacing(context, 0.0);
    }

    // Draw the glyphs. The total number of glyphs is the length
    // of the glyphs string passed to showPackedGlyphs, divided by 2 
    // since there are two bytes per glyph.
    CGContextShowGlyphsAtPoint(context, point.x, point.y, 
			(CGGlyph *)glyphs, glyphLen/2);

    // If the text drawing mode requires clipping and there is
    // a custom clipping proc, call it. This allows drawing through
    // clipped text before the graphics state is restored.
    if( (	_textMode == kCGTextClip || _textMode == kCGTextFillClip || 
			_textMode == kCGTextStrokeClip || 
			_textMode == kCGTextFillStrokeClip )
			&& _clippingDrawProc != NULL
	)	
		_clippingDrawProc(context, point.x, point.y, _clippingInfo);
    
    CGContextRestoreGState(context);
}
@end

void MyClipProc(CGContextRef c, float x, float y, void *info)
{
    CGContextTranslateCTM(c, x, y);
    CGContextSetStrokeColorWithColor(c, getRGBOpaqueBlackColor());
	// Draw a grid of lines through the clip.    
	drawGridLines(c); 
}

void drawWithCustomNSLayout(void)
{
    static MyNSLayoutManager *myLayout = nil;
    static NSTextStorage *textStorage = nil;
    static NSRange myTextRange;
    NSPoint p;
	if(myLayout == nil){
		NSTextContainer *textContainer = [[NSTextContainer alloc] init];
		NSMutableDictionary *stringAttributes;
		
		textStorage = [[NSTextStorage alloc] initWithString:getTextString()];
		// Create an instance of the MyNSLayoutManager subclass of NSLayoutManager.
		myLayout = [[MyNSLayoutManager alloc] init];
		[myLayout addTextContainer:textContainer];
		// The layout retains the text container so this code can release it.
		[textContainer release];
		[textStorage addLayoutManager:myLayout];
		// The text storage retains the layout so this code can release it.
		[myLayout release];

		// Set attributes to use when drawing the string.
		stringAttributes = [NSMutableDictionary dictionaryWithCapacity:2];

		// Use the font with the PostScript name "Times-Roman" at 40 point.
		[stringAttributes setObject:[ NSFont fontWithName:@"Times-Roman" size: 40]
			forKey:NSFontAttributeName];

		// Create the range.
		myTextRange = NSMakeRange(0, [textStorage length]);
		// Set the attributes on the entire range of text. 
		[textStorage setAttributes:stringAttributes range:myTextRange];
	}

    p = NSMakePoint(20., 400.);

    // Set the custom attributes of the layout subclass so that
    // the text will be filled with black.
    [myLayout setTextMode: kCGTextFill];
    [myLayout setFillColor: getRGBOpaqueBlackColor()];
    
    // Draw text line 1.    
    [myLayout drawGlyphsForGlyphRange:myTextRange atPoint:p];

#if doPointDrawing
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    drawPoint(context, *(CGPoint *)&p);
#endif

    // Set the custom attributes of the layout subclass so that
    // the text will be stroked with black.
    [myLayout setTextMode: kCGTextStroke];
    [myLayout setStrokeColor: getRGBOpaqueBlackColor()];
    [myLayout setTextLineWidth:2];

    // Draw text line 2.    
    p.y -= 50; 
    [myLayout drawGlyphsForGlyphRange:myTextRange atPoint:p];

#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif

    p.y -= 50; 

    // Set the custom attributes of the layout subclass so that
    // the text will be filled and stroked and the fill color
    // will be red. Since the stroke color hasn't changed it
    // will be stroked with black.
    [myLayout setTextMode: kCGTextFillStroke];
    [myLayout setFillColor: getRGBOpaqueRedColor()];
    // Draw text line 3.    
    [myLayout drawGlyphsForGlyphRange:myTextRange atPoint:p];

#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif

    p.y -= 50; 

    // Set the custom attributes of the layout subclass so that
    // the text will be filled, stroked, then clipped.
    [myLayout setTextMode: kCGTextFillStrokeClip];
    
    // Set the clipping proc to MyClipProc which requires
    // no info data.
    [myLayout setClippingDrawProc: &MyClipProc withInfo:NULL];
    
    // Draw text line 4.    
    [myLayout drawGlyphsForGlyphRange:myTextRange atPoint:p];
    
#if doPointDrawing
    drawPoint(context, *(CGPoint *)&p);
#endif
    // Set the clipping proc to NULL for future drawing.
    [myLayout setClippingDrawProc:NULL withInfo:NULL];

}
