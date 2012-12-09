/*
*  File:    Images.c
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

#include "Images.h"
#include "DataProvidersAndConsumers.h"
#include "Utilities.h"

void drawJPEGImage(CGContextRef context, CFURLRef url)
{
	CGRect  jpgRect;
	CGImageRef jpgImage = NULL;
	// Create a Quartz data provider for the supplied URL.
	CGDataProviderRef jpgProvider = CGDataProviderCreateWithURL(url);
	if(jpgProvider == NULL){
		fprintf(stderr, "Couldn't create JPEG Data provider!\n");
		return;
	}
	
	// Create the CGImageRef for the JPEG image from the data provider.
	jpgImage = CGImageCreateWithJPEGDataProvider(jpgProvider, NULL, 
									true, kCGRenderingIntentDefault);
    
	// CGImageCreateWithJPEGDataProvider retains the data provider.
	// Since this code created the data provider and this code no
	// longer needs it, it must release it.
	CGDataProviderRelease(jpgProvider);
	if(jpgImage == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for JPEG data!\n");
		return;
	}
		
	// Make a rectangle that has its origin at (0,0) and 
	// has a width and height that is 1/4 the native width 
	// and height of the image.
	jpgRect = CGRectMake(0., 0., CGImageGetWidth(jpgImage)/4, CGImageGetHeight(jpgImage)/4);
    
	// Draw the image into the rectangle.
	// This is Image 1.
	CGContextDrawImage(context, jpgRect, jpgImage);
	
	CGContextSaveGState(context);
		// Translate to the top-right corner of the image just drawn.
		CGContextTranslateCTM(context, jpgRect.size.width,
									jpgRect.size.height);
		// Rotate by -90 degrees.
		CGContextRotateCTM(context, DEGREES_TO_RADIANS(-90));
		// Translate in -x by the width of the drawing.
		CGContextTranslateCTM(context, -jpgRect.size.width, 0);
		
		// Draw the image into the same rectangle as before.
		// This is Image 2.
		CGContextDrawImage(context, jpgRect, jpgImage);
	CGContextRestoreGState(context);
	
	CGContextSaveGState(context);
		// Translate so that the next drawing of the image appears
		// below and to the right of the image just drawn.
		CGContextTranslateCTM(context, 
			jpgRect.size.width+jpgRect.size.height, jpgRect.size.height);
		// Scale the y axis by a negative value and flip the image.
		CGContextScaleCTM(context, 0.75, -1.0);
		// This is Image 3.
		CGContextDrawImage(context, jpgRect, jpgImage);
	CGContextRestoreGState(context);

	// Adjust the position of the rectangle so that its origin is
	// to the right and above where Image 3 was drawn. Adjust the
	// size of the rectangle so that it is 1/4 the image width 
	// and 1/6 the image height.
	jpgRect = CGRectMake( 1.75*jpgRect.size.width + jpgRect.size.height, 
				jpgRect.size.height,
				CGImageGetWidth(jpgImage)/4, 
				CGImageGetHeight(jpgImage)/6);
	// This is Image 4.
	CGContextDrawImage(context, jpgRect, jpgImage);

	// Release the CGImageRef when it is no longer needed.
	CGImageRelease(jpgImage);
}

void drawImageFromURL(CGContextRef context, CFURLRef url,
					size_t width, size_t height, 
					size_t bitsPerComponent, Boolean isRGB)
{
	CGImageRef image = NULL;
	CGRect imageRect;
	// This routine treats color images as RGB
	size_t bitsPerPixel = 
			isRGB ? (bitsPerComponent * 3) : bitsPerComponent;
	size_t bytesPerRow = (width * bitsPerPixel + 7)/8;
	bool shouldInterpolate = true;
	CGColorSpaceRef colorspace = NULL;
	// Create a Quartz data provider from the supplied URL.	 
	CGDataProviderRef dataProvider = CGDataProviderCreateWithURL(url);
	if(dataProvider == NULL){
		fprintf(stderr, "Couldn't create Image data provider!\n");
		return;
	}
	// Get a Quartz color space object appropriate for the image type.
	colorspace = isRGB ? 
				getTheCalibratedRGBColorSpace() : 
				getTheCalibratedGrayColorSpace();
	// Create an image of the width, height, and bitsPerComponent with
	// no alpha data, the default decode array, with interpolation,
	// and the default rendering intent for images. This code is
	// intended for Gray images of the format GGGGG... or RGB images
	// of the format RGBRGBRGB... .
	image = CGImageCreate(width, height, bitsPerComponent, 
					bitsPerPixel, bytesPerRow, colorspace,
					kCGImageAlphaNone, dataProvider, NULL,
					shouldInterpolate, kCGRenderingIntentDefault);
	// Quartz retains the data provider with the image and since this
	// code does not create any more images with the data provider, it
	// can release it.
	CGDataProviderRelease(dataProvider);
	if(image == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for this data!\n");
		return;
	}
	// Create a rectangle into which the code will draw the image.
	imageRect = CGRectMake(0., 0., width, height);
	
	// Draw the image into the rectangle.
	CGContextDrawImage(context, imageRect, image);
	// Release the CGImage object when it is no longer needed.
	CGImageRelease(image);
}

void doColorRampImage(CGContextRef context)
{
	CGImageRef image = NULL;
	CGRect imageRect;
	size_t width = 256, height = 256;
	size_t bitsPerComponent = 8, bitsPerPixel = 24;
	size_t bytesPerRow = width * 3;
	bool shouldInterpolate = true;
	CGColorSpaceRef colorspace = NULL;
	
	CGDataProviderRef imageDataProvider = createRGBRampDataProvider();
	if(imageDataProvider == NULL){
		fprintf(stderr, "Couldn't create Image Data provider!\n");
		return;
	}
	
	colorspace = getTheCalibratedRGBColorSpace();
	image = CGImageCreate(width, height, bitsPerComponent,
				bitsPerPixel, bytesPerRow, colorspace, kCGImageAlphaNone,
				imageDataProvider, NULL, shouldInterpolate, 
				kCGRenderingIntentDefault);
	// No longer need the data provider.
	CGDataProviderRelease(imageDataProvider);
	if(image == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for this data!\n");
		return;
	}
	imageRect = CGRectMake(0., 0, width, height);
	// Draw the image.
	CGContextDrawImage(context, imageRect, image);
	// Release the image. Note that depending on the use of this
	// image, this may or may not cause the imageDataProvider to
	// be released immediately.
	CGImageRelease(image);
}

void doImageWithCallbacksCreatedFromURL(CGContextRef context, CFURLRef url, size_t width,
				    size_t height, size_t bitsPerComponent, 
				    Boolean isRGB)
{
	CGImageRef image = NULL;
	CGRect imageRect;
	size_t bitsPerPixel = isRGB ? (bitsPerComponent * 3) : bitsPerComponent;
	size_t bytesPerRow = ( (width * bitsPerPixel) + 7)/8;
	bool shouldInterpolate = true;
	CGColorSpaceRef colorspace = NULL;

	CGDataProviderRef dataProvider = createSequentialAccessDPForURL(url);
	if(dataProvider == NULL){
		fprintf(stderr, "Couldn't create Image Data provider!\n");
		return;
	}
	
	// Create a Quartz color space object appropriate for the image type.
	// These user written functions create the color space object
	// and that reference must be released by this code.
	colorspace = isRGB ? getTheCalibratedRGBColorSpace() : getTheCalibratedGrayColorSpace();
	image = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorspace, 
					kCGImageAlphaNone, dataProvider, NULL, shouldInterpolate, 
					kCGRenderingIntentDefault);
	CGDataProviderRelease(dataProvider);
	if(image == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for this data!\n");
		return;
	}
	
	imageRect = CGRectMake(0., 0., width, height);
	
	// Draw the image into the rectangle.
	CGContextDrawImage(context, imageRect, image);
	// Release the CGImageRef when it is no longer needed.
	CGImageRelease(image);
}

void doGrayRamp(CGContextRef context)
{
	CGImageRef image = NULL;
	CGRect imageRect;
	size_t width = 256, height = 1;
	size_t bitsPerComponent = 8, bitsPerPixel = 8;
	size_t bytesPerRow = width;
	bool shouldInterpolate = true;
	CGDataProviderRef dataProvider = NULL;
	CGColorSpaceRef colorspace = NULL; 

	dataProvider = createGrayRampDirectAccessDP();
	if(dataProvider == NULL){
		fprintf(stderr, "Couldn't create Gray Ramp provider!\n");
		return;
	}
	colorspace = getTheCalibratedGrayColorSpace();
	image = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, 
			bytesPerRow, colorspace, kCGImageAlphaNone, dataProvider, 
			NULL, shouldInterpolate, 
			kCGRenderingIntentDefault);
	CGDataProviderRelease(dataProvider);
	if(image == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for image data!\n");
		return;
	}
	imageRect = CGRectMake(0., 0., 256, 256);
	// Drawing the image that is 256 samples wide and
	// 1 scanline high into a rectangle that is 256 x 256 units
	// on a side causes Quartz to stretch the image to fill
	// the destination rectangle.
	CGContextDrawImage(context, imageRect, image);
	CGImageRelease(image);
}

/*  This routine examines the CGImageSource at index 0 to 
    determine if the first image is a floating point image and 
    if it is, it returns an options dictionary suitable for 
    passing to CGImageSourceCreateImageAtIndex in order to create
    a CGImageRef that contains full dynamic range floating point data. 
*/
static CFDictionaryRef createFloatingPointImageOptions(CGImageSourceRef imageSource) 
{ 
	CFDictionaryRef properties, options = NULL;
	Boolean isFloat = false;
	CFTypeRef keys[1];
	CFTypeRef values[1];
	// Allow the image to be a floating point image. 
	// Without this, Quartz would return integer pixel data, even for 
	// floating point images. Typically you don't need floating point data
	// but in some special cases you might want it.
	keys[0] = kCGImageSourceShouldAllowFloat;
	values[0] = kCFBooleanTrue;

	options = CFDictionaryCreate(NULL, keys, values, 1,  
				&kCFTypeDictionaryKeyCallBacks,
				&kCFTypeDictionaryValueCallBacks);
	// Obtain the properties for the first image
	// in the image source. This is a 'Copy' function
	// so the code owns a reference to the
	// dictionary returned.
	properties = CGImageSourceCopyPropertiesAtIndex(imageSource, 
							    0, options);
	if(properties != NULL){
	    CFTypeRef value;
	    // Get the value for the kCGImagePropertyIsFloat if it exists
	    // and if the value is a CFBoolean then get the corresponding 
		// Boolean result.
	    if( CFDictionaryGetValueIfPresent(properties, kCGImagePropertyIsFloat, &value)
			 && CFGetTypeID(value) == CFBooleanGetTypeID()
	    ){
			isFloat = CFBooleanGetValue(value);
	    }
	    CFRelease(properties);
	}
	
	if(!isFloat){
	    // Release the options dictionary if the image isn't  
	    // a floating point image, otherwise return it.
	    CFRelease(options);
	    options = NULL;
	}
	
	return options;
}

static CGImageRef myCreateImageUsingImageSource(CFURLRef url, float* xdpiP, float* ydpiP)
 { 
	CGImageRef image = NULL;
	CGImageSourceRef imageSource = NULL; 
	CFDictionaryRef properties = NULL;
	CFNumberRef val;
	CFDictionaryRef options = NULL; 

	// Set to zero, indicating the property was unavailable.
	*xdpiP = *ydpiP = 0;

	// Create the image source from the URL.
	imageSource = CGImageSourceCreateWithURL(url, NULL);
	if(imageSource == NULL){
	    fprintf(stderr, "Couldn't create image source from URL!\n");
	    return NULL;
	}

#if 0	// Only use if you want floating point image data.
	options = createFloatingPointImageOptions(imageSource);
	fprintf(stderr, "image %s a floating point image\n", options ? "IS" : "is NOT");
#endif
	
	// Obtain the properties dictionary for the first image
	// in the image source. This is a copy function so this
	// code owns the reference returned and must
	// must release it.
	properties = CGImageSourceCopyPropertiesAtIndex(
			imageSource, 0, options); 
	if(properties != NULL){
	    // Check for the x and y resolution of the image.
	    val = CFDictionaryGetValue(properties, kCGImagePropertyDPIWidth);
	    if (val != NULL) 
			CFNumberGetValue(val, kCFNumberFloatType, xdpiP); 
	    val = CFDictionaryGetValue(properties, kCGImagePropertyDPIHeight);
	    if (val != NULL) 
			CFNumberGetValue(val, kCFNumberFloatType, ydpiP);
	    CFRelease(properties);
	}
	
	// Create a CGImageRef from the first image in the CGImageSource.
	image = CGImageSourceCreateImageAtIndex(imageSource, 0, options);
	// Release the CGImageSource object since it is no longer needed
	// and this code created it. This code uses CFRelease since a
	// CGImageSource object is a CoreFoundation object.
	CFRelease(imageSource);
	
	if(options)
	    CFRelease(options);
	
	if(image == NULL){
	    fprintf(stderr, "Couldn't create image from image source!\n");
	    return NULL;
	}
	
	return image;
}

static CGImageRef myCreateThumbnailFromImageSource(CFURLRef url) 
{ 
	CGImageRef thumb = NULL;
	CGImageSourceRef imageSource = NULL;
	int maxThumbSize = 160;
	CFNumberRef maxThumbSizeRef;
	CFStringRef keys[2];
	CFTypeRef values[2];
	CFDictionaryRef options = NULL;
	
	// Create the image source from the URL.
	imageSource = CGImageSourceCreateWithURL(url, NULL);
	if(imageSource == NULL){
	    fprintf(stderr, "Couldn't create image source from URL!\n");
	    return NULL;
	}

	// Specify 160 pixels as the maximum width and height of 
	// the thumbnail for Quartz to create. 
	maxThumbSizeRef = CFNumberCreate(NULL, kCFNumberIntType, 
						    &maxThumbSize);
	
	// Request that Quartz create a thumbnail image if
	// thumbnail data isn't present in the file.
	keys[0] = kCGImageSourceCreateThumbnailFromImageIfAbsent;
	values[0] = (CFTypeRef)kCFBooleanTrue;
	
	// Request that the maximum size of the thumbnail is
	// that specified by maxThumbSizeRef, 160 pixels.
	keys[1] = kCGImageSourceThumbnailMaxPixelSize; 
	values[1] = (CFTypeRef)maxThumbSizeRef;

	// Create an options dictionary with these keys
	options = CFDictionaryCreate(NULL, 
			(const void **)keys, 
			(const void **)values, 
			2,  
			&kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks); 
	// Release the CFNumber this code created.
	CFRelease(maxThumbSizeRef);
	
	// Create the thumbnail image for the first image in the
	// image source, that at index 0, using the options
	// dictionary that the code just created.
	thumb = CGImageSourceCreateThumbnailAtIndex(imageSource, 0, options);
	
	// Release the options dictionary.
	CFRelease(options);
	// Release the image source the code created.
	CFRelease(imageSource);

	if(thumb == NULL){
	    fprintf(stderr, "Couldn't create thumbnail from image source!\n");
	    return NULL;
	}
	
	return thumb;
}

bool imageHasFloatingPointSamples(CGImageRef image)
{
    if(&CGImageGetBitmapInfo != NULL){
		return (kCGBitmapFloatComponents & CGImageGetBitmapInfo(image));
    }
    return false;
}
 
void drawImageWithCGImageDataSource(CGContextRef context, CFURLRef url)
{
	CGRect  imageRect;
	CGImageRef image = NULL;
	float xdpi, ydpi;
	Boolean isFloatingImage = false;
	
	// This code would be better if it created the image source
	// once and used the same image source to create the image and its 
	// thumbnail, but the point here is to simply test the routines 
	// myCreateImageUsingImageSource and myCreateThumbnailFromImageSource.
	
	image = myCreateImageUsingImageSource(url, &xdpi, &ydpi);
	if(image == NULL){
		fprintf(stderr, "myCreateImageFromImageSource didn't create a CGImage!\n");
		return;
	}
	fprintf(stdout, "xdpi = %2.f, ydpi = %2.f\n", xdpi, ydpi);
	imageRect = CGRectMake(0., 0., CGImageGetWidth(image)/3, CGImageGetHeight(image)/3);
	CGContextDrawImage(context, imageRect, image);

#if 0
	isFloatingImage = imageHasFloatingPointSamples(image);
	fprintf(stdout, "First image %s a floating point image\n", isFloatingImage ? "IS" : "is NOT");
#endif

	CGImageRelease(image);
	
	image = myCreateThumbnailFromImageSource(url);
	if(image == NULL){
		fprintf(stderr, "myCreateThumbnailFromImageSource didn't create a CGImage!\n");
		return;
	}
	imageRect = CGRectMake(400., 0., CGImageGetWidth(image), CGImageGetHeight(image));
	CGContextDrawImage(context, imageRect, image);

	CGImageRelease(image);

	
}

 
typedef struct MyIncrementalData{
    UInt8 *data;
    size_t dataSize;
    int repCount;
    size_t chunkSize;
}MyIncrementalData;

// This is a dummy data accumulation routine used to demonstrate incremental loading
// of an image.
static CFDataRef myCreateAccumulatedDataSoFar(MyIncrementalData *myDataP, bool *doneP)
{
    CFDataRef data = NULL;
    bool done = false;
    size_t sizeToReturn;
    myDataP->repCount++;
    sizeToReturn = myDataP->chunkSize*myDataP->repCount;
    if(sizeToReturn > myDataP->dataSize)
		sizeToReturn = myDataP->dataSize;

    if(sizeToReturn == myDataP->dataSize)
		done = true;
	
    data = CFDataCreate(NULL, myDataP->data, sizeToReturn);
    
    *doneP = done;
    
    return data;
}

static void MyDrawIncrementalImage(CGContextRef context, 
			CGImageRef image,  float fullHeight)
{
    // Obtain the width and height of the image that has been 
    // accumulated so far.
    float width = CGImageGetWidth(image);
    float height = CGImageGetHeight(image);
    // Adjust the location of the imageRect so that the origin is 
    // such that the full image would be located at 0,0 and the partial 
    // image top-left corner does not move as the image is filled in. 
	// This is only needed for views where the y axis points up the
	// drawing canvas.
    CGRect imageRect = CGRectMake(0, fullHeight-height, width, height);
    CGContextDrawImage(context, imageRect, image);
}

static void myDrawFirstImageIncrementally(CGContextRef context, 
					    MyIncrementalData *myDataP) 
{ 
    bool done;
    float height = -1;
    CGRect imageRect;
    CGImageSourceStatus status;
    // Create an incremental image source.
    CGImageSourceRef imageSource = CGImageSourceCreateIncremental(NULL);
    if(imageSource == NULL){
		fprintf(stderr, "Couldn't create incremental imagesource!\n");
		return;
    }

    // Loop, gathering the necessary data to find the true
    // height of the image.
    do
	{
		/*  Fetch the data. The CFData object returned by
			myCreateAccumulatedDataSoFar is used to update the
			image source. When the data is complete, the code
			passes true in the 'done' parameter passed to 
			CGImageSourceUpdateData. Once the data is passed
			to CGImageSourceUpdateData, the code can release
			its reference to the data. */
		// Accumulate the data.
		CFDataRef data = myCreateAccumulatedDataSoFar(myDataP, &done);
		CGImageSourceUpdateData(imageSource, data, done);
		// Release the data since Quartz retains it and this code
		// no longer needs it.
		CFRelease(data);

		if(height < 0){
			// Determine the height of the full image. This is needed in order
			// to adjust the location of the drawing of the partial image in
			// a context where the y axis has the default Quartz orientation
			// pointing up the drawing canvas.
			CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(
							imageSource, 0, NULL);
			if(properties){
				CFTypeRef val = CFDictionaryGetValue(properties, kCGImagePropertyPixelHeight);
			if (val != NULL) 
				CFNumberGetValue(val, kCFNumberFloatType, &height); 
			
			CFRelease(properties);
			}
		}
		// Once the height is obtained, go ahead and see if Quartz
		// has enough data to create a CGImage object.
		if(height > 0){
			// Now create the CGImageRef from the image source for the
			// first image.
			CGImageRef image = CGImageSourceCreateImageAtIndex(
								imageSource, 0, NULL);
			if(image){ 
				// Draw the image using the height of the full image
				// to adjust the location where the image is drawn.
				MyDrawIncrementalImage(context, image, height);
				// Release the partial image once you've drawn it.
				CGImageRelease(image);
				// Potentially you would want to flush the context so
				// that drawing to a window would appear, even inside
				// this loop. Of course this flush should really be
				// done on a timer so that the flush only occurs at
				// most every 60th of a second. See Chapter 17 regarding
				// timing your usage of CGContextFlush.
				CGContextFlush(context);
			} 
		}
		// Obtain the status for the image source for the first image.
		status = CGImageSourceGetStatusAtIndex(imageSource, 0);

	// Continue the loop until either all the data has loaded
	// or the status of the first image is complete.
	} while (!done &&  status != kCGImageStatusComplete);
    CFRelease(imageSource);
}

#include <sys/syslimits.h>
#include <sys/types.h>
#include <sys/stat.h>
static void createMyIncrementalDataFromURL(CFURLRef url, MyIncrementalData *myDataP)
{
    char pathString[PATH_MAX + 1];
    Boolean success;
    
    myDataP->data = NULL;
    myDataP->dataSize = 0;
    myDataP->repCount = 0;
    
    success = CFURLGetFileSystemRepresentation(url, true, pathString, sizeof(pathString));

    if(success && pathString[0] != 0){
		struct stat sb;
		if(!stat(pathString, &sb)){
			FILE *fp;
			fp = fopen(pathString, "r");
			if(fp){
				size_t dataSize = sb.st_size;
				unsigned char *data = (unsigned char *)malloc(dataSize);
				if(data){
					dataSize = fread(data, 1, dataSize, fp);
					myDataP->data = data;
					myDataP->dataSize = dataSize;
				}else{
					fprintf(stderr, "can't malloc memory for image\n");
				}
				fclose(fp);
			}
		}
    }
    if(myDataP->dataSize > 0){
		myDataP->chunkSize = myDataP->dataSize/10;   // 10 chunks
    }
}

static void releaseIncrementalData(MyIncrementalData *myDataP)
{
    free(myDataP->data);
    myDataP->data = NULL;
}

void doIncrementalImageWithURL(CGContextRef context, CFURLRef url)
{
    MyIncrementalData myData;
    createMyIncrementalDataFromURL(url, &myData);
    if(myData.data == NULL){
		fprintf(stderr, "couldn't read data from URL!\n");
    }
    myDrawFirstImageIncrementally(context, &myData);
    releaseIncrementalData(&myData);
}

// This code requires QuickTime.framework.
#include <QuickTime/QuickTime.h>
static CGImageRef createCGImageWithQuickTimeFromURL(CFURLRef url)
{
	OSStatus err = noErr;
	CGImageRef			imageRef = NULL;
	Handle			dataRef = NULL;
	OSType			dataRefType;
	GraphicsImportComponent						gi;
	ComponentResult				result;
	result = QTNewDataReferenceFromCFURL(url,
							0, &dataRef, &dataRefType);
	if(NULL != dataRef) {
		err = GetGraphicsImporterForDataRefWithFlags(dataRef, 
									dataRefType, &gi, 0);
		if(!err && gi){
			// Tell the graphics importer that it shouldn’t perform
			// gamma correction and it should create an image in
			// the original source color space rather than matching it to
			// a generic calibrated color space.
			result = GraphicsImportSetFlags(gi,
				    		(kGraphicsImporterDontDoGammaCorrection +
				    		kGraphicsImporterDontUseColorMatching)
					);
			if(!result)
			    result = GraphicsImportCreateCGImage(gi,&imageRef,0);
			if(result)
				fprintf(stderr,"got a bad result = %d!\n", (int)result);
			DisposeHandle(dataRef);
			CloseComponent(gi);
		}
	}
	return imageRef;
}

void drawQTImageWithQuartz(CGContextRef context, CFURLRef url)
{
	CGRect  imageRect;
	CGImageRef image = NULL;
	image = createCGImageWithQuickTimeFromURL(url);
	if(image == NULL){
		fprintf(stderr, "createCGImageWithQuickTimeFromURL didn't create a CGImage!\n");
		return;
	}
	imageRect = CGRectMake(0., 0., CGImageGetWidth(image), CGImageGetHeight(image));
	CGContextDrawImage(context, imageRect, image);
	CGImageRelease(image);
}

void drawJPEGDocumentWithMultipleProfiles(CGContextRef context, CFURLRef url)
{
	CGImageRef jpgImage = NULL, updatedImage1 = NULL, updatedImage2 = NULL;
	Boolean isDeviceRGBImage = false;
	CGColorSpaceRef originalColorSpace = NULL;
	CGColorSpaceRef comparisonColorSpace = NULL;
	CGRect imageRect;
	// Create a Quartz data provider for the supplied URL.
	CGDataProviderRef jpgProvider = CGDataProviderCreateWithURL(url);
	if(jpgProvider == NULL){
		fprintf(stderr, "Couldn't create JPEG Data provider!\n");
		return;
	}
	
	// Create the CGImageRef for the JPEG image from the data provider.
	jpgImage = CGImageCreateWithJPEGDataProvider(jpgProvider, NULL, 
									true, kCGRenderingIntentDefault);
	CGDataProviderRelease(jpgProvider);
	if(jpgImage == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for JPEG data!\n");
		return;
	}
	
	// Get the color space characterizing the image. This is a 
	// function with 'Get' semantics so the code doesn't own a reference
	// to the color space returned and must not release it.
	originalColorSpace = CGImageGetColorSpace(jpgImage);
	if(originalColorSpace == NULL){
	    fprintf(stderr, "image is a masking image, not an image with color!\n");
	    return;
	}
	
	if(CGColorSpaceGetNumberOfComponents(originalColorSpace) != 3){
	    fprintf(stderr, "This example only works with 3 component JPEG images\n!");
		return;
	}
	// Determine if the original color space is DeviceRGB. If that is
	// not the case then bail.
	comparisonColorSpace = CGColorSpaceCreateDeviceRGB();
	
	// Note that this comparison of color spaces works only on 
	// Jaguar and later where a CGColorSpaceRef is a 
	// CoreFoundation object. Otherwise this will crash!
	isDeviceRGBImage = CFEqual(comparisonColorSpace, originalColorSpace);
	// This code created 'comparisonColorSpace' so it must release it.
	CGColorSpaceRelease(comparisonColorSpace);
	if(!isDeviceRGBImage){
	    fprintf(stderr, "The color space for the JPEG image is not DeviceRGB!\n");
	    return;
	}

	// Might need to adjust this based on the size of the original image.
	CGContextScaleCTM(context, 1./2, 1./2);
	
	imageRect = CGRectMake(0., CGImageGetHeight(jpgImage)/2, 
			CGImageGetWidth(jpgImage), CGImageGetHeight(jpgImage));

	// Draw the original image to the left of the other two.
	CGContextDrawImage(context, imageRect, jpgImage);

	// Recharacterize the original image with the generic Calibrated RGB
	// color space.
	updatedImage1 = CGImageCreateCopyWithColorSpace(jpgImage, 
						getTheCalibratedRGBColorSpace());
	// Release the original image since this code is done with it.
	CGImageRelease(jpgImage);
	if(updatedImage1 == NULL){
	    fprintf(stderr, "There is no updated image to draw!\n");
	    return;
	}

	// Draw the image characterized by the Generic profile
	// to the right of the other image.
	imageRect = CGRectOffset(imageRect, CGRectGetWidth(imageRect) + 10, 0);
	CGContextDrawImage(context, imageRect, updatedImage1);
	
	// Recharacterize the image but now with a color space
	// created with the sRGB profile. 
	updatedImage2 = CGImageCreateCopyWithColorSpace(updatedImage1, 
						getTheSRGBColorSpace());
	// Release updatedImage1 since this code is done with it.
	CGImageRelease(updatedImage1);
	if(updatedImage2 == NULL){
	    fprintf(stderr, "There is no second updated image to draw!\n");
	    return;
	}

	// Draw the image characterized by the sRGB profile to the right of
	// the image characterized by the generic RGB profile.
	imageRect = CGRectOffset(imageRect, CGRectGetWidth(imageRect) + 10, 0);
	CGContextDrawImage(context, imageRect, updatedImage2);
	CGImageRelease(updatedImage2);
}

static void rgbRampSubDataRelease(void *info, const void *data,
					size_t size)
{
    free((char *)info);
}

static unsigned char *createRedGreenRampImageData(size_t width, 
										size_t height, size_t size)
{
	unsigned char *p;
	unsigned char *dataP = (unsigned char *)malloc(size);
	
	if(dataP == NULL)
		return NULL;
	
	p = dataP;
	/*	Build an image that is RGB 24 bits per sample. This is a ramp
		where the red component value increases in red from left to 
		right and the green component increases from top to bottom. */
	int r, g;
	for(g = 0 ; g < height ; g++){
		for(r = 0 ; r < width ; r++){
			*p++ = r; *p++ = g; *p++ = 0;
		}
	}
	return dataP;
}

static CGDataProviderRef createRGBRampSubDataProvider(CGRect subRect)
{
	CGDataProviderRef dataProvider = NULL;
	size_t bytesPerSample = 3;
	size_t width = 256, height = 256;
	size_t bytesPerRow = width*bytesPerSample;
	size_t startOffsetX = subRect.origin.x; 
	size_t startOffsetY = subRect.origin.y;
	size_t imageDataSize = bytesPerRow*height;
	// The first image sample is at 
	// (startOffsetY*bytesPerRow + startOffsetX*bytesPerSample)
	// bytes into the RGB ramp data. 	
	size_t firstByteOffset = startOffsetY*bytesPerRow + 
									startOffsetX*bytesPerSample;
	// The actual size of the image data provided is the full image size
	// minus the amount skipped at the beginning. This is more than the
	// total amount of data that is needed for the subimage but it is
	// valid and easy to calculate.
	size_t totalBytesProvided = imageDataSize - firstByteOffset;
	// Create the full color ramp.
	unsigned char *dataP = createRedGreenRampImageData(width, height,
											 imageDataSize);
	if(dataP == NULL){
	    fprintf(stderr, "Couldn't create image data\n!");
	    return NULL;
	}
	
	// Use the pointer to the first byte as the info parameter since
	// that is the pointer to the block to free when done.
	dataProvider = CGDataProviderCreateWithData(dataP, 
				    dataP + firstByteOffset, 
				    totalBytesProvided, rgbRampSubDataRelease);
	
	if(dataProvider == NULL){
	    free(dataP);
	    return NULL;
	}
	return dataProvider;
}

void doColorRampSubImage(CGContextRef context)
{
	CGImageRef image = NULL;
	CGRect imageSubRect, rect;
	// Start 4 scanlines from the top and 16 pixels from the left edge, 
	// skip the last 40 scanlines of the image and the right 
	// most 64 pixels.
	size_t insetLeft = 16, insetTop = 4, insetRight = 64, insetBottom = 40;
 
	size_t fullImageWidth = 256, fullImageHeight = 256;
	size_t subImageWidth = fullImageWidth-insetLeft-insetRight;
	size_t subImageHeight = fullImageHeight-insetTop-insetBottom;
	size_t bitsPerComponent = 8, bitsPerPixel = 24;
	size_t bytesPerRow = fullImageWidth * 3;
	bool shouldInterpolate = true;
	CGColorSpaceRef colorspace = NULL;
	CGDataProviderRef imageDataProvider = NULL;
	imageSubRect = CGRectMake(insetLeft, insetTop, subImageWidth, subImageHeight);
	colorspace = getTheCalibratedRGBColorSpace();
	if(&CGImageCreateWithImageInRect != NULL){
	    CGImageRef fullImage = NULL;
	    imageDataProvider = createRGBRampDataProvider();
	    if(imageDataProvider == NULL){
			fprintf(stderr, "Couldn't create Image Data provider!\n");
			return;
	    }
	    fullImage = CGImageCreate(fullImageWidth, fullImageHeight,
				bitsPerComponent, 
			    bitsPerPixel, 
			    bytesPerRow, colorspace, kCGImageAlphaNone, 
			    imageDataProvider, NULL, shouldInterpolate, 
			    kCGRenderingIntentDefault);
	    if(fullImage){
			image = CGImageCreateWithImageInRect(fullImage,
												imageSubRect);
			// release the full image since it is no longer required.
			CGImageRelease(fullImage);
	    }
	}
	// If the image hasn't been created yet, this code uses the
	// customized data provider to do so.
	if(image == NULL){
	    imageDataProvider = createRGBRampSubDataProvider(imageSubRect);
	    if(imageDataProvider == NULL){
			fprintf(stderr, "Couldn't create Image Data provider!\n");
			return;
	    }
	
	    // By supplying bytesPerRow, the extra data at the end of 
		// each scanline and the beginning of the next is properly skipped.
	    image = CGImageCreate(subImageWidth, subImageHeight,
					bitsPerComponent,
 					bitsPerPixel,
					bytesPerRow, colorspace, kCGImageAlphaNone,
					imageDataProvider, NULL, shouldInterpolate,
					kCGRenderingIntentDefault);
	}
	// This code no longer needs the data provider.
	CGDataProviderRelease(imageDataProvider);
	if(image == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for this data!\n");
		return;
	}
	// Draw the subimage.
	rect = CGRectMake(0, 0, subImageWidth, subImageHeight);
	CGContextDrawImage(context, rect, image);
	CGImageRelease(image);
}

void exportCGImageToPNGFileWithDestination(CGImageRef image, CFURLRef url)
{
    float resolution = 144.;
    CFTypeRef keys[2]; 
    CFTypeRef values[2];
    CFDictionaryRef options = NULL;
    
    // Create an image destination at the supplied URL that
    // corresponds to the PNG image format.
    CGImageDestinationRef imageDestination = 
		CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
		
    if(imageDestination == NULL){
		fprintf(stderr, "couldn't create image destination!\n");
		return;
    }

    // Set the keys to be the x and y resolution of the image.
    keys[0] = kCGImagePropertyDPIWidth;
    keys[1] = kCGImagePropertyDPIHeight;

	// Create a CFNumber for the resolution and use it as the 
	// x and y resolution.
    values[0] = CFNumberCreate(NULL, kCFNumberFloatType, &resolution);
    values[1] = values[0];
    
    // Create an options dictionary with these keys.
    options = CFDictionaryCreate(NULL, 
		    (const void **)keys, 
		    (const void **)values, 
		    2,  
		    &kCFTypeDictionaryKeyCallBacks,
		    &kCFTypeDictionaryValueCallBacks); 
    
    // Release the CFNumber the code created.
    CFRelease(values[0]);
    
    // Add the image with the options dictionary to the destination.
    CGImageDestinationAddImage(imageDestination, image, options);

    // Release the options dictionary this code created.
    CFRelease(options);

    // When all the images are added to the destination, finalize it. 
    CGImageDestinationFinalize(imageDestination);
    
    // Release the destination when done with it.
    CFRelease(imageDestination);
}


// This code requires QuickTime.framework
#include <QuickTime/QuickTime.h>
static void exportCGImageToJPEGFile(CGImageRef imageRef, CFURLRef url)
{
	Handle                     dataRef = NULL;
	OSType                     dataRefType;
	GraphicsExportComponent    graphicsExporter;
	unsigned long              sizeWritten;
	ComponentResult            result;
	result = QTNewDataReferenceFromCFURL(url, 0, 
								&dataRef, &dataRefType);
	if(!result)
	{
		result = OpenADefaultComponent(GraphicsExporterComponentType,
				    kQTFileTypeJPEG, &graphicsExporter);
		if(!result){
	    		result = GraphicsExportSetInputCGImage(graphicsExporter, 
							imageRef);
	    		if(!result)
					result = GraphicsExportSetOutputDataReference(
								graphicsExporter, dataRef, dataRefType);
	    		if(!result)
					result = GraphicsExportDoExport(graphicsExporter, &sizeWritten);
	    		CloseComponent(graphicsExporter);
		}
	}
	if(dataRef)
		DisposeHandle(dataRef);
	if(result)
		fprintf(stderr, "Exporting QT image got bad result = %d!\n", (int)result);
	return;
}

void exportColorRampImageWithQT(CGContextRef context)
{
	CGImageRef image = NULL;
	size_t width = 256, height = 256;
	size_t bitsPerComponent = 8, bitsPerPixel = 24;
	size_t bytesPerRow = width * 3;
	bool shouldInterpolate = true;
	CGColorSpaceRef colorspace = NULL;
	CGRect rect;
	CGDataProviderRef imageDataProvider = createRGBRampDataProvider();
	if(imageDataProvider == NULL){
		fprintf(stderr, "Couldn't create Image Data provider!\n");
		return;
	}
	colorspace = getTheCalibratedRGBColorSpace();
	image = CGImageCreate(width, height, bitsPerComponent, 
					bitsPerPixel, bytesPerRow, colorspace,
 					kCGImageAlphaNone, imageDataProvider, 
					NULL, shouldInterpolate, kCGRenderingIntentDefault);
	CGDataProviderRelease(imageDataProvider);
	if(image == NULL){
		fprintf(stderr, "Couldn't create CGImageRef for this data!\n");
		return;
	}
	rect = CGRectMake(0.,0., width, height);
	CGContextDrawImage(context, rect, image);
	
	// Of course this is a total hack.
	UInt8 *outPath = "/tmp/imageout.jpg";
	CFURLRef exportURL = CFURLCreateFromFileSystemRepresentation(NULL,
				    outPath, strlen(outPath), false);
	if(exportURL){
	    exportCGImageToJPEGFile(image, exportURL);
	    CFRelease(exportURL);
	}
	CGImageRelease(image);
}

