/*
*  File:    BitmapContext.c
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

#include "Utilities.h"
#include "BitmapContext.h"
#include "Images.h"
#include <QuickTime/QuickTime.h>

#define BEST_BYTE_ALIGNMENT 16
#define COMPUTE_BEST_BYTES_PER_ROW(bpr)		( ( (bpr) + (BEST_BYTE_ALIGNMENT-1) ) & ~(BEST_BYTE_ALIGNMENT-1) )

CGContextRef createRGBBitmapContext(size_t width, size_t height, 
				    Boolean wantDisplayColorSpace,
				    Boolean needsTransparentBitmap)
{
    /*	This routine allocates data for a pixel array that contains width*height
		pixels where each pixel is 4 bytes. The format is 8-bit ARGB or XRGB, depending on
		whether needsTransparentBitmap is true. In order to get the recommended
		pixel alignment, the bytesPerRow is rounded up to the nearest multiple
		of BEST_BYTE_ALIGNMENT bytes. 
	*/
    CGContextRef context;
    size_t bytesPerRow;
    unsigned char *rasterData;
   
    // Minimum bytes per row is 4 bytes per sample * number of samples.
    bytesPerRow = width*4;
    // Round to nearest multiple of BEST_BYTE_ALIGNMENT.
    bytesPerRow = COMPUTE_BEST_BYTES_PER_ROW(bytesPerRow);
    
    // Allocate the data for the raster. The total amount of data is bytesPerRow
    // times the number of rows. The function 'calloc' is used so that the
    // memory is initialized to 0.
    rasterData = calloc(1, bytesPerRow * height);
    if(rasterData == NULL){
		fprintf(stderr, "Couldn't allocate the needed amount of memory!\n");
		return NULL;
    } 
    
    // The wantDisplayColorSpace argument passed to the function determines
    // whether or not to use the display color space or the generic calibrated
    // RGB color space. The needsTransparentBitmap argument determines whether
	// create a context that records alpha or not.
    context = CGBitmapContextCreate(rasterData, width, height, 8, bytesPerRow, 
		    (wantDisplayColorSpace ? getTheDisplayColorSpace(): getTheCalibratedRGBColorSpace()) ,
			(needsTransparentBitmap ? kCGImageAlphaPremultipliedFirst : kCGImageAlphaNoneSkipFirst));
    if(context == NULL){
		// If the context couldn't be created, release the raster memory.
		free(rasterData);
		fprintf(stderr, "Couldn't create the context!\n");
		return NULL;
    }

    // Either clear the rect or paint with opaque white, depending on
    // the needs of the caller.
    if(needsTransparentBitmap){
		// Clear the context bits so they are transparent.
		CGContextClearRect(context, CGRectMake(0, 0, width, height));
    }else{
		// Since the drawing destination is opaque, first paint 
		// the context bits to white.
		CGContextSaveGState(context);
			CGContextSetFillColorWithColor(context, getRGBOpaqueWhiteColor());
			CGContextFillRect(context, CGRectMake(0, 0, width, height));
		CGContextRestoreGState(context);
    }
    return context;
}

static void releaseBitmapContextImageData(void *info, 
				    const void *data, size_t size)
{
	// Only release the image data when Quartz is done with it.
	// Note that this data is the raster data from the bitmap
	// context, so the context had better not be drawn to
	// after the image is created. This is accomplished by
	// releasing the context immediately after creating the
	// image.
    free((char *)data);
}

CGBitmapInfo myCGContextGetBitmapInfo(CGContextRef c)
{
    if(&CGBitmapContextGetBitmapInfo != NULL)
		return CGBitmapContextGetBitmapInfo(c);
    else
		return CGBitmapContextGetAlphaInfo(c);
}

/*	createImageFromBitmapContext creates a CGImageRef
	from a bitmap context. Calling this routine
	transfers 'ownership' of the raster data
	in the bitmap context, to the image. If the
	image can't be created, this routine frees
	the memory associated with the raster. 
*/
CGImageRef createImageFromBitmapContext(CGContextRef c)
{
    CGImageRef image;
	unsigned char *rasterData = CGBitmapContextGetData(c);
    size_t imageDataSize = 
		CGBitmapContextGetBytesPerRow(c)*CGBitmapContextGetHeight(c);
	
	if(rasterData == NULL){
		fprintf(stderr, "Context is not a bitmap context!\n");
		return NULL;
	}
	
    // Create the data provider from the image data, using
	// the image releaser function releaseBitmapContextImageData.
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL,
					    rasterData,
					    imageDataSize,
					    releaseBitmapContextImageData);
    if(dataProvider == NULL){
		// Since this routine owns the raster memory, it must
		// free it if it can't create the data provider.
		free(rasterData);
		fprintf(stderr, "Couldn't create data provider!\n");
		return NULL;
    }
	// Now create the image. The parameters for the image closely match
	// the parameters of the bitmap context. This code uses a NULL
	// decode array and shouldInterpolate is true.
    image = CGImageCreate(CGBitmapContextGetWidth(c), 
			  CGBitmapContextGetHeight(c), 
			  CGBitmapContextGetBitsPerComponent(c), 
			  CGBitmapContextGetBitsPerPixel(c), 
			  CGBitmapContextGetBytesPerRow(c), 
			  CGBitmapContextGetColorSpace(c),
			  myCGContextGetBitmapInfo(c),
			  dataProvider,
			  NULL,
			  true,
			  kCGRenderingIntentDefault);
    // Release the data provider since the image retains it.
    CGDataProviderRelease(dataProvider);
    if(image == NULL){
		fprintf(stderr, "Couldn't create image!\n");
		return NULL;
    }
    return image;
}


static void exportCGImageToFileWithQT(CGImageRef image, CFURLRef url,
					    CFStringRef outputFormat,
					    float dpi)
{
	Handle						dataRef = NULL;
	OSType						dataRefType;
	GraphicsExportComponent		graphicsExporter;
	unsigned long				sizeWritten;
	ComponentResult				result;
	OSType						imageExportType;

	if(CFStringCompare(outputFormat, kUTTypeTIFF, kCFCompareCaseInsensitive) == kCFCompareEqualTo){
	    imageExportType = kQTFileTypeTIFF;
	}else if(CFStringCompare(outputFormat, kUTTypePNG, kCFCompareCaseInsensitive) == kCFCompareEqualTo){
	    imageExportType = kQTFileTypePNG;
	}else if(CFStringCompare(outputFormat, kUTTypeJPEG, kCFCompareCaseInsensitive) == kCFCompareEqualTo){
	    imageExportType = kQTFileTypeJPEG;
	}else{
	    fprintf(stderr, "Requested image export format %@s unsupported\n", outputFormat);
	    return;
	}
	
	result = QTNewDataReferenceFromCFURL(url, 0,  &dataRef, &dataRefType);
	if(!result){
		result = OpenADefaultComponent(GraphicsExporterComponentType,
				    imageExportType, &graphicsExporter);
		if(!result){
			result = GraphicsExportSetInputCGImage(graphicsExporter, 
						image);
			if(!result)
				result = GraphicsExportSetResolution(graphicsExporter, 
							FloatToFixed(dpi), FloatToFixed(dpi));
			if(!result)
				result = GraphicsExportSetOutputDataReference(
							graphicsExporter, dataRef, dataRefType);
			if(!result)
				result = GraphicsExportDoExport(
							graphicsExporter, &sizeWritten);
			CloseComponent(graphicsExporter);
		}
	}
	if(dataRef)
		DisposeHandle(dataRef);
	if(result)
		fprintf(stderr, "QT export got bad result = %d!\n", (int)result);
	return;
}


static void exportCGImageToFileWithDestination(CGImageRef image, CFURLRef url,
					    CFStringRef outputFormat,
					    float dpi)
{
    CFTypeRef keys[2]; 
    CFTypeRef values[2];
    CFDictionaryRef options = NULL;
    
    // Create an image destination at the supplied URL that
    // corresponds to the output image format. The destination will
    // only contain 1 image.
    CGImageDestinationRef imageDestination = 
		CGImageDestinationCreateWithURL(url, outputFormat, 1, NULL);
		
    if(imageDestination == NULL){
		fprintf(stderr, "Couldn't create image destination!\n");
		return;
    }

    // Set the keys to be the x and y resolution of the image.
    keys[0] = kCGImagePropertyDPIWidth;
    keys[1] = kCGImagePropertyDPIHeight;

    // Create a CFNumber for the resolution and use it as the 
    // x and y resolution.
    values[0] = values[1] = CFNumberCreate(NULL, kCFNumberFloatType, &dpi);
    
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

static OSStatus MakeImageDocument(CFURLRef url, CFStringRef imageType, const ExportInfo *exportInfo)
{
    OSStatus err = noErr;
    CGContextRef c = NULL;
    CGImageRef image;
    Boolean useDisplayColorSpace;
    // First make a bitmap context for a US Letter size
    // raster at the requested resolution. 
    int dpi = exportInfo->dpi;
    size_t width = (size_t)(8.5*dpi), height = (size_t)11*dpi;

    // For JPEG output type the bitmap should not be transparent. If other types are added that
	// do not support transparency, this code should be updated to check for those types as well.
    Boolean needTransparentBitmap = 
	    !(CFStringCompare(imageType, kUTTypeJPEG, kCFCompareCaseInsensitive) == kCFCompareEqualTo);

    // Create an RGB Bitmap context using the generic calibrated RGB color space
    // instead of the display color space.
    useDisplayColorSpace = false;
    c = createRGBBitmapContext(width, height, useDisplayColorSpace, 
						    needTransparentBitmap);
    
    if(c == NULL){
		fprintf(stderr, "Couldn't make destination bitmap context!\n");
		// Users of this code should update this to be an error code they find useful.
		return memFullErr;
    }
      
    // Scale the coordinate system based on the resolution in dots per inch.
    CGContextScaleCTM(c, dpi/72, dpi/72);
    
    // Set the font smoothing parameter to false since it's better to
    // draw any text without special LCD text rendering when creating
    // rendered data for export.
    if(CGContextSetShouldSmoothFonts != NULL)
		CGContextSetShouldSmoothFonts(c, false);

    // Set the scaling factor for shadows. This is a hack so that
    // drawing code that needs to know the scaling factor can
    // obtain it. Better would be that DispatchDrawing and the code
    // it calls would take this scaling factor as a parameter.
    setScalingFactor(dpi/72);
    
    // Draw into that raster...
    DispatchDrawing(c, exportInfo->command);
    
    // Set the scaling factor back to 1.0.
    setScalingFactor(1.0);
    
    // Create an image from the raster data. Calling
	// createImageFromBitmapContext gives up ownership
	// of the raster data used by the context.
    image = createImageFromBitmapContext(c);

	// Release the context now that the image is created.
    CGContextRelease(c);
	c = NULL;

    if(image == NULL){
		// Users of this code should update this to be an error code they find useful.
		return memFullErr;  
    }

    // Now export the image.
    if(exportInfo->useQTForExport)
		exportCGImageToFileWithQT(image, url, imageType, exportInfo->dpi);
    else
		exportCGImageToFileWithDestination(image, url, imageType, exportInfo->dpi);
    
    CGImageRelease(image);
    return err;
}

OSStatus MakeTIFFDocument(CFURLRef url, const ExportInfo *exportInfo)
{
    return MakeImageDocument(url, kUTTypeTIFF, exportInfo);
}

OSStatus MakePNGDocument(CFURLRef url, const ExportInfo *exportInfo)	
{
    return MakeImageDocument(url, kUTTypePNG, exportInfo);
}

OSStatus MakeJPEGDocument(CFURLRef url, const ExportInfo *exportInfo)	
{
    return MakeImageDocument(url, kUTTypeJPEG, exportInfo);
}

static CGLayerRef createCGLayerForDrawing(CGContextRef c)
{
    CGRect rect = { 0, 0, 50, 50 };
    CGSize layerSize;
    CGLayerRef layer;

    // Make the layer the size of the rectangle that
    // this code draws into the layer.
    layerSize.width = rect.size.width;
    layerSize.height = rect.size.height;

    // Create the layer to draw into.
    layer = CGLayerCreateWithContext(c, layerSize, NULL);
    if(layer == NULL)
	return NULL;
	
    // Get the context corresponding to the layer. Note
    // that this is a 'Get' function so the code must
    // not release the context.
    CGContextRef layerContext = CGLayerGetContext(layer);
    if(layerContext == NULL){
		CGLayerRelease(layer);
		return NULL;
    }
    
    // Set the fill color to opaque black.
    CGContextSetFillColorWithColor(layerContext, getRGBOpaqueBlackColor());
    
    // Draw the content into the layer.
    CGContextFillRect(layerContext, rect);
    
    // Now the layer has the contents needed.
    return layer;
}

void doSimpleCGLayer(CGContextRef context)
{
    int i,j;
    CGSize s;
    // Create the layer.
    CGLayerRef layer = createCGLayerForDrawing(context);
    if(layer == NULL){
		fprintf(stderr, "Couldn't create layer!\n");
		return;
    }
    
    // Get the size of the layer created.
    s = CGLayerGetSize(layer);
    
    // Clip to a rect that corresponds to
    // a grid of 8x8 layer objects.
    CGContextClipToRect(context, CGRectMake(0, 0, 8*s.width, 8*s.height));
    
    // Paint 8 rows of layer objects.
    for(j = 0 ; j < 8 ; j++){
		CGContextSaveGState(context);
			// Paint 4 columns of layer objects, moving
			// across the drawing canvas by skipping a
			// square on the grid each time across.
			for(i = 0 ; i < 4 ; i++){
				// Draw the layer at the current origin.
				CGContextDrawLayerAtPoint(context, 
					CGPointZero, 
					layer);
				// Translate across two layer widths.
				CGContextTranslateCTM(context, 2*s.width, 0);
			}
		CGContextRestoreGState(context);
		// Translate to the left one layer width on
		// even loop counts and to the right one
		// layer width on odd loop counts. Each
		// time through the outer loop, translate up
		// one layer height.
		CGContextTranslateCTM(context, 
			(j % 2) ? s.width: -s.width, 
			s.height);
    }
    // Release the layer when done drawing with it.
    CGLayerRelease(layer);
}


static CGContextRef createAlphaOnlyContext(size_t width, size_t height)
{
    /*	This routine allocates data for a pixel array that contains 
		width*height pixels, each pixel is 1 byte. The format is 
		8 bits per pixel, where the data is the alpha value of the pixel.
	*/
    CGContextRef context;
    size_t bytesPerRow;
    unsigned char *rasterData;
   
    // Minimum bytes per row is 1 byte per sample * number of samples.
    bytesPerRow = width;
    // Round to nearest multiple of BEST_BYTE_ALIGNMENT.
    bytesPerRow = COMPUTE_BEST_BYTES_PER_ROW(bytesPerRow);
    
    // Allocate the data for the raster. The total amount of data is bytesPerRow
    // times the number of rows. The function 'calloc' is used so that the
    // memory is initialized to 0.
    rasterData = calloc(1, bytesPerRow * height);
    if(rasterData == NULL){
		fprintf(stderr, "Couldn't allocate the needed amount of memory!\n");
		return NULL;
    } 
    
    // This type of context is only available in Panther and later, otherwise
    // this fails and returns a NULL context. The color space for an alpha
    // only context is NULL and the BitmapInfo value is kCGImageAlphaOnly.
    context = CGBitmapContextCreate(rasterData, width, height, 8, bytesPerRow, 
					NULL, kCGImageAlphaOnly);
    if(context == NULL){
		// If the context couldn't be created then release the raster memory.
		free(rasterData);
		fprintf(stderr, "Couldn't create the context!\n");
		return NULL;
    }

    // Clear the context bits so they are initially transparent.
    CGContextClearRect(context, CGRectMake(0, 0, width, height));
    
    return context;
}

/*	createMaskFromAlphaOnlyContext creates a CGImageRef
	from an alpha-only bitmap context. Calling this routine
	transfers 'ownership' of the raster data in the bitmap 
	context, to the image. If the image can't be created, this 
	routine frees the memory associated with the raster. 
*/
static CGImageRef createMaskFromAlphaOnlyContext(CGContextRef alphaContext)
{
    CGImageRef mask;
	unsigned char *rasterData = CGBitmapContextGetData(alphaContext);
    size_t imageDataSize = CGBitmapContextGetBytesPerRow(alphaContext)*
								CGBitmapContextGetHeight(alphaContext);
    float invertDecode[] = { 1. , 0. };
        
	if(rasterData == NULL){
		fprintf(stderr, "Context is not a bitmap context!\n");
		return NULL;
	}

    // Create the data provider from the image data.
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL, 
					    rasterData,
					    imageDataSize,
					    releaseBitmapContextImageData);

    if(dataProvider == NULL){
		// Must free the memory if the data provider couldn't be created
		// since this routine now owns it.
		free(rasterData);
		fprintf(stderr, "Couldn't create data provider!\n");
		return NULL;
    }
    mask = CGImageMaskCreate(CGBitmapContextGetWidth(alphaContext), 
			  CGBitmapContextGetHeight(alphaContext), 
			  CGBitmapContextGetBitsPerComponent(alphaContext), 
			  CGBitmapContextGetBitsPerPixel(alphaContext), 
			  CGBitmapContextGetBytesPerRow(alphaContext), 
			  dataProvider,
	// The decode is an inverted decode since a mask has the opposite
	// sense than alpha, i.e. 0 in a mask paints 100% and 1 in a mask
	// paints nothing.
			  invertDecode,
			  true);

    // Release the data provider since the mask retains it.
    CGDataProviderRelease(dataProvider);

    if(mask == NULL){
		fprintf(stderr, "Couldn't create image mask!\n");
		return NULL;
    }
    return mask;
}

void doAlphaOnlyContext(CGContextRef context)
{
    CGImageRef mask = NULL;
    // This code is going to capture the alpha coverage
    // of the drawing done by the doAlphaRects routine.
    // The value passed here as the width and height is
    // the size of the bounding rectangle of that drawing.
    size_t width = 520, height = 400;
    CGContextRef alphaContext = createAlphaOnlyContext(width, height);
    if(context == NULL){
		fprintf(stderr, "Couldn't create the alpha-only context!\n");
		return;
    }

    // Draw the content to the alpha-only context, capturing
    // the alpha coverage. The doAlphaRects routine paints
    // a series of translucent red rectangles.
    doAlphaRects(alphaContext);

    // Finished drawing to the context and now the raster contains
    // the alpha data captured from the drawing. Create
    // the mask from the data in the context.
    mask = createMaskFromAlphaOnlyContext(alphaContext);
    // This code is now finshed with the context so it can
    // release it.
    CGContextRelease(alphaContext);
    
    if(mask == NULL){
		return;
    }

    // Set the fill color space.
    CGContextSetFillColorSpace(context, getTheCalibratedRGBColorSpace());
    float opaqueBlue[] = { 0.11, 0.208, 0.451, 1.0 };
    // Set the painting color to opaque blue.
    CGContextSetFillColor(context, opaqueBlue);
    // Draw the mask, painting the mask with blue. This colorizes
    // the image to blue and it is as if we painted the
    // alpha rects with blue instead of red.
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), mask);

    // Releasing the mask will cause Quartz to release the data provider 
    // and therefore the raster memory used to create the context.
    CGImageRelease(mask);
}


CGPDFDocumentRef getThePDFDoc(CFURLRef url, float *w, float *h)
/*
    This function caches a CGPDFDocumentRef for
    the most recently requested PDF document.
*/
{
    static CGPDFDocumentRef pdfDoc = NULL;
    static CFURLRef pdfURL = NULL;
    static float width = 0, height = 0;
    
    if(url == NULL)
		return;
    
    // See whether to update the cached PDF document.
    if(pdfDoc == NULL || url != pdfURL){ 
		// Release any cached document or URL.
		if (pdfDoc) CGPDFDocumentRelease(pdfDoc);
		if (pdfURL) CFRelease(pdfURL);
		
		pdfDoc = CGPDFDocumentCreateWithURL(url);
		if(pdfDoc != NULL){
			CGRect pdfMediaRect = CGPDFDocumentGetMediaBox(pdfDoc, 1);
			width = pdfMediaRect.size.width;
			height = pdfMediaRect.size.height;
			// Retain the URL of the PDF file being cached.
			pdfURL = CFRetain(url);
		}else{
			pdfURL = NULL;
		}
    }
    
    if(pdfDoc){
		// Let the caller know the width and height of the document.
		*w = width;
		*h = height;
    }
    
    return pdfDoc;
}

// Defining this scales the content down by 1/3.
#define DOSCALING 1

void TilePDFNoBuffer(CGContextRef context, CFURLRef url)
{
    // The amount of area to tile should really be based on the 
    // window/document. Here it is hard coded to a US Letter
    // size document. This may draw too many or too few tiles
    // for the area actually being filled.
    float fillwidth = 612., fillheight = 792.;
    float tileX, tileY, tileOffsetX, tileOffsetY, extraOffset = 6;
    float w, h;
    CGPDFDocumentRef pdfDoc = getThePDFDoc(url, &tileX, &tileY);
    if(pdfDoc == NULL){
	    fprintf(stderr, "Couldn't get the PDF document!\n");
	    return;
    }

#if DOSCALING
    // Make the tiles 1/3 the size of the PDF document.
    tileX /= 3;
    tileY /= 3;
	extraOffset /= 3;
#endif

    // Space the tiles by the tile width and height
    // plus extraOffset units in each dimension.
    tileOffsetX = extraOffset + tileX;
    tileOffsetY = extraOffset + tileY;

    // Tile the PDF document.
    for(h = 0; h < fillheight ; h += tileOffsetY)
		for(w = 0; w < fillwidth ; w += tileOffsetX){
			CGContextDrawPDFDocument(context, 
				CGRectMake(w, h, tileX, tileY), pdfDoc, 1);
		}
}

void TilePDFWithOffscreenBitmap(CGContextRef context, CFURLRef url)
{
    // Again this should really be computed based on
    // the area intended to be tiled.
    float fillwidth = 612., fillheight = 792.;
    float tileX, tileY, tileOffsetX, tileOffsetY, extraOffset = 6;
    float w, h;
    CGContextRef bitmapContext;
    Boolean useDisplayColorSpace;
    Boolean needTransparentBitmap;

    CGPDFDocumentRef pdfDoc = getThePDFDoc(url, 
				    &tileX,
				    &tileY);
    if(pdfDoc == NULL){
	    fprintf(stderr, "Couldn't get the PDF document!\n");
	    return;
    }

#if DOSCALING
    // Make the tiles 1/3 the size of the PDF document.
    tileX /= 3;
    tileY /= 3;
	extraOffset /= 3;
#endif

    // Space the tiles by the tile width and height
    // plus extraOffset units in each dimension.
    tileOffsetX = extraOffset + tileX;
    tileOffsetY = extraOffset + tileY;
    
    // Since the bitmap context is for use with the display
    // and should capture alpha, these are the values
    // to pass to createRGBBitmapContext.
    useDisplayColorSpace = true;
    needTransparentBitmap = true;
    bitmapContext = createRGBBitmapContext(tileX, tileY, 
					useDisplayColorSpace, 
					needTransparentBitmap);
    if(bitmapContext == NULL){
		fprintf(stderr, "Couldn't create bitmap context!\n");
		return;
    }

    // Draw the PDF document one time into the bitmap context.
    CGContextDrawPDFDocument(bitmapContext, 
		    CGRectMake(0, 0, tileX, tileY), pdfDoc, 1);

    // Create an image from the raster data. Calling
	// createImageFromBitmapContext gives up ownership
	// of the raster data used by the context.
    CGImageRef image = createImageFromBitmapContext(bitmapContext);

	// Release the context now that the image is created.
    CGContextRelease(bitmapContext);

    if(image == NULL){
		return;
    }
    
    // Now tile the image.
    for(h = 0; h < fillheight ; h += tileOffsetY)
		for(w = 0; w < fillwidth ; w += tileOffsetX){
			CGContextDrawImage(context, CGRectMake(w, h, tileX, tileY), image);
		}
	
    CGImageRelease(image);
}

static CGLayerRef createLayerWithImageForContext(CGContextRef c, CFURLRef url)
{
    CGSize layerSize;
    CGLayerRef layer;
    CGPDFDocumentRef pdfDoc = getThePDFDoc(url, &layerSize.width, &layerSize.height);
    if(pdfDoc == NULL){
		return NULL;
    }

#if DOSCALING
    // Make the layer 1/3 the size of the PDF document.
    layerSize.width /= 3;
    layerSize.height /= 3;
#endif

    // Create the layer to draw into.
    layer = CGLayerCreateWithContext(c, layerSize, NULL);
    if(layer == NULL)
		return NULL;
	
    // Get the context corresponding to the layer. Note
    // that this is a 'Get' function so the code must
    // not release the context.
    CGContextRef layerContext = CGLayerGetContext(layer);
    if(layerContext == NULL){
		CGLayerRelease(layer);
		return NULL;
    }
    
    // Draw the PDF document into the layer.
    CGContextDrawPDFDocument(layerContext,
		CGRectMake(0, 0, layerSize.width, layerSize.height), pdfDoc, 1);
    
    // Now the layer has the contents needed.
    return layer;
}

void TilePDFWithCGLayer(CGContextRef context, CFURLRef url)
{
    // Again this should really be computed based on
    // the area intended to be tiled.
    float fillwidth = 612., fillheight = 792.;
    CGSize s;
    float tileX, tileY, tileOffsetX, tileOffsetY;
    float w, h;
    CGLayerRef layer = createLayerWithImageForContext(context, url);
    if(layer == NULL){
	    fprintf(stderr, "Couldn't create the layer!\n");
	    return;
    }
    
    // Compute the tile size and offset.
    s = CGLayerGetSize(layer);
    tileX = s.width;
    tileY = s.height;

#if DOSCALING
    // Space the tiles by the tile width and height
    // plus an extra 2 units in each dimension.
    tileOffsetX = 2. + tileX;
    tileOffsetY = 2. + tileY;
#else
	// Add 6 units to the offset in each direction
	// if there is no scaling of the source PDF document.
    tileOffsetX = 6. + tileX;
    tileOffsetY = 6. + tileY;
#endif

    // Now draw the contents of the layer to the context.
    // The layer is drawn at its true size (the size of
    // the tile) with its origin located at the corner
    // of each tile.
    for(h = 0; h < fillheight ; h += tileOffsetY)
		for(w = 0; w < fillwidth ; w += tileOffsetX){
			CGContextDrawLayerAtPoint(context, CGPointMake(w, h), layer);
		}
    
    // Release the layer when done drawing with it.
    CGLayerRelease(layer);
}
