#include "sip2/sipstack/SipStack.hxx"
#include "sip2/util/Data.hxx"
#include "sip2/util/Logger.hxx"
#include "sip2/util/Random.hxx"
#include "sip2/sipstack/TuIM.hxx"
#include "sip2/sipstack/Contents.hxx"
#include "sip2/sipstack/ParserCategories.hxx"
#include "sip2/sipstack/PlainContents.hxx"

#define VOCAL_SUBSYSTEM Subsystem::SIP

using namespace Vocal2;


void
TuIM::PageCallback::receivedPage( Data& msg, Uri& from )
{
   assert(0);
}


void
TuIM::ErrCallback::sendPageFailed( Uri& dest )
{
   assert(0);
}


TuIM::ErrCallback::~ErrCallback()
{
}


TuIM::PageCallback::~PageCallback()
{
}


TuIM::TuIM( SipStack* stack, 
            Uri& aor, Uri& contact,
            PageCallback* msgCallback, ErrCallback* errCallback ):
   mPageCallback(msgCallback),
   mErrCallback(errCallback),
   mStack(stack),
   mAor(aor),
   mContact(contact)
{
   assert( mStack );
   assert(mPageCallback);
   assert(mErrCallback);
}

      
void TuIM::sendPage( Data& text, Uri& dest )
{
   DebugLog( << "send to <" << dest << ">" << "\n" << text );

   NameAddr target;
   target.uri() = dest;
   
   NameAddr from;
   from.uri() = mAor;

   NameAddr contact;
   contact.uri() = mContact;
      
   SipMessage* msg = Helper::makeRequest(target, from, contact, MESSAGE);
   assert( msg );

#if 0
   PlainContents body;
   body.setText( text );
   msg->setContents( &body );
#else
   msg->header(h_ContentType).type() = Data("text");
   msg->header(h_ContentType).subType() = Data("plain");
   msg->setBody( text.data(), text.size() );
#endif

   
   mStack->send( *msg );
}


void 
TuIM::process()
{
   assert( mStack );
   
   SipMessage* msg = mStack->receive();
   if ( msg )
   {
      DebugLog ( << "got message: " << *msg);
   
      if ( msg->isResponse() )
      { 
         int number = msg->header(h_StatusLine).responseCode();
         DebugLog ( << "got response of type " << number );

         if ( number >= 300 )
         {
            Uri dest = msg->header(h_To).uri();
            mErrCallback->sendPageFailed( dest );
         }
      }
      
      if ( msg->isRequest() )
      { 
         if ( msg->header(h_RequestLine).getMethod() == MESSAGE )
         {  
            NameAddr contact; 
            contact.uri() = mContact;
            
            SipMessage* response = Helper::makeResponse(*msg, 200, contact, "OK");

            mStack->send( *response );

            delete response;
            
#if 1
            Contents* contents = msg->getContents();
            assert( contents );
            Mime mime = contents->getType();
            DebugLog ( << "got body of type  " << mime.type() << "/" << mime.subType() );
            PlainContents* body = dynamic_cast<PlainContents*>(contents);
            assert( body );
            Data text = body->getText();;
#else
            const HeaderFieldValue* hfv = msg->getBody(); 
            Data text = Data( hfv->mField,hfv->mFieldLength ); // !cj! UGLY -
                                                               // must be a
                                                               // better way
#endif

            Uri from = msg->header(h_From).uri();
            DebugLog ( << "got message from " << from );

            mPageCallback->receivedPage( text, from );
         }
      }

      delete msg;
   }
}



/* ====================================================================
 * The Vovida Software License, Version 1.0  *  * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved. *  * Redistribution and use in source and binary forms, with or without * modification, are permitted provided that the following conditions * are met: *  * 1. Redistributions of source code must retain the above copyright *    notice, this list of conditions and the following disclaimer. *  * 2. Redistributions in binary form must reproduce the above copyright *    notice, this list of conditions and the following disclaimer in *    the documentation and/or other materials provided with the *    distribution. *  * 3. The names "VOCAL", "Vovida Open Communication Application Library", *    and "Vovida Open Communication Application Library (VOCAL)" must *    not be used to endorse or promote products derived from this *    software without prior written permission. For written *    permission, please contact vocal@vovida.org. * * 4. Products derived from this software may not be called "VOCAL", nor *    may "VOCAL" appear in their name, without prior written *    permission of Vovida Networks, Inc. *  * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL, * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH * DAMAGE. *  * ==================================================================== *  * This software consists of voluntary contributions made by Vovida * Networks, Inc. and many individuals on behalf of Vovida Networks, * Inc.  For more information on Vovida Networks, Inc., please see * <http://www.vovida.org/>. *
 */
