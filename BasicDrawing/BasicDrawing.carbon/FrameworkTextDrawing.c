/*
*  File:    FrameworkTextDrawing.c
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


#include "FrameworkTextDrawing.h"
#include "Utilities.h"

#define doPoint 1

#define DRAWUNICODECHARS 0

#if DRAWUNICODECHARS
static CFStringRef getTextString()
{
    static CFStringRef textString = NULL;
    if(textString == NULL){
		// These unicode values are the characters: Q, u, a, r, t, z, 
		// eighthnote, floral heart, black chess queen, and two CJK characters.
		const UniChar chars[] = {0x0051, 0x0075, 0x0061, 0x0072, 0x0074,  
				0x007A, 0x266A, 0x2766, 0x265B, 0x3042, 0x304E };   
		textString = CFStringCreateWithCharacters(NULL, chars, sizeof(chars)/sizeof(UniChar));
    }
    return textString;
}
#endif

void drawTextWithThemeText(CGContextRef context)
{
#if DRAWUNICODECHARS
	CFStringRef	textString = getTextString();
#else
    CFStringRef	textString = CFSTR("Quartz");
#endif
    
	float opaqueRed[] = { 0.663, 0., 0.031, 1. };
    float opaqueBlue[] = { 0.11, 0.208, 0.451, 1. };
    float opaqueGreen[] = { 0.404, 0.808, 0.239, 1. };

    /*	Make the text box origin be the location for the 
		starting point of the text. By making the box suitably large, 
		the text is never wrapped to the box, which is the desired result
		for this code. Note that the rectangle bounds the text so the 
		descender on the Q in Quartz is inside the text box. 
	*/
    static const CGRect textBox = {0., 0., 1000, 1000};
    HIThemeTextInfo textInfo = { 
			0, kThemeStateActive, 
			// This uses the system font and size. There are other
			// theme text fonts you can use but they are limited.
			kThemeApplicationFont, 
			kHIThemeTextHorizontalFlushLeft, 
			kHIThemeTextVerticalFlushBottom,
			kHIThemeTextBoxOptionNone, 
			kHIThemeTextTruncationNone,
			0,
			false
	    };

    // Set the fill color space. This sets the fill color to opaque black.
    CGContextSetFillColorSpace(context, getTheCalibratedRGBColorSpace());
    
    // Theme text drawing respects the context fill color.
    CGContextSetFillColor(context, opaqueRed);
  
    CGContextTranslateCTM(context, 10, 150);
    
    /*	Draw the text into the context. The orientation parameter 
		of kHIThemeOrientationInverted is used because the HI toolbox 
		notion of flipped is the default Quartz coordinate orientation, 
		which is what is being used here. 
	*/
    (void)HIThemeDrawTextBox(textString, &textBox, &textInfo, 
			    context, kHIThemeOrientationInverted);

#if doPoint
    drawPoint(context, textBox.origin);
#endif

    CGContextSaveGState(context);
	// HIThemeDrawTextBox works with the CTM as you would expect.
	CGContextTranslateCTM(context, 0, -40);
	CGContextScaleCTM(context, 2, 2);
    
	CGContextSetFillColor(context, opaqueBlue);

	(void)HIThemeDrawTextBox(textString, &textBox, &textInfo, 
		    context, kHIThemeOrientationInverted);

#if doPoint
	CGContextSaveGState(context);
	CGContextScaleCTM(context, 0.5, 0.5);
	drawPoint(context, textBox.origin);
	CGContextRestoreGState(context);
#endif

	CGContextSetStrokeColorSpace(context, getTheCalibratedRGBColorSpace());
	CGContextSetFillColor(context, opaqueGreen);
	CGContextTranslateCTM(context, 0, -40);
	CGContextScaleCTM(context, 2, 2);
	
	CGContextSetLineWidth(context, 0.5);
	// Theme text drawing respects the text drawing mode.
	CGContextSetTextDrawingMode(context, kCGTextFillStroke);
	
	(void)HIThemeDrawTextBox(textString, &textBox, &textInfo, 
			    context, kHIThemeOrientationInverted);
#if doPoint
	CGContextSaveGState(context);
	CGContextScaleCTM(context, 0.25, 0.25);
	drawPoint(context, textBox.origin);
	CGContextRestoreGState(context);
#endif

    CGContextRestoreGState(context);
}


/*	createTheATSUIStyle creates a simple ATSUI style record
    based on a PostScript font name and a point size. The style record
    can be used in calls to the ATSUI text drawing routines. */
static OSStatus createTheATSUIStyle(const char *postScriptFontName,
				    float fontSize, ATSUStyle *theStyle) 
{
    OSStatus err = noErr;
    ATSUStyle style;
    ATSFontRef atsFontRef;
    ATSUFontID atsuFont;
    Fixed atsuSize;

    /* Create three parallel arrays for setting up attributes. */
    ATSUAttributeTag theTags[2];
    ByteCount theSizes[2];
    ATSUAttributeValuePtr theValues[2];

    // Set up to return a NULL style to the caller if there is 
    // any error before the correct style is created. 
    *theStyle = NULL;
    
    style = NULL;
    atsuFont = 0;
    atsuSize = FloatToFixed(fontSize);

    err = ATSUFindFontFromName(postScriptFontName, strlen(postScriptFontName), 
				kFontPostscriptName, kFontNoPlatformCode,
				kFontNoScriptCode,  kFontNoLanguageCode, 
				&atsuFont);
    if(err){
		fprintf(stderr, "Couldn't get ATSU font!\n");
		return err;
    }

    // The 0 element of these three arrays is the ATSU font.
    theTags[0] = kATSUFontTag;
    theSizes[0] = sizeof(ATSUFontID);
    theValues[0] = &atsuFont;
    
    // The 1 element of these three arrays is the font size.
    theTags[1] = kATSUSizeTag;
    theSizes[1] = sizeof(Fixed);
    theValues[1] = &atsuSize;

    // Create an ATSU style.
    err = ATSUCreateStyle(&style);
    if(err){
		if(style)ATSUDisposeStyle(style);
		return err;
    } 

    // Set the attributes on the style.
    err = ATSUSetAttributes(style,
		sizeof(theTags)/sizeof(theTags[0]),
		theTags, theSizes, theValues );
    if(err){
		if(style)ATSUDisposeStyle(style);
		return err;
    } 

    // Pass the style created back to the caller.
    *theStyle = style;
    return noErr;
}

static OSStatus createLayoutForString(CFStringRef theString,
				ATSUStyle style, ATSUTextLayout *layoutP) 
{
    ATSUTextLayout theLayout = NULL;
    CFIndex textLength;
    OSStatus err = noErr;
    UniChar *uniBuffer;
    CFRange uniRange;
    
    // Set up to return a NULL layout to the caller if there is an 
    // error creating the desired layout.
    *layoutP = NULL;
    
    textLength = CFStringGetLength(theString);
    if (textLength == 0) return noErr;
    
    // Get the unicode data from the CFString.
    uniRange = CFRangeMake(0, textLength);
    uniBuffer = (UniChar *) malloc( textLength * sizeof(UniChar) );
    if (uniBuffer == NULL){ 
		return memFullErr; 
    }
    CFStringGetCharacters(theString,  uniRange, uniBuffer);

    // Create the ATSUI layout as a single run with the 
    // requested style.
    err = ATSUCreateTextLayoutWithTextPtr(uniBuffer, 0,
		textLength, textLength, 1,
		(unsigned long *) &textLength, &style,
		&theLayout);
    if(err){
		free(uniBuffer);
		return err;
    }
    
    *layoutP = theLayout;

    return err;
}

static OSStatus setContextForLayout(CGContextRef context, ATSUTextLayout theLayout)
{
    ByteCount iSize = sizeof(CGContextRef);
    ATSUAttributeTag iTag = kATSUCGContextTag;
    ATSUAttributeValuePtr iValuePtr = &context; 
    // Setting this tag on the layout causes ATSUI to draw to 
    // this context. 
    return ATSUSetLayoutControls( theLayout, 1, &iTag, 
					&iSize, &iValuePtr );
}

static inline OSStatus myATSUDrawTextAtPoint(ATSUTextLayout theLayout,
				UniCharCount numUnicodeChars,
				float x, float y)
{
     return ATSUDrawText(theLayout, 0, numUnicodeChars,
			    FloatToFixed(x), FloatToFixed(y));
}

static OSStatus myFlippedATSUDrawTextAtPoint(
				CGContextRef c,
				ATSUTextLayout theLayout,
				UniCharCount numUnicodeChars,
				float x, float y)
{
    OSStatus err;
    CGContextSaveGState(c);
	// Translate so that the origin is at the starting
	// point of the text.
	CGContextTranslateCTM(c, x, y);
	// Scale by -1 in y to produce an upright coordinate system.
	CGContextScaleCTM(c, 1, -1);
	// Draw the text.
	err = ATSUDrawText(theLayout, 0, numUnicodeChars, 0,0);
    // Restore to the graphics state in effect when this
    // function was called.
    CGContextRestoreGState(c);
    
    return err;
}


void drawTextWithATSUI(CGContextRef context)
{
    OSStatus err = noErr;
    static ATSUTextLayout theLayout = NULL;
#if DRAWUNICODECHARS
	CFStringRef	textString = getTextString();
#else
    CFStringRef	textString = CFSTR("Quartz");
#endif
    UniCharCount numTextChars = CFStringGetLength(textString);

    float opaqueRed[] = { 0.663, 0., 0.031, 1. };
    float fontSize = 60;
    
    // For this drawing routine the layout will be created the first time
    // it is called and that layout will be used upon subsequent redraws.
	if(theLayout == NULL){
		ATSUStyle theTextStyle = NULL;
		// Create the style to use for the text to draw.
		err = createTheATSUIStyle("Times-Roman", fontSize, &theTextStyle);
		if(err){
			fprintf(stderr, "Got error %d creating ATSU style!\n", err);
			return;
		}
		// Create the layout with the text and style.
		err = createLayoutForString(textString, theTextStyle, &theLayout);
		if(err){
			ATSUDisposeStyle(theTextStyle);
			fprintf(stderr, "Got error %d creating the layout!\n", err);
			return;
		}
		
#if DRAWUNICODECHARS
		// Set font matching to default fallbacks. This can cause multiple
		// text showing calls and doesn't work well with clipping
		// to text.
		err = ATSUSetTransientFontMatching(theLayout, true);
#endif

	}

    // Before drawing, the context to draw to must be
    // added to the layout. This must be done to the layout
    // each time the context to draw to changes. 
    err = setContextForLayout(context, theLayout);
    if(err){
		fprintf(stderr, "Got error %d setting the context on the layout!\n", err);
		return;
    }

    // Set the fill and stroke color space. This sets the 
    // fill and stroke painting color to opaque black.
    CGContextSetFillColorSpace(context, getTheCalibratedRGBColorSpace());
    CGContextSetStrokeColorSpace(context, getTheCalibratedRGBColorSpace());

    // --- Text Segment 1: filled text ---

    // ATSUI text drawing respects the fill color as long as there isn't a 
    // fill color style attribute on the layout. Set the fill color to red.
    CGContextSetFillColor(context, opaqueRed);

    CGContextTranslateCTM(context, 20, 220);

    err = myATSUDrawTextAtPoint(theLayout, numTextChars, 0, 0);
    if(err)fprintf(stderr, "Got error %d showing ATSUI text\n", err);

#if doPoint
    drawPoint(context, CGPointZero);
#endif

    
    // --- Text Segment 2: stroked text ---
    CGContextTranslateCTM(context, 0, -65);

    // Set the text drawing mode to stroke.
    CGContextSetTextDrawingMode(context, kCGTextStroke);

    err = myATSUDrawTextAtPoint(theLayout, numTextChars, 0, 0);
    if(err)fprintf(stderr, "Got error %d showing ATSUI text\n", err);

#if doPoint
    drawPoint(context, CGPointZero);
#endif

    // --- Text Segment 3: filled and stroked text ---
    CGContextTranslateCTM(context, 0, -65);

    // Set the text drawing mode to fill+stroke.
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    
    err = myATSUDrawTextAtPoint(theLayout, numTextChars, 0, 0);
    if(err)fprintf(stderr, "Got error %d showing ATSUI text\n", err);

#if doPoint
    drawPoint(context, CGPointZero);
#endif

    // --- Text Segment 4: filled and stroked text, then clipped ---
    CGContextTranslateCTM(context, 0, -65);

    CGContextSaveGState(context);
	// Set the text drawing mode to fill+stroke, then clip.
	CGContextSetTextDrawingMode(context, kCGTextFillStrokeClip);
	err = myATSUDrawTextAtPoint(theLayout, numTextChars, 0, 0);
	if(err)fprintf(stderr, "Got error %d showing ATSUI text\n", err);
    
	// Draw a set of grid lines, clipped by the text characters.
	drawGridLines(context); 
    CGContextRestoreGState(context);

#if doPoint
    drawPoint(context, CGPointZero);
#endif


#if 1	// Draw into a simulated flipped coordinate system. 
	// Set up the flipped coordinate system.
    CGContextTranslateCTM(context, 0, 600);
    CGContextScaleCTM(context, 1, -1);

#if doPoint
    drawPoint(context, CGPointMake(20,120));
#endif

    float darkBlue[] = { 0.11, 0.208, 0.451, 1. }; 
    CGContextSetFillColor(context, darkBlue);
    myATSUDrawTextAtPoint(theLayout, numTextChars, 20, 120);

#if doPoint
    drawPoint(context, CGPointMake(220,120));
#endif

    myFlippedATSUDrawTextAtPoint(context, theLayout, numTextChars, 220, 120);
#endif
}

