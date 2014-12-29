#include "Connection.h"

void Connection::update()
{
	if( isConnected() )
	{
		m_dst.module->setChannel( m_dst.channel, m_src.module->target() );
	}
}
