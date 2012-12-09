/*
*  File:    Shadings.c
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

#include "Shadings.h"
#include "Utilities.h"

static void RedBlackRedRampEvaluate(void *info, const float *in, float *out)
{
    /*	The domain of this function is 0 - 1. For an input value of 0
		this function returns the color to paint at the start point
		of the shading. For an input value of 1 this function returns
		the color to paint at the end point of the shading. This
		is a 1 in, 4 out function where the output values correspond
		to an r,g,b,a color.
		
		For an RGB color space as the shading color space, this
		function evaluates to produce a blend from pure, opaque
		red at the start point to a pure opaque black at the 
		midpoint, and back to pure opaque red at the end point.
    */
    // The red component evaluates to 1 for an input value of 0
    // (the start point of the shading). It smoothly reduces
    // to zero at the midpoint of the shading (input value 0.5)  
    // and increases up to 1 at the endpoint of the shading (input
    // value 1.0).
    out[0] = fabs(1. - in[0]*2);
    // The green and blue components are always 0.
    out[1] = out[2] = 0;
    // The alpha component is 1 for the entire shading.
    out[3] = 1;
}

static CGFunctionRef createFunctionForRGB(CGFunctionEvaluateCallback evaluationFunction)
{
    CGFunctionRef function;
    float domain[2];	// 1 input
    float range[8];	// 4 outputs
    CGFunctionCallbacks shadingCallbacks;

    /*	This is a 1 in, 4 out function for drawing shadings 
		in a 3 component (plus alpha) color space. Shadings 
		parameterize the endpoints such that the starting point
		represents the function input value 0 and the ending point 
		represents the function input value 1. 
    */
    domain[0] = 0; domain[1] = 1;
    
    /*	The range is the range for the output colors. For an rgb
		color space the values range from 0-1 for the r,g,b, and a
		components. 
	*/
    
    // The red component, min and max.    
    range[0] = 0; range[1] = 1;
    // The green component, min and max.    
    range[2] = 0; range[3] = 1;
    // The blue component, min and max.    
    range[4] = 0; range[5] = 1;
    // The alpha component, min and max.    
    range[6] = 0; range[7] = 1;
    
    // The callbacks structure version is
    // 0, the only defined version as of Tiger.
    shadingCallbacks.version = 0;
    // The routine Quartz should call to evaluate the function.
    shadingCallbacks.evaluate = evaluationFunction;
    // This code does not use a releaseInfo parameter since this 
    // CGFunction has no resources it uses that need to be released
    // when the function is released.
    shadingCallbacks.releaseInfo = NULL;
    
    // Dimension of domain is 1 and dimension of range is 4.    
    function = CGFunctionCreate(NULL, 1, domain, 4, range, &shadingCallbacks);
    if(function == NULL){
		fprintf(stderr, "Couldn't create the CGFunction!\n");
		return NULL;
    }
    return function;
}

void doSimpleAxialShading(CGContextRef context)
{
    CGFunctionRef axialFunction;
    CGShadingRef shading;
    CGPoint startPoint, endPoint;
    bool extendStart, extendEnd;
    
    // This shading paints colors in the calibrated Generic RGB 
    // color space so it needs a function that evaluates 1 in to 4 out.
    axialFunction = createFunctionForRGB(RedBlackRedRampEvaluate);
    if(axialFunction == NULL){
		return;
    }
    
    // Start the shading at the point (20,20) and
    // end it at (420,20). The axis of the shading
    // is a line from (20,20) to (420,20).
    startPoint.x = 20;
    startPoint.y = 20;
    endPoint.x = 420;
    endPoint.y = 20;
    
    // Don't extend this shading.
    extendStart = extendEnd = false;

    shading = CGShadingCreateAxial(getTheCalibratedRGBColorSpace(), 
			    startPoint, endPoint, 
			    axialFunction, 
			    extendStart, extendEnd);
    // The shading retains the function and this code
    // is done with the function so it should release it.
    CGFunctionRelease(axialFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }
    // Draw the shading. This paints the shading to
    // the destination context, clipped by the
    // current clipping area.
    CGContextDrawShading(context, shading);
    // Release the shading once the code is done with
    // it.
    CGShadingRelease(shading);
}

void RedGreenRampEvaluate(void *info, const float *in, float *out)
{
    /*	The domain of this function is 0 - 1. For an input value of 0
		this function returns the color to paint at the start point
		of the shading. For an input value of 1 this function returns
		the color to paint at the end point of the shading. This
		is a 1 in, 4 out function where the output values correspond
		to an r,g,b,a color.
		
		For an RGB color space as the shading color space, this
		function evaluates to produce a blend from pure, opaque
		red at the start point to a pure opaque green at the end point.
    */
    // The red component starts at 1 and reduces to zero as the input 
    // goes from 0 (the start point of the shading) and increases 
    // to 1 (the end point of the shading).
    out[0] = 1. - in[0];
    // The green component starts at 0 for an input of 0
    // (the start point of the shading) and increases to 1 
    // for an input value of 1 (the end point of the shading).
    out[1] = in[0];
    // The blue component is always 0.
    out[2] = 0;
    // The alpha component is always 1, the shading is always opaque.
    out[3] = 1;
}

void doExampleAxialShading(CGContextRef context)
{
    CGFunctionRef redGreenFunction;
    CGShadingRef shading;
    CGPoint startPoint, endPoint;
    bool extendStart, extendEnd;
    CGRect rect = CGRectMake(0, 0, 240, 240);
    
    // This shading paints colors in the calibrated Generic RGB 
    // color space so it needs a function that evaluates 1 in to 4 out.
    redGreenFunction = createFunctionForRGB(RedGreenRampEvaluate);
    if(redGreenFunction == NULL){
		return;
    }
    
    // Start the shading at the point (20,20) and
    // end it at (220,220). The axis of the shading
    // is a diagonal line from (20,20) to (220,220).
    startPoint.x = 20;
    startPoint.y = 20;
    endPoint.x = 220;
    endPoint.y = 220;
    
    // Don't extend this shading.
    extendStart = extendEnd = false;
    shading = CGShadingCreateAxial(getTheCalibratedRGBColorSpace(), 
			    startPoint, endPoint, 
			    redGreenFunction, 
			    extendStart, extendEnd);
    
    if(shading == NULL){
		CGFunctionRelease(redGreenFunction);
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }

    // Position for the first portion of the drawing.
    CGContextTranslateCTM(context, 40, 260);
    
    // Stroke a black rectangle that will frame the shading.
    CGContextSetLineWidth(context, 2);
    CGContextSetStrokeColorWithColor(context, getRGBOpaqueBlackColor());
    CGContextStrokeRect(context, rect);

    CGContextSaveGState(context);
		// Clip to the rectangle that was just stroked.
		CGContextClipToRect(context, rect);
		// Draw the shading. This paints the shading to
		// the destination context, clipped to rect.
		CGContextDrawShading(context, shading);
		// Release the shading once the code is finished with it.
		CGShadingRelease(shading);
	// Restore the graphics state so that the rectangular 
	// clip is no longer present.
    CGContextRestoreGState(context);

    // Prepare for the next shading.
    CGContextTranslateCTM(context, 0, -250);
    
    // Extend this shading.
    extendStart = extendEnd = true;
    shading = CGShadingCreateAxial(getTheCalibratedRGBColorSpace(), 
			    startPoint, endPoint, 
			    redGreenFunction, 
			    extendStart, extendEnd);
    // The shading retains the function and this code
    // is done with the function so it should release it.
    CGFunctionRelease(redGreenFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }
    
    // Stroke with the current stroke color.
    CGContextStrokeRect(context, rect);

    CGContextSaveGState(context);
	CGContextClipToRect(context, rect);
	// Draw the shading. This paints the shading to
	// the destination context, clipped to rect.
	CGContextDrawShading(context, shading);
    CGContextRestoreGState(context);
    
    // Now paint some text with a shading.
    CGContextSaveGState(context);
	CGContextTranslateCTM(context, 260, 0);
	CGContextSetTextMatrix(context, CGAffineTransformIdentity);
       
	// Set the font with the PostScript name "Times-Roman", at
	// 80 points, with the MacRoman encoding.
	CGContextSelectFont(context, "Times-Roman", 80, 
					    kCGEncodingMacRoman);

	// Rotate so that the text characters are rotated
	// relative to the page.
	CGContextRotateCTM(context, DEGREES_TO_RADIANS(45));
	// Set the text drawing mode to clip so that
	// the characters in the string are intersected with
	// the clipping area.
	CGContextSetTextDrawingMode(context, kCGTextClip);
	CGContextShowTextAtPoint(context, 30, 0, "Shading", 7);
	
	// At this point nothing has been painted; the
	// glyphs in the word "Shading" have been intersected
	// with the previous clipping area to create a new
	// clipping area.
	
	// Rotate the coordinate system back so that the
	// shading is not rotated relative to the page.
	CGContextRotateCTM(context, DEGREES_TO_RADIANS(-45));
	
	// Draw the shading, painting the shading 
	// to the destination context, clipped by the glyphs.
	CGContextDrawShading(context, shading);
	
    CGContextRestoreGState(context);
    // Release the shading once the code is finished with it.
    CGShadingRelease(shading);
}

typedef struct MyStartEndColor{
    float startColor[3];
    float endColor[3];
}MyStartEndColor;

static void StartColorEndColorEvaluate(void *info, const float *in, float *out)
{
    /*	The domain of this function is 0 - 1. For an input value of 0
		this function returns the color to paint at the start point
		of the shading. For an input value of 1 this function returns
		the color to paint at the end point of the shading. This
		is a 1 in, 4 out function where the output values correspond
		to an r,g,b,a color.
		
		This function evaluates to produce a blend from startColor to endColor.

		Note that the returned results are clipped to the range
		by Quartz so this function doesn't worry about values
		that are outside the range 0-1.	
    */
    MyStartEndColor *startEndColorP = (MyStartEndColor *)info;
    float *startColor = startEndColorP->startColor;
    float *endColor = startEndColorP->endColor;
    float input = in[0];
    // Weight the starting and ending color components depending
    // on what position in the blend the input value specifies.
    out[0] = (startColor[0]*(1-input) + endColor[0]*input);
    out[1] = (startColor[1]*(1-input) + endColor[1]*input);
    out[2] = (startColor[2]*(1-input) + endColor[2]*input);
    // The alpha component is always 1, the shading is always opaque.
    out[3] = 1;
}

static void releaseStartEndColorInfo(void *info)
{
    if(info)
		free(info);
}

static CGFunctionRef createFunctionWithStartEndColorRamp(
			    const float startColor[3],
			    const float endColor[3])
{
    CGFunctionRef function;
    float domain[2];	// 1 input
    float range[8];		// 4 outputs
    CGFunctionCallbacks shadingCallbacks;
    MyStartEndColor *startEndColorP;

    // Use a pointer to a MyStartEndColor as a way of 
    // parameterizing the color ramp this function produces.
    // Don't allocate this on the stack!
    startEndColorP = 
	    (MyStartEndColor *)malloc(sizeof(MyStartEndColor));

    if(startEndColorP == NULL){
		fprintf(stderr, "Couldn't malloc memory for startEndColor!\n");
		return NULL;
    }
    
    // Set up start and end colors in the info structure.
    startEndColorP->startColor[0] = startColor[0];
    startEndColorP->startColor[1] = startColor[1];
    startEndColorP->startColor[2] = startColor[2];

    startEndColorP->endColor[0] = endColor[0];
    startEndColorP->endColor[1] = endColor[1];
    startEndColorP->endColor[2] = endColor[2];

    /*	This is a 1 in, 4 out function for drawing shadings 
		in a 3 component (plus alpha) color space. Shadings 
		parameterize the endpoints such that the starting point
		represents the function input value 0 and the ending point 
		represents the function input value 1. 
    */
    domain[0] = 0; domain[1] = 1;
    
    /*	The range is the range for the output colors. For an rgb
	color space the values range from 0-1 for the r,g,b, and a
	components. */
    
    // The red component, min and max.    
    range[0] = 0; 
    range[1] = 1;
    // The green component, min and max.    
    range[2] = 0; 
    range[3] = 1;
    // The blue component, min and max.    
    range[4] = 0;
    range[5] = 1;
    // The alpha component, min and max.    
    range[6] = 0; 
	range[7] = 1;
    
    // The callbacks structure version is
    // 0, the only defined version as of Tiger.
    shadingCallbacks.version = 0;
    // StartColorEndColorEvaluate is the function to evaluate.
    shadingCallbacks.evaluate = StartColorEndColorEvaluate;
    // releaseStartEndColorInfo releases the pointer to
    // the MyStartEndColor used as the info parameter.
    shadingCallbacks.releaseInfo = releaseStartEndColorInfo;
    
    // Pass startEndColorP as the info parameter.
    function = CGFunctionCreate(startEndColorP, 1, domain, 4, 
					range, &shadingCallbacks);
    if(function == NULL){
		// Couldn't create the function so this code must free the data.
		free(startEndColorP);
		fprintf(stderr, "Couldn't create the CGFunction!\n");
		return NULL;
    }
    return function;
}

void doSimpleRadialShading(CGContextRef context)
{
    CGFunctionRef redYellowFunction;
    CGShadingRef shading;
    CGPoint circleACenter, circleBCenter;
    float circleARadius, circleBRadius;
    bool extendStart, extendEnd;
    float startColor[3] = { 0.663, 0., 0.031 };	// Red.
    float endColor[3] = { 1., 0.8, 0.4, };		// Light yellow.

    // This function describes a color ramp where the starting color
    // is red and the ending color is blue.
    redYellowFunction = createFunctionWithStartEndColorRamp(
					startColor, endColor);
    if(redYellowFunction == NULL){
		return;
    }

    CGContextTranslateCTM(context, 120, 120);

    // Circles whose origin is the same.
    circleACenter.x = 0;
    circleACenter.y = 0;
    circleBCenter = circleACenter;
    
    // The starting circle is inside the ending circle.
    circleARadius = 50;
    circleBRadius = 100;
    
    //  Don't extend the shading.
    extendStart = extendEnd = false; 
    shading = CGShadingCreateRadial(
		getTheCalibratedRGBColorSpace(), 
		circleACenter, circleARadius,
		circleBCenter, circleBRadius,
		redYellowFunction, 
		extendStart, extendEnd);
    
    CGFunctionRelease(redYellowFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }
    CGContextDrawShading(context, shading);
    CGShadingRelease(shading);
}

void doExampleRadialShadings(CGContextRef context)
{
    CGFunctionRef magentaFunction, redGreenFunction, 
			redBlackFunction, blueGreenFunction;
    CGShadingRef shading;
    CGPoint circleACenter, circleBCenter;
    float circleARadius, circleBRadius;
    bool extendStart, extendEnd;
    const float magenta[3] = { 1, 0, 1 };		// Pure magenta.
    const float magenta30[3] = { 0.3, 0, 0.3 }; // 30% magenta.
    const float black[3] = { 0, 0, 0 };
    const float red[3] = { 1, 0, 0 };
    const float green[3] = { 0, 1, 0 };
    const float blue[3] = { 0, 0, 1 };
    const float redgreen[3] = { 0.66, 1, 0.04 }; // A red-green shade.
    
    CGContextTranslateCTM(context, 120, 550);
    
    // This function describes a color ramp where the starting color
    // is a full magenta, the ending color is 30% magenta.
    magentaFunction = createFunctionWithStartEndColorRamp(
			    magenta, magenta30);
    if(magentaFunction == NULL){
		fprintf(stderr, "Couldn't create the magenta function!\n");
		return;
    }

    // Shading 1. Circle A is completely inside circle B but with 
    // different origins. Circle A has radius 0 which produces
    // a point source.

    // The center of circle A is offset from the origin.
    circleACenter.x = 30;
    circleACenter.y = 40;
    // The center of circle B is at the origin.
    circleBCenter = CGPointZero;
    
    // A radius of zero produces a point source.
    circleARadius = 0;
    circleBRadius = 100;
    
    // Don't extend the shading.
    extendStart = extendEnd = false;
    shading = CGShadingCreateRadial(
		getTheCalibratedRGBColorSpace(), 
		circleACenter, circleARadius,
		circleBCenter, circleBRadius,
		magentaFunction, 
		extendStart, extendEnd);
    // Finished with the magenta function so release it.
    CGFunctionRelease(magentaFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }
    CGContextDrawShading(context, shading);
    // Finished with the shading so release it.
    CGShadingRelease(shading);

    // Shading 2. Circle A is completely inside
    // circle B but with different origins.

    // The starting color is red and the ending color is green.
    redGreenFunction = createFunctionWithStartEndColorRamp(
			    red, green);
    if(redGreenFunction == NULL){
		fprintf(stderr, "Couldn't create the red-Green function!\n");
		return;
    }

    circleACenter.x = 55;
    circleACenter.y = 70;
    circleBCenter.x = 20;
    circleBCenter.y = 0;
    circleARadius = 10;
    // The outer circle is outside the clipping path so the
    // color at the edge of the shape is not
    // that at the radius of the outer circle.
    circleBRadius = 200;
    // Extend the end point of this shading.
    extendStart = false;
    extendEnd = true;
    shading = CGShadingCreateRadial(
		    getTheCalibratedRGBColorSpace(), 
		    circleACenter, circleARadius,
		    circleBCenter, circleBRadius,
		    redGreenFunction, 
		    extendStart, extendEnd);
    // Finished with this function so release it.
    CGFunctionRelease(redGreenFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }

    // Set a clipping area to bound the extend. This code
    // sets a clipping area that corresponds to a circular 
    // wedge. The starting circle is inside the clipping
    // area and the ending circle is outside.
    CGContextSaveGState(context);
	CGContextTranslateCTM(context, 250, 0);
	CGContextBeginPath(context);
	CGContextMoveToPoint(context, 25, 0);
	CGContextAddArc(context, 
			25, 0, 130,
			DEGREES_TO_RADIANS(30), 
			DEGREES_TO_RADIANS(-30), 
			0);
	CGContextClip(context);
	// Paint the shading.
	CGContextDrawShading(context, shading);
	// Finished with the shading so release it.
	CGShadingRelease(shading);
    CGContextRestoreGState(context);

    CGContextTranslateCTM(context, -40, -250);

    // Shading 3. The starting circle is completely outside
    // the ending circle, no extension. The circles
    // have the same radii.
    circleACenter.x = 0;
    circleACenter.y = 0;
    circleBCenter.x = 125;
    circleBCenter.y = 0;
    
    circleARadius = 50;
    circleBRadius = 50;

    extendStart = extendEnd = false;

    // Create a function that paints a red to black ramp.
    redBlackFunction = createFunctionWithStartEndColorRamp(
			    red, black);
    if(redBlackFunction == NULL){
		fprintf(stderr, "Couldn't create the red-black function!\n");
		return;
    }
    
    shading = CGShadingCreateRadial(
		    getTheCalibratedRGBColorSpace(), 
		    circleACenter, circleARadius,
		    circleBCenter, circleBRadius,
		    redBlackFunction, 
		    extendStart, extendEnd);
    if(shading == NULL){
		CGFunctionRelease(redBlackFunction);
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }
    CGContextDrawShading(context, shading);
    // Finished with the shading so release it.
    CGShadingRelease(shading);

    // Shading 4. The starting circle is completely outside
    // the ending circle. The circles have different radii.
    circleACenter.x = 120;
    circleACenter.y = 0;
    circleBCenter.x = 0;
    circleBCenter.y = 0;
    
    circleARadius = 75;
    circleBRadius = 30;

    // Extend at the start and end.
    extendStart = extendEnd = true;
    shading = CGShadingCreateRadial(
		getTheCalibratedRGBColorSpace(), 
		circleACenter, circleARadius,
		circleBCenter, circleBRadius,
		redBlackFunction, 
		extendStart, extendEnd);
    // Finished with this function so release it.
    CGFunctionRelease(redBlackFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }
    CGContextSaveGState(context);
	CGContextTranslateCTM(context, 270, 0);
	// Clip to an elliptical path so the shading
	// does not extend to infinity at the larger end.
	CGContextBeginPath(context);
	myCGContextAddEllipseInRect(context, 
			CGRectMake(-200, -200, 450, 400));
	CGContextClip(context);
	CGContextDrawShading(context, shading);
	// Finished with the shading so release it.
	CGShadingRelease(shading);
    CGContextRestoreGState(context);

    CGContextTranslateCTM(context, 30, -200);

    // The starting color is blue, the ending color is a red-green color.
    blueGreenFunction = createFunctionWithStartEndColorRamp(
			    blue, redgreen);
    if(blueGreenFunction == NULL){
		fprintf(stderr, "Couldn't create the blue-Green function!\n");
		return;
    }
   
    // Shading 5. The circles partially overlap and have
    // different radii with the larger circle at the start.
    circleACenter.x = 0;
    circleACenter.y = 0;
    circleBCenter.x = 90;
    circleBCenter.y = 30;
    
    circleARadius = 75;
    circleBRadius = 45;

    extendStart = extendEnd = false;
    shading = CGShadingCreateRadial(
		    getTheCalibratedRGBColorSpace(), 
		    circleACenter, circleARadius,
		    circleBCenter, circleBRadius,
		    blueGreenFunction, 
		    extendStart, extendEnd);
    if(shading == NULL){
		CGFunctionRelease(blueGreenFunction);
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }

    CGContextDrawShading(context, shading);
    // Finished with the shading so release it.
    CGShadingRelease(shading);

    CGContextTranslateCTM(context, 200, 0);

    // Shading 6. The circles partially overlap and have 
    // different radii with the larger circle at the end.
    
    circleARadius = 45;
    circleBRadius = 75;
    shading = CGShadingCreateRadial(
		    getTheCalibratedRGBColorSpace(), 
		    circleACenter, circleARadius,
		    circleBCenter, circleBRadius,
		    blueGreenFunction, 
		    extendStart, extendEnd);
    // Finished with this function so release it.
    CGFunctionRelease(blueGreenFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }

    CGContextDrawShading(context, shading);
    // Finished with the shading so release it.
    CGShadingRelease(shading);
}

void doEllipseShading(CGContextRef context)
{
    CGFunctionRef redBlackFunction;
    CGShadingRef shading;
    CGPoint circleACenter, circleBCenter;
    float circleARadius, circleBRadius;
    bool extendStart, extendEnd;
    const float black[3] = { 0, 0, 0 };
    const float red[3] = { 1, 0, 0 };

    // This function describes a color ramp where the starting color
    // is red, the ending color is black.
    redBlackFunction = createFunctionWithStartEndColorRamp(
			    red, black);
    if(redBlackFunction == NULL){
		fprintf(stderr, "Couldn't create the red-black function!\n");
		return;
    }
    
    CGContextTranslateCTM(context, 100, 300);
    // Shading 1.
    /*
		To obtain an elliptical shading requires that user space
		at the time the shading is painted is transformed so that
		the circles which define the radial shading geometry are
		rotated and elliptical. User space will be rotated
		by 45 degrees, then scaled by 1 in x and 2 in y to produce
		the ellipses.
    */
    // Compute the transform needed to create the rotated ellipses. 
    CGAffineTransform t = CGAffineTransformMakeRotation(DEGREES_TO_RADIANS(45));
    t = CGAffineTransformScale(t, 1, 2);

    circleACenter.x = 0;
    circleACenter.y = 0;

    circleBCenter.x = circleACenter.x + 144;
    circleBCenter.y = circleACenter.y;

    circleARadius = 45;
    circleBRadius = 45;

    // Don't extend this shading.
    extendStart = extendEnd = false;

    shading = CGShadingCreateRadial(
		    getTheCalibratedRGBColorSpace(), 
		    circleACenter, circleARadius,
		    circleBCenter, circleBRadius,
		    redBlackFunction, 
		    extendStart, extendEnd);
    if(shading == NULL){
		// Couldn't create the shading so release
		// the function before returning.
    	CGFunctionRelease(redBlackFunction);
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }
    
    CGContextSaveGState(context);
	// Transform coordinates for the drawing of the shading.
	// This transform produces the rotated elliptical shading.
	// This produces the left shading in the figure, the
	// one where both the ellipses and the shading are
	// rotated relative to default user space.
	CGContextConcatCTM(context, t);
	
	CGContextDrawShading(context, shading);
	CGShadingRelease(shading);
    CGContextRestoreGState(context);

    CGContextTranslateCTM(context, 300, 10);
    
    
    // Shading 2.
    /*	
		Now draw the shading where the shading ellipses are
		rotated but the axis between the origins of 
		the ellipses lies parallel to the x axis in default
		user space. This is similar to the shading drawn
		manually in Chapter 5. 
		
		To compute the correct origins for the shading, 
		the code needs to compute the points that, 
		transformed by the matrix t used to paint the shading, 
		produce the desired coordinates. We want coordinates
		that are transformed as follows:
		
			P' = P x t
			
		where P' is the point in untransformed user space that
		we want as the origin, P is the point in transformed
		user space that will be transformed by t, the matrix
		which transforms the circles into rotated ellipses.

		So we want to calculate P such that P' = P x t .

		Notice that if P = P' x Inverse(t) then:
		
		P' = P' x Inverse(t) x t = P' x Identity = P'.
		
		This means that we can calculate the point P
		by computing P' x Inverse(t).
    */
    CGAffineTransform inverseT = CGAffineTransformInvert(t);
    // Now the code can transform the coordinates through the
    // inverse transform to compute the new coordinates. These
    // coordinates, when transformed with the transform t, 
    // produce the original coordinate.
    circleACenter = CGPointApplyAffineTransform(circleACenter, inverseT);
    circleBCenter = CGPointApplyAffineTransform(circleBCenter, inverseT);

    shading = CGShadingCreateRadial(
		    getTheCalibratedRGBColorSpace(), 
		    circleACenter, circleARadius,
		    circleBCenter, circleBRadius,
		    redBlackFunction, 
		    extendStart, extendEnd);
    // The code is finished with the function so release it. 
    CGFunctionRelease(redBlackFunction);
    if(shading == NULL){
		fprintf(stderr, "Couldn't create the shading!\n");
		return;
    }

    // Transform coordinates for the drawing of the shading.
    // This transform produces the rotated elliptical shading.
    CGContextConcatCTM(context, t);
    
    CGContextDrawShading(context, shading);
    CGShadingRelease(shading);
}