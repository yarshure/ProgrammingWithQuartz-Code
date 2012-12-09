/*
*  File:    PatternDrawing.c
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

#include "PatternDrawing.h"
#include "Utilities.h"

static CGSize scalePatternPhase(CGSize phase)
{
	// Adjust the pattern phase if scaling to export as bits. This is equivalent to scaling base
	// space by the scaling factor.
	float patternScaling = getScalingFactor();
    if(patternScaling != 1.0)
		phase = CGSizeApplyAffineTransform(phase, 
					CGAffineTransformMakeScale(patternScaling, patternScaling));

	return phase;
}

static CGAffineTransform scalePatternMatrix(CGAffineTransform patternTransform)
{
	// Scale the pattern by the scaling factor when exporting to bits. This is equivalent to
	// scaling base space by the scaling factor.
	float patternScaling = getScalingFactor();
    if(patternScaling != 1.0)
		patternTransform = CGAffineTransformConcat(patternTransform, 
								CGAffineTransformMakeScale(patternScaling, patternScaling));

	return patternTransform;
}


static void myDrawRedBlackCheckerBoardPattern(void *info, CGContextRef patternCellContext)
{
    /*
		This pattern proc draws a red and a black rectangle 
		patch representing the minimum cell needed to paint a 
		checkerboard with that pattern.
		
		Each 'cell' of the checkerboard is 2 units on a side.
		
		This code uses CGColorRefs which are available in Panther  
		and later only. Patterns are available in all versions of  
		Mac OS X but this code uses CGColorRefs for convenience 
		and efficiency.    
    */
    // Paint a black checkerboard box.  
    CGContextSetFillColorWithColor(patternCellContext, getRGBOpaqueBlackColor());
    // This is a 1x1 unit rect whose origin is at 0,0 in pattern space.
    CGContextFillRect(patternCellContext, CGRectMake(0., 0., 1., 1.));
    // This is a 1x1 unit rect whose origin is at 1,1 in pattern space.
    CGContextFillRect(patternCellContext, CGRectMake(1., 1., 1., 1.));

    // Paint a red checkerboard box.
    CGContextSetFillColorWithColor(patternCellContext, getRGBOpaqueRedColor());
    // This is a 1x1 unit rect whose origin is at 1,0 in pattern space,
    // that is, immediately to the right of first black checkerboard box.
    CGContextFillRect(patternCellContext, CGRectMake(1., 0., 1., 1.));
    // This is a 1x1 unit rect whose origin is at 0,1 in pattern space,
    // that is, immediately above the first black checkerboard box.
    CGContextFillRect(patternCellContext, CGRectMake(0., 1., 1., 1.));
}

static CGPatternRef createRedBlackCheckerBoardPattern(CGAffineTransform patternTransform)
{
    CGPatternCallbacks myPatternCallbacks;
    CGPatternRef pattern;
    bool patternIsColored = true;
    // Only version 0 of the pattern callbacks is defined as of Tiger.
    myPatternCallbacks.version = 0;
    // The pattern's drawPattern proc is the routine Quartz calls 
    // to draw a pattern cell when it needs to draw one.
    myPatternCallbacks.drawPattern = myDrawRedBlackCheckerBoardPattern;
	// This code has no data associated with the pattern to release
	// so the releaseInfo callback is NULL.
    myPatternCallbacks.releaseInfo = NULL; 

    pattern = CGPatternCreate(NULL, 
	    // The pattern cell origin is at (0,0) with a 
	    // width of 2 units and a height of 2 units.
	    CGRectMake(0, 0, 2, 2),
	    // Use the pattern transform supplied to this routine. 
	    scalePatternMatrix(patternTransform),
	    // In pattern space the xStep is 2 units to the next cell in x 
	    // and the yStep is 2 units to the next row of cells in y.
	    2, 2, 
	    // This value is a good choice for this type of pattern and it
	    // avoids seams between tiles.
	    kCGPatternTilingConstantSpacingMinimalDistortion, 
	    // This pattern has intrinsic color.
	    patternIsColored, 
	    &myPatternCallbacks);
    return pattern;
}

void doRedBlackCheckerboard(CGContextRef context)
{
    CGColorSpaceRef patternColorSpace;
    float color[1]; 
    float dash[1] = { 4 } ;
    CGPatternRef pattern = createRedBlackCheckerBoardPattern(
			    CGAffineTransformMakeScale(20, 20) );
    if(pattern == NULL){
		fprintf(stderr, "Couldn't create pattern!\n");
		return;
    }

    // Create the pattern color space. Since the pattern
    // itself has intrinsic color, the 'baseColorSpace' parameter
    // to CGColorSpaceCreatePattern must be NULL.
    patternColorSpace = CGColorSpaceCreatePattern(NULL);
    CGContextSetFillColorSpace(context, patternColorSpace);

    // The pattern has intrinsic color so the color components array
    // passed to CGContextSetFillPattern is just the alpha value used
    // to composite the pattern cell.
    
    // Paint the pattern with alpha = 1.
    color[0] = 1.;
	// Set the fill color to the checkerboard pattern.
	CGContextSetFillPattern(context, pattern, color);
    
    // Fill a 100x100 unit rect at (20,20). 
    CGContextFillRect(context, CGRectMake(20, 20, 100, 100));

    // Save the graphics state before changing the stroke color.
    CGContextSaveGState(context);
	// Set the stroke color space and color to the pattern.
	CGContextSetStrokeColorSpace(context, patternColorSpace);
	CGContextSetStrokePattern(context, pattern, color);

	// Stroke an ellipse with the pattern.
	CGContextSetLineWidth(context, 8);
	CGContextBeginPath(context);
	myCGContextAddEllipseInRect(context, CGRectMake(120, 20, 50, 100));
	CGContextStrokePath(context);

    // Restore to the graphics state without the
    // pattern stroke color. 
    CGContextRestoreGState(context);
            
    // Now draw text.
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
    // Choose the font with the PostScript name "Times-Roman",
    // size 80 points, with the encoding MacRoman encoding.
    CGContextSelectFont(context, "Times-Roman", 80, kCGEncodingMacRoman);

    // Using the fill text drawing mode.
    CGContextSetTextDrawingMode(context, kCGTextFill);

    // Draw text with the pattern.
    CGContextShowTextAtPoint(context, 20, 120, "Text", 4);
    
    // Rectangle 1, filled.
    CGContextFillRect(context, CGRectMake(200, 20, 90, 90));
    
    // Rectangle 2, filled and stroked with a dash.
    CGContextSetLineWidth(context, 2);
    CGContextSetLineDash(context, 0, dash, 1);
    CGContextBeginPath(context);
    CGContextAddRect(context, CGRectMake(200, 70, 90, 90));
    CGContextDrawPath(context, kCGPathFillStroke);

    // This code won't set the pattern again so it
    // can release it.
    CGPatternRelease(pattern);
    // This code doesn't need the pattern color space
    // any longer so release it.
    CGColorSpaceRelease(patternColorSpace);

}

void doPatternMatrix(CGContextRef context)
{
    CGColorSpaceRef patternColorSpace;
    float color[1]; 
    CGAffineTransform t, basePatternMatrix = CGAffineTransformMakeScale(20, 20);
    CGAffineTransform patTransform;
    CGPatternRef pattern = createRedBlackCheckerBoardPattern(basePatternMatrix);
    if(pattern == NULL){
		fprintf(stderr, "Couldn't create pattern!\n");
		return;
    }
    // Create the pattern color space. Since the pattern
    // itself has intrinsic color, the 'baseColorSpace' parameter
    // to CGColorSpaceCreatePattern must be NULL.
    patternColorSpace = CGColorSpaceCreatePattern(NULL);
    
    CGContextSetFillColorSpace(context, patternColorSpace);
    CGColorSpaceRelease(patternColorSpace);

    CGContextTranslateCTM(context, 40, 40);
    CGContextSetPatternPhase(context, scalePatternPhase( CGSizeMake(40, 40) ));

    // The pattern has intrinsic color so the color components array
    // passed to CGContextSetFillPattern is the alpha value used
    // to composite the pattern cell.
    
    // Paint the pattern first with alpha = 1.
    color[0] = 1.;
    CGContextSetFillPattern(context, pattern, color);
    
    // Rectangle 1. 
    CGContextFillRect(context, CGRectMake(0, 0, 100, 100));

    CGContextSaveGState(context);
	// Rectangle 2.
	// Paint the pattern with 65% alpha.
	color[0] = 0.65;
	CGContextSetFillPattern(context, pattern, color);
	// Rotate 45 degrees about the point (150, 50).
	CGContextTranslateCTM(context, 150., 50.);
	CGContextRotateCTM(context, DEGREES_TO_RADIANS(45.));
	CGContextTranslateCTM(context, -50., -50.);
	// Rectangle 2. Patterns do not translate, scale or 
	// rotate with the CTM. You can see that the pattern
	// tile of this filled rectangle is that of Rectangle
	// 1.
	CGContextFillRect(context, CGRectMake(0, 0, 100, 100));
	// Release the pattern.
	CGPatternRelease(pattern);
    CGContextRestoreGState(context);

    CGContextSaveGState(context);
	// Rectangle 3. The pattern is rotated with the object.
	// Rotate 45 degrees about the point 250, 50.
	t = CGAffineTransformMakeTranslation(250., 50.);
	t = CGAffineTransformRotate(t, DEGREES_TO_RADIANS(45.));
	// Translate back to -50, -50.
	t = CGAffineTransformTranslate(t, -50., -50.);
	CGContextConcatCTM(context, t);
	/*	Make a new pattern that is equivalent to
	    the old pattern but transformed to current user 
	    space. The order of transformations is crucial. 
	    This ordering is equivalent to using the same pattern 
	    matrix as before but transforming base space by t. */
	patTransform = CGAffineTransformConcat(basePatternMatrix, t);
	pattern = createRedBlackCheckerBoardPattern(patTransform);
	color[0] = 1;
	CGContextSetFillPattern(context, pattern, color);
	// Release the pattern.
	CGPatternRelease(pattern);
	CGContextFillRect(context, CGRectMake(0, 0, 100, 100));
    CGContextRestoreGState(context);

    CGContextSaveGState(context);
	// Rectangle 4. The pattern is scaled with the object.
	// Translate and scale.
	t = CGAffineTransformMakeTranslation(320, 0);
	t = CGAffineTransformScale(t, 2, 2);
	CGContextConcatCTM(context, t);
	/*	Make a new pattern that is equivalent to
	    the old pattern but transformed to current user 
	    space. The order of transformations is crucial. 
	    This ordering is equivalent to using the same pattern 
	    matrix as before but transforming base space by t. */
	patTransform = CGAffineTransformConcat(basePatternMatrix, t);
	pattern = createRedBlackCheckerBoardPattern(patTransform);
	color[0] = 1;
	CGContextSetFillPattern(context, pattern, color);
	// Release the pattern.
	CGPatternRelease(pattern);
	CGContextFillRect(context, CGRectMake(0, 0, 100, 100));
    CGContextRestoreGState(context);
}


void doPatternPhase(CGContextRef context)
{
    CGColorSpaceRef patternColorSpace;
    float color[1]; 
    CGPatternRef pattern = createRedBlackCheckerBoardPattern(
			    CGAffineTransformMakeScale(20, 20)
			);
    if(pattern == NULL){
		fprintf(stderr, "Couldn't create pattern!\n");
		return;
    }

    // Create the pattern color space for a colored pattern.
    patternColorSpace = CGColorSpaceCreatePattern(NULL);
    CGContextSetFillColorSpace(context, patternColorSpace);

    // Paint the pattern with alpha = 1.
    color[0] = 1.;
    CGContextSetFillPattern(context, pattern, color);
    
    // Rectangle 1
    CGContextFillRect(context, CGRectMake(20, 150, 100, 100));

    // Rectangle 2
    CGContextFillRect(context, CGRectMake(130, 150, 100, 100));

    // Rectangle 3
    // Set the pattern phase so that the pattern origin
    // is at the lower-left of the shape.
    CGContextSetPatternPhase(context, scalePatternPhase( CGSizeMake(20, 20) ));
    CGContextFillRect(context, CGRectMake(20, 20, 100, 100));

    // Rectangle 4
    // Set the pattern phase so that the pattern origin
    // is at the lower-left corner of the shape.
    CGContextSetPatternPhase(context, scalePatternPhase( CGSizeMake(130, 20) ));
    CGContextTranslateCTM(context, 130, 20);
    CGContextFillRect(context, CGRectMake(0, 0, 100, 100));
    
    // This code won't set the pattern again so it
    // can release it.
    CGPatternRelease(pattern);
    // This code doesn't need the pattern color space
    // any longer so release it.
    CGColorSpaceRelease(patternColorSpace);

}

static void drawRotatedRect(CGContextRef c, CGPoint p)
{
    CGRect r = CGRectMake(0, 0, 1, 1);
    CGContextSaveGState(c);
	CGContextTranslateCTM(c, p.x, p.y);
	CGContextRotateCTM(c, DEGREES_TO_RADIANS(45));
	CGContextTranslateCTM(c, -r.size.width/2, -r.size.height/2);
	CGContextFillRect(c, r);
    CGContextRestoreGState(c);
}

static void myStencilPatternProc(void *info, CGContextRef patternCellContext)
{
    drawRotatedRect(patternCellContext, CGPointMake(1, 1));
    drawRotatedRect(patternCellContext, CGPointMake(1.75, 1));
}
static CGPatternRef createStencilPattern(CGAffineTransform patternTransform)
{
    CGPatternCallbacks myPatternCallbacks;
    CGPatternRef pattern;
    bool patternIsColored;
    // Only version 0 of the pattern callbacks is defined as of Tiger.
    myPatternCallbacks.version = 0;
    // The pattern's drawPattern proc is the routine Quartz calls 
    // to draw a pattern cell when it needs to draw one.
    myPatternCallbacks.drawPattern = myStencilPatternProc;
	// This code has no data to release, so the releaseInfo 
	// proc is NULL.
    myPatternCallbacks.releaseInfo = NULL; 

    patternIsColored = false; // A stencil pattern.
    pattern = CGPatternCreate(NULL, 
		// The pattern cell origin is at (0,0) with a 
		// width of 2.5 units and a height of 2 units. This
		// pattern cell has transparent areas since
		// the pattern proc only marks a portion of the cell.
		CGRectMake(0, 0, 2.5, 2),
		// Use the pattern transform supplied to this routine. 
		scalePatternMatrix(patternTransform),
		// Use the width and height of the pattern cell for
		// the xStep and yStep.
		2.5, 2, 
		// This value is a good choice for this type of pattern and it
		// avoids seams between tiles.
		kCGPatternTilingConstantSpacingMinimalDistortion, 
		// This pattern does not have intrinsic color.
		patternIsColored,   // Must be false for a stencil pattern.
		&myPatternCallbacks);
    return pattern;
}

void doStencilPattern(CGContextRef context)
{
    CGColorSpaceRef patternColorSpace, baseColorSpace;
    float color[4]; 
    CGPatternRef pattern = createStencilPattern(CGAffineTransformMakeScale(20, 20));
    if(pattern == NULL){
		fprintf(stderr, "Couldn't create pattern!\n");
		return;
    }
    // Create the pattern color space. This pattern is a stencil
    // pattern so when the code sets the pattern it also sets the
    // color it will paint the pattern with. In order to
    // set the pattern color space in this case we also have
    // to say what underlying color space should be used when
    // the pattern proc is called.
    baseColorSpace = getTheCalibratedRGBColorSpace();
    patternColorSpace = CGColorSpaceCreatePattern(baseColorSpace);
    
    CGContextSetFillColorSpace(context, patternColorSpace);
    // This code is finished with the pattern color space and can release
    // it because Quartz retains it while it is the current color space.
    CGColorSpaceRelease(patternColorSpace);

    // The pattern has no intrinsic color so the color components array
    // passed to CGContextSetFillPattern contains the colors to paint
    // the pattern with in the baseColorSpace. In the case here, 
    // first paint the pattern with opaque blue.
    color[0] = 0.11; color[1] = 0.208 ; color[2] = 0.451 ; color[3] = 1.;
    CGContextSetFillPattern(context, pattern, color);
    
    // Rectangle 1. 
    CGContextSetPatternPhase(context, scalePatternPhase( CGSizeMake(20, 160) ));
    CGContextBeginPath(context);
    CGContextAddRect(context, CGRectMake(20, 160, 105, 80));
    CGContextDrawPath(context, kCGPathFillStroke);

    // Rectangle 2.
    // Set the pattern color so the stencil pattern
    // is painted in a yellow shade.
    color[0] = 1.; color[1] = 0.816 ; color[2] = 0. ; color[3] = 1.;
    CGContextSetFillPattern(context, pattern, color);
    // Set the pattern phase to the origin of the next object.
    CGContextSetPatternPhase(context, scalePatternPhase( CGSizeMake(140, 160) ));
    CGContextBeginPath(context);
    CGContextAddRect(context, CGRectMake(140, 160, 105, 80));
    CGContextDrawPath(context, kCGPathFillStroke);

    CGContextSaveGState(context);
		CGContextSetFillColorWithColor(context, getRGBOpaqueBlueColor());
		// Fill color is now blue. Paint two blue rectangles
		// that will be underneath the drawing which follows.
		CGContextFillRect(context, CGRectMake(20, 40, 105, 80));
		CGContextFillRect(context, CGRectMake(140, 40, 105, 80));
    CGContextRestoreGState(context);
    
    // The fill color is again the stencil pattern with
    // the underlying fill color an opaque yellow.
    
    // Rectangle 3.
    // This paints over the blue rect just painted at 20,40
    // and the blue underneath is visible where the pattern has
    // transparent areas.
    CGContextSetPatternPhase(context, scalePatternPhase( CGSizeMake(20, 40) ));
    CGContextFillRect(context, CGRectMake(20, 40, 105, 80));

    // Rectangle 4.
    // Change the alpha value of the underlying color used
    // to paint the stencil pattern.
    color[3] = 0.75;
    CGContextSetFillPattern(context, pattern, color);
    CGContextSetPatternPhase(context, scalePatternPhase( CGSizeMake(140, 40) ));
    CGContextFillRect(context, CGRectMake(140, 40, 105, 80));

    CGPatternRelease(pattern);
}

typedef struct MyPDFPatternInfo{
    CGRect				rect;
    CGPDFDocumentRef	pdfDoc;
}MyPDFPatternInfo;

static void myDrawPDFPattern(void *info, CGContextRef patternCellContext)
{
    // This pattern proc draws the first page of a PDF document to 
	// a destination rect.
    MyPDFPatternInfo *patternInfoP = (MyPDFPatternInfo *)info;
    CGContextSaveGState(patternCellContext);
    CGContextClipToRect(patternCellContext, patternInfoP->rect);
    CGContextDrawPDFDocument(patternCellContext, patternInfoP->rect,
				patternInfoP->pdfDoc, 1);
    CGContextRestoreGState(patternCellContext);
}

static void myPDFPatternRelease(void *info)
{
    if(info){
		MyPDFPatternInfo *patternInfoP = (MyPDFPatternInfo *)info;
		CGPDFDocumentRelease(patternInfoP->pdfDoc);
		free(info);
    }
}


/*
	Versions of Tiger prior to 10.4.3 have a bug such that use of an xStep that  
	doesn't match the width of pattern bounding box or a yStep that doesn't match the 
	height of the pattern bounding box produces incorrect results when drawn 
	to a bit-based context. Setting TIGERSTEPWORKAROUND works around this bug.
*/
#define TIGERSTEPWORKAROUND 1

#define SCALEPATTERN 1

#define OPTIMIZEDPERF 0


static CGPatternRef createPDFPatternPattern(CGAffineTransform *additionalTransformP,
					    CFURLRef url)
{
    CGPatternCallbacks myPatternCallbacks;
    MyPDFPatternInfo *patternInfoP;
    CGPatternRef pattern;
    CGAffineTransform patternTransform;
    float tileOffsetX, tileOffsetY;

    // Only version 0 of the pattern callbacks is defined as of Tiger.
    myPatternCallbacks.version = 0;
    // The pattern's drawPattern proc is the routine Quartz calls to draw
    // a pattern cell when it needs to draw one.
    myPatternCallbacks.drawPattern = myDrawPDFPattern;
     // Since the pattern has the PDF document as a resource, the PDF document
     // needs to be released when Quartz no longer needs the pattern.
    myPatternCallbacks.releaseInfo = myPDFPatternRelease; 

    patternInfoP = (MyPDFPatternInfo *)malloc(sizeof(MyPDFPatternInfo));
    if(patternInfoP == NULL){
		fprintf(stderr, "Couldn't allocate pattern info data!\n");
		return NULL;
    }

    patternInfoP->pdfDoc = CGPDFDocumentCreateWithURL(url);
    if(patternInfoP->pdfDoc == NULL){
		fprintf(stderr, "Couldn't create PDF document reference!\n");
		free(patternInfoP);
		return;
    }
	
    patternInfoP->rect = CGPDFDocumentGetMediaBox(patternInfoP->pdfDoc, 1);
    // Set the origin of the media rect for the PDF document to (0,0).
    patternInfoP->rect.origin = CGPointZero;

    if(additionalTransformP)
		patternTransform = *additionalTransformP;
    else
		patternTransform = CGAffineTransformIdentity;

    // To emulate the example from the bitmap context drawing chapter,
    // the tile offset in each dimension is the tile size in that
    // dimension, plus 6 units.
#if SCALEPATTERN
    tileOffsetX = 6. + patternInfoP->rect.size.width;
    tileOffsetY = 6. + patternInfoP->rect.size.height;
#else
    tileOffsetX = 2. + patternInfoP->rect.size.width;
    tileOffsetY = 2. + patternInfoP->rect.size.height;
#endif

    pattern = CGPatternCreate(patternInfoP, 
	    // The pattern cell size is the size
	    // of the media rect of the PDF document.
	    
	    // Tiger versions 10.4.0 - 10.4.2 have a bug such that the bounds
	    // width and height is incorrectly used as the xstep,ystep.
	    // To workaround this bug, we can make the bounds rect incorporate
	    // the xstep,ystep since xstep,ystep are larger than the bounds. 
#if OPTIMIZEDPERF || TIGERSTEPWORKAROUND
	    CGRectMake(0, 0, tileOffsetX, tileOffsetY),
#else
	    patternInfoP->rect,
#endif
	    scalePatternMatrix(patternTransform),
	    tileOffsetX, tileOffsetY, 
	    // This value is a good choice for this type of pattern and
	    //  it avoids seams between tiles.
#if OPTIMIZEDPERF
	    kCGPatternTilingConstantSpacing,	// Produces best performance if bbox == xstep/ystep
#else
	    kCGPatternTilingConstantSpacingMinimalDistortion,
#endif
	    // This pattern has intrinsic color.
	    true, 
	    &myPatternCallbacks);
    // If the pattern can't be created then release the 
    // pattern resources and info parameter.
    if(pattern == NULL){
		myPatternCallbacks.releaseInfo(patternInfoP);
		patternInfoP = NULL;
    }
    return pattern;
}


void drawWithPDFPattern(CGContextRef context, CFURLRef url)
{
    CGColorSpaceRef patternColorSpace;
    float color[1];
#if SCALEPATTERN
    CGAffineTransform patternMatrix = CGAffineTransformMakeScale(1./3, 1./3);
#else
    CGAffineTransform patternMatrix = CGAffineTransformMakeScale(1, 1);
#endif
    // Scale the PDF pattern down to 1/3 its original size.
    CGPatternRef pdfPattern = createPDFPatternPattern(&patternMatrix, url);
    if(pdfPattern == NULL){
		fprintf(stderr, "Couldn't create pattern!\n");
		return;
    }
    // Create the pattern color space. Since the pattern
    // itself has intrinsic color, the 'baseColorSpace' parameter
    // to CGColorSpaceCreatePattern must be NULL.
    patternColorSpace = CGColorSpaceCreatePattern(NULL);
    CGContextSetFillColorSpace(context, patternColorSpace);
    // Quartz retains the color space so this code
    // can now release it since it no longer needs it.
    CGColorSpaceRelease(patternColorSpace);
    
    // Paint the pattern with an alpha of 1.
    color[0] = 1;
    CGContextSetFillPattern(context, pdfPattern, color);
    // Quartz retains the pattern so this code
    // can now release it since it no longer needs it.
    CGPatternRelease(pdfPattern);
    
    // Fill a US Letter size rect with the pattern.
    CGContextFillRect(context, CGRectMake(0, 0, 612, 792));
     
}