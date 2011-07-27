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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 video file.
// Implementation

#include "H264VideoEncoderServerMediaSubsession.h"
#include "H264VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H264VideoStreamFramer.hh"
#include "H264VideoRTPSource.hh"
#include "server.h"

H264VideoEncoderServerMediaSubsession*
H264VideoEncoderServerMediaSubsession::createNew(UsageEnvironment& env,
					       FramedSource** source,
					       Boolean reuseFirstSource) {
  return new H264VideoEncoderServerMediaSubsession(env, source, reuseFirstSource);
}

H264VideoEncoderServerMediaSubsession::H264VideoEncoderServerMediaSubsession(UsageEnvironment& env,
								       FramedSource** source, Boolean reuseFirstSource)
  : FileServerMediaSubsession(env, "Teste", reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL), mSource(source) {
}

H264VideoEncoderServerMediaSubsession::~H264VideoEncoderServerMediaSubsession() {
  delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData) {
  H264VideoEncoderServerMediaSubsession* subsess = (H264VideoEncoderServerMediaSubsession*)clientData;
  subsess->afterPlayingDummy1();
}

void H264VideoEncoderServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
  H264VideoEncoderServerMediaSubsession* subsess = (H264VideoEncoderServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void H264VideoEncoderServerMediaSubsession::checkForAuxSDPLine1() {
  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL && fDummyRTPSink->auxSDPLine() != NULL) {
    fAuxSDPLine = strDup(fDummyRTPSink->auxSDPLine());
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();
  } else {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* H264VideoEncoderServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
  if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
    // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
    // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
    // and we need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}

FramedSource* H264VideoEncoderServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = 500; // kbps, estimate

  // Create the video source:
  /*ByteStreamFileSource* fileSource*/ //source = ByteStreamFileSource::createNew(envir(), "./raw_video.264");
  //Groupsock* gr = new Groupsock(envir(), inet_makeaddr(127,1), Port(mPort), 0);
  *mSource = EncoderDeviceSource::createNew(envir(), EncoderDeviceParameters(getFrameFunc, &g_run));
  //H264VideoStreamDiscreteFramer* dF = 
    //        H264VideoStreamDiscreteFramer::createNew(envir(), eds);
   // mSource =   eds;
    //gr->changeDestinationParameters()
//  source = H264VideoRTPSource::createNew(envir(), gr, 96);
  
  if (*mSource == NULL) return NULL;
  //if (fileSource == NULL) return NULL;
  fFileSize = 0;//((ByteStreamFileSource*)source)->fileSize();

  // Create a framer for the Video Elementary Stream:
  return H264VideoStreamFramer::createNew(envir(), *mSource);
}

RTPSink* H264VideoEncoderServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}