/****************************************************************************
** Copyright (c) 2001-2004 quickfixengine.org  All rights reserved.
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#include "stdafx.h"
#else
#include "config.h"
#endif

#include "ParserTestCase.h"
#include <string>
#include <sstream>

namespace FIX
{
bool ParserTestCase::extractLength::onSetup( Parser*& pObject )
{
  m_normalLength   = "8=FIX.4.2\0019=12\00135=A\001108=30\00110=31\001";
  m_badLength      = "8=FIX.4.2\0019=A\00135=A\001108=30\00110=31\001";
  m_negativeLength = "8=FIX.4.2\0019=-1\00135=A\001108=30\00110=31\001";
  m_incomplete_1   = "8=FIX.4.2";
  m_incomplete_2   = "8=FIX.4.2\0019=12";
  return true;
}

void ParserTestCase::extractLength::onRun( Parser& object )
{
  int length = 0;
  std::string::size_type pos = 0;

  assert( object.extractLength(length, pos, m_normalLength) );
  assert( length == 12 );
  assert( pos == 15 );

  pos = 0;
  try { object.extractLength(length, pos, m_badLength);
        assert(false); }
  catch( MessageParseError& ) {}

  assert(pos == 0);
  try { object.extractLength(length, pos, m_negativeLength);
        assert(false); }
  catch( MessageParseError& ) {}

  assert(pos == 0);
  try { assert( object.extractLength(length, pos, m_incomplete_1) == false);
        assert(pos == 0); }
  catch( MessageParseError& ) { assert(false) }

  try { assert( object.extractLength(length, pos, m_incomplete_2) == false);
        assert(pos == 0); }
  catch( MessageParseError& ) { assert(false) }

  assert( !object.extractLength(length, pos, "") );
}

bool ParserTestCase::readFixMessage::onSetup( Parser*& pObject )
{
  m_fixMsg1 = "8=FIX.4.2\0019=12\00135=A\001108=30\00110=31\001";
  m_fixMsg2 = "8=FIX.4.2\0019=17\00135=4\00136=88\001123=Y\00110=34\001";
  m_fixMsg3 = "8=FIX.4.2\0019=19\00135=A\001108=30\0019710=8\00110=31\001";

  m_pStream = new std::stringstream( m_fixMsg1 + m_fixMsg2 + m_fixMsg3 );

  pObject = new Parser( *m_pStream );
  return true;
}

void ParserTestCase::readFixMessage::onRun( Parser& object )
{
  std::string fixMsg1;
  assert( object.readFixMessage( fixMsg1 ) );
  assert( fixMsg1 == m_fixMsg1 );

  std::string fixMsg2;
  assert( object.readFixMessage( fixMsg2 ) );
  assert( fixMsg2 == m_fixMsg2 );

  std::string fixMsg3;
  assert( object.readFixMessage( fixMsg3 ) );
  assert( fixMsg3 == m_fixMsg3 );
}

bool ParserTestCase::readPartialFixMessage::onSetup( Parser*& pObject )
{
  m_partFixMsg1 = "8=FIX.4.2\0019=17\00135=4\00136=";
  m_partFixMsg2 = "88\001123=Y\00110=34\001";

  m_pStream = new std::stringstream( m_partFixMsg1 );
  pObject = new Parser( *m_pStream );
  return true;
}

void ParserTestCase::readPartialFixMessage::onRun( Parser& object )
{
  std::string partFixMsg;
  assert( !object.readFixMessage( partFixMsg ) );
  std::stringstream finishStream( m_partFixMsg2 );
  object.setStream( finishStream );
  assert( object.readFixMessage( partFixMsg ) );
  assert( partFixMsg == ( m_partFixMsg1 + m_partFixMsg2 ) );
}

bool ParserTestCase::readFromSocket::onSetup( Parser*& pObject )
{
  m_fixMsg1 = "8=FIX.4.2\0019=12\00135=A\001108=30\00110=31\001";
  m_fixMsg2 = "8=FIX.4.2\0019=17\00135=4\00136=88\001123=Y\00110=34\001";
  m_fixMsg3 = "8=FIX.4.2\0019=19\00135=A\001108=30\0019710=8\00110=31\001";
  m_fixMsgWithNull.append("8=FIX.4.2\0019=13\00135=A\000\001108=30\00110=31\001",34);

  m_pServer = new SocketServer( m_port, 0, true );
  m_pConnector = new SocketConnector;
  int connSocket = m_pConnector->connect( "127.0.0.1", m_port );
  assert( connSocket > 0 );
  int recvSocket = m_pServer->accept();

  std::string buffer = m_fixMsg1 + m_fixMsg2 + m_fixMsg3;
  buffer.append( m_fixMsgWithNull );
  if ( !socket_send( connSocket, buffer.c_str(), buffer.length() ) )
    return false;

  pObject = new Parser( recvSocket );
  return true;
}

void ParserTestCase::readFromSocket::onRun( Parser& object )
{
  std::string fixMsg1;
  assert( object.readFixMessage( fixMsg1 ) );
  assert( fixMsg1 == m_fixMsg1 );

  std::string fixMsg2;
  assert( object.readFixMessage( fixMsg2 ) );
  assert( fixMsg2 == m_fixMsg2 );

  std::string fixMsg3;
  assert( object.readFixMessage( fixMsg3 ) );
  assert( fixMsg3 == m_fixMsg3 );

  std::string fixMsgWithNull;
  assert( object.readFixMessage( fixMsgWithNull ) );
  assert( fixMsgWithNull == m_fixMsgWithNull );
}

}