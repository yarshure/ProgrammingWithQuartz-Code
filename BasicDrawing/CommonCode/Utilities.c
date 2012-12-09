/*
*  File:    Utilities.c
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

#include <ApplicationServices/ApplicationServices.h>

static float gScalingFactor = 1.0;

// These routines are used for getting the correct results when
// drawing shadows and patterns with Quartz and exporting the
// results as bits. This is a hack; in principle the scaling
// factor should be passed to the draw proc.
void setScalingFactor(float scalingFactor)
{
    if(scalingFactor > 0)
		gScalingFactor = scalingFactor;
}

float getScalingFactor(void)
{
    return gScalingFactor;
}

CFBundleRef getAppBundle()
{
    static CFBundleRef appBundle = NULL;
    if(appBundle == NULL)
		appBundle = CFBundleGetMainBundle();
    
    return appBundle;
}


#if 0
/*
    This version of getTheRGBColorSpace returns
    the DeviceRGB color space.
*/
static CGColorSpaceRef getTheRGBColorSpace(void)
{
    static CGColorSpaceRef deviceRGB = NULL;
    // Set once, the first time this function is called.
    if(deviceRGB == NULL)
	    deviceRGB = CGColorSpaceCreateDeviceRGB();
	    
    return deviceRGB;
}
#endif

// The full path to the generic RGB ICC profile.
#define 	kGenericRGBProfilePathStr \
	"/System/Library/ColorSync/Profiles/Generic RGB Profile.icc"
static CGColorSpaceRef getTheRGBColorSpace(void)
{
	static CGColorSpaceRef genericRGBColorSpace = NULL;
	if(genericRGBColorSpace == NULL)
	{
		CMProfileRef genericRGBProfile = NULL;
		OSStatus err = noErr;
		CMProfileLocation loc;
		// Build up a profile location for ColorSync.
		loc.locType = cmPathBasedProfile;
		strcpy (loc.u.pathLoc.path, kGenericRGBProfilePathStr);
		// Open the profile with ColorSync.
		err = CMOpenProfile(&genericRGBProfile, &loc);
		if(err == noErr){
			genericRGBColorSpace =
				CGColorSpaceCreateWithPlatformColorSpace(genericRGBProfile);
			if(genericRGBColorSpace == NULL)
				fprintf(stderr, "couldn't create the generic RGB color space\n");
			// This code opened the profile so it is 
			// up to it to close it.
			CMCloseProfile(genericRGBProfile); 
		}else{
			// ColorSync could not open the profile so log a message 
			// to the console.
			fprintf(stderr, "couldn't open generic profile due to error %d\n",
						(int)err);
		}
	}
	return genericRGBColorSpace;
}

// This only builds on Tiger and later.
CGColorSpaceRef getTheCalibratedRGBColorSpace(void)
{
	static CGColorSpaceRef genericRGBColorSpace = NULL;
	if(genericRGBColorSpace == NULL)
	{
		// Test the symbol kCGColorSpaceGenericRGB to see if 
		// it is available. If so, use it.
		if(&kCGColorSpaceGenericRGB != NULL)
			genericRGBColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

	    // If genericRGBColorSpace is still NULL, use the technique
	    // of using ColorSync to open the disk based profile by using 
	    // getTheRGBColorSpace.
	    if(genericRGBColorSpace == NULL){
			genericRGBColorSpace = getTheRGBColorSpace();
	    }
	}
	return genericRGBColorSpace;
}

/*
    This routine has 'Get' semantics and returns a the
    generic Gray colorspace.
*/
#define	kGenericGrayProfilePathStr       "/System/Library/ColorSync/Profiles/Generic Gray Profile.icc"
CGColorSpaceRef getTheCalibratedGrayColorSpace(void)
{
    static CGColorSpaceRef genericGrayColorSpace = NULL;
    if (genericGrayColorSpace == NULL)
    {
	    CMProfileRef genericGrayProfile = NULL;
	    OSStatus err = noErr;
	    CMProfileLocation loc;
	    // Build up a profile location for ColorSync.
	    loc.locType = cmPathBasedProfile;
	    strcpy (loc.u.pathLoc.path, kGenericGrayProfilePathStr);
	    // Open the profile with ColorSync.
	    err = CMOpenProfile(&genericGrayProfile, &loc);
	    if(err == noErr){
		    genericGrayColorSpace =
			    CGColorSpaceCreateWithPlatformColorSpace(genericGrayProfile);
		    if(genericGrayColorSpace == NULL)
			    fprintf(stderr, "Couldn't create the generic Gray color space\n");
		    // This code opened the profile so it is 
		    // up to it to close it.
		    CMCloseProfile(genericGrayProfile); 
	    }else{
		    // ColorSync could not open the profile so log a message 
		    // to the console.
		    fprintf(stderr, "Couldn't open generic profile due to error %d\n", (int)err);
	    }
    }
    return genericGrayColorSpace;
}

/*
    Return the sRGB color space. This routine has 'Get' semantics and the caller should
    not release the returned value unless the caller retains it first. Usually callers
    of this routine will immediately use the returned colorspace with CoreGraphics
    so they typically do not need to retain it themselves.
    
    This function creates the sRGB color space once and hangs onto it so it can
    return it whenever this function is called.
*/
// The full path to the sRGB ICC profile.
#define 	ksRGBProfilePathStr \
	"/System/Library/ColorSync/Profiles/sRGB Profile.icc"

CGColorSpaceRef getTheSRGBColorSpace(void)
{
	static CGColorSpaceRef sRGBColorSpace = NULL;
	if (sRGBColorSpace == NULL)
	{
		CMProfileRef sRGBProfile = NULL;
		OSStatus err = noErr;
		CMProfileLocation loc;
		// Build up a profile location for ColorSync.
		loc.locType = cmPathBasedProfile;
		strcpy (loc.u.pathLoc.path, ksRGBProfilePathStr);
		// Open the profile with ColorSync.
		err = CMOpenProfile(&sRGBProfile, &loc);
		if(err == noErr){
			sRGBColorSpace =
				CGColorSpaceCreateWithPlatformColorSpace(sRGBProfile);
			if(sRGBColorSpace == NULL)
				fprintf(stderr, "Couldn't create the generic RGB color space\n");
			// This code opened the profile so it is 
			// up to it to close it.
			CMCloseProfile(sRGBProfile); 
		}else{
			// ColorSync could not open the profile so log a message 
			// to the console.
			fprintf(stderr, "couldn't open generic profile due to error %d\n", (int)err);
		}
	}
	return sRGBColorSpace;
}

#if 1
/*
    This version of getTheDisplayColorSpace is
    useful if you only want a display color space
    for the main display.
*/  
CGColorSpaceRef getTheDisplayColorSpace(void)
{
    static CGColorSpaceRef displayCS = NULL;
    if(displayCS == NULL){
		CMProfileRef systemProfile = NULL;
		OSStatus err = CMGetSystemProfile(&systemProfile);
		if (!err){
			displayCS = CGColorSpaceCreateWithPlatformColorSpace(systemProfile);
			CMCloseProfile(systemProfile);
		}else{
			fprintf(stderr, 
			"Got error %d when getting system profile!\n", err);
			return NULL;
		}
    }
    return displayCS;
}
#else
/*
    This version of getTheDisplayColorSpace is equivalent
    to the above version but is a useful starting point
    if you want to support more than the main display.
*/  
CGColorSpaceRef getTheDisplayColorSpace(void)
{
    static CGColorSpaceRef displayCS = NULL;
    if(displayCS == NULL){
		CMProfileRef displayProfile = NULL;
		/*
			Get the display ID of the main display.
			
			For displays other than the main display use
			the functions CGGetDisplaysWithPoint, 
			CGGetDisplaysWithRect, etc. in CGDirectDisplay.h.
		*/
		CGDirectDisplayID displayID = CGMainDisplayID();
		// The CGDirectDisplayID is the same as the 
		// CMDisplayIDType passed to CMGetProfileByAVID.
		CMError err = CMGetProfileByAVID((CMDisplayIDType)displayID, 
						&displayProfile);
		if(err || displayProfile == NULL){
			fprintf(stderr, 
			"Got error %d when getting profile for main display!\n", err);
			return NULL;
		}
		
		displayCS = CGColorSpaceCreateWithPlatformColorSpace(displayProfile);
		CMCloseProfile(displayProfile);
    }
    return displayCS;
}
#endif

// Some routines with 'Get' semantics that return a CGColorRef that
// represents a given opaque RGB color.

CGColorRef getRGBOpaqueWhiteColor(void)
{
    static CGColorRef rgbWhite = NULL;
    if(rgbWhite == NULL){
		float opaqueWhite[4] = { 1., 1., 1., 1. };
		rgbWhite = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueWhite);
    }
    return rgbWhite;
}

CGColorRef getRGBOpaqueBlackColor(void)
{
    static CGColorRef rgbBlack = NULL;
    if(rgbBlack == NULL){
		float opaqueBlack[4] = { 0., 0., 0., 1. };
		rgbBlack = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueBlack);
    }
    return rgbBlack;
}

CGColorRef getOpaqueGrayColor(void)
{
    static CGColorRef rgbGray = NULL;
    if(rgbGray == NULL){
		float opaqueGray[4] = { 0.9, 0.9, 0.9, 1. };
		rgbGray = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueGray);
    }
    return rgbGray;
}

CGColorRef getRGBOpaqueRedColor(void)
{
    static CGColorRef rgbRed = NULL;
    if(rgbRed == NULL){
		float opaqueRed[4] = { 0.663, 0., 0.031, 1. };
		rgbRed = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueRed);
    }
    return rgbRed;
}

CGColorRef getRGBOpaqueBlueColor(void)
{
    static CGColorRef rgbBlue = NULL;
    if(rgbBlue == NULL){
		float opaqueBlue[4] = { 0.482, 0.62, 0.871, 1. };   // This is a light blue.
		rgbBlue = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueBlue);
    }
    return rgbBlue;
}

CGColorRef getRGBOpaquePurpleColor(void)
{
    static CGColorRef rgbPurple = NULL;
    if(rgbPurple == NULL){
		float opaquePurple[] = { 0.69, 0.486, 0.722, 1. };   // This is a purple.
		rgbPurple = CGColorCreate(getTheCalibratedRGBColorSpace(), opaquePurple);
    }
    return rgbPurple;
}

CGColorRef getRGBOpaqueDarkBlueColor(void)
{
    static CGColorRef rgbBlue = NULL;
    if(rgbBlue == NULL){
		float opaqueBlue[4] = { 0.11, 0.208, 0.451, 1. };   // This is a dark blue.
		rgbBlue = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueBlue);
    }
    return rgbBlue;
}

CGColorRef getRGBOpaqueBrownColor(void)
{
    static CGColorRef rgbBrown = NULL;
    if(rgbBrown == NULL){
		float opaqueBrown[4] = { 0.325, 0.208, 0.157, 1. };   // This is a brown.
		rgbBrown = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueBrown);
    }
    return rgbBrown;
}

CGColorRef getRGBOpaqueOrangeColor(void)
{
    static CGColorRef rgbOrange = NULL;
    if(rgbOrange == NULL){
		float opaqueOrange[4] = { 0.965, 0.584, 0.059, 1. };   // This is an orange.
		rgbOrange = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueOrange);
    }
    return rgbOrange;
}

CGColorRef getRGBOpaqueYellowColor(void)
{
    static CGColorRef rgbOrange = NULL;
    if(rgbOrange == NULL){
		float opaqueOrange[4] = { 1., 0.816, 0., 1. };   // This is a yellow.
		rgbOrange = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueOrange);
    }
    return rgbOrange;
}

CGColorRef getRGBOpaqueGreenColor(void)
{
    static CGColorRef rgbGreen = NULL;
    if(rgbGreen == NULL){
		float opaqueGreen[4] = { 0.584, 0.871, 0.318, 1. };   // This is a light green.
		rgbGreen = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueGreen);
    }
    return rgbGreen;
}

CGColorRef getRGBOpaqueDarkGreenColor(void)
{
    static CGColorRef rgbGreen = NULL;
    if(rgbGreen == NULL){
		float opaqueGreen[4] = { 0.404, 0.808, 0.239, 1. };   // This is a dark green.
		rgbGreen = CGColorCreate(getTheCalibratedRGBColorSpace(), opaqueGreen);
    }
    return rgbGreen;
}

void myCGContextAddEllipseInRect(CGContextRef context, CGRect r)
{
    if(&CGContextAddEllipseInRect != NULL){
		CGContextAddEllipseInRect(context, r);
	}else{
		/*
			This is not a perfect emulation but is correct as long as there is not an open 
			subpath already in the current path. In that case the CGContextClosePath here
			would not necessarily produce the desired result.
		*/
		CGContextSaveGState(context);
			// Translate to the center of the ellipse.
			CGContextTranslateCTM(context, 
					CGRectGetMidX(r),
					CGRectGetMidY(r));
			// Scale by half the width and height of the rectangle 
			// bounding the ellipse.
			CGContextScaleCTM(context, r.size.width/2, r.size.height/2);
			// Establish a current point at the first point
			// on the ellipse. This ensures that there
			// is no line segment connecting the previous
			// current point to the first point on the subpath.
			CGContextMoveToPoint(context, 1, 0);
			// Circular arc around the ellipse center with
			// a radius that, when scaled by the CTM, produces
			// the major and minor axes of the ellipse. Since
			// CGContextAddEllipseInRect defines the direction
			// of the path as clockwise, this routine will
			// draw the arc clockwise also.
			CGContextAddArc(context, 0, 0, 1, 0, 2*M_PI, 1);
			CGContextClosePath(context);
		CGContextRestoreGState(context);
	}
}

// Routines that are useful for debugging.

void drawPoint(CGContextRef context, CGPoint p)
{
    CGContextSaveGState(context);
	// Opaque black.
	CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
	CGContextSetLineWidth(context, 5);
	CGContextSetLineCap(context, kCGLineCapRound);
	CGContextMoveToPoint(context, p.x, p.y);
	CGContextAddLineToPoint(context, p.x, p.y);
	CGContextStrokePath(context);
    CGContextRestoreGState(context);
}


void printCTM(CGContextRef context)
{
    CGAffineTransform t = CGContextGetCTM(context);
    fprintf(stderr, "CurrentCTM is a = %f, b = %f, c = %f, d = %f, tx = %f, ty = %f\n",
			t.a, t.b, t.c, t.d, t.tx, t.ty);
}

#define kTickLength 5.0
#define kTickDistance 72.0
#define kAxesLength (20*kTickDistance)

void drawCoordinateAxes(CGContextRef context)
{
	int i;
	float t;
	float tickLength = kTickLength;

	CGContextSaveGState(context);
	
	CGContextBeginPath(context);
	// Paint the x-axis in red.
	CGContextSetRGBStrokeColor(context, 1, 0, 0, 1);
	CGContextMoveToPoint(context, -kTickLength, 0.);
	CGContextAddLineToPoint(context, kAxesLength, 0.);
	CGContextDrawPath(context, kCGPathStroke);

	// Paint the y-axis in blue.
	CGContextSetRGBStrokeColor(context, 0, 0, 1, 1);
	CGContextMoveToPoint(context, 0, -kTickLength);
	CGContextAddLineToPoint(context, 0, kAxesLength);
	CGContextDrawPath(context, kCGPathStroke);
	
	// Paint the x-axis tick marks in red.
	CGContextSetRGBStrokeColor(context, 1, 0, 0, 1);
	for(i = 0; i < 2 ; i++)
	{
		for(t=0.; t < kAxesLength ; t += kTickDistance){
			CGContextMoveToPoint(context, t, -tickLength);
			CGContextAddLineToPoint(context, t, tickLength);
		}
		CGContextDrawPath(context, kCGPathStroke);
		CGContextRotateCTM(context, M_PI/2.);
		// Paint the y-axis tick marks in blue.
		CGContextSetRGBStrokeColor(context, 0, 0, 1, 1);
	}
	drawPoint(context, CGPointZero);
	CGContextRestoreGState(context);
}

void drawDebuggingRect(CGContextRef context, CGRect rect)
{
	CGContextSaveGState(context);
		CGContextSetLineWidth(context, 4.);
		// Draw opaque red from top-left to bottom-right.
		CGContextSetRGBStrokeColor(context, 1, 0, 0, 1.);
		CGContextMoveToPoint(context, rect.origin.x,
						rect.origin.y + rect.size.height);
		CGContextAddLineToPoint(context, 
						rect.origin.x + rect.size.width,
						rect.origin.y);
		CGContextStrokePath(context);
		// Draw opaque blue from top-right to bottom-left.
		CGContextSetRGBStrokeColor(context, 0, 0, 1, 1.);
		CGContextMoveToPoint(context, rect.origin.x + rect.size.width,
						rect.origin.y + rect.size.height);
		CGContextAddLineToPoint(context, rect.origin.x, 
										rect.origin.y);
		CGContextStrokePath(context);
		// Opaque black.
		CGContextSetRGBStrokeColor(context, 0, 0, 0, 1.);
		CGContextStrokeRect(context, rect);
	CGContextRestoreGState(context);
}

