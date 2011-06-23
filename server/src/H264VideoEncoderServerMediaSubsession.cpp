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

#include "../include/H264VideoEncoderServerMediaSubsession.h"
#include "H264VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H264VideoStreamFramer.hh"

H264VideoEncoderServerMediaSubsession*
H264VideoEncoderServerMediaSubsession::createNew(UsageEnvironment& env,
                                               EncoderDeviceSource* device,
					       Boolean reuseFirstSource) {
  return new H264VideoEncoderServerMediaSubsession(env, device, reuseFirstSource);
}

H264VideoEncoderServerMediaSubsession::H264VideoEncoderServerMediaSubsession(UsageEnvironment& env,
								       EncoderDeviceSource* device, Boolean reuseFirstSource)
  : OnDemandServerMediaSubsession(env, reuseFirstSource),
    fDoneFlag(0), source(device) {
}

H264VideoEncoderServerMediaSubsession::~H264VideoEncoderServerMediaSubsession() {
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
  if (fDummyRTPSink->auxSDPLine() != NULL) {
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
  // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
  // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
  // and we need to start reading data from our file until this changes.
  fDummyRTPSink = rtpSink;

  // Start reading the file:
  fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

  // Check whether the sink's 'auxSDPLine()' is ready:
  checkForAuxSDPLine(this);

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  char const* auxSDPLine = fDummyRTPSink->auxSDPLine();
  return auxSDPLine;
}

FramedSource* H264VideoEncoderServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = 500; // kbps, estimate

  // Create the video source:
  
  // Create a framer for the Video Elementary Stream:
  return H264VideoStreamFramer::createNew(envir(), source);
}

RTPSink* H264VideoEncoderServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}