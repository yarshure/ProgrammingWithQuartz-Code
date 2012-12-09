/*
*  File:    PSToPDF.c
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

#include "PSToPDF.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG 0

typedef struct MyConverterData
{
    bool doProgress;
    bool abortConverter;
    FILE *outStatusFile;
    CGPSConverterRef converter;
}MyConverterData;

/* Converter callbacks */

static void
begin_document_callback(void *info)
{
    fprintf( ((MyConverterData *)info)->outStatusFile, 
		    "\nBegin document\n");
}

static void
end_document_callback(void *info, bool success)
{
    fprintf( ((MyConverterData *)info)->outStatusFile,
	"\nEnd document: %s\n",
	success ? "success" : "failed");
}

static void
begin_page_callback(void *info, size_t pageno, 
			CFDictionaryRef page_info)
{
    fprintf( ((MyConverterData *)info)->outStatusFile,
		"\nBeginning page %zd\n", pageno);
#if DEBUG
    if(page_info != NULL)
		CFShow(page_info);
#endif
}

static void
end_page_callback(void *info, size_t pageno, 
			CFDictionaryRef page_info)
{
    fprintf(((MyConverterData *)info)->outStatusFile,
	"\nEnding page %zd\n", pageno);
#if DEBUG
    if(page_info != NULL)
		CFShow(page_info);
#endif
}

static void
progress_callback(void *info)
{
    MyConverterData *converterDataP = 
	    (MyConverterData *)info;

    if(converterDataP->doProgress)
	fprintf(converterDataP->outStatusFile, ".");
    
    // Here would be a callout to code that
    // would conceivably return whether to abort
    // the conversion process.
    
    //myUpdateStatus(converterDataP);
    
    if(converterDataP->abortConverter){
		CGPSConverterAbort(converterDataP->converter);
		fprintf(converterDataP->outStatusFile, "ABORTED!\n");
    }
}

static void
message_callback(void *info, CFStringRef cfmessage)
{
    char message[256];
    /*	Extract an ASCII version of the message. Typically
	these messages are similar to those obtained from
	any PostScript interpreter. Messages of the form
	"%%[ Error: undefined; OffendingCommand: bummer ]%%"
	are PostScript error messages and are the typical
	reason a conversion job fails.
    */ 
    if(CFStringGetCString(cfmessage, message, sizeof(message), kCFStringEncodingASCII))
	{
		fprintf(((MyConverterData *)info)->outStatusFile,
			"\nMessage: %s\n", message);
    }
}

// These are the callbacks this code uses when
// converting PostScript data to PDF. 
static const CGPSConverterCallbacks myCallbacks = {
    // Callbacks version 0 is the only one defined
    // as of Tiger.
    0,
    begin_document_callback,
    end_document_callback,
    begin_page_callback,
    end_page_callback,
    progress_callback,
    message_callback,
    // There is no releaseInfo callback for this example.
    NULL
};

/*  Given an input URL and a destination output URL, convert
    an input PS or EPS file to an output PDF file. This conversion
    can be time intensive and perhaps should be performed on
    a secondary thread or by another process. */
bool convertPStoPDF(CFURLRef inputPSURL, CFURLRef outPDFURL)
{
    CGDataProviderRef provider = NULL;
    CGDataConsumerRef consumer = NULL;
    bool success = false;
    MyConverterData myConverterData;

    provider = CGDataProviderCreateWithURL(inputPSURL);
    consumer = CGDataConsumerCreateWithURL(outPDFURL);

    if(provider == NULL || consumer == NULL)
    {
		if(provider == NULL)
			fprintf(stderr, "Couldn't create provider\n");
			
		if(consumer == NULL)
			fprintf(stderr, "Couldn't create consumer\n");
			
		CGDataProviderRelease(provider);
		CGDataConsumerRelease(consumer);
		return false;
    }

    // Setup the info data for the callbacks to
    // do progress reporting, set the initial state
    // of the abort flag to false and use stdout
    // as the file to write status and other information.
    myConverterData.doProgress = true;
    myConverterData.abortConverter = false;
    myConverterData.outStatusFile = stdout;

    // Create a converter object with myConverterData as the
    // info parameter and myCallbacks as the set of callbacks
    // to use for the conversion. There are no converter options 
    // defined as of Tiger so the options dictionary passed is NULL.
    myConverterData.converter = CGPSConverterCreate(&myConverterData, 
						    &myCallbacks, NULL);
    if(myConverterData.converter == NULL){
		CGDataProviderRelease(provider);
		CGDataConsumerRelease(consumer);
		fprintf(stderr, "Couldn't create converter object!\n");
		return false;
    }

    // There are no conversion options so the options
    // dictionary for the conversion is NULL.
    success = CGPSConverterConvert(myConverterData.converter, 
		    provider, consumer, NULL);
    if(!success)
		fprintf(stderr, "Conversion failed!\n");

    // There is no CGPSConverterRelease function. Since
    // a CGPSConverter object is a CF object, use CFRelease
    // instead.
    CFRelease(myConverterData.converter);
    CGDataProviderRelease(provider);
    CGDataConsumerRelease(consumer);
    
    return success;
}

static CFURLRef createTempFileURL(void)
{
    char tmpfilepath[PATH_MAX + 1];
    char *filename;
    // This routine uses the tmpname
    // function to obtain a temporary
    // file name to use for the intermediate
    // PDF document. You may want to use
    // a more secure temporary file location
    // or one that is more reliably unique.
    filename = tmpnam(tmpfilepath);
    
    // Create the URL from the temp file path.
    return CFURLCreateFromFileSystemRepresentation(NULL, 
			filename, strlen(filename), false);
}

/*  This is the direct access data provider callback used
    to read the bytes from the open temporary file. The
    info parameter is the (FILE *) used to seek the
    file and read the data. */
static size_t getBytesDirectAccessDP(void *info, void *buffer,
					    size_t offset, size_t count)
{
    FILE *fp = (FILE *)info;
    // Seek to the offset of the bytes requested.
    int result = fseek(fp, offset, SEEK_SET);
    if(result != 0){
	fprintf(stderr, 
	    "Couldn't seek to offset %zd because of: %s!\n", 
	    offset, strerror(errno));
	return 0;
    }
    // This reads 'count' 1-byte objects and returns
    // the number of objects (i.e. bytes) read.
    return fread(buffer, 1, count, fp);
}

static void releaseInfoDP(void *info)
{
    // Close the file now that the data
    // provider is done with it. Note that
    // this deletes the file since the file
    // was unlinked after it was opened.
    fclose( (FILE *)info);
}

CGDataProviderRef createTempFileDirectAccessDP(CFURLRef tempURL)
{
    CGDataProviderRef provider = NULL;
    struct stat sb;
    size_t fileSize;
    char path[PATH_MAX + 1];
    FILE *fp;
    
    // Get the pathname of the URL for the temporary file.
    if(!CFURLGetFileSystemRepresentation(tempURL, true, path, sizeof(path))){
		fprintf(stderr, "Couldn't get the path for tempURL!\n");
		return NULL;
    }

    // Stat the file to obtain its size.
    if(stat(path, &sb) != 0){
		fprintf(stderr, "Couldn't stat the file %s!\n", path);
		return NULL;
    }

    fileSize = sb.st_size;
    
    // Open the file for reading. This provides access for the
    // data provider to read the PDF data.
    fp = fopen(path, "r");
    if(fp == NULL){
		fprintf(stderr, "Can't open the file!\n");
		return NULL;
    }

    // Unlink the file so it can no longer be accessed from 
    // the file system. Closing the file later will delete it.
    if(unlink(path) != 0){
		// Unlink failed!
		fprintf(stderr, "Couldn't unlink the file!\n");
		fclose(fp);
		return NULL;
    }
    
    // Now set up the callbacks for the direct access
    // data provider. By supplying a non-NULL getBytes
    // proc and a NULL getBytePointer proc, Quartz will
    // call the getBytes proc to obtain the data it needs.
    CGDataProviderDirectAccessCallbacks callbacks;
    callbacks.getBytes = getBytesDirectAccessDP;
    callbacks.releaseProvider = releaseInfoDP;
    callbacks.getBytePointer = NULL;
    callbacks.releaseBytePointer = NULL;
    
    // This code uses a direct access data provider to access the data
    // since PDF data is not typically accessed sequentially and it is
    // more efficient to directly seek to the needed data. The code
    // passes the file pointer as the info parameter.
    provider = CGDataProviderCreateDirectAccess(fp, fileSize, &callbacks);
    if(provider == NULL){
		fprintf(stderr, "Couldn't create data provider!\n");
    }
    return provider;
}

CGPDFDocumentRef createCGPDFDocFromPSDoc(CFURLRef inputPSURL)
{
    CGDataProviderRef provider;
    CGPDFDocumentRef pdfDoc = NULL;
    CFURLRef tempPDFURLRef = NULL;
    bool conversionResult;

    // Create a URL to use for the temporary
    // PDF file on disk.    
    tempPDFURLRef = createTempFileURL();
    if(tempPDFURLRef == NULL){
		fprintf(stderr, "Couldn't create temporary file URL!\n");
		return NULL;
    }
    
    // Step 1: create a PDF document from the input PostScript
    // data. The convertPStoPDF is that from code listing X.Y.   
    conversionResult = convertPStoPDF(inputPSURL, tempPDFURLRef);
    // Test whether the conversion succeeded.
    if(!conversionResult){
		fprintf(stderr, "Conversion to a PDF document failed!\n");
		CFRelease(tempPDFURLRef);
		return NULL;
    }
    
    // Step 2: create a create a direct access data provider
    // the temporary PDF document on disk. To do so, use
    // a custom data provider that accesses the underlying 
    // temporary PDF file. The custom data provider creation 
    // code unlinks the file as part of its creation so that
    // the file is deleted when the file is closed when
    // the data provider closes the file.
    provider = createTempFileDirectAccessDP(tempPDFURLRef);
    // This code is done with the temporary URL object. The 
    // data provider still has access to the underlying file.
    CFRelease(tempPDFURLRef);
    if(provider == NULL){
		return NULL;
    }
    
    // Step 3: create the PDF document reference from
    // the data provider. When the document is released
    // the data provider will be released and the temporary 
    // PDF file will be closed.
    pdfDoc = CGPDFDocumentCreateWithProvider(provider);
    // Release the data provider since this code
    // no longer needs it.
    CGDataProviderRelease(provider);
    if(pdfDoc == NULL){
		fprintf(stderr, "Couldn't create PDFDocument reference!\n");
		return NULL;
    }
    return pdfDoc;
}

