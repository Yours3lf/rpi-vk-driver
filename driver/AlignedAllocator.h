#pragma once


void* alignedAlloc( unsigned bytes, unsigned alignment = 16 )
{
  if( !bytes )
  {
	return 0;
  }

  const unsigned maxBytes = 1024 * 1024 * 1024; //1GB is max on RPi

  if( bytes > maxBytes )
  {
	return 0; //bad alloc
  }

  void* pv = 0;

  if( posix_memalign( &pv, alignment, bytes ) )
  {
	pv = 0; //allocation failed
  }

  return pv;
}

void alignedFree( void* p )
{
  free( p );
}
