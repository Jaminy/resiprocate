#ifndef TURNSOCKET_HXX
#define TURNSOCKET_HXX

#include <vector>
#include <asio.hpp>
#include <rutil/Data.hxx>

#include "../StunTuple.hxx"
#include "../StunMessage.hxx"

namespace reTurn {

class TurnSocket
{
public:
   static unsigned int UnspecifiedLifetime;
   static unsigned int UnspecifiedBandwidth;
   static unsigned short UnspecifiedPort;
   static asio::ip::address UnspecifiedIpAddress;

   explicit TurnSocket(const asio::ip::address& address, unsigned short port);
   virtual ~TurnSocket();

   virtual asio::error_code connect(const asio::ip::address& address, unsigned short port) = 0;

   asio::error_code requestSharedSecret(const asio::ip::address& turnServerAddress, 
                                        unsigned short turnServerPort,
                                        char* username, unsigned int usernameSize, 
                                        char* password, unsigned int passwordSize);

   asio::error_code createAllocation(const asio::ip::address& turnServerAddress, 
                                     unsigned short turnServerPort, 
                                     char* username,
                                     char* password,
                                     unsigned int lifetime = UnspecifiedLifetime,
                                     unsigned int bandwidth = UnspecifiedBandwidth,
                                     unsigned short requestedPortProps = StunMessage::PortPropsNone, 
                                     unsigned short requestedPort = UnspecifiedPort,
                                     StunTuple::TransportType requestedTransportType = StunTuple::None, 
                                     const asio::ip::address &requestedIpAddress = UnspecifiedIpAddress);

   asio::error_code refreshAllocation();
   asio::error_code destroyAllocation();

   // Accessors for properties associated with an allocation
   StunTuple& getRelayTuple();
   StunTuple& getReflexiveTuple();
   unsigned int getLifetime();
   unsigned int getBandwidth();

   // Turn Send Methods
   asio::error_code send(unsigned char channelNumber, const char* buffer, unsigned int size);
   asio::error_code sendTo(const asio::ip::address& address, unsigned short port, const char* buffer, unsigned int size);

   // Receive Methods
   asio::error_code receive(char* buffer, unsigned int& size, unsigned int timeout, asio::ip::address* sourceAddress=0, unsigned short* sourcePort=0);
   asio::error_code receiveFrom(const asio::ip::address& address, unsigned short port, char* buffer, unsigned int& size, unsigned int timeout);

protected:
   virtual asio::error_code rawWrite(const char* buffer, unsigned int size) = 0;
   virtual asio::error_code rawWrite(const std::vector<asio::const_buffer>& buffers) = 0;
   virtual asio::error_code rawRead(char* buffer, unsigned int size, unsigned int timeout, unsigned int* bytesRead, asio::ip::address* sourceAddress=0, unsigned short* sourcePort=0) = 0;
   virtual void cancelSocket() = 0;

   // Local binding info
   StunTuple mLocalBinding;

   // Turn Allocation Properties used in request
   StunTuple mTurnServer;
   resip::Data mUsername;
   resip::Data mPassword;
   unsigned int mRequestedLifetime;
   unsigned int mRequestedBandwidth;
   unsigned int mRequestedPortProps;
   unsigned short mRequestedPort;
   StunTuple::TransportType mRequestedTransportType;
   asio::ip::address mRequestedIpAddress;

   // Turn Allocation Properties from response
   bool mHaveAllocation;
   StunTuple mRelayTuple;
   StunTuple mReflexiveTuple;
   unsigned int mLifetime;
   unsigned int mBandwidth;

   StunTuple mActiveDestination;

   asio::io_service mIOService;

   // handlers and timers required to do a timed out read
   asio::deadline_timer mReadTimer;
   size_t mBytesRead;
   asio::error_code mReadErrorCode;
   void startReadTimer(unsigned int timeout);
   void handleRawRead(const asio::error_code& errorCode, size_t bytesRead);
   void handleRawReadTimeout(const asio::error_code& errorCode);

private:
   StunMessage* sendRequestAndGetResponse(StunMessage& request, asio::error_code& errorCode);
   asio::error_code handleStunMessage(StunMessage& stunMessage, char* buffer, unsigned int& size, asio::ip::address* sourceAddress=0, unsigned short* sourcePort=0);
   asio::error_code handleRawData(char* data, unsigned int dataSize,  unsigned int expectedSize, char* buffer, unsigned int& bufferSize, asio::ip::address* sourceAddress=0, unsigned short* sourcePort=0);
   char mBuffer[8192];
};

} 

#endif


/* ====================================================================

 Original contribution Copyright (C) 2007 Plantronics, Inc.
 Provided under the terms of the Vovida Software License, Version 2.0.

 The Vovida Software License, Version 2.0 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution. 
 
 THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 DAMAGE.

 ==================================================================== */

