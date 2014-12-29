#include "Connection.h"

void Connection::update()
{
	if( isConnected() )
	{
		switch( m_dst.type )
		{
		case Destination::TypeModuleRenderer:
			m_dst.module->setChannel( m_dst.channel, m_src.module->target() );
			break;
		case Destination::TypeRenderArea:
			m_dst.set->setModule( m_dst.area, m_src.module );
			break;
		default:
		case Destination::TypeNone:
			// Should never happen!
			throw;
		}
	}
}
