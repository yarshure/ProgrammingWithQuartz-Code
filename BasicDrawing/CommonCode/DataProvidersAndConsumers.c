/*
*  File:    DataProvidersAndConsumers.c
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

#include "DataProvidersAndConsumers.h"

CGDataProviderRef createDataProviderFromPathName (CFStringRef path)
{
	CFURLRef url;

	// Create a CFURL for the supplied file system path.
	url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);
	if(url == NULL){
		fprintf(stderr, "Couldn't create url!\n");
		return NULL;
	}
	// Create a Quartz data provider for the URL.
	CGDataProviderRef dataProvider = CGDataProviderCreateWithURL(url);
	// Release the URL when done with it.
	CFRelease(url);
	if(dataProvider == NULL){
		fprintf(stderr, "Couldn't create data provider!\n");
		return NULL;
	}
	return dataProvider;
}

static void rgbReleaseRampData(void *info, const void *data, size_t size)
{
    free((char *)data);
}

CGDataProviderRef createRGBRampDataProvider(void)
{
	CGDataProviderRef dataProvider = NULL;
	size_t width = 256, height = 256;
	size_t imageDataSize = width*height*3;
	unsigned char *p;
	unsigned char *dataP = (unsigned char *)malloc(imageDataSize);
	if(dataP == NULL){
	    return NULL;
	}
	
	p = dataP;
	/*
	    Build an image that is RGB 24 bits per sample. This is a ramp
	    where the red component value increases in red from left to 
	    right and the green component increases from top to bottom.
	*/
	int r, g;
	for(g = 0 ; g < height ; g++){
	    for(r = 0 ; r < width ; r++){
		    *p++ = r; *p++ = g; *p++ = 0;
	    }
	}
	// Once this data provider is created, the data associated 
	// with dataP MUST be available until Quartz calls the data 
	// releaser function 'rgbReleaseRampData'.
	dataProvider = CGDataProviderCreateWithData(NULL, dataP, 
						imageDataSize, rgbReleaseRampData);
	return dataProvider;
}

typedef struct MyImageDataInfo{
    FILE *fp;
#if DEBUG
    size_t totalBytesRead;
    size_t skippedBytes;
    size_t numRewinds;
#endif
}MyImageDataInfo;

static size_t getBytesSequentialAccessDP(void *data, void *buffer, size_t count)
{
    size_t readSize = 0;
    FILE *fp = ((MyImageDataInfo *)data)->fp;
    // This reads 'count' 1 byte objects and returns
    // the number of objects (i.e. bytes) read.
    readSize = fread(buffer, 1, count, fp);
#if DEBUG
    ((MyImageDataInfo *)data)->totalBytesRead += readSize;
#endif
    return readSize;
}

static void skipBytesSequentialAccessDP(void *data, size_t count)
{
    int result = fseek(((MyImageDataInfo *)data)->fp, count, SEEK_CUR);
    if(result != 0)
		fprintf(stderr, "Couldn't seek by %zd bytes because of: %s!\n", 
		    						count, strerror(errno));
#if DEBUG
    ((MyImageDataInfo *)data)->skippedBytes += count;
#endif
}

static void rewindSequentialAccessDP(void *data)
{
    // Rewind the beginning of the data.
    rewind(((MyImageDataInfo *)data)->fp);
#if DEBUG
    ((MyImageDataInfo *)data)->numRewinds ++;
#endif
}

static void releaseSequentialAccessDP(void *data)
{
    if(data){
		MyImageDataInfo *imageDataInfoP = (MyImageDataInfo *)data;
#if DEBUG
		fprintf(stderr, 
			"read %zd bytes, skipped %zd bytes, rewind called %zd times\n",
		    imageDataInfoP->totalBytesRead, imageDataInfoP->skippedBytes,
		    imageDataInfoP->numRewinds);
#endif
		if(imageDataInfoP->fp != NULL)
			fclose(imageDataInfoP->fp);
		free(imageDataInfoP);
    }
}

CGDataProviderRef createSequentialAccessDPForURL(CFURLRef url)
{
	MyImageDataInfo *imageDataInfoP = NULL;
	CGDataProviderRef provider = NULL;
	FILE *fp = NULL;
	CGDataProviderCallbacks callbacks;
	char pathString[PATH_MAX + 1];
	Boolean success = CFURLGetFileSystemRepresentation(url, true, 
							pathString, sizeof(pathString));
	if(!success)
	{
		fprintf(stderr, "Couldn't get the path name C string!\n");
		return NULL;
	}

	fp = fopen(pathString, "r");
	if(fp == NULL){
		fprintf(stderr, "Couldn't open path to file %s!\n", pathString);
		return NULL;
	}
	imageDataInfoP = (MyImageDataInfo *)malloc(sizeof(MyImageDataInfo));
	if(imageDataInfoP == NULL){
		fclose(fp);
		fprintf(stderr, "Couldn't malloc block for info data!\n");
		return NULL;
	}
	imageDataInfoP->fp = fp;

#if DEBUG
    imageDataInfoP->totalBytesRead = 
    imageDataInfoP->skippedBytes = 
    imageDataInfoP->numRewinds = 0;
#endif

	callbacks.getBytes = getBytesSequentialAccessDP;
	callbacks.skipBytes = skipBytesSequentialAccessDP;
	callbacks.rewind = rewindSequentialAccessDP;
	callbacks.releaseProvider = releaseSequentialAccessDP;
	provider = CGDataProviderCreate(imageDataInfoP, &callbacks);
	if(provider == NULL){
		fprintf(stderr, "Couldn't create data provider!\n");
		// Release the info data and cleanup.
		releaseSequentialAccessDP(imageDataInfoP);
		return NULL;
	}
	return provider;
}


static size_t getBytesGrayRampDirectAccess(void *info, void *buffer,
					    size_t offset, size_t count)
{
	/* This computes a linear gray ramp that is 256 samples wide and
		1 sample high. The ith byte in the image is the sample
		value i. This produces a gray ramp that goes from 0 (black) to 
		FF (white). 
	*/
	unsigned char *p = buffer;
	int i;
	// This data provider provides 256 bytes total. If Quartz
	// requests more data than is available, only return
	// the available data.
	if( (offset + count) > 256 )
		count = 256 - offset;
	
	for(i = offset; i < (offset + count) ; i++)
		*p++ = i;
    
	return count;
}

CGDataProviderRef createGrayRampDirectAccessDP(void)
{
	CGDataProviderRef provider = NULL;
	CGDataProviderDirectAccessCallbacks callbacks;
	callbacks.getBytes = getBytesGrayRampDirectAccess;
	callbacks.getBytePointer = NULL;
	callbacks.releaseBytePointer = NULL;
	callbacks.releaseProvider = NULL;
	provider = CGDataProviderCreateDirectAccess(NULL, 256, &callbacks);
	if(provider == NULL){
		fprintf(stderr, "Couldn't create data provider!\n");
	}
	return provider;
}

static void myCFDataRelease(void *info, const void *data, size_t size)
{
	if(info)
		CFRelease(info);
}


// This only builds on Tiger and later.
CGDataProviderRef myCGDataProviderCreateWithCFData(CFDataRef data)
{
	CGDataProviderRef provider = NULL;
	// If the CFData object passed in is NULL, this code returns
	// a NULL data provider.
	if(data == NULL)
		return NULL;

	// Test to see if the Quartz version is available and if so, use it.
	if(&CGDataProviderCreateWithCFData != NULL){
		return CGDataProviderCreateWithCFData(data);
	}
	if(provider == NULL){
		size_t dataSize = CFDataGetLength(data);
		const UInt8 *dataPtr = CFDataGetBytePtr(data);
		// Retain the data so that this code owns a reference.
		CFRetain(data);
		provider = CGDataProviderCreateWithData((void *)data, dataPtr,
						dataSize, myCFDataRelease);
		if(provider == NULL){
			// Release the data if for some reason the
			// data provider couldn't be created.
			CFRelease(data);
		}
	}
	return provider;
}

CGDataConsumerRef createDataConsumerFromPathName(CFStringRef path)
{
	CFURLRef url;

	// Create a CFURL for the supplied file system path.
	url = CFURLCreateWithFileSystemPath (NULL, path, 
						kCFURLPOSIXPathStyle, false);
	if(url == NULL){
		fprintf(stderr, "Couldn't create url!\n");
		return NULL;
	}
	// Create a Quartz data provider for the URL.
	CGDataConsumerRef dataConsumer = CGDataConsumerCreateWithURL(url);
	// Release the url when the code is done with it.
	CFRelease(url);
	if(dataConsumer == NULL){
		fprintf(stderr, "Couldn't create data consumer!\n");
		return NULL;
	}
	return dataConsumer;
}

size_t myCFDataConsumerPutBytes(void *info, 
					const void *buffer, size_t count)
{
    CFMutableDataRef data  = (CFMutableDataRef)info;
	// Append 'count' bytes from 'buffer' to the CFData 
	// object 'data'.
	CFDataAppendBytes(data, buffer, count);
	return count;
}

void myCFDataConsumerRelease(void *info)
{
    if(info != NULL)
		CFRelease((CFDataRef)info);
}

// This only builds on Tiger and later.
CGDataConsumerRef myCGDataConsumerCreateWithCFData(
							CFMutableDataRef data)
{
    CGDataConsumerRef consumer = NULL;
	// If the CFData object passed in is NULL, this code returns
	// a NULL data consumer.
	if(data == NULL)
		return NULL;

    // Test to see if the Quartz version is available.
    if(&CGDataConsumerCreateWithCFData != NULL){
		return CGDataConsumerCreateWithCFData(data);
    }

    if(consumer == NULL){
		CGDataConsumerCallbacks callbacks;
		callbacks.putBytes = myCFDataConsumerPutBytes;
		callbacks.releaseConsumer = myCFDataConsumerRelease;
	
		// Retain the data so that this code owns a reference.
		CFRetain(data);
		consumer = CGDataConsumerCreate(data, &callbacks);
		if(consumer == NULL){
			// Release the data if for some reason the
			// data consumer couldn't be created.
			CFRelease(data);
		}
    }
    return consumer;
}

