#!/usr/bin/python
#
#  File:    pdftojpg.py
#  
#  Copyright:  Copyright (c) 2005 Apple Computer, Inc., All Rights Reserved
# 
#  Disclaimer:  IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
#        consideration of your agreement to the following terms, and your use, installation, modification 
#        or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
#        not agree with these terms, please do not use, install, modify or redistribute this Apple 
#        software.
#
#        In consideration of your agreement to abide by the following terms, and subject to these terms, 
#        Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
#        original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
#        Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
#        redistribute the Apple Software in its entirety and without modifications, you must retain this 
#        notice and the following text and disclaimers in all such redistributions of the Apple Software. 
#        Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
#        endorse or promote products derived from the Apple Software without specific prior written 
#        permission from Apple.  Except as expressly stated in this notice, no other rights or 
#        licenses, express or implied, are granted by Apple herein, including but not limited to any 
#        patent rights that may be infringed by your derivative works or by other works in which the 
#        Apple Software may be incorporated.
#
#        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
#        IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
#        AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
#        OR IN COMBINATION WITH YOUR PRODUCTS.
#
#        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
#        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
#        OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
#        REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
#        UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
#        IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from CoreGraphics import *
import sys, os
from math import ceil

if len(sys.argv) != 2:
  print "usage: %s PDF-FILE" % sys.argv[0]
  sys.exit(1)

pdf_file = sys.argv[1]
out_file_prefix , ext = os.path.splitext(pdf_file)

if not os.path.isfile(pdf_file) :
    print "Input file \"%s\" does not exist!" % pdf_file
    sys.exit(2)

# Create a Quartz data provider from the file.
provider = CGDataProviderCreateWithFilename(pdf_file)

# Create a CGPDFDocumentRef for the file.
pdf = CGPDFDocumentCreateWithProvider(provider)

numPages = pdf.getNumberOfPages()

# Create a color space for the destination bitmap context.
# To run on Panther also, this must be kCGColorSpaceUserRGB.
cs = CGColorSpaceCreateWithName (kCGColorSpaceGenericRGB)

for i in range(1, numPages + 1):
    # Get the media box of the content for this page.
    r = pdf.getMediaBox(i)
    
    # Set the media box origin to the Quartz origin.
    r.origin.x = r.origin.y = 0
    
    # Compute an integer width and height value that
    # encloses the width and height of the media box.
    w = int(ceil(r.size.width))
    h = int(ceil(r.size.height))
    # Create a bitmap context with the appropriate color space.
    # The bits in the bitmap will be initialized with the supplied
    # color; in this case opaque white.
    f[5] = {1.0 , 1.0 , 1.0 , 1.0 , 1.0}
    ctx = CGBitmapContextCreateWithColor( w , h , cs, f )
    # Draw the PDF document to the bitmap context.
    ctx.drawPDFDocument(r, pdf, i)

    # Compute a file name for the output data. Don't serialize
    #  the output file names if there is only one page.
    if numPages is 1 :
	out_file = out_file_prefix + ".jpg"
    else :
	out_file = out_file_prefix + "." + str(i) + ".jpg"

    # Write the bitmap context data to the output file as
    # JPEG format. 
    ctx.writeToFile (out_file, kCGImageFormatJPEG)
    
    # Finish the context. This is the last statement
    # in the loop.
    ctx.finish()

# Write a message indicating how many files were written.
print "%s created %d JPEG files from the PDF document" % \
			(sys.argv[0] , numPages)
