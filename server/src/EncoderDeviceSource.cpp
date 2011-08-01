/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2011 Live Networks, Inc.  All rights reserved.
// A template for a MediaSource encapsulating an audio/video input device
//
// NOTE: Sections of this code labeled "%%% TO BE WRITTEN %%%" are incomplete, and need to be written by the programmer
// (depending on the features of the particular device).
// Implementation

#include "../include/EncoderDeviceSource.hh"
#include <GroupsockHelper.hh> // for "gettimeofday()"
#include "server.h"

static void signalNewFrameData0(void* clientData);

EncoderDeviceSource*
EncoderDeviceSource::createNew(UsageEnvironment& env,
			EncoderDeviceParameters params) {
  return new EncoderDeviceSource(env, params);
}

EventTriggerId EncoderDeviceSource::eventTriggerId = 0;

unsigned EncoderDeviceSource::referenceCount = 0;

EncoderDeviceSource::EncoderDeviceSource(UsageEnvironment& env,
			   EncoderDeviceParameters params)
  : FramedSource(env), fParams(params), fLastPlayTime(0), frameCount(0), bin(NULL) {
  if (referenceCount == 0) {
    // Any global initialization of the device would be done here:
    //%%% TO BE WRITTEN %%%
  }
  ++referenceCount;

  // Any instance-specific initialization of the device would be done here:
  //%%% TO BE WRITTEN %%%
  FILE* f = fopen("IwasHere.txt", "wt");
  FILE* f2 = fopen("viddeo.264", "wb");
  FILE* f3 = fopen("vidddeo.264", "wb");
  bin = fopen("jumper2.h264", "rb");
  fprintf(f, "Constructor!\n");
  fclose(f);
  fclose(f2);
  fclose(f3);

  // We arrange here for our "deliverFrame" member function to be called
  // whenever the next frame of data becomes available from the device.
  //
  // If the device can be accessed as a readable socket, then one easy way to do this is using a call to
  //     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
  // (See examples of this call in the "liveMedia" directory.)
  //
  // If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
  // Create an 'event trigger' for this device (if it hasn't already been done):
  if (eventTriggerId == 0) {
    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
  }
}

EncoderDeviceSource::~EncoderDeviceSource() {
  // Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
  //%%% TO BE WRITTEN %%%

  --referenceCount;
  if (referenceCount == 0) {
    // Any global 'destruction' (i.e., resetting) of the device would be done here:
    //%%% TO BE WRITTEN %%%

    // Reclaim our 'event trigger'
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
  }
  fclose(bin);
}

void EncoderDeviceSource::doGetNextFrame() {
  // This function is called (by our 'downstream' object) when it asks for new data.
  // Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
    if(mutex_lock(mutex))
      g_Messages.push(string("EncoderDeviceSource::doGetNextFrame</b>"
                             "<font color='red'>Fehler bei mutex_lock!</font>"));
  if (!*fParams.running){//!fParams.running /* the source stops being readable */ /*%%% TO BE WRITTEN %%%*/) {
      if(mutex_unlock(mutex))
          g_Messages.push(string("EncoderDeviceSource::doGetNextFrame</b>"
                             "<font color='red'>Fehler bei mutex_unlock 1!</font>"));
    handleClosure(this);
    return;
  }
  
  //deliverFrame();
  FILE* f = fopen("IwasHere.txt", "at");
  FILE* f2 = fopen("vidddeo.264", "ab");
           
  //printf("\n\ndoGetNextFrame\n\n");
  if(!fParams.used)
  {
      /*do*/ {
          //memcpy
          int temp_size = 0;
          unsigned char* temp = (unsigned char*)fParams.getFrame(&temp_size, &fParams.frame_time); 
          //fprintf(f, "temp: %d, size: %d\n", (int)temp, temp_size);
         
          if(temp)
          {
            //fParams.tempData = (unsigned char*)malloc(fParams.tempSize);
            fParams.setData(temp, temp_size);
          //fParams.tempData = (unsigned char*)fParams.getFrame(&fParams.tempSize); 
            //memmove(fParams.tempData, temp, fParams.tempSize);
          //  fParams.tempData = (unsigned char*)fParams.getFrame(&fParams.tempSize);
          }
          else
          {
              //fParams.tempData = NULL;
              //fParams.tempSize = 0;
              fParams.clear();
          }
          if(!fParams.tempData && fParams.tempSize)
          {
              printf("sehr ungewohnlich: size: %d, pointer: %d\n", fParams.tempSize, (int)fParams.tempData);
              fParams.tempSize = 0;
          }
      } //while(!fParams.tempSize || !fParams.tempData);
      frameCount++;
      fwrite(fParams.tempData, fParams.tempSize, 1, f2);      
  }
  fclose(f);
  fclose(f2);
  
  if(mutex_unlock(mutex))
      g_Messages.push(string("EncoderDeviceSource::doGetNextFrame</b> "
                             "<font color='red'>Fehler bei mutex_unlock 2</font>"));
  // If a new frame of data is immediately available to be delivered, then do this now:
  if (fParams.tempSize /* a new frame of data is immediately available to be delivered*/ /*%%% TO BE WRITTEN %%%*/) {
    deliverFrame();
  }
/*  else
  {
      int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)signalNewFrameData0, this);
  }//*/
  

  // No new data is immediately available to be delivered.  We don't do anything more here.
  // Instead, our event trigger must be called (e.g., from a separate thread) when new data becomes available.
}

void EncoderDeviceSource::deliverFrame0(void* clientData) {
  ((EncoderDeviceSource*)clientData)->deliverFrame();
}

void EncoderDeviceSource::deliverFrame() {
  // This function is called when new frame data is available from the device.
  // We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
  // 'in' parameters (these should *not* be modified by this function):
  //     fTo: The frame data is copied to this address.
  //         (Note that the variable "fTo" is *not* modified.  Instead,
  //          the frame data is copied to the address pointed to by "fTo".)
  //     fMaxSize: This is the maximum number of bytes that can be copied
  //         (If the actual frame is larger than this, then it should
  //          be truncated, and "fNumTruncatedBytes" set accordingly.)
  // 'out' parameters (these are modified by this function):
  //     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
  //     fNumTruncatedBytes: Should be set iff the delivered frame would have been
  //         bigger than "fMaxSize", in which case it's set to the number of bytes
  //         that have been omitted.
  //     fPresentationTime: Should be set to the frame's presentation time
  //         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
  //         by calling "gettimeofday()".
  //     fDurationInMicroseconds: Should be set to the frame's duration, if known.
  //         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
  //         to set this variable, because - in this case - data will never arrive 'early'.
  // Note the code below.
   
  if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet
  if(mutex_lock(mutex)) g_Messages.push(string("EncoderDeviceSource::deliverFrame</b> "
                                "<font color='red'>Fehler bei mutex_lock</font>"));
  
  FILE* f = fopen("IwasHere.txt", "at");
  FILE* f2 = fopen("viddeo.264", "ab");
  fprintf(f, "Hallo!\n");
  fflush(f);
        
  fprintf(f, "frameSize1: %d, maxSize: %d\n", fParams.tempSize, fMaxSize);
  fflush(f);  
  
   int newFrameSize = fParams.tempSize; //%%% TO BE WRITTEN %%%
   unsigned char* newFrameDataStart = fParams.tempData;
   if(!newFrameDataStart) 
   {
       if(!fParams.used)
       {
           newFrameDataStart = (unsigned char*)fParams.getFrame(&newFrameSize, &fParams.frame_time);
           if(newFrameDataStart)
                fParams.setData(newFrameDataStart, newFrameSize);
           else
               fParams.clear();
       }
       else
       {
            printf("server.encoderDeviceSource::deliverFrame newFrameDataStart NULL-Pointer\n");
            if(mutex_unlock(mutex)) g_Messages.push(string("EncoderDeviceSource::deliverFrame</b> "
                        "<font color='red'>Fehler bei mutex_unlock 1</font>"));
            return;
       }
   }  
  
  // Deliver the data here:
  if (newFrameSize > fMaxSize) {
    fFrameSize = fMaxSize;
    fNumTruncatedBytes = newFrameSize - fMaxSize;
  } else {
    fFrameSize = newFrameSize;
  }
   if(fFrameSize != newFrameSize)
   {
       fprintf(f, "newFrameSize: %d, frameSize: %d\n", newFrameSize, fFrameSize);
       fflush(f);
   }
   fprintf(f, "fMaxSize: %d, fFrameSize: %d\n", fMaxSize, fFrameSize);
   fflush(f);
   
          
   
   /*        
   if(frameCount <= 1)
   {
        gettimeofday(&fPresentationTime, NULL);
   }
   else
   {
       fLastPlayTime = frameCount*100000;
       unsigned uSeconds	= fPresentationTime.tv_usec + fLastPlayTime;
       fPresentationTime.tv_sec += uSeconds/1000000;
       fPresentationTime.tv_usec = uSeconds%1000000;
   
       fDurationInMicroseconds = fLastPlayTime;
       fprintf(f, "fLastPlayTime: %d\n", fLastPlayTime);
       fflush(f);
   }
   
     //*/  
  memcpy(&fPresentationTime, &fParams.frame_time, sizeof(struct timeval));
   
  //gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
  // If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
  memmove(fTo, newFrameDataStart, fFrameSize);
  
  fwrite(newFrameDataStart, fFrameSize, 1, f2);
  fclose(f2);
  
  if(fNumTruncatedBytes)
  {
        unsigned char* truncatedBuffer = (unsigned char*)malloc(fNumTruncatedBytes);
        memmove(truncatedBuffer, &newFrameDataStart[fFrameSize], fNumTruncatedBytes);
        fParams.setData(truncatedBuffer, fNumTruncatedBytes);
        free(truncatedBuffer);
  }
  else
  {
        fParams.clear();
  }
  unsigned* tag = (unsigned*)newFrameDataStart;
  size_t s = fFrameSize / sizeof(unsigned);
  fprintf(f, "size: %d: short: %d%d%d%d\n", fFrameSize, tag[s-4], tag[s-3], tag[s-2], tag[s-1]);
  //fprintf(f, "frameSize2: %d, p: %d\n", fFrameSize, (int)newFrameDataStart);
  fflush(f);
  /*
  fFrameSize = fread(fTo, 1, fMaxSize, bin);
  if(fMaxSize != fFrameSize)
  {
      fprintf(f, "Fehler, weniger bytes gelesen als erwartet: %d, %d\n", fMaxSize, fFrameSize);
      fflush(f);
  }
  // * */
  
  //fprintf(f, "frame wrote\n");
  //fflush(f);
  
  
  fclose(f);

  if(mutex_unlock(mutex)) g_Messages.push(string("EncoderDeviceSource::deliverFrame</b> "
                        "<font color='red'>Fehler bei mutex_unlock 2</font>"));
  // After delivering the data, inform the reader that it is now available:
  FramedSource::afterGetting(this);
  
}


// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
void signalNewFrameData() {
    //printf("signalNewFrameData\n");
  TaskScheduler* ourScheduler = scheduler; //%%% TO BE WRITTEN %%%
  EncoderDeviceSource* ourDevice  = (EncoderDeviceSource*)eds; //%%% TO BE WRITTEN %%%

  if (ourScheduler != NULL) { // sanity check
    ourScheduler->triggerEvent(EncoderDeviceSource::eventTriggerId, ourDevice);
  }
}

static void signalNewFrameData0(void* clientData) {
    signalNewFrameData();
}